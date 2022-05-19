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

#include "form_constants.h"
#include "form_host_client.h"
#include "form_mgr.h"
#include "hilog_wrapper.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
const int32_t NUM_THREADS = 4;

class FormManagerTest : public benchmark::Fixture {
public:
    FormManagerTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~FormManagerTest() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
    }

    void TearDown(const ::benchmark::State &state) override
    {
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
    // sleep 100ms
    const int32_t usleepTime = 1000 * 100;
    std::string bundleName = "ohos.samples.FormApplication";
    std::string moduleName = "entry";
};

BENCHMARK_F(FormManagerTest, GetAllFormsInfoTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<FormInfo> formInfos {};
        ErrCode errCode = FormMgr::GetInstance().GetAllFormsInfo(formInfos);
        if (errCode != ERR_OK) {
            HILOG_ERROR("%{public}s error, failed to GetAllFormsInfoTestCase, error code is %{public}d.", __func__,
                errCode);
            state.SkipWithError("GetAllFormsInfoTestCase failed, return error.");
        }
        if (formInfos.empty()) {
            HILOG_ERROR("%{public}s error, failed to GetAllFormsInfoTestCase, formInfos empty.", __func__);
            state.SkipWithError("GetAllFormsInfoTestCase failed, formInfos empty.");
        }
    }
}

BENCHMARK_F(FormManagerTest, GetFormsInfoByAppTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<FormInfo> formInfos {};
        ErrCode errCode = FormMgr::GetInstance().GetFormsInfoByApp(bundleName, formInfos);
        if (errCode != ERR_OK) {
            HILOG_ERROR("%{public}s error, failed to GetFormsInfoByAppTestCase, error code is %{public}d.", __func__,
                errCode);
            state.SkipWithError("GetFormsInfoByAppTestCase failed, return error.");
        }
        if (formInfos.empty()) {
            HILOG_ERROR("%{public}s error, failed to GetFormsInfoByAppTestCase, formInfos empty.", __func__);
            state.SkipWithError("GetFormsInfoByAppTestCase failed, formInfos empty.");
        }
    }
}

BENCHMARK_F(FormManagerTest, GetFormsInfoByModuleTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::vector<FormInfo> formInfos {};
        ErrCode errCode = FormMgr::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
        if (errCode != ERR_OK) {
            HILOG_ERROR("%{public}s error, failed to GetFormsInfoByModuleTestCase, error code is %{public}d.", __func__,
                errCode);
            state.SkipWithError("GetFormsInfoByAppTestCase failed, return error.");
        }
        if (formInfos.empty()) {
            HILOG_ERROR("%{public}s error, failed to GetFormsInfoByModuleTestCase, formInfos empty.", __func__);
            state.SkipWithError("GetFormsInfoByAppTestCase failed, formInfos empty.");
        }
    }
}

BENCHMARK_REGISTER_F(FormManagerTest, GetAllFormsInfoTestCase)->Threads(NUM_THREADS);
BENCHMARK_REGISTER_F(FormManagerTest, GetFormsInfoByAppTestCase)->Threads(NUM_THREADS);
BENCHMARK_REGISTER_F(FormManagerTest, GetFormsInfoByModuleTestCase)->Threads(NUM_THREADS);

class FormManagerTestAddForm : public benchmark::Fixture {
public:
    FormManagerTestAddForm()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~FormManagerTestAddForm() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
        int32_t numFormsDeleted = 0;
        std::vector<int64_t> validFormIds {};
        FormMgr::GetInstance().DeleteInvalidForms(validFormIds, formHostClient, numFormsDeleted);
        std::string deviceId;
        std::string bundleName = "ohos.samples.FormApplication";
        std::string moduleName = "entry";
        std::string abilityName = "FormAbility";
        std::string formName = "widget";
        int32_t dimension = 3;

        want.SetElementName(deviceId, bundleName, abilityName);
        want.SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName);
        want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName);
        want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, dimension);
    }

    void TearDown(const ::benchmark::State &state) override
    {
        for (int64_t formId: formIds) {
            FormMgr::GetInstance().DeleteForm(formId, formHostClient);
        }
    }

protected:
    const int32_t repetitions = 2;
    const int32_t iterations = 8;
    // sleep 100ms
    const int32_t usleepTime = 1000 * 100;
    Want want;
    std::vector<int64_t> formIds {};
    sptr<FormHostClient> formHostClient = FormHostClient::GetInstance();
};

BENCHMARK_F(FormManagerTestAddForm, AddFormTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        FormJsInfo formInfo;
        FormMgr::GetInstance().AddForm(0, want, formHostClient, formInfo);
        formIds.push_back(formInfo.formId);
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
