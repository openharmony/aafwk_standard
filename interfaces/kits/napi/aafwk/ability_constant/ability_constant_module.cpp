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

#include "launch_param.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AAFwk {
static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitLaunchReasonObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "UNKNOWN", LAUNCHREASON_UNKNOWN));
    NAPI_CALL(env, SetEnumItem(env, object, "START_ABILITY", LAUNCHREASON_START_ABILITY));
    NAPI_CALL(env, SetEnumItem(env, object, "CALL", LAUNCHREASON_CALL));
    NAPI_CALL(env, SetEnumItem(env, object, "CONTINUATION", LAUNCHREASON_CONTINUATION));

    return object;
}

static napi_value InitLastExitReasonObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "UNKNOWN", LASTEXITREASON_UNKNOWN));
    NAPI_CALL(env, SetEnumItem(env, object, "ABILITY_NOT_RESPONDING", LASTEXITREASON_ABILITY_NOT_RESPONDING));
    NAPI_CALL(env, SetEnumItem(env, object, "NORMAL", LASTEXITREASON_NORMAL));

    return object;
}

static napi_value InitOnContinueResultObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "AGREE", ONCONTINUE_AGREE));
    NAPI_CALL(env, SetEnumItem(env, object, "REJECT", ONCONTINUE_REJECT));
    NAPI_CALL(env, SetEnumItem(env, object, "MISMATCH", ONCONTINUE_MISMATCH));

    return object;
}

/*
 * The module initialization.
 */
static napi_value AbilityConstantInit(napi_env env, napi_value exports)
{
    napi_value launchReason = InitLaunchReasonObject(env);
    NAPI_ASSERT(env, launchReason != nullptr, "failed to create launch reason object");

    napi_value lastExitReason = InitLastExitReasonObject(env);
    NAPI_ASSERT(env, lastExitReason != nullptr, "failed to create last exit reason object");

    napi_value onContinueResult = InitOnContinueResultObject(env);
    NAPI_ASSERT(env, onContinueResult != nullptr, "failed to create onContinue result object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("LaunchReason", launchReason),
        DECLARE_NAPI_PROPERTY("LastExitReason", lastExitReason),
        DECLARE_NAPI_PROPERTY("OnContinueResult", onContinueResult),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");

    return exports;
}

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AbilityConstantInit,
    .nm_modname = "application.AbilityConstant",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * The module registration.
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace AAFwk
}  // namespace OHOS