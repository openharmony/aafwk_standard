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
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr int INT_64_LENGTH = 19;
constexpr int ZERO_VALUE = 0;
constexpr int REF_COUNT = 1;
constexpr int DECIMAL_VALUE = 10;
constexpr int BASE_NUMBER = 9;
constexpr int CALLBACK_FLG = 1;
static constexpr int32_t ERR_COMMON = 1;
static constexpr int32_t ERR_PERMISSION_DENY = 2;
static constexpr int32_t ERR_GET_INFO_FAILED = 4;
static constexpr int32_t ERR_GET_BUNDLE_FAILED = 5;
static constexpr int32_t ERR_GET_LAYOUT_FAILED = 6;
static constexpr int32_t ERR_ADD_INVALID_PARAM = 7;
static constexpr int32_t ERR_CFG_NOT_MATCH_ID = 8;
static constexpr int32_t ERR_NOT_EXIST_ID = 9;
static constexpr int32_t ERR_BIND_PROVIDER_FAILED = 10;
static constexpr int32_t ERR_MAX_SYSTEM_FORMS = 11;
static constexpr int32_t ERR_MAX_INSTANCES_PER_FORM = 12;
static constexpr int32_t ERR_OPERATION_FORM_NOT_SELF = 13;
static constexpr int32_t ERR_PROVIDER_DEL_FAIL = 14;
static constexpr int32_t ERR_MAX_FORMS_PER_CLIENT = 15;
static constexpr int32_t ERR_MAX_SYSTEM_TEMP_FORMS = 16;
static constexpr int32_t ERR_FORM_NO_SUCH_MODULE = 17;
static constexpr int32_t ERR_FORM_NO_SUCH_ABILITY = 18;
static constexpr int32_t ERR_FORM_NO_SUCH_DIMENSION = 19;
static constexpr int32_t ERR_FORM_FA_NOT_INSTALLED = 20;
static constexpr int32_t ERR_SYSTEM_RESPONSES_FAILED = 30;
static constexpr int32_t ERR_FORM_DUPLICATE_ADDED = 31;
static constexpr int32_t ERR_IN_RECOVERY = 36;
const std::map<int32_t, int32_t> ERROR_CODE_MAP = {
    { ERR_OK,                                        ERR_OK },
    { ERR_APPEXECFWK_FORM_COMMON_CODE,               ERR_COMMON },
    { ERR_APPEXECFWK_FORM_PERMISSION_DENY,           ERR_PERMISSION_DENY },
    { ERR_APPEXECFWK_FORM_PERMISSION_DENY_BUNDLE,    ERR_PERMISSION_DENY },
    { ERR_APPEXECFWK_FORM_PERMISSION_DENY_SYS,       ERR_PERMISSION_DENY },
    { ERR_APPEXECFWK_FORM_GET_INFO_FAILED,           ERR_GET_INFO_FAILED },
    { ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED,         ERR_GET_BUNDLE_FAILED },
    { ERR_APPEXECFWK_FORM_INVALID_PARAM,             ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_INVALID_FORM_ID,           ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_FORM_ID_NUM_ERR,           ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_FORM_ARRAY_ERR,            ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_RELEASE_FLG_ERR,           ERR_COMMON },
    { ERR_APPEXECFWK_FORM_REFRESH_TIME_NUM_ERR,      ERR_COMMON },
    { ERR_APPEXECFWK_FORM_INVALID_BUNDLENAME,        ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_INVALID_MODULENAME,        ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_INVALID_PROVIDER_DATA,     ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_INVALID_REFRESH_TIME,      ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_FORM_ID_ARRAY_ERR,         ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_SERVER_STATUS_ERR,         ERR_COMMON },
    { ERR_APPEXECFWK_FORM_CFG_NOT_MATCH_ID,          ERR_CFG_NOT_MATCH_ID },
    { ERR_APPEXECFWK_FORM_NOT_EXIST_ID,              ERR_NOT_EXIST_ID },
    { ERR_APPEXECFWK_FORM_PROVIDER_DATA_EMPTY,       ERR_ADD_INVALID_PARAM },
    { ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED,      ERR_BIND_PROVIDER_FAILED },
    { ERR_APPEXECFWK_FORM_MAX_SYSTEM_FORMS,          ERR_MAX_SYSTEM_FORMS },
    { ERR_APPEXECFWK_FORM_EXCEED_INSTANCES_PER_FORM, ERR_MAX_INSTANCES_PER_FORM },
    { ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF,        ERR_OPERATION_FORM_NOT_SELF },
    { ERR_APPEXECFWK_FORM_PROVIDER_DEL_FAIL,         ERR_PROVIDER_DEL_FAIL },
    { ERR_APPEXECFWK_FORM_MAX_FORMS_PER_CLIENT,      ERR_MAX_FORMS_PER_CLIENT },
    { ERR_APPEXECFWK_FORM_MAX_SYSTEM_TEMP_FORMS,     ERR_MAX_SYSTEM_TEMP_FORMS },
    { ERR_APPEXECFWK_FORM_NO_SUCH_MODULE,            ERR_FORM_NO_SUCH_MODULE },
    { ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY,           ERR_FORM_NO_SUCH_ABILITY },
    { ERR_APPEXECFWK_FORM_NO_SUCH_DIMENSION,         ERR_FORM_NO_SUCH_DIMENSION },
    { ERR_APPEXECFWK_FORM_FA_NOT_INSTALLED,          ERR_FORM_FA_NOT_INSTALLED },
    { ERR_APPEXECFWK_FORM_MAX_REQUEST,               ERR_MAX_SYSTEM_FORMS },
    { ERR_APPEXECFWK_FORM_MAX_REFRESH,               ERR_MAX_SYSTEM_FORMS },
    { ERR_APPEXECFWK_FORM_GET_BMS_FAILED,            ERR_COMMON },
    { ERR_APPEXECFWK_FORM_GET_HOST_FAILED,           ERR_COMMON },
    { ERR_APPEXECFWK_FORM_GET_FMS_FAILED,            ERR_COMMON },
    { ERR_APPEXECFWK_FORM_SEND_FMS_MSG,              ERR_COMMON },
    { ERR_APPEXECFWK_FORM_FORM_DUPLICATE_ADDED,      ERR_FORM_DUPLICATE_ADDED },
    { ERR_APPEXECFWK_FORM_IN_RECOVER,                ERR_IN_RECOVERY },
    { ERR_APPEXECFWK_FORM_GET_SYSMGR_FAILED,         ERR_SYSTEM_RESPONSES_FAILED }
};
const std::map<int32_t, std::string> CODE_MSG_MAP = {
    { ERR_OK, "success" },
    { ERR_COMMON, "internal error" },
    { ERR_PERMISSION_DENY, "does not have permission to use forms" },
    { ERR_GET_INFO_FAILED, "failed to obtain the configuration information" },
    { ERR_GET_BUNDLE_FAILED, "failed to obtain the bundle information" },
    { ERR_GET_LAYOUT_FAILED, "failed to initialize the form layout" },
    { ERR_ADD_INVALID_PARAM, "invalid input parameter during form operation" },
    { ERR_CFG_NOT_MATCH_ID, "the form id is different from that obtained for the first time" },
    { ERR_NOT_EXIST_ID, "the ID of the form to be operated does not exist" },
    { ERR_BIND_PROVIDER_FAILED, "failed to bind the Form Manager Service to the provider service" },
    { ERR_MAX_SYSTEM_FORMS, "the total number of added forms exceeds the maximum allowed by the system" },
    { ERR_MAX_INSTANCES_PER_FORM, "the number of form instances exceeds the maximum allowed by the system" },
    { ERR_OPERATION_FORM_NOT_SELF, "can not be operated by the current application" },
    { ERR_PROVIDER_DEL_FAIL, "failed to instruct the form provider to delete the form" },
    { ERR_MAX_FORMS_PER_CLIENT, "the total number of added forms exceeds the maximum per client" },
    { ERR_MAX_SYSTEM_TEMP_FORMS, "the total number of added temp forms exceeds the maximum in system" },
    { ERR_FORM_NO_SUCH_MODULE, "the module can not be find in system" },
    { ERR_FORM_NO_SUCH_ABILITY, "the ability can not be find in system" },
    { ERR_FORM_NO_SUCH_DIMENSION, "the dimension is not exist in the form" },
    { ERR_FORM_FA_NOT_INSTALLED, "the ability is not installed" },
    { ERR_SYSTEM_RESPONSES_FAILED, "failed to obtain the RPC object" },
    { ERR_FORM_DUPLICATE_ADDED, "failed to obtain the form requested by the client" },
    { ERR_IN_RECOVERY, "the form is being restored" }
};
} // namespace

/**
 * @brief query the error message by error code
 *
 * @param[in] errorCode the error code return by form JS API.
 *
 * @return the error detail message
 */
std::string QueryRetMsg(int32_t errorCode)
{
    auto iter = CODE_MSG_MAP.find(errorCode);
    if (iter != CODE_MSG_MAP.end()) {
        return iter->second;
    } else {
        return CODE_MSG_MAP.at(ERR_COMMON);
    }
}

/**
 * @brief query the error code by FMS errcode
 *
 * @param[in] innerErrorCode the error code return by form native API.
 *
 * @return the error detail message
 */
int32_t QueryRetCode(int32_t innerErrorCode)
{
    auto iter = ERROR_CODE_MAP.find(innerErrorCode);
    if (iter != ERROR_CODE_MAP.end()) {
        return iter->second;
    } else {
        return ERR_COMMON;
    }
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
 * @brief Create return message(callback)
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] code result code
 * @param[out] result result message
 *
 * @return void
 */
void InnerCreateCallbackRetMsg(napi_env env, int32_t code, napi_value* result)
{
    HILOG_DEBUG("%{public}s called. code:%{public}d", __func__, code);
    napi_value error = nullptr;
    napi_create_object(env, &error);

    auto retCode = QueryRetCode(code);
    auto retMsg = QueryRetMsg(retCode);

    // create error code
    napi_value errCode = nullptr;
    napi_create_int32(env, retCode, &errCode);
    napi_set_named_property(env, error, "code", errCode);

    // create error msg
    napi_value errMsg = nullptr;
    napi_create_string_utf8(env, retMsg.c_str(), NAPI_AUTO_LENGTH, &errMsg);
    napi_set_named_property(env, error, "msg", errMsg);

    result[0] = error;
    napi_get_undefined(env, &result[1]);
    HILOG_DEBUG("%{public}s, end.", __func__);
}

/**
 * @brief Create return message(promise)
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] code result code
 * @param[out] result result message
 *
 * @return void
 */
void InnerCreatePromiseRetMsg(napi_env env, int32_t code, napi_value* result)
{
    HILOG_DEBUG("%{public}s called. code:%{public}d", __func__, code);
    if (code == ERR_OK) {
        napi_get_undefined(env, result);
        return;
    }
    napi_value errInfo = nullptr;
    napi_create_object(env, &errInfo);

    auto retCode = QueryRetCode(code);
    auto retMsg = QueryRetMsg(retCode);

    // create error code
    napi_value errCode = nullptr;
    napi_create_int32(env, retCode, &errCode);
    napi_set_named_property(env, errInfo, "code", errCode);

    // create error msg
    napi_value errMsg = nullptr;
    napi_create_string_utf8(env, retMsg.c_str(), NAPI_AUTO_LENGTH, &errMsg);
    napi_set_named_property(env, errInfo, "msg", errMsg);

    *result = errInfo;
    HILOG_DEBUG("%{public}s, end.", __func__);
}

/**
 * @brief Send error message.
 *
 * @param[in] env The environment that the Node-API call is invoked under
 * @param[in] asyncCallbackInfo Reference, callback info via Node-API
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
                    napi_value result[ARGS_SIZE_TWO] = {0};
                    InnerCreateCallbackRetMsg(env, asyncCallbackInfo->code, result);
                    napi_get_undefined(env, &undefined);
                    napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                    napi_value callResult;
                    napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, result, &callResult);
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
                InnerCreatePromiseRetMsg(env, asyncCallbackInfo->code, &result);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
            },
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
        return promise;
    }
}
