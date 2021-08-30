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
#include <cstring>
#include <uv.h>
#include "securec.h"
#include "napi_context.h"
#include "hilog_wrapper.h"
#include "ability_process.h"
#include "feature_ability_common.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
napi_value g_classContext;

CallbackInfo aceCallbackInfoPermission;

napi_value ContextConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));
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
    std::string permission(asyncCallbackInfo->param.paramArgs.GetStringValue("permission"));
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifySelfPermission(permission);
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
}

bool UnwrapRequestPermissionsFromUser(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 3;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_INFO("%{public}s called, parameters is invalid", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM2], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    }

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

    asyncCallbackInfo->aceCallback = &aceCallbackInfoPermission;
    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_RequestPermissionsFromUserCallback";
        asyncParamEx.execute = RequestPermissionsFromUserExecuteCallbackWork;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_RequestPermissionsFromUserPromise";
        asyncParamEx.execute = RequestPermissionsFromUserExecutePromiseWork;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
}

/**
 * @brief OnRequestPermissionsFromUserResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_OnRequestPermissionsFromUserResultWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > 0) {
        HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

        napi_value resourceName = 0;
        NAPI_CALL(env,
            napi_create_string_latin1(env, "NAPI_OnRequestPermissionsFromUserResult", NAPI_AUTO_LENGTH, &resourceName));

        aceCallbackInfoPermission.env = env;
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, args[PARAM0], 1, &aceCallbackInfoPermission.callback);

        NAPI_CALL(env,
            napi_create_async_work(env,
                nullptr,
                resourceName,
                [](napi_env env, void *data) {
                    HILOG_INFO("Context::NAPI_OnRequestPermissionsFromUserResult worker pool thread execute.");
                },
                [](napi_env env, napi_status status, void *data) {
                    HILOG_INFO("Conext::NAPI_OnRequestPermissionsFromUserResult main event thread complete.");

                    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
                    if (asyncCallbackInfo != nullptr) {
                        napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                        delete asyncCallbackInfo;
                    }
                },
                (void *)asyncCallbackInfo,
                &asyncCallbackInfo->asyncWork));

        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    }

    return WrapInt32ToJS(env, 1);
}

/**
 * @brief Called back after permissions are requested by using
 * AbilityContext.requestPermissionsFromUser(java.lang.String[],int).
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 */
napi_value NAPI_OnRequestPermissionsFromUserResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    napi_value rev = NAPI_OnRequestPermissionsFromUserResultWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
}

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
    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(callbackInfo.env, &loop);
#endif  // NAPI_VERSION >= 2

    uv_work_t *work = new uv_work_t;
    OnRequestPermissionsFromUserResultCallback *onRequestPermissionCB =
        new (std::nothrow) OnRequestPermissionsFromUserResultCallback{
            .requestCode = requestCode,
            .permissions = permissions,
            .grantResults = grantResults,
            .cb = callbackInfo,
        };
    work->data = (void *)onRequestPermissionCB;

    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            OnRequestPermissionsFromUserResultCallback *event =
                (OnRequestPermissionsFromUserResultCallback *)work->data;
            if (event == nullptr) {
                return;
            }

            napi_value result[ARGS_TWO] = {0};
            result[PARAM0] = GetCallbackErrorValue(event->cb.env, 0);
            napi_create_object(event->cb.env, &result[PARAM1]);

            // create requestCode
            napi_value jsValue = 0;
            napi_create_int32(event->cb.env, event->requestCode, &jsValue);
            napi_set_named_property(event->cb.env, result[PARAM1], "requestCode", jsValue);

            // create permissions
            napi_value perValue = 0;
            napi_value perArray = 0;
            napi_create_array(event->cb.env, &perArray);

            for (size_t i = 0; i < event->permissions.size(); i++) {
                napi_create_string_utf8(event->cb.env, event->permissions[i].c_str(), NAPI_AUTO_LENGTH, &perValue);
                napi_set_element(event->cb.env, perArray, i, perValue);
            }
            napi_set_named_property(event->cb.env, result[PARAM1], "permissions", perArray);

            // create grantResults
            napi_value grantArray;
            napi_create_array(event->cb.env, &grantArray);

            for (size_t i = 0; i < event->grantResults.size(); i++) {
                napi_create_int32(event->cb.env, event->grantResults[i], &perValue);
                napi_set_element(event->cb.env, grantArray, i, perValue);
            }
            napi_set_named_property(event->cb.env, result[PARAM1], "grantResults", grantArray);

            // call CB function
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(event->cb.env, &undefined);

            napi_value callResult = 0;
            napi_get_reference_value(event->cb.env, event->cb.callback, &callback);
            napi_call_function(event->cb.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (event->cb.callback != nullptr) {
                napi_delete_reference(event->cb.env, event->cb.callback);
            }
            delete event;
            delete work;
        });
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

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetBundleName();
    HILOG_INFO("%{public}s called. bundleName=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
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

    napi_value rev = NAPI_GetBundleNameWrap(env, info, asyncCallbackInfo);
    if (rev == nullptr) {
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
}

bool UnwrapParamVerifyPermission(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);

    const size_t argcMax = 4;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_INFO("%{public}s called, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM3], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
            return false;
        }
    }

    std::string permission("");
    if (!UnwrapStringFromJS2(env, argv[PARAM0], permission)) {
        HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
        return false;
    }

    int pid = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[PARAM1], &pid));

    int uid = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[PARAM2], &uid));

    asyncCallbackInfo->param.paramArgs.PutStringValue("permission", permission);
    asyncCallbackInfo->param.paramArgs.PutIntValue("pid", pid);
    asyncCallbackInfo->param.paramArgs.PutIntValue("uid", uid);

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
    int pid = asyncCallbackInfo->param.paramArgs.GetIntValue("pid");
    int uid = asyncCallbackInfo->param.paramArgs.GetIntValue("uid");

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->VerifyPermission(permission, pid, uid);
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
}

void VerifyCallingOrSelfPermissionExecuteCallback(napi_env env, void *data)
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
        delete asyncCallbackInfo;
        asyncCallbackInfo = nullptr;
        rev = WrapVoidToJS(env);
    }
    return rev;
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
        DECLARE_NAPI_FUNCTION("onRequestPermissionsFromUserResult", NAPI_OnRequestPermissionsFromUserResult),
        DECLARE_NAPI_FUNCTION("getBundleName", NAPI_GetBundleName),
        DECLARE_NAPI_FUNCTION("canRequestPermission", NAPI_CanRequestPermission),
        DECLARE_NAPI_FUNCTION("verifyPermission", NAPI_VerifyPermission),
        DECLARE_NAPI_FUNCTION("verifyCallingPermission", NAPI_VerifyCallingPermission),
        DECLARE_NAPI_FUNCTION("verifyCallingOrSelfPermission", NAPI_VerifyCallingOrSelfPermission),
    };

    NAPI_CALL(env,
        napi_define_class(env,
            "context",
            NAPI_AUTO_LENGTH,
            ContextConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &g_classContext));

    return exports;
}
}  // namespace AppExecFwk
}  // namespace OHOS