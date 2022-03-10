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

#include "napi_form_util.h"

#include <cinttypes>
#include <regex>
#include <uv.h>
#include <vector>

#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "runtime.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
    constexpr size_t ARGS_SIZE_ONE = 1;
    constexpr int INT_64_LENGTH = 19;
    constexpr int ZERO_VALUE = 0;
    constexpr int REF_COUNT = 1;
    constexpr int DECIMAL_VALUE = 10;
    constexpr int BASE_NUMBER = 9;
    constexpr int CALLBACK_FLG = 1;
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
 * @brief NapiGetResut
 *
 * @param[in] env The environment that the Node-API call is invoked under
 *
 * @return napi_value
 */
napi_value NapiGetResut(napi_env env, int iResult)
{
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, iResult, &result));
    return result;
}

/**
 * @brief Convert string to int64_t
 *
 * @param[in] strInfo The string information
 * @param[out] int64Value Convert string to int64_t
 *
 * @return Return the convert result
 */
bool ConvertStringToInt64(const std::string &strInfo, int64_t &int64Value)
{
    size_t strLength = strInfo.size();
    if (strLength == ZERO_VALUE) {
        int64Value = ZERO_VALUE;
        return true;
    }
    std::regex pattern("^0|-?[1-9][0-9]{0,18}$"); // "^-?[0-9]{1,19}$"
    std::smatch match;
    if (regex_match(strInfo, match, pattern)) {
        HILOG_DEBUG("%{public}s, regex_match successed.", __func__);
        if (strInfo.substr(ZERO_VALUE, ZERO_VALUE + 1) != "-") { // maximum: 9223372036854775807
            if (strLength < INT_64_LENGTH) {
                int64Value = std::stoll(strInfo);
                return true;
            }
            int maxSubValue = std::stoi(strInfo.substr(ZERO_VALUE, ZERO_VALUE + 1));
            if (strLength == INT_64_LENGTH && maxSubValue < BASE_NUMBER) {
                int64Value = std::stoll(strInfo);
                return true;
            }
            // Means 0x7FFFFFFFFFFFFFFF remove the first number:(2^63 - 1 - 9 * 10 ^ 19)
            int subValue = std::stoll(strInfo.substr(ZERO_VALUE + 1, INT_64_LENGTH - 1));
            if (strLength == INT_64_LENGTH && subValue <= INT64_MAX - BASE_NUMBER *
                pow(DECIMAL_VALUE, INT_64_LENGTH - 1)) {
                int64Value = std::stoll(strInfo);
                return true;
            }
        }
        if (strLength < INT_64_LENGTH + 1) { // The minimum value: -9223372036854775808
            int64Value = std::stoll(strInfo);
            return true;
        }
        if (strLength == INT_64_LENGTH + 1) {
            int minSubValue = std::stoi(strInfo.substr(1, 1));
            if (minSubValue < BASE_NUMBER) {
                int64Value = std::stoll(strInfo);
                return true;
            }

            // Means 0x8000000000000000 remove the first number:-(2^63 - 9 * 10 ^ 19)
            if (std::stoll(strInfo.substr(ZERO_VALUE + 2, INT_64_LENGTH - 1)) <=
                (INT64_MAX - BASE_NUMBER * pow(DECIMAL_VALUE, INT_64_LENGTH) + 1)) {
                int64Value = std::stoll(strInfo);
                return true;
            }
        }
    }
    HILOG_DEBUG("%{public}s, regex_match failed.", __func__);
    return false;
}

/**
 * @brief Create return message
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] code result code
 * @param[out] result result message
 *
 * @return void
 */
void InnerCreateRetMsg(napi_env env, int32_t code, napi_value* result)
{
    HILOG_DEBUG("%{public}s called. code:%{public}d", __func__, code);
    if (code == ERR_OK) {
        napi_create_int32(env, ERR_OK, result);
        return;
    }

    OHOS::AppExecFwk::Ability* ability = GetGlobalAbility(env);
    std::string msg = ability->GetErrorMsg(code);
    HILOG_DEBUG("message: %{public}s", msg.c_str());
    napi_value errInfo = nullptr;
    napi_value errCode = nullptr;
    napi_value errMsg = nullptr;

    napi_create_object(env, &errInfo);
    napi_create_int32(env, code, &errCode);
    napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errMsg);
    napi_set_named_property(env, errInfo, "code", errCode);
    napi_set_named_property(env, errInfo, "data", errMsg);
    result[0] = errInfo;
    HILOG_DEBUG("%{public}s, end.", __func__);
}

/**
 * @brief Send error message.
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] value Use create callback
 * @param[in] code Result code
 * @param[in] type Callback or promise type
 * @param[out] result result message
 *
 * @return void
 */
napi_value RetErrMsg(AsyncErrMsgCallbackInfo* asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_env env = asyncCallbackInfo->env;
    napi_value value = asyncCallbackInfo->callbackValue;

    if (asyncCallbackInfo->type == CALLBACK_FLG) {
        HILOG_INFO("%{public}s, asyncCallback.", __func__);

        // Check the value type of the arguments
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, value, &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "The arguments[1] type of deleteForm is incorrect,\
            expected type is function.");

        napi_create_reference(env, value, REF_COUNT, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                AsyncErrMsgCallbackInfo *asyncCallbackInfo = (AsyncErrMsgCallbackInfo *)data;
                HILOG_INFO("%{public}s, napi_create_async_work complete", __func__);

                if (asyncCallbackInfo->callback != nullptr) {
                    napi_value callback;
                    napi_value undefined;
                    napi_value result;
                    InnerCreateRetMsg(env, asyncCallbackInfo->code, &result);
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
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("%{public}s, promise complete", __func__);
                AsyncErrMsgCallbackInfo *asyncCallbackInfo = (AsyncErrMsgCallbackInfo *)data;

                napi_value result;
                InnerCreateRetMsg(env, asyncCallbackInfo->code, &result);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
        return promise;
    }
}
