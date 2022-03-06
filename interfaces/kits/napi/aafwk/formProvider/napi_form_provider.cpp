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

#include "napi_form_provider.h"

#include <cinttypes>
#include <regex>
#include <uv.h>
#include <vector>

#include "hilog_wrapper.h"
#include "napi_form_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "runtime.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
    constexpr size_t ARGS_SIZE_ONE = 1;
    constexpr size_t ARGS_SIZE_TWO = 2;
    constexpr size_t ARGS_SIZE_THREE = 3;
    constexpr int REF_COUNT = 1;
    OHOS::AppExecFwk::Ability* g_ability = nullptr;
}

/**
 * @brief GetGlobalAbility
 *
 * @param[in] env The environment that the Node-API call is invoked under
 *
 * @return OHOS::AppExecFwk::Ability*
 */
static OHOS::AppExecFwk::Ability* GetGlobalAbility(napi_env env)
{
    // get global value
    napi_value global = nullptr;
    napi_get_global(env, &global);

    // get ability
    napi_value abilityObj = nullptr;
    napi_get_named_property(env, global, "ability", &abilityObj);

    // get ability pointer
    OHOS::AppExecFwk::Ability* ability = nullptr;
    napi_get_value_external(env, abilityObj, (void**)&ability);
    HILOG_INFO("%{public}s, ability = [%{public}p]", __func__, ability);
    if (ability == nullptr) {
        if (g_ability == nullptr) {
            std::unique_ptr<AbilityRuntime::Runtime> runtime;
            g_ability = OHOS::AppExecFwk::Ability::Create(runtime);
        }
        ability = g_ability;
        HILOG_INFO("%{public}s, Use Local tmp Ability for Stage Module", __func__);
    }
    return ability;
}

/**
 * @brief Get a C++ string value from Node-API
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] value This is an opaque pointer that is used to represent a JavaScript value
 *
 * @return Return a C++ string
 */
static std::string GetStringFromNAPI(napi_env env, napi_value value)
{
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, value, nullptr, 0, &size) != napi_ok) {
        HILOG_ERROR("%{public}s, can not get string size", __func__);
        return "";
    }
    result.reserve(size + 1);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + 1), &size) != napi_ok) {
        HILOG_ERROR("%{public}s, can not get string value", __func__);
        return "";
    }
    return result;
}

/**
 * @brief  Call native kit function: SetFormNextRefreshTime
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[out] asyncCallbackInfo Reference, callback info via Node-API
 *
 * @return void
 */
static void InnerSetFormNextRefreshTime(napi_env env, AsyncNextRefreshTimeFormCallbackInfo* const asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    OHOS::AppExecFwk::Ability *ability = asyncCallbackInfo->ability;
    bool ret = ability->SetFormNextRefreshTime(asyncCallbackInfo->formId, asyncCallbackInfo->time);
    if (ret) {
        asyncCallbackInfo->result = 1;
    } else {
        asyncCallbackInfo->result = 0;
    }
    HILOG_DEBUG("%{public}s, end", __func__);
}

/**
 * @brief  The implementation of Node-API interface: setFormNextRefreshTime
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[out] info An opaque datatype that is passed to a callback function
 *
 * @return This is an opaque pointer that is used to represent a JavaScript value
 */
napi_value NAPI_SetFormNextRefreshTime(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    // Check the number of the arguments
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc > ARGS_SIZE_THREE || argc < ARGS_SIZE_TWO) {
        HILOG_ERROR("%{public}s, wrong number of arguments.", __func__);
        return nullptr;
    }
    HILOG_INFO("%{public}s, argc = [%{public}zu]", __func__, argc);

    // Check the value type of the arguments
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "The arguments[0] type of setFormNextRefreshTime is incorrect,\
    expected type is string.");

    std::string strFormId = GetStringFromNAPI(env, argv[0]);
    int64_t formId;
    bool isConversionSucceeded = ConvertStringToInt64(strFormId, formId);
    NAPI_ASSERT(env, isConversionSucceeded, "The arguments[0] type of setFormNextRefreshTime is incorrect,\
    expected type is string and the content must be numeric,\
    value range is: 0x8000000000000000~0x7FFFFFFFFFFFFFFF.");

    valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "The arguments[1] type of setFormNextRefreshTime is incorrect,\
    expected type is number.");

    int32_t time;
    napi_get_value_int32(env, argv[1], &time);

    AsyncNextRefreshTimeFormCallbackInfo *asyncCallbackInfo = new
    AsyncNextRefreshTimeFormCallbackInfo {
        .env = env,
        .ability = GetGlobalAbility(env),
        .asyncWork = nullptr,
        .deferred = nullptr,
        .callback = nullptr,
        .formId = formId,
        .time = time,
        .result = 0,
    };

    if (argc == ARGS_SIZE_THREE) {
        HILOG_INFO("%{public}s, asyncCallback.", __func__);

        // Check the value type of the arguments
        valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARGS_SIZE_TWO], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "The arguments[2] type of setFormNextRefreshTime is incorrect,\
        expected type is function.");

        napi_create_reference(env, argv[1], REF_COUNT, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                HILOG_INFO("%{public}s, napi_create_async_work running", __func__);

                AsyncNextRefreshTimeFormCallbackInfo *asyncCallbackInfo =
                (AsyncNextRefreshTimeFormCallbackInfo *)data;

                InnerSetFormNextRefreshTime(env, asyncCallbackInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncNextRefreshTimeFormCallbackInfo *asyncCallbackInfo =
                (AsyncNextRefreshTimeFormCallbackInfo *)data;

                HILOG_INFO("%{public}s, napi_create_async_work complete", __func__);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_value result;
                    napi_create_int32(env, asyncCallbackInfo->result, &result);
                    napi_value callback;
                    napi_value undefined;
                    napi_get_undefined(env, &undefined);
                    napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                    napi_value callResult;
                    napi_call_function(env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        return NapiGetResut(env, 1);
    } else {
        HILOG_INFO("%{public}s, promise.", __func__);
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                HILOG_INFO("%{public}s, promise running", __func__);
                AsyncNextRefreshTimeFormCallbackInfo *asyncCallbackInfo =
                (AsyncNextRefreshTimeFormCallbackInfo *)data;

                InnerSetFormNextRefreshTime(env, asyncCallbackInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("%{public}s, promise complete", __func__);
                AsyncNextRefreshTimeFormCallbackInfo *asyncCallbackInfo =
                (AsyncNextRefreshTimeFormCallbackInfo *)data;

                napi_value result;
                napi_create_int32(env, asyncCallbackInfo->result, &result);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
        return promise;
    }
}

/**
 * @brief  Call native kit function: UpdateForm
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[out] asyncCallbackInfo Reference, callback info via Node-API
 *
 * @return void
 */
static void InnerUpdateForm(napi_env env, AsyncUpdateFormCallbackInfo* const asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    OHOS::AppExecFwk::Ability *ability = asyncCallbackInfo->ability;
    bool ret = ability->UpdateForm(asyncCallbackInfo->formId, *asyncCallbackInfo->formProviderData);
    if (ret) {
        asyncCallbackInfo->result = 1;
    } else {
        asyncCallbackInfo->result = 0;
    }
    HILOG_DEBUG("%{public}s, end", __func__);
}

/**
 * @brief  The implementation of Node-API interface: updateForm
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[out] info An opaque datatype that is passed to a callback function
 *
 * @return This is an opaque pointer that is used to represent a JavaScript value
 */
napi_value NAPI_UpdateForm(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);

    // Check the number of the arguments
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc > ARGS_SIZE_THREE || argc < ARGS_SIZE_TWO) {
        HILOG_ERROR("%{public}s, wrong number of arguments.", __func__);
        return nullptr;
    }
    HILOG_INFO("%{public}s, argc = [%{public}zu]", __func__, argc);

    // Check the value type of the arguments
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "The arguments[0] type of updateForm is incorrect,\
    expected type is string.");

    std::string strFormId = GetStringFromNAPI(env, argv[0]);
    int64_t formId;
    bool isConversionSucceeded = ConvertStringToInt64(strFormId, formId);
    NAPI_ASSERT(env, isConversionSucceeded, "The arguments[0] type of updateForm is incorrect,\
    expected type is string and the content must be numeric,\
    value range is: 0x8000000000000000~0x7FFFFFFFFFFFFFFF.");

    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "The arguments[1] type of updateForm is incorrect,\
    expected type is string.");

    OHOS::AppExecFwk::FormProviderData *formProviderData = nullptr;
    napi_unwrap(env, argv[1], (void**)&formProviderData);

    AsyncUpdateFormCallbackInfo *asyncCallbackInfo = new
    AsyncUpdateFormCallbackInfo {
        .env = env,
        .ability = GetGlobalAbility(env),
        .asyncWork = nullptr,
        .deferred = nullptr,
        .callback = nullptr,
        .formId = formId,
        .formProviderData = std::shared_ptr<OHOS::AppExecFwk::FormProviderData>(formProviderData),
        .result = 0,
    };

    if (argc == ARGS_SIZE_THREE) {
        HILOG_INFO("%{public}s, asyncCallback.", __func__);

        // Check the value type of the arguments
        valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARGS_SIZE_TWO], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "The arguments[2] type of updateForm is incorrect,\
        expected type is function.");

        napi_create_reference(env, argv[1], REF_COUNT, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                HILOG_INFO("%{public}s, napi_create_async_work running", __func__);
                AsyncUpdateFormCallbackInfo *asyncCallbackInfo = (AsyncUpdateFormCallbackInfo *)data;
                InnerUpdateForm(env, asyncCallbackInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                AsyncUpdateFormCallbackInfo *asyncCallbackInfo = (AsyncUpdateFormCallbackInfo *)data;
                HILOG_INFO("%{public}s, napi_create_async_work complete", __func__);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_value result;
                    napi_create_int32(env, asyncCallbackInfo->result, &result);
                    napi_value callback;
                    napi_value undefined;
                    napi_get_undefined(env, &undefined);
                    napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                    napi_value callResult;
                    napi_call_function(env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        return NapiGetResut(env, 1);
    } else {
        HILOG_INFO("%{public}s, promise.", __func__);
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {
                HILOG_INFO("%{public}s, promise running", __func__);
                AsyncUpdateFormCallbackInfo *asyncCallbackInfo = (AsyncUpdateFormCallbackInfo *)data;
                InnerUpdateForm(env, asyncCallbackInfo);
            },
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("%{public}s, promise complete", __func__);
                AsyncUpdateFormCallbackInfo *asyncCallbackInfo = (AsyncUpdateFormCallbackInfo *)data;

                napi_value result;
                napi_create_int32(env, asyncCallbackInfo->result, &result);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
        return promise;
    }
}