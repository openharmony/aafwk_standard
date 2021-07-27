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
#include "napi_data_ability_helper.h"
#include "data_ability_helper.h"
#include "uri.h"
#include <cstring>
#include <vector>
#include <uv.h>
#include "securec.h"
#include "hilog_wrapper.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
napi_value g_dataAbilityHelper;
std::list<std::shared_ptr<DataAbilityHelper>> g_dataAbilityHelperList;

/**
 * @brief DataAbilityHelper NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value DataAbilityHelperInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("insert", NAPI_Insert),  // NotifyChange
        // DECLARE_NAPI_FUNCTION("delete", NAPI_StartAbilityForResult), // NotifyChange
        // DECLARE_NAPI_FUNCTION("query", NAPI_SetResult),
        // DECLARE_NAPI_FUNCTION("update", NAPI_TerminateAbility),  // NotifyChange
        // DECLARE_NAPI_FUNCTION("call", NAPI_HasWindowFocus),
        // DECLARE_NAPI_FUNCTION("batchInsert", NAPI_GetContext),   // NotifyChange
        // DECLARE_NAPI_FUNCTION("openFile", NAPI_GetWant),
        // DECLARE_NAPI_FUNCTION("executeBatch", NAPI_GetApplicationInfo),
        // DECLARE_NAPI_FUNCTION("getType", NAPI_GetAppType),
        // DECLARE_NAPI_FUNCTION("getFileTypes", NAPI_GetAppType),
        // DECLARE_NAPI_FUNCTION("normalizeUri", NAPI_GetAppType),
        // DECLARE_NAPI_FUNCTION("denormalizeUri", NAPI_GetAppType),
        // DECLARE_NAPI_FUNCTION("realse", NAPI_GetAppType),
        // DECLARE_NAPI_FUNCTION("on", NAPI_GetElementName),
        // DECLARE_NAPI_FUNCTION("off", NAPI_GetAbilityName),
    };
    NAPI_CALL(env,
        napi_define_class(env,
            "dataAbilityHelper",
            NAPI_AUTO_LENGTH,
            DataAbilityHelperConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &g_dataAbilityHelper));
    g_dataAbilityHelperList.clear();
    return exports;
}

napi_value DataAbilityHelperConstructor(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc > 1, "Wrong number of arguments");
    std::string strUri = NapiValueToStringUtf8(env, argv[0]);

    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper =
        DataAbilityHelper::Creator(std::shared_ptr<Ability>(ability), std::make_shared<Uri>(strUri));
    HILOG_INFO("dataAbilityHelper = %{public}p", dataAbilityHelper.get());
    g_dataAbilityHelperList.emplace_back(dataAbilityHelper);

    napi_wrap(
        env,
        thisVar,
        dataAbilityHelper.get(),
        [](napi_env env, void *data, void *hint) {
            std::shared_ptr<DataAbilityHelper> objectInfo =
                std::shared_ptr<DataAbilityHelper>((DataAbilityHelper *)data);
            HILOG_INFO("dataAbilityHelper this = %{public}p, destruct", objectInfo.get());
            g_dataAbilityHelperList.remove_if(
                [objectInfo](const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) {
                    return objectInfo == dataAbilityHelper;
                });
        },
        nullptr,
        nullptr);

    HILOG_INFO("%{public}s,called end", __func__);
    return thisVar;
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Insert(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperInsertCB *insertCB = new (std::nothrow) DAHelperInsertCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = nullptr,
    };

    napi_value ret = InsertWrap(env, info, insertCB);
    if (ret == nullptr) {
        if (insertCB != nullptr) {
            delete insertCB;
            insertCB = nullptr;
        }
    }
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value InsertWrap(napi_env env, napi_callback_info info, DAHelperInsertCB *insertCB)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        insertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, insertCB->uri.c_str());
    }

    std::string strValue;
    UnwrapValuesBucket(strValue, env, args[PARAM1]);
    HILOG_INFO("%{public}s,valueBucket=%{public}s", __func__, strValue.c_str());
    ValuesBucket valueBucket(strValue);
    insertCB->valueBucket = ValuesBucket(valueBucket);

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("DataAbilityHelper objectInfo = %{public}p", objectInfo);
    insertCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = InsertAsync(env, args, argcAsync, argcPromise, insertCB);
    } else {
        ret = InsertPromise(env, insertCB);
    }

    return ret;
}

napi_value InsertAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperInsertCB *insertCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || insertCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &insertCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            InsertExecuteCB,
            InsertAsyncCompleteCB,
            (void *)insertCB,
            &insertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, insertCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value InsertPromise(napi_env env, DAHelperInsertCB *insertCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (insertCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    insertCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            InsertExecuteCB,
            InsertPromiseCompleteCB,
            (void *)insertCB,
            &insertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, insertCB->cbBase.asyncWork));
    return promise;
}

void InsertExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Insert, worker pool thread execute.");
    DAHelperInsertCB *insertCB = (DAHelperInsertCB *)data;
    if (insertCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(insertCB->uri);
        insertCB->result = insertCB->dataAbilityHelper->Insert(uri, insertCB->valueBucket);
        // insertCB->dataAbilityHelper->NotifyChange(uri);
    }
}

void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Insert, main event thread complete.");
    DAHelperInsertCB *insertCB = (DAHelperInsertCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, insertCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_int32(env, insertCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (insertCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, insertCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
}

void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Insert,  main event thread complete.");
    DAHelperInsertCB *insertCB = (DAHelperInsertCB *)data;
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, insertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
}

/**
 * @brief Parse the ValuesBucket parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapValuesBucket(std::string &value, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, valueType != napi_object.", __func__);
        return nullptr;
    }

    napi_value jsObject = nullptr;
    jsObject = GetPropertyValueByPropertyName(env, args, "valueBucket", napi_object);
    if (jsObject == nullptr) {
        HILOG_ERROR("%{public}s, jsObject == nullptr.", __func__);
        return nullptr;
    }

    std::string strValue = "";
    if (UnwrapStringByPropertyName(env, jsObject, "value", strValue)) {
        HILOG_INFO("%{public}s,strValue=%{public}s", __func__, strValue.c_str());
        value = strValue;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

}  // namespace AppExecFwk
}  // namespace OHOS