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

#include "js_ability_delegator_utils.h"

#include <map>
#include "ability_runtime/js_ability.h"
#include "hilog_wrapper.h"
#include "js_ability_monitor.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
NativeValue *CreateJsAbilityDelegator(NativeEngine &engine)
{
    HILOG_INFO("CreateJsAbilityDelegator is called");

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::unique_ptr<JSAbilityDelegator> jsAbilityDelegator = std::make_unique<JSAbilityDelegator>();
    object->SetNativePointer(jsAbilityDelegator.release(), JSAbilityDelegator::Finalizer, nullptr);

    BindNativeFunction(engine, *object, "addAbilityMonitor", JSAbilityDelegator::AddAbilityMonitor);
    BindNativeFunction(engine, *object, "removeAbilityMonitor", JSAbilityDelegator::RemoveAbilityMonitor);
    BindNativeFunction(engine, *object, "waitAbilityMonitor", JSAbilityDelegator::WaitAbilityMonitor);
    BindNativeFunction(engine, *object, "getAppContext", JSAbilityDelegator::GetAppContext);
    BindNativeFunction(engine, *object, "getAbilityState", JSAbilityDelegator::GetAbilityState);
    BindNativeFunction(engine, *object, "getCurrentTopAbility", JSAbilityDelegator::GetCurrentTopAbility);
    BindNativeFunction(engine, *object, "doAbilityForeground", JSAbilityDelegator::DoAbilityForeground);
    BindNativeFunction(engine, *object, "doAbilityBackground", JSAbilityDelegator::DoAbilityBackground);
    BindNativeFunction(engine, *object, "print", JSAbilityDelegator::Print);
    BindNativeFunction(engine, *object, "executeShellCommand", JSAbilityDelegator::ExecuteShellCommand);
    return objValue;
}

napi_value WrapStringToJS(napi_env env, const std::string &value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result));
    return result;
}

void SetAbilityDelegatorArgumentsPara(napi_env env, const std::map<std::string, std::string> &paras)
{
    HILOG_INFO("SetAbilityDelegatorArgumentsPara is called");
    napi_value parameter = nullptr;
    napi_value result = nullptr;

    auto iter = paras.begin();
    for (; iter != paras.end(); ++iter) {
        NAPI_CALL_RETURN_VOID(
            env, napi_set_named_property(env, parameter, iter->first.c_str(), WrapStringToJS(env, iter->second)));
    }
    napi_set_named_property(env, result, "parameters", parameter);
    return;
}

NativeValue *CreateJsAbilityDelegatorArguments(
    NativeEngine &engine, const std::shared_ptr<AbilityDelegatorArgs> &abilityDelegatorArgs)
{
    HILOG_INFO("CreateJsAbilityDelegatorArguments is called");

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("bundleName", CreateJsValue(engine, abilityDelegatorArgs->GetTestBundleName()));
    SetAbilityDelegatorArgumentsPara(reinterpret_cast<napi_env>(&engine), abilityDelegatorArgs->GetTestParam());
    object->SetProperty("testCaseNames", CreateJsValue(engine, abilityDelegatorArgs->GetTestCaseName()));
    object->SetProperty("testRunnerClassName", CreateJsValue(engine, abilityDelegatorArgs->GetTestRunnerClassName()));

    return objValue;
}

NativeValue *CreateJsAbilityState(NativeEngine &engine, AbilityDelegator::AbilityState &lifeState)
{
    HILOG_INFO("CreateJsAbilityState is called");
    NativeValue *objValue = CreateJsValue(engine, static_cast<int>(lifeState));
    if (!objValue) {
        HILOG_ERROR("CreateJsAbilityState objValue is nullptr");
        return nullptr;
    }
    return objValue;
}

NativeValue *CreateJsBool(NativeEngine &engine, bool &flag)
{
    HILOG_INFO("CreateJsBool is called");
    NativeValue *objValue = CreateJsValue(engine, static_cast<bool>(flag));
    if (!objValue) {
        HILOG_ERROR("CreateJsBool objValue is nullptr");
        return nullptr;
    }
    return objValue;
}

NativeValue *CreateJsShellCmdResult(NativeEngine &engine, std::unique_ptr<ShellCmdResult> &shellResult)
{
    HILOG_INFO("CreateJsShellCmdResult is called");

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("stdResult", CreateJsValue(engine, shellResult->GetStdResult()));
    object->SetProperty("exitCode", CreateJsValue(engine, shellResult->GetExitCode()));

    return objValue;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS