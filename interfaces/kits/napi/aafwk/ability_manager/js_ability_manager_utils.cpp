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

#include "js_ability_manager_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "hilog_wrapper.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJSToken(napi_env env, const sptr<IRemoteObject> target)
{
    napi_value tokenClass = nullptr;
    auto constructorcb = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };
    napi_define_class(
        env, "TokenClass", NAPI_AUTO_LENGTH, constructorcb, nullptr, 0, nullptr, &tokenClass);
    napi_value jsToken = nullptr;
    napi_new_instance(env, tokenClass, 0, nullptr, &jsToken);
    auto finalizecb = [](napi_env env, void *data, void *hint) {};
    napi_wrap(env, jsToken, (void *)target.GetRefPtr(), finalizecb, nullptr, nullptr);
    return jsToken;
}

NativeValue* CreateJsAbilityRunningInfoArray(
    NativeEngine &engine, const std::vector<AAFwk::AbilityRunningInfo> &infos)
{
    NativeValue* arrayValue = engine.CreateArray(infos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        array->SetElement(index++, CreateJsAbilityRunningInfo(engine, runningInfo));
    }
    return arrayValue;
}

NativeValue* CreateJsElementName(NativeEngine &engine, const AppExecFwk::ElementName &elementName)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    napi_value napiElementName =
        OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine), elementName);
    object->SetProperty("ability", reinterpret_cast<NativeValue*>(napiElementName));
    return objValue;
}

NativeValue* CreateJsExtensionRunningInfoArray(
    NativeEngine &engine, const std::vector<AAFwk::ExtensionRunningInfo> &infos)
{
    NativeValue* arrayValue = engine.CreateArray(infos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &runningInfo : infos) {
        array->SetElement(index++, CreateJsExtensionRunningInfo(engine, runningInfo));
    }
    return arrayValue;
}

NativeValue* CreateJsAbilityRunningInfo(NativeEngine &engine, const AAFwk::AbilityRunningInfo &info)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    napi_value napiElementName =
        OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine), info.ability);
    object->SetProperty("ability", reinterpret_cast<NativeValue*>(napiElementName));
    object->SetProperty("pid", CreateJsValue(engine, info.pid));
    object->SetProperty("uid", CreateJsValue(engine, info.uid));
    object->SetProperty("processName", CreateJsValue(engine, info.processName));
    object->SetProperty("startTime", CreateJsValue(engine, info.startTime));
    object->SetProperty("abilityState", CreateJsValue(engine, info.abilityState));
    return objValue;
}

NativeValue* CreateJsExtensionRunningInfo(NativeEngine &engine, const AAFwk::ExtensionRunningInfo &info)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    napi_value napiElementName =
        OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine), info.extension);
    object->SetProperty("extension", reinterpret_cast<NativeValue*>(napiElementName));
    object->SetProperty("pid", CreateJsValue(engine, info.pid));
    object->SetProperty("uid", CreateJsValue(engine, info.uid));
    object->SetProperty("type", CreateJsValue(engine, info.type));
    object->SetProperty("processName", CreateJsValue(engine, info.processName));
    object->SetProperty("startTime", CreateJsValue(engine, info.startTime));
    object->SetProperty("clientPackage", CreateNativeArray(engine, info.clientPackage));
    return objValue;
}

NativeValue *AbilityStateInit(NativeEngine *engine)
{
    HILOG_INFO("enter");

    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("INITIAL", CreateJsValue(*engine, (int32_t)AAFwk::AbilityState::INITIAL));
    object->SetProperty("FOREGROUND", CreateJsValue(*engine, (int32_t)AAFwk::AbilityState::FOREGROUND));
    object->SetProperty("BACKGROUND", CreateJsValue(*engine, (int32_t)AAFwk::AbilityState::BACKGROUND));
    object->SetProperty("FOREGROUNDING", CreateJsValue(*engine, (int32_t)AAFwk::AbilityState::FOREGROUNDING));
    object->SetProperty("BACKGROUNDING", CreateJsValue(*engine, (int32_t)AAFwk::AbilityState::BACKGROUNDING));

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
