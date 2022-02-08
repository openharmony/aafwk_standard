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

#include "napi_base_context.h"

#define OHOS_CALL_NAPI_RETURN(call) \
    do {                            \
        napi_status ret = (call);   \
        if (ret != napi_ok) {       \
            return ret;             \
        }                           \
    } while (0)

namespace OHOS {
namespace AbilityRuntime {
napi_value* GetFAModeContextClassObject()
{
    static thread_local napi_value contextClassObject = {0};
    return &contextClassObject;
}

napi_status IsStageContext(napi_env env, napi_value object, bool& stageMode)
{
    napi_value boolValue;
    OHOS_CALL_NAPI_RETURN(napi_get_named_property(env, object, "stageMode", &boolValue));

    bool value = false;
    OHOS_CALL_NAPI_RETURN(napi_get_value_bool(env, boolValue, &value));

    stageMode = value;
    return napi_ok;
}

std::shared_ptr<Context> GetStageModeContext(napi_env env, napi_value object)
{
    void* wrapped = nullptr;
    napi_status ret = napi_unwrap(env, object, &wrapped);
    if (ret != napi_ok) {
        return nullptr;
    }

    auto weakContext = static_cast<std::weak_ptr<Context>*>(wrapped);
    return weakContext != nullptr ? weakContext->lock() : nullptr;
}

AppExecFwk::Ability* GetCurrentAbility(napi_env env)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        return nullptr;
    }

    napi_value abilityObj;
    status = napi_get_named_property(env, global, "ability", &abilityObj);
    if (status != napi_ok || abilityObj == nullptr) {
        return nullptr;
    }

    void* pointer = nullptr;
    status = napi_get_value_external(env, abilityObj, &pointer);
    if (status != napi_ok) {
        return nullptr;
    }

    return static_cast<AppExecFwk::Ability*>(pointer);
}
}  // namespace AbilityRuntime
}  // namespace OHOS