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
#include "feature_ability_constant.h"

#include <cstring>
#include <vector>

#include "hilog_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief FeatureAbilityConstantInit NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FAConstantInit(napi_env env, napi_value exports)
{
    const int Window_Configuration_Zero = 100;
    const int Window_Configuration_One = 101;
    const int Window_Configuration_Two = 102;
    const int NO_ERROR = 0;
    const int INVALID_PARAMETER = -1;
    const int ABILITY_NOT_FOUND = -2;
    const int PERMISSION_DENY = -3;
    HILOG_INFO("%{public}s,called", __func__);
    napi_value abilityStartSetting = nullptr;
    napi_value abilityWindowConfiguration = nullptr;
    napi_value errorCode = nullptr;
    napi_create_object(env, &abilityStartSetting);
    napi_create_object(env, &abilityWindowConfiguration);
    napi_create_object(env, &errorCode);

    SetNamedProperty(env, abilityStartSetting, "abilityBounds", "BOUNDS_KEY");
    SetNamedProperty(env, abilityStartSetting, "windowMode", "WINDOW_MODE_KEY");
    SetNamedProperty(env, abilityStartSetting, "displayId", "DISPLAY_ID_KEY");

    SetNamedProperty(env, abilityWindowConfiguration, 0, "WINDOW_MODE_UNDEFINED");
    SetNamedProperty(env, abilityWindowConfiguration, 1, "WINDOW_MODE_FULLSCREEN");
    SetNamedProperty(env, abilityWindowConfiguration, Window_Configuration_Zero, "WINDOW_MODE_SPLIT_PRIMARY");
    SetNamedProperty(env, abilityWindowConfiguration, Window_Configuration_One, "WINDOW_MODE_SPLIT_SECONDARY");
    SetNamedProperty(env, abilityWindowConfiguration, Window_Configuration_Two, "WINDOW_MODE_FLOATING");

    SetNamedProperty(env, errorCode, NO_ERROR, "NO_ERROR");
    SetNamedProperty(env, errorCode, INVALID_PARAMETER, "INVALID_PARAMETER");
    SetNamedProperty(env, errorCode, ABILITY_NOT_FOUND, "ABILITY_NOT_FOUND");
    SetNamedProperty(env, errorCode, PERMISSION_DENY, "PERMISSION_DENY");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("AbilityStartSetting", abilityStartSetting),
        DECLARE_NAPI_PROPERTY("AbilityWindowConfiguration", abilityWindowConfiguration),
        DECLARE_NAPI_PROPERTY("ErrorCode", errorCode),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);

    return exports;
}

void SetNamedProperty(napi_env env, napi_value dstObj, const char *objName, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_string_utf8(env, objName, NAPI_AUTO_LENGTH, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

void SetNamedProperty(napi_env env, napi_value dstObj, const int32_t objValue, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_int32(env, objValue, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

}  // namespace AppExecFwk
}  // namespace OHOS