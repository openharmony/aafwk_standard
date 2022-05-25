/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <benchmark/benchmark.h>
#include <unistd.h>
#include <queue>

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "mission_listener_stub.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
class BenchmarkMissionListener : public MissionListenerStub {
public:
    BenchmarkMissionListener() = default;

    ~BenchmarkMissionListener() override = default;

    void OnMissionCreated(int32_t missionId) override
    {
    }

    void OnMissionDestroyed(int32_t missionId) override
    {
    }

    void OnMissionSnapshotChanged(int32_t missionId) override
    {
    }

    void OnMissionMovedToFront(int32_t missionId) override
    {
    }

    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override
    {
    }
};

class MissionManagerTest : public benchmark::Fixture {
public:
    MissionManagerTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~MissionManagerTest() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        usleep(usleepTime);
    }

    void TearDown(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        usleep(usleepTime);
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
    const int32_t upperLimit = 32;
    // sleep 1000ms
    const int32_t usleepTime = 1000 * 1000;
};

class MissionManagerTestReg : public MissionManagerTest {
public:
    void TearDown(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        while (!missionListeners.empty()) {
            sptr<BenchmarkMissionListener> missionListener = missionListeners.front();
            AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
            missionListeners.pop();
        }
        usleep(usleepTime);
    }

protected:
    std::queue<sptr<BenchmarkMissionListener>> missionListeners {};
};

class MissionManagerTestUnReg : public MissionManagerTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        for (int32_t i = 0; i < iterations; i++) {
            sptr<BenchmarkMissionListener> missionListener = new BenchmarkMissionListener();
            AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener);
            missionListeners.push(missionListener);
        }
        usleep(usleepTime);
    }

    void TearDown(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        while (!missionListeners.empty()) {
            sptr<BenchmarkMissionListener> missionListener = missionListeners.front();
            AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
            missionListeners.pop();
        }
        usleep(usleepTime);
    }

protected:
    std::queue<sptr<BenchmarkMissionListener>> missionListeners {};
};

class MissionManagerTestWithAbility : public MissionManagerTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        usleep(usleepTime);
        Want want;
        want.SetElementName(deviceId, bundleName, abilityName);
        AbilityManagerClient::GetInstance()->StartAbility(want);
        std::vector<MissionInfo> info {};
        AbilityManagerClient::GetInstance()->GetMissionInfos("", upperLimit, info);
        missionId = info.front().id;
    }

    void TearDown(const ::benchmark::State &state) override
    {
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        usleep(usleepTime);
    }

protected:
    const string deviceId;
    const string bundleName = "com.example.distributedcalc";
    const string abilityName = "com.example.distributedcalc.default";
    int32_t missionId = 0;
};

BENCHMARK_F(MissionManagerTestReg, RegisterMissionListenerTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        sptr<BenchmarkMissionListener> missionListener = new BenchmarkMissionListener();
        ErrCode errCode = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener);
        if (errCode != ERR_OK) {
            state.SkipWithError("RegisterMissionListenerTestCase failed.");
        }
        missionListeners.push(missionListener);
    }
}

BENCHMARK_F(MissionManagerTestUnReg, UnregisterMissionListenerTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        if (missionListeners.empty()) {
            state.SkipWithError("UnregisterMissionListenerTestCase failed, no missionListener");
        }
        sptr<BenchmarkMissionListener> missionListener = missionListeners.front();
        ErrCode errCode = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
        if (errCode != ERR_OK) {
            state.SkipWithError("UnregisterMissionListenerTestCase failed.");
        }
        missionListeners.pop();
    }
}

BENCHMARK_F(MissionManagerTest, GetMissionInfosTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<MissionInfo> info {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->GetMissionInfos("", upperLimit, info);
        if (errCode != ERR_OK) {
            state.SkipWithError("GetMissionInfosTestCase failed.");
        }
    }
}

BENCHMARK_F(MissionManagerTestWithAbility, GetMissionInfoTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        MissionInfo missionInfo {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->GetMissionInfo("", missionId, missionInfo);
        if (errCode != ERR_OK) {
            state.SkipWithError("GetMissionInfoTestCase failed.");
        }
    }
}

BENCHMARK_F(MissionManagerTestWithAbility, LockMissionForCleanupTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        ErrCode errCode = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
        if (errCode != ERR_OK) {
            state.SkipWithError("LockMissionForCleanupTestCase failed.");
        }
        state.PauseTiming();
        errCode = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
        if (errCode != ERR_OK) {
            state.SkipWithError("LockMissionForCleanupTestCase failed.");
        }
        state.ResumeTiming();
    }
}

BENCHMARK_F(MissionManagerTestWithAbility, UnlockMissionForCleanupTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        ErrCode errCode = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
        if (errCode != ERR_OK) {
            state.SkipWithError("UnlockMissionForCleanupTestCase failed.");
        }
        state.ResumeTiming();
        errCode = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
        if (errCode != ERR_OK) {
            state.SkipWithError("UnlockMissionForCleanupTestCase failed.");
        }
    }
}

BENCHMARK_F(MissionManagerTestWithAbility, CleanMissionTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        MissionSnapshot snapshot {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->CleanMission(missionId);
        if (errCode != ERR_OK) {
            state.SkipWithError("CleanMissionTestCase failed.");
        }
    }
}

BENCHMARK_F(MissionManagerTest, CleanAllMissionsTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        ErrCode errCode = AbilityManagerClient::GetInstance()->CleanAllMissions();
        if (errCode != ERR_OK) {
            state.SkipWithError("CleanAllMissionsTestCase failed.");
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
