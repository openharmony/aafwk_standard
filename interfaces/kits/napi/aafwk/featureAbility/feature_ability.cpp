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
#include "feature_ability.h"
#include <cstring>
#include <vector>
#include <uv.h>
#include "securec.h"
#include "ability_process.h"
#include "want_wrapper.h"
#include "hilog_wrapper.h"
#include "napi_context.h"
#include "element_name.h"
#include "napi_data_ability_helper.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
extern napi_value g_classContext;
extern napi_value g_dataAbilityHelper;

CallbackInfo g_aceCallbackInfo;

/**
 * @brief FeatureAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FeatureAbilityInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startAbility", NAPI_StartAbility),
        DECLARE_NAPI_FUNCTION("startAbilityForResult", NAPI_StartAbilityForResult),
        DECLARE_NAPI_FUNCTION("finishWithResult", NAPI_SetResult),
        DECLARE_NAPI_FUNCTION("terminateAbility", NAPI_TerminateAbility),
        DECLARE_NAPI_FUNCTION("hasWindowFocus", NAPI_HasWindowFocus),
        DECLARE_NAPI_FUNCTION("getContext", NAPI_GetContext),
        DECLARE_NAPI_FUNCTION("getWant", NAPI_GetWant),
        DECLARE_NAPI_FUNCTION("getApplicationInfo", NAPI_GetApplicationInfo),
        DECLARE_NAPI_FUNCTION("getAppType", NAPI_GetAppType),
        DECLARE_NAPI_FUNCTION("getElementName", NAPI_GetElementName),
        DECLARE_NAPI_FUNCTION("getAbilityName", NAPI_GetAbilityName),
        DECLARE_NAPI_FUNCTION("getProcessInfo", NAPI_GetProcessInfo),
        DECLARE_NAPI_FUNCTION("getProcessName", NAPI_GetProcessName),
        DECLARE_NAPI_FUNCTION("getCallingBundle", NAPI_GetCallingBundle),
        DECLARE_NAPI_FUNCTION("getAbilityInfo", NAPI_GetAbilityInfo),
        DECLARE_NAPI_FUNCTION("getHapModuleInfo", NAPI_GetHapModuleInfo),
        DECLARE_NAPI_FUNCTION("getDataAbilityHelper", NAPI_GetDataAbilityHelper),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

/**
 * @brief FeatureAbility NAPI method : startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = StartAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief StartAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapParam(param, env, args[0]) == nullptr) {
        HILOG_ERROR("%{public}s, call UnwrapParam failed.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->param = param;
    asyncCallbackInfo->aceCallback = &g_aceCallbackInfo;

    if (argcAsync > argcPromise) {
        ret = StartAbilityAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = StartAbilityPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value StartAbilityAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbility, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_create_int32(env, 0, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value StartAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbility,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_create_int32(env, 0, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief FeatureAbility NAPI method : startAbilityForResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityForResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, env);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = StartAbilityForResultWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief StartAbilityForResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityForResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 3;
    const size_t argcPromise = 2;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapParam(param, env, args[0]) == nullptr) {
        HILOG_ERROR("%{public}s, call UnwrapParam failed.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->param = param;
    asyncCallbackInfo->aceCallback = &g_aceCallbackInfo;

    g_aceCallbackInfo.env = env;
    if (argcAsync > PARAM1) {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_function) {
            // resultCallback: AsyncCallback<StartAbilityResult>
            napi_create_reference(env, args[PARAM1], 1, &g_aceCallbackInfo.callback);
        }
    }

    if (argcAsync > argcPromise) {
        ret = StartAbilityForResultAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = StartAbilityForResultPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value StartAbilityForResultAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    g_aceCallbackInfo.env = env;
    if (argcAsync > PARAM2) {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM2], &valuetype);
        if (valuetype == napi_function) {
            // callback: AsyncCallback<number>
            napi_create_reference(env, args[PARAM2], 1, &asyncCallbackInfo->cbInfo.callback);
        }
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_create_int32(env, 0, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value StartAbilityForResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr) {
                AbilityProcess::GetInstance()->StartAbility(
                    asyncCallbackInfo->ability, asyncCallbackInfo->param, *asyncCallbackInfo->aceCallback);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_StartAbilityForResult,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_create_int32(env, 0, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief FeatureAbility NAPI method : setResult.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_SetResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = SetResultWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief SetResult processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value SetResultWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapAbilityResult(param, env, args[0]) == nullptr) {
        HILOG_ERROR("%{public}s, call unwrapWant failed.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->param = param;

    if (argcAsync > argcPromise) {
        ret = SetResultAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = SetResultPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value SetResultAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_SetResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->SetResult(
                    asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_SetResult, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result[ARGS2] = {0};
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value SetResultPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_SetResult, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->SetResult(
                    asyncCallbackInfo->param.requestCode, asyncCallbackInfo->param.want);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_SetResult,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief FeatureAbility NAPI method : terminateAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_TerminateAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = TerminateAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief TerminateAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value TerminateAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = TerminateAbilityAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = TerminateAbilityPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value TerminateAbilityAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value TerminateAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_TerminateAbility, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_TerminateAbility,  main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return promise;
}

/**
 * @brief Checks whether the main window of this ability has window focus.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_HasWindowFocus(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = HasWindowFocusWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContext(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = GetContextWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
    return ret;
}

/**
 * @brief GetContext processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetContextWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetContextAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = GetContextPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value GetContextAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) { HILOG_INFO("GetContextAsync, worker pool thread execute."); },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_new_instance(env, g_classContext, 0, nullptr, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value GetContextPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) { HILOG_INFO("GetContextPromise, worker pool thread execute."); },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_new_instance(env, g_classContext, 0, nullptr, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return promise;
}

/**
 * @brief HasWindowFocus processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value HasWindowFocusWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = HasWindowFocusAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = HasWindowFocusPromise(env, asyncCallbackInfo);
    }

    return ret;
}

napi_value HasWindowFocusAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->native_result = asyncCallbackInfo->ability->HasWindowFocus();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_boolean(env, asyncCallbackInfo->native_result, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value HasWindowFocusPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->native_result = asyncCallbackInfo->ability->HasWindowFocus();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_HasWindowFocus, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = 0;
            napi_get_boolean(env, asyncCallbackInfo->native_result, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return promise;
}

/**
 * @brief The interface of onAbilityResult provided for ACE to call back to JS.
 *
 * @param requestCode Indicates the request code returned after the ability is started.
 * @param resultCode Indicates the result code returned after the ability is started.
 * @param resultData Indicates the data returned after the ability is started.
 * @param cb The environment and call back info that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
void CallOnAbilityResult(int requestCode, int resultCode, const Want &resultData, CallbackInfo cb)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, cb.env);

    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(cb.env, &loop);
#endif  // NAPI_VERSION >= 2

    uv_work_t *work = new uv_work_t;
    OnAbilityCallback *onAbilityCB = new (std::nothrow) OnAbilityCallback{
        .requestCode = requestCode,
        .resultCode = resultCode,
        .resultData = resultData,
        .cb = cb,
    };
    work->data = (void *)onAbilityCB;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            HILOG_INFO("CallOnAbilityResult, uv_queue_work");
            // JS Thread
            OnAbilityCallback *event = (OnAbilityCallback *)work->data;
            napi_value result[ARGS2] = {0};
            result[PARAM0] = GetCallbackErrorValue(event->cb.env, NO_ERROR);

            napi_create_object(event->cb.env, &result[PARAM1]);
            // create requestCode
            napi_value jsValue = 0;
            napi_create_int32(event->cb.env, event->requestCode, &jsValue);
            napi_set_named_property(event->cb.env, result[PARAM1], "requestCode", jsValue);
            // create resultCode
            napi_create_int32(event->cb.env, event->resultCode, &jsValue);
            napi_set_named_property(event->cb.env, result[PARAM1], "resultCode", jsValue);
            // create want
            napi_value jsWant = WrapWant(event->resultData, event->cb.env);
            napi_set_named_property(event->cb.env, result[PARAM1], "want", jsWant);
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(event->cb.env, &undefined);
            napi_value callResult = 0;
            napi_get_reference_value(event->cb.env, event->cb.callback, &callback);

            napi_call_function(event->cb.env, undefined, callback, ARGS2, &result[PARAM0], &callResult);
            if (event->cb.callback != nullptr) {
                napi_delete_reference(event->cb.env, event->cb.callback);
            }
            delete event;
            delete work;
        });

    return;
}

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapParam(CallAbilityParam &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // unwrap the param
    napi_valuetype valueType = napi_undefined;

    // unwrap the param : want object
    UnwrapWant(param.want, env, args);

    // unwrap the param : requestCode (optional)
    napi_value requestCodeProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "requestCode", &requestCodeProp));
    NAPI_CALL(env, napi_typeof(env, requestCodeProp, &valueType));
    // there is requestCode in param, set forResultOption = true
    if (valueType == napi_number) {
        NAPI_CALL(env, napi_get_value_int32(env, requestCodeProp, &param.requestCode));
        param.forResultOption = true;
    } else {
        param.forResultOption = false;
    }
    HILOG_INFO("%{public}s, reqCode=%{public}d forResultOption=%{public}d.",
        __func__,
        param.requestCode,
        param.forResultOption);

    // unwrap the param : abilityStartSetting (optional)
    napi_value abilityStartSettingProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "abilityStartSetting", &abilityStartSettingProp));
    NAPI_CALL(env, napi_typeof(env, abilityStartSettingProp, &valueType));
    if (valueType == napi_object) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        HILOG_INFO("%{public}s, abilityStartSetting=%{public}p.", __func__, param.setting.get());
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Parse the abilityResult parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapAbilityResult(CallAbilityParam &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // unwrap the param
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "param type mismatch!");
    // get resultCode property
    napi_value property = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, args, "resultCode", &property));
    NAPI_CALL(env, napi_typeof(env, property, &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "property type mismatch!");
    NAPI_CALL(env, napi_get_value_int32(env, property, &param.requestCode));
    HILOG_INFO("%{public}s, requestCode=%{public}d.", __func__, param.requestCode);

    // unwrap the param : want object
    UnwrapWant(param.want, env, args);

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env)
{
    napi_value global = 0;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = 0;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo{
        .cbInfo.env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
        .ability = ability,
        .native_result = false,
    };
    return asyncCallbackInfo;
}

napi_value GetWantAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("GetWantAsync, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            if (asyncCallbackInfo != nullptr && asyncCallbackInfo->ability != nullptr) {
                std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
                if (ptrWant != nullptr) {
                    asyncCallbackInfo->param.want = *ptrWant;
                }
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS2] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            result[PARAM1] = WrapWant(asyncCallbackInfo->param.want, env);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

napi_value GetWantPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("GetWantPromise, worker pool thread execute.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;

            if (asyncCallbackInfo != nullptr && asyncCallbackInfo->ability != nullptr) {
                std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();

                if (ptrWant != nullptr) {
                    asyncCallbackInfo->param.want = *ptrWant;
                }
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = WrapWant(asyncCallbackInfo->param.want, env);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return promise;
}

/**
 * @brief GetWantWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetWantWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetWantAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
    } else {
        ret = GetWantPromise(env, asyncCallbackInfo);
    }

    return ret;
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWant(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }

    napi_value ret = GetWantWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
    }
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
        return nullptr;
    }

    napi_value ret = GetApplicationInfoWrap(env, info, appInfoCB);
    if (ret == nullptr) {
        if (appInfoCB != nullptr) {
            delete appInfoCB;
            appInfoCB = nullptr;
        }
    }
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
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppInfoCB *appInfoCB = new (std::nothrow) AppInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return appInfoCB;
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
        ret = GetApplicationInfoAsync(env, args, argcAsync, argcPromise, appInfoCB);
    } else {
        ret = GetApplicationInfoPromise(env, appInfoCB);
    }

    return ret;
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
napi_value GetApplicationInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &appInfoCB->cbBase.cbInfo.callback));
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
    return result;
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

    return promise;
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
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    if (appInfoCB->cbBase.ability != nullptr) {
        std::shared_ptr<ApplicationInfo> appInfoPtr = appInfoCB->cbBase.ability->GetApplicationInfo();
        if (appInfoPtr != nullptr) {
            SaveAppInfo(appInfoCB->appInfo, *appInfoPtr);
        }
    }
}

void SaveAppInfo(AppInfo_ &appInfo, const ApplicationInfo &appInfoOrg)
{
    HILOG_INFO("%{public}s.", __func__);
    appInfo.name = appInfoOrg.name;
    appInfo.description = appInfoOrg.description;
    appInfo.descriptionId = appInfoOrg.descriptionId;
    appInfo.systemApp = appInfoOrg.isSystemApp;
    appInfo.enabled = appInfoOrg.enabled;
    appInfo.label = appInfoOrg.label;
    appInfo.labelId = std::to_string(appInfoOrg.labelId);
    appInfo.icon = appInfoOrg.iconPath;
    appInfo.iconId = std::to_string(appInfoOrg.iconId);
    appInfo.process = appInfoOrg.process;
    appInfo.supportedModes = appInfoOrg.supportedModes;
    for (size_t i = 0; i < appInfoOrg.moduleInfos.size(); i++) {
        appInfo.moduleSourceDirs.emplace_back(appInfoOrg.moduleInfos.at(i).moduleSourceDir);
    }
    for (size_t i = 0; i < appInfoOrg.permissions.size(); i++) {
        appInfo.permissions.emplace_back(appInfoOrg.permissions.at(i));
    }
    for (size_t i = 0; i < appInfoOrg.moduleInfos.size(); i++) {
        appInfo.moduleInfos.emplace_back(appInfoOrg.moduleInfos.at(i));
    }
    appInfo.flags = appInfoOrg.flags;
    appInfo.entryDir = appInfoOrg.entryDir;
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
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapAppInfo(env, appInfoCB->appInfo);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (appInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appInfoCB->cbBase.asyncWork));
    delete appInfoCB;
    appInfoCB = nullptr;
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
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    napi_value result = WrapAppInfo(env, appInfoCB->appInfo);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, appInfoCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appInfoCB->cbBase.asyncWork));
    delete appInfoCB;
    appInfoCB = nullptr;
}

napi_value WrapAppInfo(napi_env env, const AppInfo_ &appInfo)
{
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_int32(env, appInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_get_boolean(env, appInfo.systemApp, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "systemApp", proValue));
    NAPI_CALL(env, napi_get_boolean(env, appInfo.enabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "enabled", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.labelId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.iconId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));
    NAPI_CALL(env, napi_create_int32(env, appInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));
    napi_value jsArrayModDirs = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayModDirs));
    for (size_t i = 0; i < appInfo.moduleSourceDirs.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, appInfo.moduleSourceDirs.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayModDirs, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleSourceDirs", jsArrayModDirs));
    napi_value jsArrayPermissions = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayPermissions));
    for (size_t i = 0; i < appInfo.permissions.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, appInfo.permissions.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayPermissions, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "permissions", jsArrayPermissions));
    napi_value jsArrayModuleInfo = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayModuleInfo));
    for (size_t i = 0; i < appInfo.moduleInfos.size(); i++) {
        napi_value jsModuleInfoObject = nullptr;
        NAPI_CALL(env, napi_create_object(env, &jsModuleInfoObject));
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, appInfo.moduleInfos.at(i).moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleName", proValue));
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, appInfo.moduleInfos.at(i).moduleSourceDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleSourceDir", proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayModuleInfo, i, jsModuleInfoObject));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleInfos", jsArrayModuleInfo));
    NAPI_CALL(env, napi_create_int32(env, appInfo.flags, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "flags", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.entryDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "entryDir", proValue));
    return result;
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AppTypeCB *appTypeCB = CreateAppTypeCBInfo(env);
    if (appTypeCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetAppTypeWrap(env, info, appTypeCB);
    if (ret == nullptr) {
        if (appTypeCB != nullptr) {
            delete appTypeCB;
            appTypeCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppTypeCB on success, nullptr on failure.
 */
AppTypeCB *CreateAppTypeCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppTypeCB *appTypeCB = new (std::nothrow) AppTypeCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return appTypeCB;
}

/**
 * @brief GetAppType processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeWrap(napi_env env, napi_callback_info info, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
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
        ret = GetAppTypeAsync(env, args, argcAsync, argcPromise, appTypeCB);
    } else {
        ret = GetAppTypePromise(env, appTypeCB);
    }

    return ret;
}

/**
 * @brief GetAppType Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypeAsyncCompleteCB,
            (void *)appTypeCB,
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

/**
 * @brief GetAppType Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypePromise(napi_env env, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appTypeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypePromiseCompleteCB,
            (void *)appTypeCB,
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));

    return promise;
}

/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
    if (appTypeCB->cbBase.ability != nullptr) {
        appTypeCB->name = appTypeCB->cbBase.ability->GetAppType();
    }
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);

    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(
            env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appTypeCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (appTypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appTypeCB->cbBase.asyncWork));
    delete appTypeCB;
    appTypeCB = nullptr;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("GetAppTypePromiseCompleteCB, main event thread complete.");
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, appTypeCB->cbBase.deferred, result));

    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appTypeCB->cbBase.asyncWork));
    delete appTypeCB;
    appTypeCB = nullptr;
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
    HILOG_INFO("%{public}s,called", __func__);
    ElementNameCB *elementNameCB = CreateElementNameCBInfo(env);
    if (elementNameCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetElementNameWrap(env, info, elementNameCB);
    if (ret == nullptr) {
        if (elementNameCB != nullptr) {
            delete elementNameCB;
            elementNameCB = nullptr;
        }
    }
    return ret;
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
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ElementNameCB *elementNameCB = new (std::nothrow) ElementNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return elementNameCB;
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
        ret = GetElementNameAsync(env, args, argcAsync, argcPromise, elementNameCB);
    } else {
        ret = GetElementNamePromise(env, elementNameCB);
    }

    return ret;
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
napi_value GetElementNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &elementNameCB->cbBase.cbInfo.callback));
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

    return promise;
}

/**
 * @brief GetElementName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    if (elementNameCB->cbBase.ability != nullptr) {
        std::shared_ptr<ElementName> elementName = elementNameCB->cbBase.ability->GetElementName();
        if (elementNameCB != nullptr) {
            elementNameCB->deviceId = elementName->GetDeviceID();
            elementNameCB->bundleName = elementName->GetBundleName();
            elementNameCB->abilityName = elementName->GetAbilityName();
            elementNameCB->uri = elementNameCB->cbBase.ability->GetWant()->GetUriString();
            elementNameCB->shortName = "";
        }
    }
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapElementName(env, elementNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, elementNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (elementNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, elementNameCB->cbBase.asyncWork));
    delete elementNameCB;
    elementNameCB = nullptr;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    napi_value result = WrapElementName(env, elementNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, elementNameCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, elementNameCB->cbBase.asyncWork));
    delete elementNameCB;
    elementNameCB = nullptr;
}

napi_value WrapElementName(napi_env env, ElementNameCB *elementNameCB)
{
    if (elementNameCB == nullptr) {
        HILOG_ERROR("Invalid param(appInfoCB = nullptr)");
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

    return result;
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityName(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AbilityNameCB *ablityNameCB = CreateAbilityNameCBInfo(env);
    if (ablityNameCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetAbilityNameWrap(env, info, ablityNameCB);
    if (ret == nullptr) {
        if (ablityNameCB != nullptr) {
            delete ablityNameCB;
            ablityNameCB = nullptr;
        }
    }
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
    HILOG_INFO("%{public}s,called", __func__);
    ProcessInfoCB *processInfoCB = CreateProcessInfoCBInfo(env);
    if (processInfoCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetProcessInfoWrap(env, info, processInfoCB);
    if (ret == nullptr) {
        if (processInfoCB != nullptr) {
            delete processInfoCB;
            processInfoCB = nullptr;
        }
    }
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
    HILOG_INFO("%{public}s,called", __func__);

    ProcessNameCB *processNameCB = CreateProcessNameCBInfo(env);
    if (processNameCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetProcessNameWrap(env, info, processNameCB);
    if (ret == nullptr) {
        if (processNameCB != nullptr) {
            delete processNameCB;
            processNameCB = nullptr;
        }
    }
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
    HILOG_INFO("%{public}s,called", __func__);
    CallingBundleCB *calloingBundleCB = CreateCallingBundleCBInfo(env);
    if (calloingBundleCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetCallingBundleWrap(env, info, calloingBundleCB);
    if (ret == nullptr) {
        if (calloingBundleCB != nullptr) {
            delete calloingBundleCB;
            calloingBundleCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
AbilityNameCB *CreateAbilityNameCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AbilityNameCB *abilityNameCB = new (std::nothrow) AbilityNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    return abilityNameCB;
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
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessInfoCB *processInfoCB = new (std::nothrow) ProcessInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return processInfoCB;
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
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessNameCB *processNameCB = new (std::nothrow) ProcessNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return processNameCB;
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
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    CallingBundleCB *callingBundleCB = new (std::nothrow) CallingBundleCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return callingBundleCB;
}

/**
 * @brief GetAbilityName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameWrap(napi_env env, napi_callback_info info, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
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
        ret = GetAbilityNameAsync(env, args, argcAsync, argcPromise, abilityNameCB);
    } else {
        ret = GetAbilityNamePromise(env, abilityNameCB);
    }

    return ret;
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
        ret = GetProcessInfoAsync(env, args, argcAsync, argcPromise, processInfoCB);
    } else {
        ret = GetProcessInfoPromise(env, processInfoCB);
    }

    return ret;
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
        ret = GetProcessNameAsync(env, args, argcAsync, argcPromise, processNameCB);
    } else {
        ret = GetProcessNamePromise(env, processNameCB);
    }

    return ret;
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
        ret = GetCallingBundleAsync(env, args, argcAsync, argcPromise, callingBundleCB);
    } else {
        ret = GetCallingBundlePromise(env, callingBundleCB);
    }

    return ret;
}

/**
 * @brief GetAbilityName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNameAsyncCompleteCB,
            (void *)abilityNameCB,
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
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
napi_value GetProcessInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &processInfoCB->cbBase.cbInfo.callback));
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
    return result;
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
napi_value GetProcessNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &processNameCB->cbBase.cbInfo.callback));
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
    return result;
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
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &callingBundleCB->cbBase.cbInfo.callback));
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
    return result;
}

/**
 * @brief GetAbilityName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNamePromise(napi_env env, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNamePromiseCompleteCB,
            (void *)abilityNameCB,
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));

    return promise;
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

    return promise;
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

    return promise;
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

    return promise;
}

/**
 * @brief GetAbilityName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetAbilityName, worker pool thread execute.");
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
    if (abilityNameCB->cbBase.ability != nullptr) {
        abilityNameCB->name = abilityNameCB->cbBase.ability->GetAbilityName();
    }
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
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    if (processInfoCB->cbBase.ability != nullptr) {
        std::shared_ptr<ProcessInfo> processInfoPtr = processInfoCB->cbBase.ability->GetProcessInfo();
        if (processInfoPtr != nullptr) {
            processInfoCB->processName = processInfoPtr->GetProcessName();
            processInfoCB->pid = processInfoPtr->GetPid();
        }
    }
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
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    if (processNameCB->cbBase.ability != nullptr) {
        processNameCB->processName = processNameCB->cbBase.ability->GetProcessName();
    }
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
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    if (callingBundleCB->cbBase.ability != nullptr) {
        callingBundleCB->callingBundleName = callingBundleCB->cbBase.ability->GetCallingBundle();
    }
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete.");
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapAbilityName(env, abilityNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));
    if (abilityNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork));
    delete abilityNameCB;
    abilityNameCB = nullptr;
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
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapProcessInfo(env, processInfoCB);

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (processInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processInfoCB->cbBase.asyncWork));
    delete processInfoCB;
    processInfoCB = nullptr;
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
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapProcessName(env, processNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (processNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processNameCB->cbBase.asyncWork));
    delete processNameCB;
    processNameCB = nullptr;
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
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapCallingBundle(env, callingBundleCB);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callingBundleCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (callingBundleCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork));
    delete callingBundleCB;
    callingBundleCB = nullptr;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete.");
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
    napi_value result = WrapAbilityName(env, abilityNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, abilityNameCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork));
    delete abilityNameCB;
    abilityNameCB = nullptr;
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
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    napi_value result = WrapProcessInfo(env, processInfoCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, processInfoCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processInfoCB->cbBase.asyncWork));
    delete processInfoCB;
    processInfoCB = nullptr;
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
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    napi_value result = WrapProcessName(env, processNameCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, processNameCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processNameCB->cbBase.asyncWork));
    delete processNameCB;
    processNameCB = nullptr;
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
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    napi_value result = WrapCallingBundle(env, callingBundleCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, callingBundleCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork));
    delete callingBundleCB;
    callingBundleCB = nullptr;
}

napi_value WrapAbilityName(napi_env env, AbilityNameCB *abilityNameCB)
{
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("Invalid param(abilityNameCB == nullptr)");
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, abilityNameCB->name.c_str(), NAPI_AUTO_LENGTH, &result));

    return result;
}

napi_value WrapProcessInfo(napi_env env, ProcessInfoCB *processInfoCB)
{
    if (processInfoCB == nullptr) {
        HILOG_ERROR("Invalid param(processInfoCB == nullptr)");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, processInfoCB->pid, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "pid", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, processInfoCB->processName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "processName", proValue));

    return result;
}

napi_value WrapProcessName(napi_env env, ProcessNameCB *processNameCB)
{
    if (processNameCB == nullptr) {
        HILOG_ERROR("Invalid param(processNameCB == nullptr)");
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, processNameCB->processName.c_str(), NAPI_AUTO_LENGTH, &result));

    return result;
}

napi_value WrapCallingBundle(napi_env env, CallingBundleCB *callingBundleCB)
{
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("Invalid param(callingBundleCB == nullptr)");
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, callingBundleCB->callingBundleName.c_str(), NAPI_AUTO_LENGTH, &result));

    return result;
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AbilityInfoCB *abilityInfoCB = CreateAbilityInfoCBInfo(env);
    if (abilityInfoCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetAbilityInfoWrap(env, info, abilityInfoCB);
    if (ret == nullptr) {
        if (abilityInfoCB != nullptr) {
            delete abilityInfoCB;
            abilityInfoCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief GetAbilityInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoWrap(napi_env env, napi_callback_info info, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
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
        ret = GetAbilityInfoAsync(env, args, argcAsync, argcPromise, abilityInfoCB);
    } else {
        ret = GetAbilityInfoPromise(env, abilityInfoCB);
    }

    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityInfoCB on success, nullptr on failure.
 */
AbilityInfoCB *CreateAbilityInfoCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AbilityInfoCB *abilityInfoCB = new (std::nothrow) AbilityInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return abilityInfoCB;
}

/**
 * @brief GetAbilityInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoAsyncCompleteCB,
            (void *)abilityInfoCB,
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

/**
 * @brief GetAbilityInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoPromise(napi_env env, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoPromiseCompleteCB,
            (void *)abilityInfoCB,
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));

    return promise;
}

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
    if (abilityInfoCB->cbBase.ability != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfoPtr = abilityInfoCB->cbBase.ability->GetAbilityInfo();
        if (abilityInfoPtr != nullptr) {
            SaveAbilityInfo(abilityInfoCB->abilityInfo, *abilityInfoPtr);
        }
    }
}

void SaveAbilityInfo(AbilityInfo_ &abilityInfo, const AbilityInfo &abilityInfoOrg)
{
    abilityInfo.bundleName = abilityInfoOrg.bundleName;
    abilityInfo.name = abilityInfoOrg.name;
    abilityInfo.label = abilityInfoOrg.label;
    abilityInfo.description = abilityInfoOrg.description;
    abilityInfo.icon = abilityInfoOrg.iconPath;
    abilityInfo.labelId = abilityInfoOrg.applicationInfo.labelId;
    abilityInfo.descriptionId = abilityInfoOrg.applicationInfo.descriptionId;
    abilityInfo.iconId = abilityInfoOrg.applicationInfo.iconId;
    abilityInfo.moduleName = abilityInfoOrg.moduleName;
    abilityInfo.process = abilityInfoOrg.process;
    abilityInfo.isVisible = abilityInfoOrg.visible;

    abilityInfo.type = static_cast<int32_t>(abilityInfoOrg.type);
    abilityInfo.orientation = static_cast<int32_t>(abilityInfoOrg.orientation);
    abilityInfo.launchMode = static_cast<int32_t>(abilityInfoOrg.launchMode);

    abilityInfo.uri = abilityInfoOrg.uri;
    abilityInfo.targetAbility = abilityInfoOrg.targetAbility;

    for (size_t i = 0; i < abilityInfoOrg.permissions.size(); i++) {
        abilityInfo.permissions.emplace_back(abilityInfoOrg.permissions.at(i));
    }
    for (size_t i = 0; i < abilityInfoOrg.deviceTypes.size(); i++) {
        abilityInfo.deviceTypes.emplace_back(abilityInfoOrg.deviceTypes.at(i));
    }
    for (size_t i = 0; i < abilityInfoOrg.deviceCapabilities.size(); i++) {
        abilityInfo.deviceCapabilities.emplace_back(abilityInfoOrg.deviceCapabilities.at(i));
    }

    SaveAppInfo(abilityInfo.appInfo, abilityInfoOrg.applicationInfo);

    abilityInfo.readPermission = abilityInfoOrg.readPermission;
    abilityInfo.writePermission = abilityInfoOrg.writePermission;
    abilityInfo.formEntity = 0;         // no data
    abilityInfo.minFormHeight = 0;      // no data
    abilityInfo.defaultFormHeight = 0;  // no data
    abilityInfo.minFormWidth = 0;       // no data
    abilityInfo.defaultFormWidth = 0;   // no data
    abilityInfo.backgroundModes = 0;    // no data
    abilityInfo.subType = 0;            // no data
    abilityInfo.formEnabled = false;    // no data
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (abilityInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork));
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
    napi_value result = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, abilityInfoCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork));
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
}

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo_ &abilityInfo)
{
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.type), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "type", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "orientation", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "launchMode", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.readPermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "readPermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.writePermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "writePermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.targetAbility.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "targetAbility", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.formEntity, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEntity", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.backgroundModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.subType, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "subType", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.isVisible, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isVisible", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.formEnabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEnabled", proValue));

    napi_value jsArrayPermissions = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayPermissions));
    for (size_t i = 0; i < abilityInfo.permissions.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, abilityInfo.permissions.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayPermissions, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "permissions", jsArrayPermissions));

    napi_value jsArrayDeviceCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayDeviceCapabilities));
    for (size_t i = 0; i < abilityInfo.deviceCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, abilityInfo.deviceCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayDeviceCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceCapabilities", jsArrayDeviceCapabilities));

    napi_value jsArrayDeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayDeviceTypes));
    for (size_t i = 0; i < abilityInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, abilityInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayDeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArrayDeviceTypes));

    napi_value appInfo = nullptr;
    appInfo = WrapAppInfo(env, abilityInfo.appInfo);
    NAPI_CALL(env, napi_set_named_property(env, result, "applicationInfo", appInfo));

    return result;
}

napi_value NAPI_GetHapModuleInfo(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    HapModuleInfoCB *hapModuleInfoCB = CreateHapModuleInfoCBInfo(env);
    if (hapModuleInfoCB == nullptr) {
        return nullptr;
    }

    napi_value ret = GetHapModuleInfoWrap(env, info, hapModuleInfoCB);
    if (ret == nullptr) {
        if (hapModuleInfoCB != nullptr) {
            delete hapModuleInfoCB;
            hapModuleInfoCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to HapModuleInfoCB on success, nullptr on failure.
 */
HapModuleInfoCB *CreateHapModuleInfoCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    HapModuleInfoCB *hapModuleInfoCB = new (std::nothrow) HapModuleInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    return hapModuleInfoCB;
}

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
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
        ret = GetHapModuleInfoAsync(env, args, argcAsync, argcPromise, hapModuleInfoCB);
    } else {
        ret = GetHapModuleInfoPromise(env, hapModuleInfoCB);
    }

    return ret;
}

/**
 * @brief GetHapModuleInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoAsyncCompleteCB,
            (void *)hapModuleInfoCB,
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

/**
 * @brief GetHapModuleInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoPromise(napi_env env, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    hapModuleInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoPromiseCompleteCB,
            (void *)hapModuleInfoCB,
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));

    return promise;
}

void GetHapModuleInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, worker pool thread execute.");
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
    if (hapModuleInfoCB->cbBase.ability != nullptr) {
        std::shared_ptr<HapModuleInfo> hapModuleInfoPtr = hapModuleInfoCB->cbBase.ability->GetHapModuleInfo();
        if (hapModuleInfoPtr != nullptr) {
            SaveHapModuleInfo(hapModuleInfoCB->hapModuleInfo, *hapModuleInfoPtr);
        }
    }
}

void SaveHapModuleInfo(HapModuleInfo_ &hapModuleInfo, const HapModuleInfo &hapModuleInfoOrg)
{

    hapModuleInfo.name = hapModuleInfoOrg.name;
    hapModuleInfo.description = hapModuleInfoOrg.description;
    hapModuleInfo.icon = hapModuleInfoOrg.iconPath;
    hapModuleInfo.label = hapModuleInfoOrg.label;
    hapModuleInfo.backgroundImg = hapModuleInfoOrg.backgroundImg;
    hapModuleInfo.moduleName = hapModuleInfoOrg.moduleName;
    hapModuleInfo.supportedModes = hapModuleInfoOrg.supportedModes;
    hapModuleInfo.descriptionId = 0;         // no data
    hapModuleInfo.labelId = 0;               // no data
    hapModuleInfo.iconId = 0;                // no data
    hapModuleInfo.mainAbilityName = "";      // no data
    hapModuleInfo.installationFree = false;  // no data

    for (size_t i = 0; i < hapModuleInfoOrg.reqCapabilities.size(); i++) {
        hapModuleInfo.reqCapabilities.emplace_back(hapModuleInfoOrg.reqCapabilities.at(i));
    }

    for (size_t i = 0; i < hapModuleInfoOrg.deviceTypes.size(); i++) {
        hapModuleInfo.deviceTypes.emplace_back(hapModuleInfoOrg.deviceTypes.at(i));
    }

    for (size_t i = 0; i < hapModuleInfoOrg.abilityInfos.size(); i++) {
        AbilityInfo_ abilityInfo;
        SaveAbilityInfo(abilityInfo, hapModuleInfoOrg.abilityInfos.at(i));
        hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
    }
}

void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapHapModuleInfo(env, *hapModuleInfoCB);
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, hapModuleInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (hapModuleInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
}

void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
    napi_value result = WrapHapModuleInfo(env, *hapModuleInfoCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, hapModuleInfoCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
}

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &hapModuleInfoCB)
{
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.backgroundImg.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundImg", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(
            env, hapModuleInfoCB.hapModuleInfo.mainAbilityName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "mainAbilityName", proValue));

    NAPI_CALL(env, napi_get_boolean(env, hapModuleInfoCB.hapModuleInfo.installationFree, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "installationFree", proValue));

    napi_value jsArrayreqCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayreqCapabilities));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.reqCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.reqCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayreqCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "reqCapabilities", jsArrayreqCapabilities));

    napi_value jsArraydeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArraydeviceTypes));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArraydeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArraydeviceTypes));

    napi_value abilityInfos = nullptr;
    NAPI_CALL(env, napi_create_array(env, &abilityInfos));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.abilityInfos.size(); i++) {
        napi_value abilityInfo = nullptr;
        abilityInfo = WrapAbilityInfo(env, hapModuleInfoCB.hapModuleInfo.abilityInfos.at(i));
        NAPI_CALL(env, napi_set_element(env, abilityInfos, i, abilityInfo));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityInfos", abilityInfos));

    return result;
}

/**
 * @brief FeatureAbility NAPI method : getDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetDataAbilityHelper(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DataAbilityHelperCB *dataAbilityHelperCB = new (std::nothrow) DataAbilityHelperCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = nullptr,
        .result = nullptr,
    };

    napi_value ret = GetDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief getDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        NAPI_CALL(env, napi_create_reference(env, args[PARAM0], 1, &dataAbilityHelperCB->uri));
    }

    if (argcAsync > argcPromise) {
        ret = GetDataAbilityHelperAsync(env, args, argcAsync, argcPromise, dataAbilityHelperCB);
    } else {
        ret = GetDataAbilityHelperPromise(env, dataAbilityHelperCB);
    }

    return ret;
}

napi_value GetDataAbilityHelperAsync(napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise,
    DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &dataAbilityHelperCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_GetDataAbilityHelper, worker pool thread execute."); },
            GetDataAbilityHelperAsyncCompleteCB,
            (void *)dataAbilityHelperCB,
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value GetDataAbilityHelperPromise(napi_env env, DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    dataAbilityHelperCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) { HILOG_INFO("NAPI_GetDataAbilityHelper, worker pool thread execute."); },
            GetDataAbilityHelperPromiseCompleteCB,
            (void *)dataAbilityHelperCB,
            &dataAbilityHelperCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    return promise;
}

void GetDataAbilityHelperAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetDataAbilityHelper, main event thread complete.");
    DataAbilityHelperCB *dataAbilityHelperCB = (DataAbilityHelperCB *)data;
    napi_value uri = nullptr;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS2] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_new_instance(env, g_dataAbilityHelper, 1, &uri, &dataAbilityHelperCB->result));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = dataAbilityHelperCB->result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS2, &result[PARAM0], &callResult));

    if (dataAbilityHelperCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, dataAbilityHelperCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
}

void GetDataAbilityHelperPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetDataAbilityHelper,  main event thread complete.");
    DataAbilityHelperCB *dataAbilityHelperCB = (DataAbilityHelperCB *)data;
    napi_value uri = nullptr;
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, dataAbilityHelperCB->uri, &uri));
    NAPI_CALL_RETURN_VOID(env, napi_new_instance(env, g_dataAbilityHelper, 1, &uri, &dataAbilityHelperCB->result));
    result = dataAbilityHelperCB->result;

    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, dataAbilityHelperCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, dataAbilityHelperCB->cbBase.asyncWork));
    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
}

}  // namespace AppExecFwk
}  // namespace OHOS