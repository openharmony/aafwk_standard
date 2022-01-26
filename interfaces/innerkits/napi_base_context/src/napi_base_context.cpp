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
namespace {
napi_status GetStageModeContextClassObject(napi_env env, napi_value& classObject)
{
    static thread_local napi_ref contextClassObjectRef = {0};

    napi_status ret = napi_get_reference_value(env, contextClassObjectRef, &classObject);
    if (ret == napi_ok) {
        return napi_ok;
    }

    napi_value global;
    OHOS_CALL_NAPI_RETURN(napi_get_global(env, &global));

    napi_value requireNapi;
    OHOS_CALL_NAPI_RETURN(napi_get_named_property(env, global, "requireNapi", &requireNapi));

    napi_value className;
    OHOS_CALL_NAPI_RETURN(napi_create_string_utf8(env, "application.Context", NAPI_AUTO_LENGTH, &className));

    OHOS_CALL_NAPI_RETURN(napi_call_function(env, global, requireNapi, 1, &className, &classObject));

    // Ignore return value
    napi_create_reference(env, classObject, 1, &contextClassObjectRef);
    return napi_ok;
}
} // namespace

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

    napi_value classObject;
    if (value) {
        OHOS_CALL_NAPI_RETURN(GetStageModeContextClassObject(env, classObject));
    } else {
        napi_value* clsObjPtr = GetFAModeContextClassObject();
        if (clsObjPtr == nullptr) {
            return napi_generic_failure;
        }
        classObject = *clsObjPtr;
    }

    bool result = false;
    OHOS_CALL_NAPI_RETURN(napi_instanceof(env, object, classObject, &result));
    if (!result) {
        return napi_generic_failure;
    }

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