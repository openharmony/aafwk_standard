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

#ifndef NAPI_FORM_UTIL_H_
#define NAPI_FORM_UTIL_H_

#include "ability.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "nlohmann/json.hpp"

const int32_t ERR_COMMON = 1;
const int32_t ERR_PERMISSION_DENY = 2;
const int32_t ERR_GET_INFO_FAILED = 4;
const int32_t ERR_GET_BUNDLE_FAILED = 5;
const int32_t ERR_GET_LAYOUT_FAILED = 6;
const int32_t ERR_ADD_INVALID_PARAM = 7;
const int32_t ERR_CFG_NOT_MATCH_ID = 8;
const int32_t ERR_NOT_EXIST_ID = 9;
const int32_t ERR_BIND_PROVIDER_FAILED = 10;
const int32_t ERR_MAX_SYSTEM_FORMS = 11;
const int32_t ERR_MAX_INSTANCES_PER_FORM = 12;
const int32_t ERR_OPERATION_FORM_NOT_SELF = 13;
const int32_t ERR_PROVIDER_DEL_FAIL = 14;
const int32_t ERR_MAX_FORMS_PER_CLIENT = 15;
const int32_t ERR_MAX_SYSTEM_TEMP_FORMS = 16;
const int32_t ERR_FORM_NO_SUCH_MODULE = 17;
const int32_t ERR_FORM_NO_SUCH_ABILITY = 18;
const int32_t ERR_FORM_NO_SUCH_DIMENSION = 19;
const int32_t ERR_FORM_FA_NOT_INSTALLED = 20;
const int32_t ERR_SYSTEM_RESPONSES_FAILED = 30;
const int32_t ERR_FORM_DUPLICATE_ADDED = 31;
const int32_t ERR_IN_RECOVERY = 36;

struct AsyncErrMsgCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    napi_value callbackValue;
    int code;
    int type;
};

std::string QueryRetMsg(int32_t errorCode);
int32_t QueryRetCode(int32_t innerErrorCode);
napi_value NapiGetResut(napi_env env, int iResult);
bool ConvertStringToInt64(const std::string &strInfo, int64_t &int64Value);
void InnerCreateCallbackRetMsg(napi_env env, int32_t code, napi_value* result);
void InnerCreatePromiseRetMsg(napi_env env, int32_t code, napi_value* result);
napi_value RetErrMsg(AsyncErrMsgCallbackInfo* asyncCallbackInfo);
AsyncErrMsgCallbackInfo *InitErrMsg(napi_env env, int32_t code, int32_t type, napi_value callbackValue);

#endif /* NAPI_FORM_UTIL_H_ */
