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

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
class AbilityContextTest : public benchmark::Fixture {
public:
    AbilityContextTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~AbilityContextTest() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
        sleep(1);
        AbilityManagerClient::GetInstance()->CleanAllMissions();
        sleep(1);
    }

    void TearDown(const ::benchmark::State &state) override
    {
    }

protected:
    const string deviceId;
    const string bundleName = "com.example.distributedcalc";
    const string abilityName = "com.example.distributedcalc.default";
    const int32_t repetitions = 16;
    const int32_t iterations = 1;
};

// StartAbility
BENCHMARK_F(AbilityContextTest, StartAbilityTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        Want want;
        want.SetElementName(deviceId, bundleName, abilityName);
        ErrCode errCode = AbilityManagerClient::GetInstance()->StartAbility(want);
        if (errCode != ERR_OK) {
            state.SkipWithError("StartAbilityTestCase failed.");
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
