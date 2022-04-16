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
#include "test_runner.h"

#include "hilog_wrapper.h"
#include "bundle_mgr_interface.h"
#include "runtime.h"
#include "runner_runtime/js_test_runner.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t UNSPECIFIED_USER = -2;
}

std::unique_ptr<TestRunner> TestRunner::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, bool isFaJsModel)
{
    if (!runtime) {
        return std::make_unique<TestRunner>();
    }

    auto bundleObj =
        OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!bundleObj) {
        HILOG_ERROR("Failed to get bundle manager service");
        return nullptr;
    }

    auto bms = iface_cast<IBundleMgr>(bundleObj);
    if (!bms) {
        HILOG_ERROR("Cannot convert to IBundleMgr");
        return nullptr;
    }

    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    BundleInfo bundleInfo;
    if (!bms->GetBundleInfo(args->GetTestBundleName(), BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, UNSPECIFIED_USER)) {
        HILOG_ERROR("Failed to GetBundleInfo");
        return nullptr;
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return RunnerRuntime::JsTestRunner::Create(runtime, args, bundleInfo, isFaJsModel);
        default:
            return std::make_unique<TestRunner>();
    }
}

void TestRunner::Prepare()
{}

void TestRunner::Run()
{}

bool TestRunner::Initialize()
{
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
