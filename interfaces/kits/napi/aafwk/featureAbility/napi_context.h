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

#ifndef OHOS_APPEXECFWK_NAPI_CONTEXT_H
#define OHOS_APPEXECFWK_NAPI_CONTEXT_H
#include "../inner/napi_common/napi_common.h"
#include "ability.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

using Ability = OHOS::AppExecFwk::Ability;
#define MODE 0771
namespace OHOS {
namespace AppExecFwk {

struct CallAbilityPermissionParam {
    std::vector<std::string> permission_list;
    int requestCode = 0;
    int syncOption = false;
};

struct OnRequestPermissionsFromUserResultCallback {
    int requestCode = 0;
    std::vector<std::string> permissions;
    std::vector<int> grantResults;
    CallbackInfo cb;
};

/**
 * @brief Context NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ContextPermissionInit(napi_env env, napi_value exports);

EXTERN_C_START
/**
 * @brief The interface of onRequestPermissionsFromUserResult provided for ACE to call back to JS.
 *
 * @param requestCode Indicates the request code returned after the ability is started.
 * @param permissions Indicates list of permission.
 * @param grantResults Indicates List of authorization results.
 * @param callbackInfo The environment and call back info that the Node-API call is invoked under.
 */
void CallOnRequestPermissionsFromUserResult(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults, OHOS::AppExecFwk::CallbackInfo callbackInfo);
EXTERN_C_END
napi_value ContextConstructor(napi_env env, napi_callback_info info);

/**
 * @brief Set lock screen Permit list.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetShowOnLockScreen(napi_env env, napi_callback_info info);

/**
 * @brief Obtains information about the current application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetApplicationInfo(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the elementName object of the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetElementName(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the process Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessInfo(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the name of the current process.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessName(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCallingBundle(napi_env env, napi_callback_info info);

napi_value NAPI_GetOrCreateDistributedDir(napi_env env, napi_callback_info info);

napi_value NAPI_GetFilesDir(napi_env env, napi_callback_info info);

napi_value NAPI_GetCacheDir(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxAppType(napi_env env, napi_callback_info info);

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxHapModuleInfo(napi_env env, napi_callback_info info);

napi_value NAPI_GetAppVersionInfo(napi_env env, napi_callback_info info);

napi_value NAPI_GetApplicationContext(napi_env env, napi_callback_info info);

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxAbilityInfo(napi_env env, napi_callback_info info);

/**
 * @brief Set wake up screen screen Permit list.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetWakeUpScreen(napi_env env, napi_callback_info info);

/**
 * @brief Set the display orientation of the main window.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetDisplayOrientation(napi_env env, napi_callback_info info);
#ifdef SUPPORT_GRAPHICS
napi_value NAPI_SetDisplayOrientationWrap(napi_env env, napi_callback_info info,
    AsyncJSCallbackInfo *asyncCallbackInfo);
bool UnwrapSetDisplayOrientation(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
void SetDisplayOrientationExecuteCallbackWork(napi_env env, void *data);
#endif

/**
 * @brief Get the display orientation of the main window.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDisplayOrientation(napi_env env, napi_callback_info info);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_NAPI_CONTEXT_H */
