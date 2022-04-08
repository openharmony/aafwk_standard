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

#ifndef OHOS_APPEXECFWK_FEATURE_ABILITY_H
#define OHOS_APPEXECFWK_FEATURE_ABILITY_H
#include "../inner/napi_common/feature_ability_common.h"
using Want = OHOS::AAFwk::Want;

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief FeatureAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FeatureAbilityInit(napi_env env, napi_value exports);

EXTERN_C_START
/**
 * @brief The interface of onAbilityResult provided for ACE to call back to JS.
 *
 * @param requestCode Indicates the request code returned after the ability is started.
 * @param resultCode Indicates the result code returned after the ability is started.
 * @param resultData Indicates the data returned after the ability is started.
 * @param callbackInfo The environment and call back info that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
void CallOnAbilityResult(int requestCode, int resultCode, const Want &resultData, CallbackInfo callbackInfo);
EXTERN_C_END

/**
 * @brief FeatureAbility NAPI method : startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbility(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : startAbilityForResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityForResult(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : setResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetResult(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : terminateAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_TerminateAbility(napi_env env, napi_callback_info info);

/**
 * @brief Checks whether the main window of this ability has window focus.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_HasWindowFocus(napi_env env, napi_callback_info info);

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContext(napi_env env, napi_callback_info info);

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWant(napi_env env, napi_callback_info info);

/**
 * @brief Get want(sync).
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantSync(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppType(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityName(napi_env env, napi_callback_info info);

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfo(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : getDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDataAbilityHelper(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : acquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelper(napi_env env, napi_callback_info info);

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapForResultParam(CallAbilityParam &param, napi_env env, napi_value args);

/**
 * @brief Parse the abilityResult parameters.
 *
 * @param param Indicates the abilityResult parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapAbilityResult(CallAbilityParam &param, napi_env env, napi_value args);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppInfoCB on success, nullptr on failure.
 */
AppInfoCB *CreateAppInfoCBInfo(napi_env env);

/**
 * @brief HasWindowFocus processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value HasWindowFocusWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value HasWindowFocusAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo);
napi_value HasWindowFocusPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief TerminateAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value TerminateAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value TerminateAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo);
napi_value TerminateAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief SetResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value SetResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value SetResultAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo);
napi_value SetResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief StartAbilityForResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityForResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value StartAbilityForResultAsync(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);
napi_value StartAbilityForResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief TerminateAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ContinueAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value ContinueAbilityAsync(napi_env env, napi_value *args, AsyncCallbackInfo *asyncCallbackInfo, size_t argc);
napi_value ContinueAbilityPromise(napi_env env, napi_value *args, AsyncCallbackInfo *asyncCallbackInfo, size_t argc);

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfo(napi_env env, napi_callback_info info);

/**
 * @brief getDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB);

/**
 * @brief GetDataAbilityHelper Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperAsync(
    napi_env env, napi_value *args, const size_t argCallback, DataAbilityHelperCB *dataAbilityHelperCB);

/**
 * @brief GetDataAbilityHelper Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperPromise(napi_env env, DataAbilityHelperCB *dataAbilityHelperCB);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetDataAbilityHelperAsyncCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetDataAbilityHelperPromiseCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief FeatureAbility NAPI method : connectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FAConnectAbility(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : disconnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FADisConnectAbility(napi_env env, napi_callback_info info);

/**
 * @brief FeatureAbility NAPI method : continueAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_FAContinueAbility(napi_env env, napi_callback_info info);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_FEATURE_ABILITY_H */
