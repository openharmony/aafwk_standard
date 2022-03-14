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

#ifndef NAPI_FORM_UTIL_H_
#define NAPI_FORM_UTIL_H_

#include "ability.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "nlohmann/json.hpp"

struct AsyncErrMsgCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    napi_value callbackValue;
    int code;
    int type;
};

napi_value NapiGetResut(napi_env env, int iResult);
bool ConvertStringToInt64(const std::string &strInfo, int64_t &int64Value);
void InnerCreateCallbackRetMsg(napi_env env, int32_t code, napi_value* result);
void InnerCreatePromiseRetMsg(napi_env env, int32_t code, napi_value* result);
napi_value RetErrMsg(AsyncErrMsgCallbackInfo* asyncCallbackInfo);

#endif /* NAPI_FORM_UTIL_H_ */
