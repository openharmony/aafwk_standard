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
#include "napi_zlib.h"
#include <cstring>
#include <vector>
#include <uv.h>
#include "hilog_wrapper.h"
#include "napi/native_node_api.h"
#include "napi/native_common.h"
#include "napi_zlib_common.h"
#include "file_path.h"
#include "zip.h"

namespace OHOS {
namespace AAFwk {
namespace LIBZIP {

#define NO_ERROR 0

#define COMPRESS_LEVE_CHECK(level, ret)                                              \
    if (!(level == 0 || level == -1 || level == 1 || level == 9)) {                  \
        HILOG_ERROR("level parameter =[%{public}d] value is incorrect", (int)level); \
        return ret;                                                                  \
    }

#define COMPRESS_STRATEGY_CHECK(strategy, false)                                                \
    if (!(strategy == 0 || strategy == 1 || strategy == 2 || strategy == 3 || strategy == 4)) { \
        HILOG_ERROR("strategy parameter= [%{public}d] value is incorrect", (int)strategy);      \
        return ret;                                                                             \
    }

#define COMPRESS_MEM_CHECK(mem, false)                                                \
    if (!(mem == 1 || mem == 8 || mem == 9)) {                                        \
        HILOG_ERROR("memLevel parameter =[%{public}d] value is incorrect", (int)mem); \
        return ret;                                                                   \
    }

std::shared_ptr<ZlibCallbackInfo> g_zipAceCallbackInfo = nullptr;
std::shared_ptr<ZlibCallbackInfo> g_unzipAceCallbackInfo = nullptr;
void ZipFileAsyncCallBack(int result);
void UnzipFileAsyncCallBack(int result);
napi_value UnwrapZipParam(CallZipUnzipParam &param, napi_env env, napi_value *args);
napi_value UnwrapUnZipParam(CallZipUnzipParam &param, napi_env env, napi_value args[ARGS_MAX_COUNT]);
napi_value ZipFileWrap(napi_env env, napi_callback_info info, AsyncZipCallbackInfo *asyncZipCallbackInfo);
napi_value UnwrapStringParam(std::string &str, napi_env env, napi_value args);
bool UnwrapOptionsParams(OPTIONS &options, napi_env env, napi_value arg);
napi_value ZipFileAsync(napi_env env, napi_value *args, size_t argcAsync, AsyncZipCallbackInfo *asyncZipCallbackInfo);
napi_value UnzipFileAsync(napi_env env, napi_value *args, size_t argcAsync, AsyncZipCallbackInfo *asyncZipCallbackInfo);
void ZipAndUnzipFileAsyncCallBack(std::shared_ptr<ZlibCallbackInfo> &zipAceCallbackInfo, int result);

/**
 * @brief FeatureAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ZlibInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("zipFile", NAPI_ZipFile),
        DECLARE_NAPI_FUNCTION("unzipFile", NAPI_UnzipFile),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));

    return exports;
}

AsyncZipCallbackInfo *CreateZipAsyncCallbackInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_status ret;
    napi_value global = 0;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_global=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    AsyncZipCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncZipCallbackInfo{
        .asyncWork = nullptr,
        .deferred = nullptr,
        .aceCallback = nullptr,
    };
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return asyncCallbackInfo;
}

/**
 * @brief Zlib NAPI method : zipFile.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_ZipFile(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, env);
    size_t argCountWithAsync = 4;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    size_t argcAsync = 4;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync < argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }
    // parse param
    CallZipUnzipParam param;
    if (UnwrapZipParam(param, env, args) == nullptr) {
        HILOG_ERROR("%{public}s, call unwrap param failed.", __func__);
        return nullptr;
    }

    AsyncZipCallbackInfo *asyncZipCallbackInfo = CreateZipAsyncCallbackInfo(env);
    if (asyncZipCallbackInfo == nullptr) {
        return nullptr;
    }

    ret = ZipFileWrap(env, info, asyncZipCallbackInfo);
    if (ret == nullptr) {
        if (asyncZipCallbackInfo != nullptr) {
            delete asyncZipCallbackInfo;
            asyncZipCallbackInfo = nullptr;
        }
    }

    return ret;
}

napi_value ZipFileWrap(napi_env env, napi_callback_info info, AsyncZipCallbackInfo *asyncZipCallbackInfo)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argCountWithAsync = 4;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    size_t argcAsync;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync < argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallZipUnzipParam param;
    if (UnwrapZipParam(param, env, args) == nullptr) {
        HILOG_ERROR("%{public}s, call unwrapWant failed.", __func__);
        return nullptr;
    }
    g_zipAceCallbackInfo = std::make_shared<ZlibCallbackInfo>();
    if (g_zipAceCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, call param failed.", __func__);
        return nullptr;
    }
    asyncZipCallbackInfo->aceCallback = g_zipAceCallbackInfo;
    g_zipAceCallbackInfo->param = param;
    g_zipAceCallbackInfo->env = env;

    if (argcAsync > PARAM3) {
        ret = ZipFileAsync(env, args, argcAsync, asyncZipCallbackInfo);
    } else {
        HILOG_INFO("%{public}s called, wrong number of parameters", __func__);
        return nullptr;
    }

    return ret;
}

napi_value UnwrapStringParam(std::string &str, napi_env env, napi_value argv)
{
    HILOG_INFO("%{public}s,called", __func__);

    // unwrap the param[0]
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    // NAPI_CALL(env, napi_typeof(env, argv, &valueType));
    napi_status rev = napi_typeof(env, argv, &valueType);
    if (rev != napi_ok) {
        return nullptr;
    }

    if (valueType != napi_valuetype::napi_string) {
        HILOG_INFO("%{public}s called, Parameter type does not match", __func__);
        return nullptr;
    }

    size_t len;
    napi_status status = napi_get_value_string_utf8(env, argv, nullptr, 0, &len);
    if (status != napi_ok) {
        HILOG_INFO("%{public}s called, Get locale tag length failed", __func__);
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HILOG_INFO("%{public}s called, Get locale tag failed", __func__);
        return nullptr;
    }
    str = std::string(buf.data());

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

bool UnwrapOptionsParams(OPTIONS &options, napi_env env, napi_value arg)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, arg, napi_object)) {
        return false;
    }
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, arg, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);
    HILOG_INFO("%{public}s called. Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;

    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);
        std::string strProName = UnwrapStringFromJS(env, jsProName, std::string());
        HILOG_INFO("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, arg, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        int ret = 0;
        if (strProName == std::string("flush")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            options.flush = static_cast<FLUSH_TYPE>(ret);
        } else if (strProName == std::string("finishFlush")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            options.finishFlush = static_cast<FLUSH_TYPE>(ret);
        } else if (strProName == std::string("chunkSize")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            options.chunkSize = ret;
        } else if (strProName == std::string("level")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            COMPRESS_LEVE_CHECK(ret, false)
            options.level = static_cast<COMPRESS_LEVEL>(ret);
        } else if (strProName == std::string("memLevel")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            COMPRESS_MEM_CHECK(ret, false)
            options.memLevel = static_cast<MEMORY_LEVEL>(ret);
        } else if (strProName == std::string("strategy")) {
            NAPI_CALL_BASE_BOOL(UnwrapIntValue(env, jsProValue, ret), false);
            COMPRESS_STRATEGY_CHECK(ret, false)
            options.strategy = static_cast<COMPRESS_STRATEGY>(ret);
        } else if (strProName == std::string("dictionary")) {
            continue;
        }
    }
    return true;
}

napi_value UnwrapZipParam(CallZipUnzipParam &param, napi_env env, napi_value *args)
{
    HILOG_INFO("%{public}s,called", __func__);

    // unwrap the param[0]
    if (UnwrapStringParam(param.src, env, args[0]) == nullptr) {
        HILOG_INFO("%{public}s called, args[0] error", __func__);
        return nullptr;
    }

    // unwrap the param[1]
    if (UnwrapStringParam(param.dest, env, args[1]) == nullptr) {
        HILOG_INFO("%{public}s called, args[1] error", __func__);
        return nullptr;
    }

    // unwrap the param[2]
    if (!UnwrapOptionsParams(param.options, env, args[2])) {
        HILOG_INFO("%{public}s called, args[2] error", __func__);
        return nullptr;
    }
    // create reutrn
    napi_value ret = 0;
    NAPI_CALL_BASE(env, napi_create_int32(env, 0, &ret), nullptr);
    return ret;
}

napi_value UnwrapUnZipParam(CallZipUnzipParam &param, napi_env env, napi_value args[ARGS_MAX_COUNT])
{
    HILOG_INFO("%{public}s,called", __func__);

    // unwrap the param[0]
    if (UnwrapStringParam(param.src, env, args[0]) == nullptr) {
        return nullptr;
    }

    // unwrap the param[1]
    if (UnwrapStringParam(param.dest, env, args[1]) == nullptr) {
        return nullptr;
    }

    // create reutrn
    napi_value ret = 0;
    NAPI_CALL_BASE(env, napi_create_int32(env, 0, &ret), nullptr);
    return ret;
}
/**
 * @brief ZipFileAsync
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */

napi_value ZipFileAsync(napi_env env, napi_value *args, size_t argcAsync, AsyncZipCallbackInfo *asyncZipCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncZipCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncZipCallbackInfo == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL_BASE(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName), nullptr);

    if (argcAsync > PARAM3) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, args[PARAM3], &valuetype), nullptr);
        if (valuetype == napi_function) {
            // resultCallback: AsyncCallback<ZipRestult>
            napi_create_reference(env, args[PARAM3], 1, &g_zipAceCallbackInfo->callback);
        } else {
            HILOG_ERROR("%{public}s, args[3] error. It should be a function type.", __func__);
            return nullptr;
        }
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_ZipFile, worker pool thread execute.");
            AsyncZipCallbackInfo *asyncCallbackInfo = (AsyncZipCallbackInfo *)data;

            if (asyncCallbackInfo != nullptr && asyncCallbackInfo->aceCallback != nullptr) {
                Zip(FilePath(asyncCallbackInfo->aceCallback->param.src),
                    FilePath(asyncCallbackInfo->aceCallback->param.dest),
                    asyncCallbackInfo->aceCallback->param.options,
                    ZipFileAsyncCallBack,
                    false);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_ZipFile, main event thread complete.");
            AsyncZipCallbackInfo *asyncCallbackInfo = (AsyncZipCallbackInfo *)data;
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncZipCallbackInfo,
        &asyncZipCallbackInfo->asyncWork);

    napi_queue_async_work(env, asyncZipCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

/**
 * @brief Zlib NAPI method : unzipFile.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_UnzipFile(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argCountWithAsync = 4;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    size_t argcAsync = 4;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync < argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }
    // parse param
    CallZipUnzipParam param;
    if (UnwrapUnZipParam(param, env, args) == nullptr) {
        HILOG_ERROR("%{public}s, call unwrap param failed.", __func__);
        return nullptr;
    }

    AsyncZipCallbackInfo *asyncZipCallbackInfo = CreateZipAsyncCallbackInfo(env);
    if (asyncZipCallbackInfo == nullptr) {
        return nullptr;
    }

    g_unzipAceCallbackInfo = std::make_shared<ZlibCallbackInfo>();
    asyncZipCallbackInfo->aceCallback = g_unzipAceCallbackInfo;
    g_unzipAceCallbackInfo->param = param;
    g_unzipAceCallbackInfo->env = env;

    if (argcAsync > PARAM3) {
        ret = UnzipFileAsync(env, args, argcAsync, asyncZipCallbackInfo);
    } else {
        HILOG_INFO("%{public}s called, wrong number of parameters", __func__);
        if (asyncZipCallbackInfo != nullptr) {
            delete asyncZipCallbackInfo;
            asyncZipCallbackInfo = nullptr;
        }
        return nullptr;
    }

    if (ret == nullptr) {
        if (asyncZipCallbackInfo != nullptr) {
            delete asyncZipCallbackInfo;
            asyncZipCallbackInfo = nullptr;
        }
    }

    return ret;
}

napi_value UnzipFileAsync(napi_env env, napi_value *args, size_t argcAsync, AsyncZipCallbackInfo *asyncZipCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncZipCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    if (argcAsync > PARAM3) {
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, args[PARAM3], &valuetype), nullptr);
        if (valuetype == napi_function) {
            // resultCallback: AsyncCallback<ZipRestult>
            napi_create_reference(env, args[PARAM3], 1, &g_unzipAceCallbackInfo->callback);
        } else {
            HILOG_ERROR("%{public}s, args[3] error. It should be a function type.", __func__);
            return nullptr;
        }
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("NAPI_UnzipFile, worker pool thread execute.");
            AsyncZipCallbackInfo *asyncCallbackInfo = (AsyncZipCallbackInfo *)data;

            // Unzip
            if (asyncCallbackInfo != nullptr) {
                Unzip(FilePath(asyncCallbackInfo->aceCallback->param.src),
                    FilePath(asyncCallbackInfo->aceCallback->param.dest),
                    asyncCallbackInfo->aceCallback->param.options,
                    UnzipFileAsyncCallBack);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("NAPI_UnzipFile, main event thread complete.");
            AsyncZipCallbackInfo *asyncCallbackInfo = (AsyncZipCallbackInfo *)data;
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncZipCallbackInfo,
        &asyncZipCallbackInfo->asyncWork);

    napi_queue_async_work(env, asyncZipCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}
void ZipFileAsyncCallBack(int result)
{
    ZipAndUnzipFileAsyncCallBack(g_zipAceCallbackInfo, result);
}
void UnzipFileAsyncCallBack(int result)
{
    ZipAndUnzipFileAsyncCallBack(g_unzipAceCallbackInfo, result);
}
void ZipAndUnzipFileAsyncCallBack(std::shared_ptr<ZlibCallbackInfo> &zipAceCallbackInfo, int result)
{
    if (zipAceCallbackInfo == nullptr) {
        return;
    }
    HILOG_INFO("%{public}s,called env=%{public}p", __func__, zipAceCallbackInfo->env);
    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(zipAceCallbackInfo->env, &loop);
#endif  // NAPI_VERSION >= 2
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work == nullptr.", __func__);
        return;
    }
    ZlibCallbackInfo *asyncCallbackInfo = new (std::nothrow) ZlibCallbackInfo();
    if (asyncCallbackInfo == nullptr) {
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    asyncCallbackInfo->callbackResult = result;
    *asyncCallbackInfo = *zipAceCallbackInfo;
    zipAceCallbackInfo.reset();
    zipAceCallbackInfo = nullptr;
    work->data = (void *)asyncCallbackInfo;

    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            HILOG_INFO("ZipAndUnzipFileAsyncCallBack, uv_queue_work");
            // JS Thread
            ZlibCallbackInfo *asyncCallbackInfo = (ZlibCallbackInfo *)work->data;
            if (asyncCallbackInfo == nullptr) {
                if (work != nullptr) {
                    delete work;
                    work = nullptr;
                }
                return;
            }

            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            // get napi last error
            const napi_extended_error_info *errorInfo = nullptr;
            napi_get_last_error_info(asyncCallbackInfo->env, &errorInfo);
            // callback(err, data)  errorInfo->error_code;
            result[PARAM0] = GetCallbackErrorValue(asyncCallbackInfo->env, NO_ERROR);
            // callback result
            napi_create_int32(asyncCallbackInfo->env, (int32_t)asyncCallbackInfo->callbackResult, &result[PARAM1]);
            // get callback
            napi_get_reference_value(asyncCallbackInfo->env, asyncCallbackInfo->callback, &callback);
            napi_get_undefined(asyncCallbackInfo->env, &undefined);
            // call callback
            napi_value jsResult = 0;
            napi_call_function(asyncCallbackInfo->env, undefined, callback, ARGS_TWO, &result[PARAM0], &jsResult);
            // free data
            if (asyncCallbackInfo->callback != nullptr) {
                napi_delete_reference(asyncCallbackInfo->env, asyncCallbackInfo->callback);
            }
            if (asyncCallbackInfo != nullptr) {
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            }
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        });
    if (rev != 0) {
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    return;
}

}  // namespace LIBZIP
}  // namespace AAFwk
}  // namespace OHOS
