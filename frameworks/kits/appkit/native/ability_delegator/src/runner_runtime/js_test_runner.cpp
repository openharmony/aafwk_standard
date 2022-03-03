/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ability_delegator_registry.h"
#include "app_log_wrapper.h"
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

namespace OHOS {
namespace RunnerRuntime {
std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
{
    if (!args) {
        APP_LOGE("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo);
    if (!pTestRunner) {
        APP_LOGE("Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
    : jsRuntime_(jsRuntime)
{
    std::string prefix;
    std::string tempTestRunnerName = args->GetTestRunnerClassName();
    auto testRunnerName = tempTestRunnerName;

    auto pos = tempTestRunnerName.find(":");
    if (pos != std::string::npos) {
        prefix = tempTestRunnerName.substr(0, pos);
        testRunnerName = tempTestRunnerName.substr(pos + 1);
    }

    std::string srcPath;
    if (bundleInfo.hapModuleInfos.back().isModuleJson) {
        srcPath.append(prefix);
        srcPath.append("/ets/TestRunner/");
    } else {
        srcPath.append(args->GetTestBundleName());
        srcPath.append("/assets/js/TestRunner/");
    }
    srcPath.append(testRunnerName);
    srcPath.append(".abc");

    APP_LOGI("JsTestRunner srcPath is %{public}s", srcPath.c_str());

    std::string moduleName;
    jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
}

JsTestRunner::~JsTestRunner() = default;

void JsTestRunner::Prepare()
{
    APP_LOGI("Enter");
    TestRunner::Prepare();
    CallObjectMethod("onPrepare");
    APP_LOGI("End");
}

void JsTestRunner::Run()
{
    APP_LOGI("Enter");
    TestRunner::Run();
    CallObjectMethod("onRun");
    APP_LOGI("End");
}

void JsTestRunner::CallObjectMethod(const char *name, NativeValue *const *argv, size_t argc)
{
    APP_LOGI("JsTestRunner::CallObjectMethod(%{public}s)", name);

    if (!jsTestRunnerObj_) {
        APP_LOGE("Not found test_runner.js");
        ReportFinished("Not found test_runner.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsTestRunnerObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        APP_LOGE("Failed to get Test Runner object");
        ReportFinished("Failed to get Test Runner object");
        return;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        APP_LOGE("Failed to get '%{public}s' from Test Runner object", name);
        ReportStatus("Failed to get " + std::string(name) + " from Test Runner object");
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}

void JsTestRunner::ReportFinished(const std::string &msg)
{
    APP_LOGI("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        APP_LOGE("delegator is null");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void JsTestRunner::ReportStatus(const std::string &msg)
{
    APP_LOGI("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        APP_LOGE("delegator is null");
        return;
    }

    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS
