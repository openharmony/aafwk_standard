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

#include "napi_context.h"

#include <cstring>
#include <uv.h>

#include "../inner/napi_common/napi_common_ability.h"
#include "ability_process.h"
#include "directory_ex.h"
#include "feature_ability_common.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
const std::string NAPI_CONTEXT_FILE_SEPARATOR = std::string("/");
const std::string NAPI_CONTEXT_DATABASE = std::string("database");
const std::string NAPI_CONTEXT_PREFERENCES = std::string("preferences");

napi_value ContextConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

    napi_value value = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &value));

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("stageMode", value),
    };
    NAPI_CALL(env, napi_define_properties(env, jsthis, sizeof(desc) / sizeof(desc[0]), desc));

    return jsthis;
}

bool UnwrapParamVerifySelfPermission(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_INFO("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);
    return true;
}

void VerifySelfPermissionExecuteCallbackWork(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifySelfPermission(
        asyncCallbackInfo->param.paramArgs.GetStringValue("permission"));
}

/**
 * @brief VerifySelfPermission processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_VerifySelfPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifySelfPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifySelfPermissionCallback";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifySelfPermissionPromise";
        asyncParamEx.execute = VerifySelfPermissionExecuteCallbackWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

/**
 * @brief Checks whether the current process has the given permission.
 * You need to call requestPermissionsFromUser(java.lang.std::string[],int) to request a permission only
 * if the current process does not have the specific permission.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
 * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
 */
napi_value NAPI_VerifySelfPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value rev = NAPI_VerifySelfPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

bool UnwrapRequestPermissionsFromUser(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 3;
    if (argc != argcMax) {
        HILOG_INFO("%{public}s called, parameters is invalid", __func__);
        return false;
    }

    CallbackInfo* aceCallbackInfoPermission = new CallbackInfo();
    aceCallbackInfoPermission->env = env;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM2], &valuetype), false);
    if (valuetype != napi_function) {
        HILOG_INFO("%{public}s called, parameters is invalid", __func__);
        return false;
    }
    NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM2], 1, &(aceCallbackInfoPermission->callback)), false);
    asyncCallbackInfo->aceCallback = aceCallbackInfoPermission;

    std::vector<std::string> permissionList;
    if (!UnwrapArrayStringFromJS(env, argv[PARAM0], permissionList)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }

    int requestCode = 0;
    if (!UnwrapInt32FromJS2(env, argv[PARAM1], requestCode)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        return false;
    }

    asyncCallbackInfo->param.paramArgs.PutIntValue("requestCode", requestCode);
    asyncCallbackInfo->param.paramArgs.PutStringValueArray("permissionList", permissionList);
    return true;
}

void RequestPermissionsFromUserExecuteCallbackWork(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    CallAbilityPermissionParam permissionParam;
    permissionParam.requestCode = asyncCallbackInfo->param.paramArgs.GetIntValue("requestCode");
    asyncCallbackInfo->param.paramArgs.GetStringValueArray("permissionList", permissionParam.permission_list);
    if (permissionParam.permission_list.size() == 0) {
        asyncCallbackInfo->error_code = NAPI_ERR_PARAM_INVALID;
        return;
    }

    AbilityProcess::GetInstance()->RequestPermissionsFromUser(
        asyncCallbackInfo->ability, permissionParam, *asyncCallbackInfo->aceCallback);
    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = 1;
}

void RequestPermissionsFromUserExecutePromiseWork(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    CallAbilityPermissionParam permissionParam;
    permissionParam.requestCode = asyncCallbackInfo->param.paramArgs.GetIntValue("requestCode");
    asyncCallbackInfo->param.paramArgs.GetStringValueArray("permissionList", permissionParam.permission_list);
    if (permissionParam.permission_list.size() == 0) {
        asyncCallbackInfo->error_code = NAPI_ERR_PARAM_INVALID;
        return;
    }

    AbilityProcess::GetInstance()->RequestPermissionsFromUser(
        asyncCallbackInfo->ability, permissionParam, *asyncCallbackInfo->aceCallback);
}

void RequestPermissionsFromUserCompleteAsyncCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }

    if (asyncCallbackInfo->error_code != NAPI_ERR_NO_ERROR && asyncCallbackInfo->aceCallback != nullptr) {
        napi_value callback = 0;
        napi_value undefined = 0;
        napi_get_undefined(env, &undefined);
        napi_value callResult = 0;
        napi_value revParam[ARGS_TWO] = {nullptr};

        revParam[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->error_code);
        revParam[PARAM1] = WrapVoidToJS(env);

        if (asyncCallbackInfo->aceCallback->callback != nullptr) {
            napi_get_reference_value(env, asyncCallbackInfo->aceCallback->callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, revParam, &callResult);
            napi_delete_reference(env, asyncCallbackInfo->aceCallback->callback);

            asyncCallbackInfo->aceCallback->callback = nullptr;
            asyncCallbackInfo->aceCallback->env = nullptr;
        }
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

/**
 * @brief RequestPermissionsFromUser processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_RequestPermissionsFromUserWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapRequestPermissionsFromUser(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapRequestPermissionsFromUser failed.", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    asyncParamEx.resource = "NAPI_RequestPermissionsFromUserCallback";
    asyncParamEx.execute = RequestPermissionsFromUserExecuteCallbackWork;
    asyncParamEx.complete = RequestPermissionsFromUserCompleteAsyncCallbackWork;

    return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
}

/**
 * @brief Requests certain permissions from the system.
 * This method is called for permission request. This is an asynchronous method. When it is executed,
 * the Ability.onRequestPermissionsFromUserResult(int, String[], int[]) method will be called back.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 */
napi_value NAPI_RequestPermissionsFromUser(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_RequestPermissionsFromUserWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        if (asyncCallbackInfo->aceCallback->callback != nullptr && asyncCallbackInfo->aceCallback->env != nullptr) {
            napi_delete_reference(asyncCallbackInfo->aceCallback->env, asyncCallbackInfo->aceCallback->callback);
        }
        asyncCallbackInfo->aceCallback->env = nullptr;
        asyncCallbackInfo->aceCallback->callback = nullptr;

        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

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
    const std::vector<int> &grantResults, CallbackInfo callbackInfo)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, callbackInfo.env);

    if (permissions.empty()) {
        HILOG_ERROR("%{public}s, the string vector permissions is empty.", __func__);
    }
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(callbackInfo.env, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==null.", __func__);
        return;
    }
    OnRequestPermissionsFromUserResultCallback *onRequestPermissionCB =
        new (std::nothrow) OnRequestPermissionsFromUserResultCallback;
    if (onRequestPermissionCB == nullptr) {
        HILOG_ERROR("%{public}s, the string vector permissions is empty.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    onRequestPermissionCB->requestCode = requestCode;
    onRequestPermissionCB->permissions = permissions;
    onRequestPermissionCB->grantResults = grantResults;
    onRequestPermissionCB->cb = callbackInfo;

    work->data = (void *)onRequestPermissionCB;

    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            OnRequestPermissionsFromUserResultCallback *onRequestPermissionCB =
                (OnRequestPermissionsFromUserResultCallback *)work->data;
            if (onRequestPermissionCB == nullptr) {
                if (work != nullptr) {
                    delete work;
                    work = nullptr;
                }
                return;
            }

            napi_value result[ARGS_TWO] = {0};
            result[PARAM0] = GetCallbackErrorValue(onRequestPermissionCB->cb.env, 0);
            napi_create_object(onRequestPermissionCB->cb.env, &result[PARAM1]);

            // create requestCode
            napi_value jsValue = 0;
            napi_create_int32(onRequestPermissionCB->cb.env, onRequestPermissionCB->requestCode, &jsValue);
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "requestCode", jsValue);

            // create permissions
            napi_value perValue = 0;
            napi_value perArray = 0;
            napi_create_array(onRequestPermissionCB->cb.env, &perArray);

            for (size_t i = 0; i < onRequestPermissionCB->permissions.size(); i++) {
                napi_create_string_utf8(onRequestPermissionCB->cb.env,
                    onRequestPermissionCB->permissions[i].c_str(),
                    NAPI_AUTO_LENGTH,
                    &perValue);
                napi_set_element(onRequestPermissionCB->cb.env, perArray, i, perValue);
            }
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "permissions", perArray);

            // create grantResults
            napi_value grantArray;
            napi_create_array(onRequestPermissionCB->cb.env, &grantArray);

            for (size_t j = 0; j < onRequestPermissionCB->grantResults.size(); j++) {
                napi_create_int32(onRequestPermissionCB->cb.env, onRequestPermissionCB->grantResults[j], &perValue);
                napi_set_element(onRequestPermissionCB->cb.env, grantArray, j, perValue);
            }
            napi_set_named_property(onRequestPermissionCB->cb.env, result[PARAM1], "authResults", grantArray);

            // call CB function
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(onRequestPermissionCB->cb.env, &undefined);

            napi_value callResult = 0;
            napi_get_reference_value(onRequestPermissionCB->cb.env, onRequestPermissionCB->cb.callback, &callback);
            napi_call_function(
                onRequestPermissionCB->cb.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (onRequestPermissionCB->cb.callback != nullptr) {
                napi_delete_reference(onRequestPermissionCB->cb.env, onRequestPermissionCB->cb.callback);
            }
            if (onRequestPermissionCB != nullptr) {
                delete onRequestPermissionCB;
                onRequestPermissionCB = nullptr;
            }
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        });

    if (rev != 0) {
        if (onRequestPermissionCB != nullptr) {
            delete onRequestPermissionCB;
            onRequestPermissionCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}
EXTERN_C_END

/**
 * @brief Get Files Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetFilesDirCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Get OrCreateDistributed Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetOrCreateDistributedDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetOrCreateDistributedDirCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Get Cache Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_GetCacheDirCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppTypeCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetHapModuleInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAppVersionInfoCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains the Application Context object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetApplicationContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetContextCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCtxAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return NAPI_GetAbilityInfoCommon(env, info, AbilityType::PAGE);
}

void CanRequestPermissionExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission"));
    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->CanRequestPermission(permission);
}

napi_value NAPI_CanRequestPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifySelfPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_CanRequestPermissionCallback";
        asyncParamEx.execute = CanRequestPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_CanRequestPermissionPromise";
        asyncParamEx.execute = CanRequestPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_CanRequestPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_CanRequestPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

bool UnwrapVerifyPermissionOptions(napi_env env, napi_value argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return false;
    }

    if (!IsTypeForNapiValue(env, argv, napi_object)) {
        HILOG_INFO("%{public}s called, type of parameter is error.", __func__);
        return false;
    }

    int value = 0;
    if (UnwrapInt32ByPropertyName(env, argv, "pid", value)) {
        asyncCallbackInfo->param.paramArgs.PutIntValue("pid", value);
    }

    value = 0;
    if (UnwrapInt32ByPropertyName(env, argv, "uid", value)) {
        asyncCallbackInfo->param.paramArgs.PutIntValue("uid", value);
    }
    return true;
}

bool UnwrapParamVerifyPermission(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = ARGS_THREE;
    if (argc > argcMax || argc < 1) {
        HILOG_INFO("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }
    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }

        if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    } else if (argc == ARGS_TWO) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            if (!UnwrapVerifyPermissionOptions(env, argv[PARAM1], asyncCallbackInfo)) {
                HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
                return false;
            }
        }
    } else if (argc == ARGS_ONE) {
        asyncCallbackInfo->cbInfo.callback = nullptr;
    } else {
        HILOG_INFO("%{public}s called, the parameter is invalid.", __func__);
        return false;
    }
    return true;
}

void VerifyPermissionExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission").c_str());
    bool hasUid = asyncCallbackInfo->param.paramArgs.HasKey("uid");
    int pid = asyncCallbackInfo->param.paramArgs.GetIntValue("pid");
    int uid = asyncCallbackInfo->param.paramArgs.GetIntValue("uid");

    asyncCallbackInfo->native_data.data_type = NVT_INT32;

    if (hasUid) {
        asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifyPermission(permission, pid, uid);
    } else {
        asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifySelfPermission(permission);
    }
}

napi_value NAPI_VerifyPermissionWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifyPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifyPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionCallback";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifyPermissionPromise";
        asyncParamEx.execute = VerifyPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifyPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_VerifyPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

void VerifyCallingPermissionExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission").c_str());
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifyCallingPermission(permission);
}

napi_value NAPI_VerifyCallingPermissionWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifySelfPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifyCallingPermissionCallback";
        asyncParamEx.execute = VerifyCallingPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifyCallingPermissionPromise";
        asyncParamEx.execute = VerifyCallingPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifyCallingPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_VerifyCallingPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

void VerifyCallingOrSelfPermissionExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission").c_str());
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifyCallingOrSelfPermission(permission);
}

napi_value NAPI_VerifyCallingOrSelfPermissionWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamVerifySelfPermission(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamVerifySelfPermission fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_VerifyCallingOrSelfPermissionCallback";
        asyncParamEx.execute = VerifyCallingOrSelfPermissionExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_VerifyCallingOrSelfPermissionPromise";
        asyncParamEx.execute = VerifyCallingOrSelfPermissionExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_VerifyCallingOrSelfPermission(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_VerifyCallingOrSelfPermissionWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        rev = WrapVoidToJS(env);
    }
    return rev;
}

/**
 * @brief GetApplicationInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    if (appInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ApplicationInfo> appInfoPtr = appInfoCB->cbBase.ability->GetApplicationInfo();
    if (appInfoPtr != nullptr) {
        SaveAppInfo(appInfoCB->appInfo, *appInfoPtr);
    } else {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoPtr == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAppInfo(env, appInfoCB->appInfo);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appInfoCB->cbBase.asyncWork));
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief GetApplicationInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoAsyncCompleteCB,
            (void *)appInfoCB,
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = static_cast<AppInfoCB *>(data);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoCB == nullptr");
        return;
    }

    napi_value result = nullptr;
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAppInfo(env, appInfoCB->appInfo);
        napi_resolve_deferred(env, appInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
        napi_reject_deferred(env, appInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appInfoCB->cbBase.asyncWork);
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief GetApplicationInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoPromise(napi_env env, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoPromiseCompleteCB,
            (void *)appInfoCB,
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetApplicationInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoWrap(napi_env env, napi_callback_info info, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetApplicationInfoAsync(env, args, 0, appInfoCB);
    } else {
        ret = GetApplicationInfoPromise(env, appInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppInfoCB on success, nullptr on failure.
 */
AppInfoCB *CreateAppInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppInfoCB *appInfoCB = new (std::nothrow) AppInfoCB;
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appInfoCB->cbBase.cbInfo.env = env;
    appInfoCB->cbBase.asyncWork = nullptr;
    appInfoCB->cbBase.deferred = nullptr;
    appInfoCB->cbBase.ability = ability;
    appInfoCB->cbBase.abilityType = AbilityType::UNKNOWN;
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    HILOG_INFO("%{public}s, end.", __func__);
    return appInfoCB;
}

/**
 * @brief GetBundleName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetBundleNameExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetBundleName();
    HILOG_INFO("%{public}s end. bundleName=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief GetBundleName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetBundleNameWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNameCallback";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNamePromise";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value WrapProcessInfo(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s called", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s Invalid param(processInfoCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, processInfoCB->pid, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "pid", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, processInfoCB->processName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "processName", proValue));
    HILOG_INFO("%{public}s end", __func__);
    return result;
}

/**
 * @brief GetProcessInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    if (processInfoCB == nullptr) {
        return;
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessInfo, ability == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ProcessInfo> processInfoPtr = processInfoCB->cbBase.ability->GetProcessInfo();
    if (processInfoPtr != nullptr) {
        processInfoCB->processName = processInfoPtr->GetProcessName();
        processInfoCB->pid = processInfoPtr->GetPid();
    } else {
        HILOG_ERROR("NAPI_GetProcessInfo, processInfoPtr == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessInfo(env, processInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processInfoCB->cbBase.asyncWork));
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

/**
 * @brief GetProcessInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoAsyncCompleteCB,
            (void *)processInfoCB,
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = static_cast<ProcessInfoCB *>(data);
    napi_value result = nullptr;
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessInfo(env, processInfoCB);
        napi_resolve_deferred(env, processInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
        napi_reject_deferred(env, processInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processInfoCB->cbBase.asyncWork);
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

/**
 * @brief GetProcessInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoPromise(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoPromiseCompleteCB,
            (void *)processInfoCB,
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetProcessInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoWrap(napi_env env, napi_callback_info info, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessInfoAsync(env, args, 0, processInfoCB);
    } else {
        ret = GetProcessInfoPromise(env, processInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to ProcessInfoCB on success, nullptr on failure.
 */
ProcessInfoCB *CreateProcessInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessInfoCB *processInfoCB = new (std::nothrow) ProcessInfoCB;
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
        return nullptr;
    }
    processInfoCB->cbBase.cbInfo.env = env;
    processInfoCB->cbBase.asyncWork = nullptr;
    processInfoCB->cbBase.deferred = nullptr;
    processInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return processInfoCB;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to ElementNameCB on success, nullptr on failure.
 */
ElementNameCB *CreateElementNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ElementNameCB *elementNameCB = new (std::nothrow) ElementNameCB;
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, elementNameCB == nullptr.", __func__);
        return nullptr;
    }
    elementNameCB->cbBase.cbInfo.env = env;
    elementNameCB->cbBase.asyncWork = nullptr;
    elementNameCB->cbBase.deferred = nullptr;
    elementNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return elementNameCB;
}

napi_value WrapElementName(napi_env env, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(appInfoCB = nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->abilityName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->deviceId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceId", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->shortName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "shortName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief GetElementName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetElementName, worker pool thread execute.");
    if (data == nullptr) {
        HILOG_ERROR("%{public}s, data == nullptr.", __func__);
        return;
    }
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetElementName, elementNameCB == nullptr");
        return;
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (elementNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetElementName, ability == nullptr");
        elementNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::shared_ptr<ElementName> elementName = elementNameCB->cbBase.ability->GetElementName();
    if (elementNameCB != nullptr) {
        elementNameCB->deviceId = elementName->GetDeviceID();
        elementNameCB->bundleName = elementName->GetBundleName();
        elementNameCB->abilityName = elementName->GetAbilityName();
        elementNameCB->uri = elementNameCB->cbBase.ability->GetWant()->GetUriString();
        elementNameCB->shortName = "";
    } else {
        elementNameCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetElementName, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetElementName, main event thread complete.");
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapElementName(env, elementNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, elementNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (elementNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, elementNameCB->cbBase.asyncWork));
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetElementName, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetElementName, main event thread complete.");
    ElementNameCB *elementNameCB = static_cast<ElementNameCB *>(data);
    napi_value result = nullptr;
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapElementName(env, elementNameCB);
        napi_resolve_deferred(env, elementNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
        napi_reject_deferred(env, elementNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, elementNameCB->cbBase.asyncWork);
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetElementName, main event thread complete end.");
}

/**
 * @brief GetElementName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNamePromise(napi_env env, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    elementNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNamePromiseCompleteCB,
            (void *)elementNameCB,
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetElementName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNameAsync(napi_env env, napi_value *args, const size_t argCallback, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNameAsyncCompleteCB,
            (void *)elementNameCB,
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
}

/**
 * @brief GetElementName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNameWrap(napi_env env, napi_callback_info info, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetElementNameAsync(env, args, 0, elementNameCB);
    } else {
        ret = GetElementNamePromise(env, elementNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to ProcessNameCB on success, nullptr on failure.
 */
ProcessNameCB *CreateProcessNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessNameCB *processNameCB = new (std::nothrow) ProcessNameCB;
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }
    processNameCB->cbBase.cbInfo.env = env;
    processNameCB->cbBase.asyncWork = nullptr;
    processNameCB->cbBase.deferred = nullptr;
    processNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return processNameCB;
}

/**
 * @brief GetProcessName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    if (processNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, processNameCB == nullptr");
        return;
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, ability == nullptr");
        processNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    processNameCB->processName = processNameCB->cbBase.ability->GetProcessName();
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute end.");
}

napi_value WrapProcessName(napi_env env, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, Invalid param(processNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, processNameCB->processName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessName(env, processNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processNameCB->cbBase.asyncWork));
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = static_cast<ProcessNameCB *>(data);
    napi_value result = nullptr;
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessName(env, processNameCB);
        napi_resolve_deferred(env, processNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
        napi_reject_deferred(env, processNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processNameCB->cbBase.asyncWork);
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

/**
 * @brief GetProcessName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNameAsync(napi_env env, napi_value *args, const size_t argCallback, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNameAsyncCompleteCB,
            (void *)processNameCB,
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetProcessName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNamePromise(napi_env env, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNamePromiseCompleteCB,
            (void *)processNameCB,
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetProcessName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNameWrap(napi_env env, napi_callback_info info, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetProcessNameAsync(env, args, 0, processNameCB);
    } else {
        ret = GetProcessNamePromise(env, processNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to CallingBundleCB on success, nullptr on failure.
 */
CallingBundleCB *CreateCallingBundleCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    CallingBundleCB *callingBundleCB = new (std::nothrow) CallingBundleCB;
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }
    callingBundleCB->cbBase.cbInfo.env = env;
    callingBundleCB->cbBase.asyncWork = nullptr;
    callingBundleCB->cbBase.deferred = nullptr;
    callingBundleCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return callingBundleCB;
}

/**
 * @brief GetCallingBundle asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundleExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, callingBundleCB == nullptr");
        return;
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (callingBundleCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, ability == nullptr");
        callingBundleCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    callingBundleCB->callingBundleName = callingBundleCB->cbBase.ability->GetCallingBundle();
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute end.");
}

napi_value WrapCallingBundle(napi_env env, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(callingBundleCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, callingBundleCB->callingBundleName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundleAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapCallingBundle(env, callingBundleCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callingBundleCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (callingBundleCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork));
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundlePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = static_cast<CallingBundleCB *>(data);
    napi_value result = nullptr;
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapCallingBundle(env, callingBundleCB);
        napi_resolve_deferred(env, callingBundleCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
        napi_reject_deferred(env, callingBundleCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork);
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

/**
 * @brief GetCallingBundle Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundleAsync(
    napi_env env, napi_value *args, const size_t argCallback, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundleAsyncCompleteCB,
            (void *)callingBundleCB,
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetCallingBundle Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundlePromise(napi_env env, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    callingBundleCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundlePromiseCompleteCB,
            (void *)callingBundleCB,
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetCallingBundle processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundleWrap(napi_env env, napi_callback_info info, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetCallingBundleAsync(env, args, 0, callingBundleCB);
    } else {
        ret = GetCallingBundlePromise(env, callingBundleCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to CallingBundleCB on success, nullptr on failure.
 */
GetOrCreateLocalDirCB *CreateGetOrCreateLocalDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = new (std::nothrow) GetOrCreateLocalDirCB;
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }
    getOrCreateLocalDirCB->cbBase.cbInfo.env = env;
    getOrCreateLocalDirCB->cbBase.asyncWork = nullptr;
    getOrCreateLocalDirCB->cbBase.deferred = nullptr;
    getOrCreateLocalDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getOrCreateLocalDirCB;
}

/**
 * @brief GetOrCreateLocalDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetOrCreateLocalDirExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, worker pool thread execute.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("NAPI_GetOrCreateLocalDir, callingBundleCB == nullptr");
        return;
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getOrCreateLocalDirCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetOrCreateLocalDir, ability == nullptr");
        getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    std::string abilityName = getOrCreateLocalDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getOrCreateLocalDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getOrCreateLocalDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getOrCreateLocalDirCB->rootDir = "";
        HILOG_INFO("NAPI_GetOrCreateLocalDir, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            dataDirWithModuleName.c_str());
        return;
    }

    getOrCreateLocalDirCB->rootDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, GetDir rootDir:%{public}s", getOrCreateLocalDirCB->rootDir.c_str());
    if (!OHOS::FileExists(getOrCreateLocalDirCB->rootDir)) {
        HILOG_INFO("NAPI_GetOrCreateLocalDir dir is not exits, create dir.");
        OHOS::ForceCreateDirectory(getOrCreateLocalDirCB->rootDir);
        OHOS::ChangeModeDirectory(getOrCreateLocalDirCB->rootDir, MODE);
    }
    HILOG_INFO("NAPI_GetOrCreateLocalDir, worker pool thread execute end.");
}

napi_value WrapGetOrCreateLocalDir(napi_env env, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(getOrCreateLocalDirCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getOrCreateLocalDirCB->rootDir.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetOrCreateLocalDirAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, getOrCreateLocalDirCB->cbBase.errCode);
    if (getOrCreateLocalDirCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapGetOrCreateLocalDir(env, getOrCreateLocalDirCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, getOrCreateLocalDirCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (getOrCreateLocalDirCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, getOrCreateLocalDirCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    delete getOrCreateLocalDirCB;
    getOrCreateLocalDirCB = nullptr;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetOrCreateLocalDirPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete.");
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = static_cast<GetOrCreateLocalDirCB *>(data);
    napi_value result = nullptr;
    if (getOrCreateLocalDirCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapGetOrCreateLocalDir(env, getOrCreateLocalDirCB);
        napi_resolve_deferred(env, getOrCreateLocalDirCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, getOrCreateLocalDirCB->cbBase.errCode);
        napi_reject_deferred(env, getOrCreateLocalDirCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork);
    delete getOrCreateLocalDirCB;
    getOrCreateLocalDirCB = nullptr;
    HILOG_INFO("NAPI_GetOrCreateLocalDir, main event thread complete end.");
}

/**
 * @brief GetOrCreateLocalDir Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetOrCreateLocalDirAsync(
    napi_env env, napi_value *args, const size_t argCallback, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(
            env, napi_create_reference(env, args[argCallback], 1, &getOrCreateLocalDirCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetOrCreateLocalDirExecuteCB,
            GetOrCreateLocalDirAsyncCompleteCB,
            (void *)getOrCreateLocalDirCB,
            &getOrCreateLocalDirCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetOrCreateLocalDir Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetOrCreateLocalDirPromise(napi_env env, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    getOrCreateLocalDirCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetOrCreateLocalDirExecuteCB,
            GetOrCreateLocalDirPromiseCompleteCB,
            (void *)getOrCreateLocalDirCB,
            &getOrCreateLocalDirCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getOrCreateLocalDirCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetOrCreateLocalDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetOrCreateLocalDirWrap(napi_env env, napi_callback_info info, GetOrCreateLocalDirCB *getOrCreateLocalDirCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (getOrCreateLocalDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getOrCreateLocalDirCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetOrCreateLocalDirAsync(env, args, 0, getOrCreateLocalDirCB);
    } else {
        ret = GetOrCreateLocalDirPromise(env, getOrCreateLocalDirCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Get bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetBundleName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value ret = NAPI_GetBundleNameWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief Obtains information about the current application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetApplicationInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AppInfoCB *appInfoCB = CreateAppInfoCBInfo(env);
    if (appInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    napi_value ret = GetApplicationInfoWrap(env, info, appInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (appInfoCB != nullptr) {
            delete appInfoCB;
            appInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

/**
 * @brief Obtains the process Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessInfoCB *processInfoCB = CreateProcessInfoCBInfo(env);
    if (processInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessInfoWrap(env, info, processInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (processInfoCB != nullptr) {
            delete processInfoCB;
            processInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return ret;
}

/**
 * @brief Obtains the elementName object of the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetElementName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ElementNameCB *elementNameCB = CreateElementNameCBInfo(env);
    if (elementNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetElementNameWrap(env, info, elementNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (elementNameCB != nullptr) {
            delete elementNameCB;
            elementNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Obtains the name of the current process.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessNameCB *processNameCB = CreateProcessNameCBInfo(env);
    if (processNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetProcessNameWrap(env, info, processNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (processNameCB != nullptr) {
            delete processNameCB;
            processNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCallingBundle(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    CallingBundleCB *callingBundleCB = CreateCallingBundleCBInfo(env);
    if (callingBundleCB == nullptr) {
        return WrapVoidToJS(env);
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetCallingBundleWrap(env, info, callingBundleCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (callingBundleCB != nullptr) {
            delete callingBundleCB;
            callingBundleCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Get the local root dir of an app. If it is the first call, the dir will be created.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetOrCreateLocalDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    GetOrCreateLocalDirCB *getOrCreateLocalDirCB = CreateGetOrCreateLocalDirCBInfo(env);
    if (getOrCreateLocalDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    getOrCreateLocalDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetOrCreateLocalDirWrap(env, info, getOrCreateLocalDirCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (getOrCreateLocalDirCB != nullptr) {
            delete getOrCreateLocalDirCB;
            getOrCreateLocalDirCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to CallingBundleCB on success, nullptr on failure.
 */
DatabaseDirCB *CreateGetDatabaseDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    DatabaseDirCB *getDatabaseDirCB = new (std::nothrow) DatabaseDirCB;
    if (getDatabaseDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }
    getDatabaseDirCB->cbBase.cbInfo.env = env;
    getDatabaseDirCB->cbBase.asyncWork = nullptr;
    getDatabaseDirCB->cbBase.deferred = nullptr;
    getDatabaseDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getDatabaseDirCB;
}

/**
 * @brief GetOrCreateLocalDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDatabaseDirWrap(napi_env env, napi_callback_info info, DatabaseDirCB *getDatabaseDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getDatabaseDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getDatabaseDirCB == nullptr.", __func__);
        return nullptr;
    }

    getDatabaseDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getDatabaseDirCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetDatabaseDir, ability == nullptr");
        getDatabaseDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getDatabaseDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getDatabaseDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    HILOG_INFO("%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getDatabaseDirCB->dataBaseDir = "";
        HILOG_INFO("%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getDatabaseDirCB->dataBaseDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                        NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_DATABASE;
        HILOG_INFO("%{public}s, GetDir dataBaseDir:%{public}s", __func__, getDatabaseDirCB->dataBaseDir.c_str());
        if (!OHOS::FileExists(getDatabaseDirCB->dataBaseDir)) {
            HILOG_INFO("NAPI_GetDatabaseDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getDatabaseDirCB->dataBaseDir);
            OHOS::ChangeModeDirectory(getDatabaseDirCB->dataBaseDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, getDatabaseDirCB->dataBaseDir.c_str(), NAPI_AUTO_LENGTH, &result));

    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief Obtains the local database path. If it is the first call, the dir will be created.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDatabaseDirSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    DatabaseDirCB *getDatabaseDirCB = CreateGetDatabaseDirCBInfo(env);
    if (getDatabaseDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    getDatabaseDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetDatabaseDirWrap(env, info, getDatabaseDirCB);

    delete getDatabaseDirCB;
    getDatabaseDirCB = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to CallingBundleCB on success, nullptr on failure.
 */
PreferencesDirCB *CreateGetPreferencesDirCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    PreferencesDirCB *getPreferencesDirCB = new (std::nothrow) PreferencesDirCB;
    if (getPreferencesDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }
    getPreferencesDirCB->cbBase.cbInfo.env = env;
    getPreferencesDirCB->cbBase.asyncWork = nullptr;
    getPreferencesDirCB->cbBase.deferred = nullptr;
    getPreferencesDirCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return getPreferencesDirCB;
}

/**
 * @brief GetOrCreateLocalDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetPreferencesDirWrap(napi_env env, napi_callback_info info, PreferencesDirCB *getPreferencesDirCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (getPreferencesDirCB == nullptr) {
        HILOG_ERROR("%{public}s, getPreferencesDirCB == nullptr.", __func__);
        return nullptr;
    }

    getPreferencesDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (getPreferencesDirCB->cbBase.ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        getPreferencesDirCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return nullptr;
    }

    std::string abilityName = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->name;
    std::string dataDir = getPreferencesDirCB->cbBase.ability->GetAbilityInfo()->applicationInfo.dataDir;
    std::shared_ptr<HapModuleInfo> hap = getPreferencesDirCB->cbBase.ability->GetHapModuleInfo();
    std::string moduleName = (hap != nullptr) ? hap->name : std::string();
    std::string dataDirWithModuleName = dataDir + NAPI_CONTEXT_FILE_SEPARATOR + moduleName;
    HILOG_INFO("%{public}s, dataDir:%{public}s moduleName:%{public}s abilityName:%{public}s",
        __func__,
        dataDir.c_str(),
        moduleName.c_str(),
        abilityName.c_str());

    // if dataDirWithModuleName is not exits, do nothing and return.
    if (!OHOS::FileExists(dataDirWithModuleName)) {
        getPreferencesDirCB->preferencesDir = "";
        HILOG_INFO("%{public}s, dirWithModuleName is not exits:%{public}s, do nothing and return null.",
            __func__,
            dataDirWithModuleName.c_str());
    } else {
        getPreferencesDirCB->preferencesDir = dataDirWithModuleName + NAPI_CONTEXT_FILE_SEPARATOR + abilityName +
                                              NAPI_CONTEXT_FILE_SEPARATOR + NAPI_CONTEXT_PREFERENCES;
        HILOG_INFO(
            "%{public}s, GetDir preferencesDir:%{public}s", __func__, getPreferencesDirCB->preferencesDir.c_str());
        if (!OHOS::FileExists(getPreferencesDirCB->preferencesDir)) {
            HILOG_INFO("NAPI_GetPreferencesDir dir is not exits, create dir.");
            OHOS::ForceCreateDirectory(getPreferencesDirCB->preferencesDir);
            OHOS::ChangeModeDirectory(getPreferencesDirCB->preferencesDir, MODE);
        }
    }
    napi_value result = nullptr;
    NAPI_CALL(
        env, napi_create_string_utf8(env, getPreferencesDirCB->preferencesDir.c_str(), NAPI_AUTO_LENGTH, &result));

    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief Obtains the local database path. If it is the first call, the dir will be created.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetPreferencesDirSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    PreferencesDirCB *preferencesDirCB = CreateGetPreferencesDirCBInfo(env);
    if (preferencesDirCB == nullptr) {
        return WrapVoidToJS(env);
    }

    preferencesDirCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = GetPreferencesDirWrap(env, info, preferencesDirCB);

    delete preferencesDirCB;
    preferencesDirCB = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

/**
 * @brief Checks whether the configuration of this ability is changing.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return Returns true if the configuration of this ability is changing and false otherwise.
 */
napi_value NAPI_IsUpdatingConfigurations(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_IsUpdatingConfigurationsCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Informs the system of the time required for drawing this Page ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_PrintDrawnCompleted(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called", __func__);
    return NAPI_PrintDrawnCompletedCommon(env, info, AbilityType::PAGE);
}

/**
 * @brief Context NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ContextPermissionInit(napi_env env, napi_value exports)
{
    HILOG_INFO("Context::ContextPermissionInit called.");

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("verifySelfPermission", NAPI_VerifySelfPermission),
        DECLARE_NAPI_FUNCTION("requestPermissionsFromUser", NAPI_RequestPermissionsFromUser),
        DECLARE_NAPI_FUNCTION("getBundleName", NAPI_GetBundleName),
        DECLARE_NAPI_FUNCTION("canRequestPermission", NAPI_CanRequestPermission),
        DECLARE_NAPI_FUNCTION("verifyPermission", NAPI_VerifyPermission),
        DECLARE_NAPI_FUNCTION("verifyCallingPermission", NAPI_VerifyCallingPermission),
        DECLARE_NAPI_FUNCTION("verifyCallingOrSelfPermission", NAPI_VerifyCallingOrSelfPermission),
        DECLARE_NAPI_FUNCTION("getApplicationInfo", NAPI_GetApplicationInfo),
        DECLARE_NAPI_FUNCTION("getProcessInfo", NAPI_GetProcessInfo),
        DECLARE_NAPI_FUNCTION("getElementName", NAPI_GetElementName),
        DECLARE_NAPI_FUNCTION("getProcessName", NAPI_GetProcessName),
        DECLARE_NAPI_FUNCTION("getCallingBundle", NAPI_GetCallingBundle),
        DECLARE_NAPI_FUNCTION("getOrCreateLocalDir", NAPI_GetOrCreateLocalDir),
        DECLARE_NAPI_FUNCTION("getFilesDir", NAPI_GetFilesDir),
        DECLARE_NAPI_FUNCTION("isUpdatingConfigurations", NAPI_IsUpdatingConfigurations),
        DECLARE_NAPI_FUNCTION("printDrawnCompleted", NAPI_PrintDrawnCompleted),
        DECLARE_NAPI_FUNCTION("getDatabaseDirSync", NAPI_GetDatabaseDirSync),
        DECLARE_NAPI_FUNCTION("getPreferencesDirSync", NAPI_GetPreferencesDirSync),
        DECLARE_NAPI_FUNCTION("getCacheDir", NAPI_GetCacheDir),
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_GetCtxAppType),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_GetCtxHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getAppVersionInfo", NAPI_GetAppVersionInfo),
        DECLARE_NAPI_FUNCTION("getApplicationContext", NAPI_GetApplicationContext),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_GetCtxAbilityInfo),
        DECLARE_NAPI_FUNCTION("getOrCreateDistributedDir", NAPI_GetOrCreateDistributedDir),
    };

    NAPI_CALL(env,
        napi_define_class(env,
            "context",
            NAPI_AUTO_LENGTH,
            ContextConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            GetGlobalClassContext()));

    return exports;
}
}  // namespace AppExecFwk
}  // namespace OHOS