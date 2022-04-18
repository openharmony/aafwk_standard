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

#ifndef OHOS_APPEXECFWK_NAPI_COMMON_DATA_H
#define OHOS_APPEXECFWK_NAPI_COMMON_DATA_H

#include <map>
#include <string>
#include <vector>

#include "ability.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "pac_map.h"

using Ability = OHOS::AppExecFwk::Ability;
namespace OHOS {
namespace AppExecFwk {

#define NATIVE_C_BUFFER_SIZE 1024 /* Converted to C-style string buffer size */
#define ARGS_MAX_COUNT 10
#define ARGS_ASYNC_COUNT 1

#define ARGS_ZERO 0
#define ARGS_ONE 1
#define ARGS_TWO 2
#define ARGS_THREE 3
#define ARGS_FOUR 4
#define ARGS_FIVE 5
#define ARGS_SIX 6
#define ARGS_SEVEN 7
#define ARGS_EIGHT 8
#define ARGS_NINE 9
#define ARGS_TEN 10

#define PARAM0 0
#define PARAM1 1
#define PARAM2 2
#define PARAM3 3
#define PARAM4 4
#define PARAM5 5
#define PARAM6 6
#define PARAM7 7
#define PARAM8 8
#define PARAM9 9
#define PARAM10 10

struct CallbackInfo {
    napi_env env;
    napi_ref callback = 0;
    napi_deferred deferred;
    int errCode = 0;
};

struct CallAbilityParamData {
    PacMap paramArgs;
    Want want;
};

typedef enum {
    NVT_NONE = 0,
    NVT_UNDEFINED,
    NVT_INT32,
    NVT_BOOL,
    NVT_STRING,
    NVT_DOUBLE,
    NVT_PACMAP,
} TNativeValueType;

typedef struct __ThreadReturnData {
    TNativeValueType data_type;
    int32_t int32_value;
    bool bool_value;
    std::string str_value;
    double double_value;
    PacMap pac_value;
} ThreadReturnData;

typedef struct __AsyncJSCallbackInfo {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability;
    AbilityType abilityType;
    CallAbilityParamData param;
    ThreadReturnData native_data;
    napi_value result;
    int error_code;
    CallbackInfo *aceCallback;
} AsyncJSCallbackInfo;

typedef struct __AsyncParamEx {
    std::string resource;
    size_t argc;
    napi_async_execute_callback execute;
    napi_async_complete_callback complete;
} AsyncParamEx;

typedef struct __ComplexArrayData {
    std::vector<int> intList;
    std::vector<long> longList;
    std::vector<bool> boolList;
    std::vector<double> doubleList;
    std::vector<std::string> stringList;
    std::vector<napi_value> objectList;
} ComplexArrayData;

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APPEXECFWK_NAPI_COMMON_DATA_H
