/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "particle_ability.h"

#include <cstring>
#include <uv.h>
#include <vector>

#include "../inner/napi_common/napi_common_ability.h"
#include "hilog_wrapper.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetWant(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetWantCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAGetAbilityName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityNameCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_StartAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStopAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_StopAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : connectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_ConnectAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : disconnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PADisConnectAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_DisConnectAbilityCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief FeatureAbility NAPI method : acquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAAcquireDataAbilityHelper(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_AcquireDataAbilityHelperCommon(env, info, AbilityType::UNKNOWN);
}

/**
 * @brief ParticleAbility NAPI method : startBackgroundRunning.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PAStartBackgroundRunning(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_StartBackgroundRunningCommon(env, info);
}

/**
 * @brief ParticleAbility NAPI method : cancelBackgroundRunning.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PACancelBackgroundRunning(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    return NAPI_CancelBackgroundRunningCommon(env, info);
}

/**
 * @brief ParticleAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ParticleAbilityInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_PAGetAppType),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_PAGetAbilityInfo),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_PAGetHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getContext", NAPI_PAGetContext),
        DECLARE_NAPI_FUNCTION("getWant", NAPI_PAGetWant),
        DECLARE_NAPI_FUNCTION("getAbilityName", NAPI_PAGetAbilityName),
        DECLARE_NAPI_FUNCTION("startAbility", NAPI_PAStartAbility),
        DECLARE_NAPI_FUNCTION("stopAbility", NAPI_PAStopAbility),
        DECLARE_NAPI_FUNCTION("connectAbility", NAPI_PAConnectAbility),
        DECLARE_NAPI_FUNCTION("disconnectAbility", NAPI_PADisConnectAbility),
        DECLARE_NAPI_FUNCTION("acquireDataAbilityHelper", NAPI_PAAcquireDataAbilityHelper),
        DECLARE_NAPI_FUNCTION("startBackgroundRunning", NAPI_PAStartBackgroundRunning),
        DECLARE_NAPI_FUNCTION("cancelBackgroundRunning", NAPI_PACancelBackgroundRunning),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);

    return exports;
}

}  // namespace AppExecFwk
}  // namespace OHOS