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
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

namespace OHOS {
namespace RunnerRuntime {
std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
{
    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo);
    if (!pTestRunner) {
        HILOG_ERROR("Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
    : jsRuntime_(jsRuntime)
{
    if (args) {
        std::string srcPath;
        if (bundleInfo.hapModuleInfos.back().isModuleJson) {
            srcPath.append(args->GetTestModuleName());
            srcPath.append("/ets/TestRunner/");
        } else {
            srcPath.append(args->GetTestPackageName());
            srcPath.append("/assets/js/TestRunner/");
        }
        srcPath.append(args->GetTestRunnerClassName());
        srcPath.append(".abc");
        srcPath_ = srcPath;
    }
    HILOG_INFO("JsTestRunner srcPath is %{public}s", srcPath_.c_str());

    std::string moduleName;
    jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_);
}

JsTestRunner::~JsTestRunner() = default;

void JsTestRunner::Prepare()
{
    HILOG_INFO("Enter");
    TestRunner::Prepare();
    CallObjectMethod("onPrepare");
    HILOG_INFO("End");
}

void JsTestRunner::Run()
{
    HILOG_INFO("Enter");
    TestRunner::Run();
    CallObjectMethod("onRun");
    HILOG_INFO("End");
}

void JsTestRunner::CallObjectMethod(const char *name, NativeValue *const *argv, size_t argc)
{
    HILOG_INFO("JsTestRunner::CallObjectMethod(%{public}s)", name);

    if (!jsTestRunnerObj_) {
        HILOG_ERROR("Not found %{public}s", srcPath_.c_str());
        ReportFinished("Not found " + srcPath_);
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsTestRunnerObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Test Runner object");
        ReportFinished("Failed to get Test Runner object");
        return;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Test Runner object", name);
        ReportStatus("Failed to get " + std::string(name) + " from Test Runner object");
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}

void JsTestRunner::ReportFinished(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void JsTestRunner::ReportStatus(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }

    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS
