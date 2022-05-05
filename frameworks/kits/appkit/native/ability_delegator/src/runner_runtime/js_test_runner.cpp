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

#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

extern const char _binary_delegator_mgmt_abc_start[];
extern const char _binary_delegator_mgmt_abc_end[];
namespace OHOS {
namespace RunnerRuntime {
std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo, bool isFaJsModel)
{
    if (!runtime) {
        HILOG_ERROR("Invalid runtime");
        return nullptr;
    }

    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo,
        isFaJsModel);
    if (!pTestRunner) {
        HILOG_ERROR("Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo,
    bool isFaJsModel)
    : jsRuntime_(jsRuntime), isFaJsModel_(isFaJsModel)
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
    if (!isFaJsModel) {
        std::string moduleName;
        jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_);
    }
}

JsTestRunner::~JsTestRunner() = default;

bool JsTestRunner::Initialize()
{
    if (isFaJsModel_) {
        if (!jsRuntime_.RunScript("/system/etc/strip.native.min.abc")) {
            HILOG_ERROR("RunScript err");
            return false;
        }
        std::vector<uint8_t> buffer((uint8_t*)_binary_delegator_mgmt_abc_start,
            (uint8_t*)_binary_delegator_mgmt_abc_end);
        auto mgmtResult = jsRuntime_.GetNativeEngine().RunBufferScript(buffer);
        if (mgmtResult == nullptr) {
            HILOG_ERROR("mgmtResult init error");
            return false;
        }
        if (!jsRuntime_.RunSandboxScript(srcPath_)) {
            HILOG_ERROR("RunScript srcPath_ err");
            return false;
        }
        NativeEngine& engine = jsRuntime_.GetNativeEngine();
        NativeObject* object = ConvertNativeValueTo<NativeObject>(engine.GetGlobal());
        if (object == nullptr) {
            HILOG_ERROR("Failed to get global object");
            return false;
        }
        NativeValue* mainEntryFunc = object->GetProperty("___mainEntry___");
        if (mainEntryFunc == nullptr) {
            HILOG_ERROR("Failed to get mainEntryFunc");
            return false;
        }
        NativeValue* value = engine.GetGlobal();
        if (value == nullptr) {
            HILOG_ERROR("Failed to get global");
            return false;
        }
        engine.CallFunction(engine.GetGlobal(), mainEntryFunc, &value, 1);
    }
    return true;
}

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
    if (isFaJsModel_) {
        NativeEngine& engine = jsRuntime_.GetNativeEngine();
        NativeObject* global = ConvertNativeValueTo<NativeObject>(engine.GetGlobal());
        if (global == nullptr) {
            HILOG_ERROR("Failed to get global object");
            return;
        }

        NativeObject* exportObject = ConvertNativeValueTo<NativeObject>(global->GetProperty("exports"));
        if (exportObject == nullptr) {
            HILOG_ERROR("Failed to get exportObject");
            return;
        }

        NativeObject* defaultObject = ConvertNativeValueTo<NativeObject>(exportObject->GetProperty("default"));
        if (defaultObject == nullptr) {
            HILOG_ERROR("Failed to get defaultObject");
            return;
        }

        NativeValue* func = defaultObject->GetProperty(name);
        if (func == nullptr || !func->IsCallable()) {
            HILOG_ERROR("CallRequest func is %{public}s", func == nullptr ? "nullptr" : "not func");
            return;
        }
        engine.CallFunction(engine.CreateUndefined(), func, argv, argc);
        return;
    }

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
