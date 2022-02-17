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
#include "napi_data_uri_utils.h"

#include <cinttypes>
#include <cstring>
#include <vector>

#include "data_uri_utils.h"
#include "feature_ability_common.h"
#include "hilog_wrapper.h"
#include "securec.h"
#include "uri.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief DataUriUtilsInit NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value DataUriUtilsInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getIdSync", NAPI_GetIdSync),
        DECLARE_NAPI_FUNCTION("attachIdSync", NAPI_AttachIdSync),
        DECLARE_NAPI_FUNCTION("deleteIdSync", NAPI_DeleteIdSync),
        DECLARE_NAPI_FUNCTION("updateIdSync", NAPI_UpdateIdSync),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

/**
 * @brief DataUriUtils NAPI method : getIdSync.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetIdSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return WrapVoidToJS(env);
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. Numbers expected.");

    Uri uri(NapiValueToStringUtf8(env, args[0]));

    napi_value id = nullptr;
    int64_t result = DataUriUtils::GetId(uri);
    NAPI_CALL(env, napi_create_int64(env, result, &id));
    HILOG_INFO("%{public}s, uri=%{public}s, id=%{public}" PRId64, __func__, uri.ToString().c_str(), result);

    return id;
}

/**
 * @brief DataUriUtils NAPI method : attachIdSync.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AttachIdSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return WrapVoidToJS(env);
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. Numbers expected.");

    Uri uri(NapiValueToStringUtf8(env, args[0]));

    int64_t id;
    NAPI_CALL(env, napi_get_value_int64(env, args[1], &id));
    std::string result = DataUriUtils::AttachId(uri, id).ToString();

    napi_value uriAttached = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &uriAttached));
    HILOG_INFO("%{public}s, uri=%{public}s, result=%{public}s, id=%{public}" PRId64,
        __func__,
        uri.ToString().c_str(),
        result.c_str(),
        id);

    return uriAttached;
}

/**
 * @brief DataUriUtils NAPI method : deleteIdSync.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_DeleteIdSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return WrapVoidToJS(env);
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. Numbers expected.");

    Uri uri(NapiValueToStringUtf8(env, args[0]));

    std::string result = DataUriUtils::DeleteId(uri).ToString();
    napi_value uriDeleted = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &uriDeleted));

    HILOG_INFO("%{public}s, uri=%{public}s, result=%{public}s.", __func__, uri.ToString().c_str(), result.c_str());

    return uriDeleted;
}

/**
 * @brief DataUriUtils NAPI method : updateIdSync.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_UpdateIdSync(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return WrapVoidToJS(env);
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. Numbers expected.");

    Uri uri(NapiValueToStringUtf8(env, args[0]));

    int64_t id;
    NAPI_CALL(env, napi_get_value_int64(env, args[1], &id));

    std::string result = DataUriUtils::UpdateId(uri, id).ToString();

    napi_value uriUpdated = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &uriUpdated));

    HILOG_INFO("%{public}s, uri=%{public}s, result=%{public}s, id=%{public}" PRId64,
        __func__,
        uri.ToString().c_str(),
        result.c_str(),
        id);

    return uriUpdated;
}
}  // namespace AppExecFwk
}  // namespace OHOS
