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
#include <vector>

#include "ability_manager_client.h"
#include "hilog_wrapper.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
class AbilityManagerTest : public benchmark::Fixture {
public:
    AbilityManagerTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~AbilityManagerTest() override = default;

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
    // sleep 100ms
    const int32_t usleepTime = 1000 * 100;
};

BENCHMARK_F(AbilityManagerTest, GetProcessRunningInfosTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<AppExecFwk::RunningProcessInfo> info {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->GetProcessRunningInfos(info);
        if (errCode != ERR_OK) {
            state.SkipWithError("GetProcessRunningInfosTestCase failed.");
        }
    }
}

BENCHMARK_F(AbilityManagerTest, GetAbilityRunningInfosTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<AbilityRunningInfo> info {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(info);
        if (errCode != ERR_OK) {
            state.SkipWithError("GetAbilityRunningInfosTestCase failed.");
        }
    }
}

BENCHMARK_F(AbilityManagerTest, GetExtensionRunningInfosTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<ExtensionRunningInfo> info {};
        ErrCode errCode = AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, info);
        if (errCode != ERR_OK) {
            state.SkipWithError("GetExtensionRunningInfosTestCase failed.");
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
