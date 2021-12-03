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
#include "data_ability_observer_interface.h"
#include "data_ability_helper.h"
#include "uri.h"
#include <cstring>
#include <vector>
#include <uv.h>
#include "securec.h"
#include "hilog_wrapper.h"
#include "napi_result_set.h"
#include "napi_rdb_predicates.h"
#include "napi_data_ability_predicates.h"
#include "data_ability_result.h"
#include "data_ability_operation.h"
#include "napi_data_ability_operation.h"
#include "../inner/napi_common/napi_common_ability.h"
#include "message_parcel.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
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
        DECLARE_NAPI_FUNCTION("insert", NAPI_Insert),
        DECLARE_NAPI_FUNCTION("notifyChange", NAPI_NotifyChange),
        DECLARE_NAPI_FUNCTION("on", NAPI_Register),
        DECLARE_NAPI_FUNCTION("off", NAPI_UnRegister),
        DECLARE_NAPI_FUNCTION("delete", NAPI_Delete),
        DECLARE_NAPI_FUNCTION("query", NAPI_Query),
        DECLARE_NAPI_FUNCTION("update", NAPI_Update),
        DECLARE_NAPI_FUNCTION("batchInsert", NAPI_BatchInsert),
        DECLARE_NAPI_FUNCTION("openFile", NAPI_OpenFile),
        DECLARE_NAPI_FUNCTION("getType", NAPI_GetType),
        DECLARE_NAPI_FUNCTION("getFileTypes", NAPI_GetFileTypes),
        DECLARE_NAPI_FUNCTION("normalizeUri", NAPI_NormalizeUri),
        DECLARE_NAPI_FUNCTION("denormalizeUri", NAPI_DenormalizeUri),
        DECLARE_NAPI_FUNCTION("release", NAPI_Release),
        DECLARE_NAPI_FUNCTION("executeBatch", NAPI_ExecuteBatch),
    };
    NAPI_CALL(env,
        napi_define_class(env,
            "dataAbilityHelper",
            NAPI_AUTO_LENGTH,
            DataAbilityHelperConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            GetGlobalDataAbilityHelper()));
    g_dataAbilityHelperList.clear();
    return exports;
}

napi_value DataAbilityHelperConstructor(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
    std::string strUri = NapiValueToStringUtf8(env, argv[0]);

    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    HILOG_INFO("ability = %{public}p strUri = %{public}s", ability, strUri.c_str());
    HILOG_INFO("dataAbilityHelperList.size = %{public}zu", g_dataAbilityHelperList.size());
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper =
        DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
    if (dataAbilityHelper == nullptr) {
        HILOG_INFO("%{public}s, dataAbilityHelper is nullptr", __func__);
        return nullptr;
    }
    HILOG_INFO("dataAbilityHelper = %{public}p", dataAbilityHelper.get());
    g_dataAbilityHelperList.emplace_back(dataAbilityHelper);
    HILOG_INFO("dataAbilityHelperList.size = %{public}zu", g_dataAbilityHelperList.size());

    napi_wrap(
        env,
        thisVar,
        dataAbilityHelper.get(),
        [](napi_env env, void *data, void *hint) {
            DataAbilityHelper *objectInfo = static_cast<DataAbilityHelper *>(data);
            HILOG_INFO("DAHelper finalize_cb objectInfo = %{public}p", objectInfo);
            HILOG_INFO("DAHelper finalize_cb regInstances_.size = %{public}zu", registerInstances_.size());
            auto helper = std::find_if(registerInstances_.begin(),
                registerInstances_.end(),
                [&objectInfo](const DAHelperOnOffCB *helper) { return helper->dataAbilityHelper == objectInfo; });
            if (helper != registerInstances_.end()) {
                HILOG_INFO("DataAbilityHelper finalize_cb find helper");
                (*helper)->dataAbilityHelper->Release();
                delete *helper;
                registerInstances_.erase(helper);
            }
            HILOG_INFO("DAHelper finalize_cb regInstances_.size = %{public}zu", registerInstances_.size());
            g_dataAbilityHelperList.remove_if(
                [objectInfo](const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) {
                    return objectInfo == dataAbilityHelper.get();
                });
            HILOG_INFO("DAHelper finalize_cb dataAbilityHelperList.size = %{public}zu", g_dataAbilityHelperList.size());
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
    DAHelperInsertCB *insertCB = new (std::nothrow) DAHelperInsertCB;
    if (insertCB == nullptr) {
        HILOG_ERROR("%{public}s, insertCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    insertCB->cbBase.cbInfo.env = env;
    insertCB->cbBase.asyncWork = nullptr;
    insertCB->cbBase.deferred = nullptr;
    insertCB->cbBase.ability = nullptr;

    napi_value ret = InsertWrap(env, info, insertCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (insertCB != nullptr) {
            delete insertCB;
            insertCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,called end", __func__);
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
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    insertCB->valueBucket.Clear();
    AnalysisValuesBucket(insertCB->valueBucket, env, args[PARAM1]);

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    insertCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = InsertAsync(env, args, ARGS_TWO, insertCB);
    } else {
        ret = InsertPromise(env, insertCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

void AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg)
{
    napi_value keys = 0;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        HILOG_ERROR("ValuesBucket errr");
        return;
    }
    HILOG_INFO("ValuesBucket num:%{public}d ", arrLen);
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = 0;
        status = napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = 0;
        napi_get_property(env, arg, key, &value);

        SetValuesBucketObject(valuesBucket, env, keyStr, value);
    }
}

void SetValuesBucketObject(
    NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:%{public}s",
            valueType,
            keyStr.c_str(),
            valueString.c_str());
        valuesBucket.PutString(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        valuesBucket.PutDouble(keyStr, valueNumber);
        HILOG_INFO(
            "ValueObject type:%{public}d, key:%{public}s, value:%{public}lf", valueType, keyStr.c_str(), valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        HILOG_INFO(
            "ValueObject type:%{public}d, key:%{public}s, value:%{public}d", valueType, keyStr.c_str(), valueBool);
        valuesBucket.PutBool(keyStr, valueBool);
    } else if (valueType == napi_null) {
        valuesBucket.PutNull(keyStr);
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:null", valueType, keyStr.c_str());
    } else if (valueType == napi_object) {
        HILOG_INFO("ValueObject type:%{public}d, key:%{public}s, value:Uint8Array", valueType, keyStr.c_str());
        valuesBucket.PutBlob(keyStr, ConvertU8Vector(env, value));
    } else {
        HILOG_ERROR("valuesBucket error");
    }
}
napi_value InsertAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperInsertCB *insertCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || insertCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &insertCB->cbBase.cbInfo.callback));
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
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
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
    HILOG_INFO("%{public}s, promise end", __func__);
    return promise;
}

void InsertExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Insert, worker pool thread execute.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    if (insertCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(insertCB->uri);
        insertCB->result = insertCB->dataAbilityHelper->Insert(uri, insertCB->valueBucket);
    } else {
        HILOG_ERROR("NAPI_Insert, dataAbilityHelper == nullptr.");
    }
    HILOG_INFO("NAPI_Insert, worker pool thread execute end.");
}

void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Insert, main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
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
    HILOG_INFO("NAPI_Insert, main event thread complete end.");
}

void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Insert,  main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, insertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    HILOG_INFO("NAPI_Insert,  main event thread complete end.");
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

    std::string strValue = "";
    if (UnwrapStringByPropertyName(env, args, "value", strValue)) {
        HILOG_INFO("%{public}s,strValue=%{public}s", __func__, strValue.c_str());
        value = strValue;
    } else {
        HILOG_ERROR("%{public}s, value == nullptr.", __func__);
        return nullptr;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    HILOG_INFO("%{public}s,end", __func__);
    return result;
}

/**
 * @brief DataAbilityHelper NAPI method : notifyChange.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_NotifyChange(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperNotifyChangeCB *notifyChangeCB = new (std::nothrow) DAHelperNotifyChangeCB;
    if (notifyChangeCB == nullptr) {
        HILOG_ERROR("%{public}s, notifyChangeCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    notifyChangeCB->cbBase.cbInfo.env = env;
    notifyChangeCB->cbBase.asyncWork = nullptr;
    notifyChangeCB->cbBase.deferred = nullptr;
    notifyChangeCB->cbBase.ability = nullptr;

    napi_value ret = NotifyChangeWrap(env, info, notifyChangeCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (notifyChangeCB != nullptr) {
            delete notifyChangeCB;
            notifyChangeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

/**
 * @brief NotifyChange processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param notifyChangeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NotifyChangeWrap(napi_env env, napi_callback_info info, DAHelperNotifyChangeCB *notifyChangeCB)
{

    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
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
        notifyChangeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, notifyChangeCB->uri.c_str());
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("DataAbilityHelper objectInfo = %{public}p", objectInfo);
    notifyChangeCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = NotifyChangeAsync(env, args, argcAsync, argcPromise, notifyChangeCB);
    } else {
        ret = NotifyChangePromise(env, notifyChangeCB);
    }
    return ret;
}

napi_value NotifyChangeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperNotifyChangeCB *notifyChangeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || notifyChangeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &notifyChangeCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NotifyChangeExecuteCB,
            NotifyChangeAsyncCompleteCB,
            (void *)notifyChangeCB,
            &notifyChangeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, notifyChangeCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value NotifyChangePromise(napi_env env, DAHelperNotifyChangeCB *notifyChangeCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (notifyChangeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    notifyChangeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NotifyChangeExecuteCB,
            NotifyChangePromiseCompleteCB,
            (void *)notifyChangeCB,
            &notifyChangeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, notifyChangeCB->cbBase.asyncWork));
    return promise;
}

void NotifyChangeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_NotifyChange, worker pool thread execute.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    if (notifyChangeCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(notifyChangeCB->uri);
        notifyChangeCB->dataAbilityHelper->NotifyChange(uri);
    }
}

void NotifyChangeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_NotifyChange, main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, notifyChangeCB->cbBase.cbInfo.callback, &callback));

    if (!IsTypeForNapiValue(env, callback, napi_function)) {
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        HILOG_INFO("NAPI_NotifyChange, callback is invalid.");
        return;
    }

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapVoidToJS(env);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (notifyChangeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, notifyChangeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

void NotifyChangePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_NotifyChange,  main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, notifyChangeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

/**
 * @brief DataAbilityHelper NAPI method : on.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Register(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperOnOffCB *onCB = new (std::nothrow) DAHelperOnOffCB;
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s, onCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->cbBase.ability = nullptr;

    napi_value ret = RegisterWrap(env, info, onCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (onCB != nullptr) {
            delete onCB;
            onCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,called end", __func__);
    return ret;
}

/**
 * @brief On processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value RegisterWrap(napi_env env, napi_callback_info info, DAHelperOnOffCB *onCB)
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
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type != "dataChange") {
            HILOG_ERROR("%{public}s, Wrong type=%{public}s", __func__, type.c_str());
            return nullptr;
        }
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        onCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, onCB->uri.c_str());
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("DataAbilityHelper objectInfo = %{public}p", objectInfo);
    onCB->dataAbilityHelper = objectInfo;

    ret = RegisterAsync(env, args, argcAsync, argcPromise, onCB);
    return ret;
}

napi_value RegisterAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperOnOffCB *onCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || onCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        HILOG_INFO("valuetype is napi_function");
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &onCB->cbBase.cbInfo.callback));
    } else {
        HILOG_INFO("not valuetype isn't napi_function");
    }

    sptr<NAPIDataAbilityObserver> observer(new (std::nothrow) NAPIDataAbilityObserver());
    observer->SetEnv(env);
    observer->SetCallbackRef(onCB->cbBase.cbInfo.callback);
    onCB->observer = observer;

    registerInstances_.emplace_back(onCB);

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            RegisterExecuteCB,
            [](napi_env env, napi_status status, void *data) {
                HILOG_INFO("NAPI_Register, main event thread complete.");
            },
            (void *)onCB,
            &onCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, onCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

void RegisterExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Register, worker pool thread execute.");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    if (onCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(onCB->uri);
        onCB->dataAbilityHelper->RegisterObserver(uri, onCB->observer);
    }
}

/**
 * @brief DataAbilityHelper NAPI method : Off.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_UnRegister(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperOnOffCB *offCB = new (std::nothrow) DAHelperOnOffCB;
    if (offCB == nullptr) {
        HILOG_ERROR("%{public}s, offCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    offCB->cbBase.cbInfo.env = env;
    offCB->cbBase.asyncWork = nullptr;
    offCB->cbBase.deferred = nullptr;
    offCB->cbBase.ability = nullptr;

    napi_value ret = UnRegisterWrap(env, info, offCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (offCB != nullptr) {
            delete offCB;
            offCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,called end", __func__);
    return ret;
}

/**
 * @brief Off processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value UnRegisterWrap(napi_env env, napi_callback_info info, DAHelperOnOffCB *offCB)
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
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type != "dataChange") {
            HILOG_ERROR("%{public}s, Wrong type=%{public}s", __func__, type.c_str());
            return nullptr;
        }
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    offCB->uri = "";
    if (argcAsync > ARGS_TWO) {
        // parse uri and callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            HILOG_INFO("%{public}s,uri=%{public}s", __func__, offCB->uri.c_str());
        } else {
            HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
            return nullptr;
        }
        NAPI_CALL(env, napi_typeof(env, args[PARAM2], &valuetype));
        if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM2], 1, &offCB->cbBase.cbInfo.callback));
        }
    } else {
        // parse uri or callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            HILOG_INFO("%{public}s,uri=%{public}s", __func__, offCB->uri.c_str());
        } else if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &offCB->cbBase.cbInfo.callback));
        } else {
            HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
            return nullptr;
        }
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("DataAbilityHelper objectInfo = %{public}p", objectInfo);
    offCB->dataAbilityHelper = objectInfo;

    ret = UnRegisterAsync(env, args, argcAsync, argcPromise, offCB);
    return ret;
}

napi_value UnRegisterAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperOnOffCB *offCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || offCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    FindRegisterObs(env, offCB);

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            UnRegisterExecuteCB,
            UnRegisterCompleteCB,
            (void *)offCB,
            &offCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, offCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}


static void FindRegisterObsByCallBack(napi_env env, DAHelperOnOffCB *data)
{
    HILOG_INFO("NAPI_UnRegister, UnRegisterExecuteCB callback is not null.");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        HILOG_ERROR("NAPI_UnRegister, param is null.");
        return;
    }
    // if match callback ,or match both callback and uri
    napi_value callbackA = 0;
    napi_get_reference_value(data->cbBase.cbInfo.env, data->cbBase.cbInfo.callback, &callbackA);
    std::string strUri = data->uri;
    do {
        auto helper = std::find_if(
            registerInstances_.begin(),
            registerInstances_.end(),
            [callbackA, strUri](const DAHelperOnOffCB *helper) {
                bool result = false;
                if (helper == nullptr || helper->cbBase.cbInfo.callback == nullptr) {
                    HILOG_ERROR("UnRegisterExecuteCB %{public}s is nullptr",
                        ((helper == nullptr) ? "helper" : "helper->cbBase.cbInfo.callback"));
                    return result;
                }
                if (helper->uri != strUri) {
                    HILOG_ERROR("UnRegisterExecuteCB find uri inconsistent, h=[%{public}s] u=[%{public}s]",
                        helper->uri.c_str(), strUri.c_str());
                    return result;
                }
                napi_value callbackB = 0;
                napi_get_reference_value(helper->cbBase.cbInfo.env, helper->cbBase.cbInfo.callback, &callbackB);
                auto ret = napi_strict_equals(helper->cbBase.cbInfo.env, callbackA, callbackB, &result);
                HILOG_INFO("NAPI_UnRegister cb equals status=%{public}d result=%{public}d.", ret, result);
                return result;
            });
        if (helper != registerInstances_.end()) {
            data->NotifyList.emplace_back(*helper);
            registerInstances_.erase(helper);
                HILOG_INFO("NAPI_UnRegister Instances erase size = %{public}zu", registerInstances_.size());
        } else {
            HILOG_INFO("NAPI_UnRegister not match any callback. %{public}zu", registerInstances_.size());
            break;  // not match any callback
        }
    } while (true);
    HILOG_INFO("NAPI_UnRegister, UnRegisterExecuteCB FindRegisterObsByCallBack Called End.");
}

void FindRegisterObs(napi_env env, DAHelperOnOffCB *data)
{
    HILOG_INFO("NAPI_UnRegister, FindRegisterObs main event thread execute.");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        HILOG_ERROR("NAPI_UnRegister, param is null.");
        return;
    }
    if (data->cbBase.cbInfo.callback != nullptr) {
        HILOG_INFO("NAPI_UnRegister, UnRegisterExecuteCB callback is not null.");
        FindRegisterObsByCallBack(env, data);
    } else {
        HILOG_INFO("NAPI_UnRegister, uri=%{public}s.", data->uri.c_str());
        if (data->uri != "") {
            // if match uri, unregister all observers corresponding the uri
            std::string strUri = data->uri;
            do {
                auto helper = std::find_if(registerInstances_.begin(),
                    registerInstances_.end(),
                    [strUri](const DAHelperOnOffCB *helper) { return helper->uri == strUri; });
                if (helper != registerInstances_.end()) {
                    // match uri
                    OHOS::Uri uri((*helper)->uri);
                    data->NotifyList.emplace_back(*helper);
                    registerInstances_.erase(helper);
                    HILOG_INFO("NAPI_UnRegister Instances erase size = %{public}zu", registerInstances_.size());
                } else {
                    HILOG_INFO("NAPI_UnRegister not match any uri.");
                    break;  // not match any uri
                }
            } while (true);
        } else {
            HILOG_ERROR("NAPI_UnRegister, error: uri is null.");
        }
    }
    HILOG_INFO("NAPI_UnRegister, FindRegisterObs main event thread execute.end %{public}zu", data->NotifyList.size());
}

void UnRegisterExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_UnRegister, UnRegisterExecuteCB main event thread execute.");
    DAHelperOnOffCB *offCB = static_cast<DAHelperOnOffCB *>(data);
    if (offCB == nullptr || offCB->dataAbilityHelper == nullptr) {
        HILOG_ERROR("NAPI_UnRegister, param is null.");
        if (offCB != nullptr) {
            delete offCB;
            offCB = nullptr;
        }
        return;
    }
    HILOG_INFO("NAPI_UnRegister, offCB->DestoryList size is %{public}zu", offCB->NotifyList.size());
    for (auto &iter : offCB->NotifyList) {
        if (iter != nullptr && iter->observer != nullptr) {
            OHOS::Uri uri(iter->uri);
            iter->dataAbilityHelper->UnregisterObserver(uri, iter->observer);
            offCB->DestoryList.emplace_back(iter);
        }
    }
    offCB->NotifyList.clear();
    HILOG_INFO("NAPI_UnRegister, UnRegisterExecuteCB main event thread execute. end");
}

void UnRegisterCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_UnRegister, main event thread complete.");
    // cannot run it in executeCB, because need to use napi_strict_equals compare callbacks.
    DAHelperOnOffCB *offCB = static_cast<DAHelperOnOffCB *>(data);
    if (offCB == nullptr || offCB->dataAbilityHelper == nullptr) {
        HILOG_ERROR("NAPI_UnRegister, param is null.");
        if (offCB != nullptr) {
            delete offCB;
            offCB = nullptr;
        }
        return;
    }
    HILOG_INFO("NAPI_UnRegister, offCB->DestoryList size is %{public}zu", offCB->DestoryList.size());
    for (auto &iter : offCB->DestoryList) {
        HILOG_INFO("NAPI_UnRegister ReleaseJSCallback. 1 ---");
        if (iter->observer != nullptr) {
            if (iter->observer->GetWorkPre() == 1 && iter->observer->GetWorkRun() == 0) {
                iter->observer->SetAssociatedObject(iter);
                iter->observer->ChangeWorkInt();
                HILOG_INFO("NAPI_UnRegister ReleaseJSCallback. 3 ---");
            } else {
                iter->observer->ReleaseJSCallback();
                delete iter;
                iter = nullptr;
                HILOG_INFO("NAPI_UnRegister ReleaseJSCallback. 4 ---");
            }
        }
    }

    offCB->DestoryList.clear();
    if (offCB != nullptr) {
        delete offCB;
        offCB = nullptr;
    }

    HILOG_INFO("NAPI_UnRegister, main event thread complete. end");
}

void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    if (ref_ == nullptr) {
        HILOG_ERROR("NAPIDataAbilityObserver::ReleaseJSCallback, ref_ is null.");
        return;
    }
    napi_delete_reference(env_, ref_);
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SetAssociatedObject(DAHelperOnOffCB* object)
{
    onCB_ = object;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::ChangeWorkPre()
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called.", __func__);
    std::lock_guard<std::mutex> lock_l(mutex_);
    workPre_ = 1;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, workPre_);
}
void NAPIDataAbilityObserver::ChangeWorkRun()
{
    workRun_ = 1;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, workRun_);
}
void NAPIDataAbilityObserver::ChangeWorkInt()
{
    intrust_ = 1;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, intrust_);
}

void NAPIDataAbilityObserver::ChangeWorkPreDone()
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called.", __func__);
    std::lock_guard<std::mutex> lock_l(mutex_);
    workPre_ = 0;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, workPre_);
}

void NAPIDataAbilityObserver::ChangeWorkRunDone()
{
    workRun_ = 0;
    intrust_ = 0;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called end %{public}d %{public}d", __func__, workRun_, intrust_);
}

int NAPIDataAbilityObserver::GetWorkPre()
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called.", __func__);
    std::lock_guard<std::mutex> lock_l(mutex_);
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, workPre_);
    return workPre_;
}

int NAPIDataAbilityObserver::GetWorkInt()
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end %{public}d", __func__, intrust_);
    return intrust_;
}

int NAPIDataAbilityObserver::GetWorkRun()
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. %{public}d", __func__, workRun_);
    return workRun_;
}

DAHelperOnOffCB* NAPIDataAbilityObserver::GetAssociatedObject(void)
{
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called.", __func__);
    return onCB_;
}

void NAPIDataAbilityObserver::SetEnv(const napi_env &env)
{
    env_ = env;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    HILOG_INFO("NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}


static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    HILOG_INFO("OnChange, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work input work is nullptr");
        return;
    }
    DAHelperOnOffCB *onCB = (DAHelperOnOffCB *)work->data;
    NAPIDataAbilityObserver* obs = onCB->observer;
    onCB->observer = nullptr;
    if (obs != nullptr) {
        obs->ChangeWorkRun();
    }
    napi_value result[ARGS_TWO] = {0};
    result[PARAM0] = GetCallbackErrorValue(onCB->cbBase.cbInfo.env, NO_ERROR);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(onCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = 0;
    napi_get_reference_value(onCB->cbBase.cbInfo.env, onCB->cbBase.cbInfo.callback, &callback);
    napi_call_function(onCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (obs != nullptr) {
        if (obs->GetWorkInt() == 1) {
            obs->ReleaseJSCallback();
            DAHelperOnOffCB* assicuated = obs->GetAssociatedObject();
            if (assicuated != nullptr) {
                HILOG_INFO("OnChange, uv_queue_work ReleaseJSCallback Called");
                obs->SetAssociatedObject(nullptr);
                delete assicuated;
                assicuated = nullptr;
            }
        } else {
            obs->ChangeWorkRunDone();
            obs->ChangeWorkPreDone();
        }
    }
    if (onCB != nullptr) {
        delete onCB;
        onCB = nullptr;
    }
    if (work != nullptr) {
        delete work;
        work = nullptr;
    }
    HILOG_INFO("OnChange, uv_queue_work. end");
}

void NAPIDataAbilityObserver::OnChange()
{
    HILOG_INFO("%{public}s, called.", __func__);
    ChangeWorkPre();

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        ChangeWorkPreDone();
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        ChangeWorkPreDone();
        return;
    }

    DAHelperOnOffCB *onCB = new (std::nothrow) DAHelperOnOffCB;
    if (onCB == nullptr) {
        HILOG_ERROR("%{public}s, onCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        ChangeWorkPreDone();
        return;
    }
    onCB->cbBase.cbInfo.env = env_;
    onCB->cbBase.cbInfo.callback = ref_;
    onCB->observer = this;
    work->data = (void *)onCB;

    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        OnChangeJSThreadWorker);
    if (rev != 0) {
        if (onCB != nullptr) {
            delete onCB;
            onCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, called. end", __func__);
}

napi_value NAPI_GetType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperGetTypeCB *gettypeCB = new (std::nothrow) DAHelperGetTypeCB;
    if (gettypeCB == nullptr) {
        HILOG_ERROR("%{public}s, gettypeCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    gettypeCB->cbBase.cbInfo.env = env;
    gettypeCB->cbBase.asyncWork = nullptr;
    gettypeCB->cbBase.deferred = nullptr;
    gettypeCB->cbBase.ability = nullptr;

    napi_value ret = GetTypeWrap(env, info, gettypeCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (gettypeCB != nullptr) {
            delete gettypeCB;
            gettypeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value GetTypeWrap(napi_env env, napi_callback_info info, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
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
        gettypeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, gettypeCB->uri.c_str());
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    gettypeCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = GetTypeAsync(env, args, ARGS_ONE, gettypeCB);
    } else {
        ret = GetTypePromise(env, gettypeCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value GetTypeAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || gettypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &gettypeCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetTypeExecuteCB,
            GetTypeAsyncCompleteCB,
            (void *)gettypeCB,
            &gettypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, gettypeCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value GetTypePromise(napi_env env, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (gettypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    gettypeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetTypeExecuteCB,
            GetTypePromiseCompleteCB,
            (void *)gettypeCB,
            &gettypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, gettypeCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void GetTypeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetType, worker pool thread execute.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    if (gettypeCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(gettypeCB->uri);
        gettypeCB->result = gettypeCB->dataAbilityHelper->GetType(uri);
    } else {
        HILOG_ERROR("NAPI_GetType, dataAbilityHelper == nullptr.");
    }
    HILOG_INFO("NAPI_GetType, worker pool thread execute end.");
}

void GetTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetType, main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, gettypeCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]);

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (gettypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, gettypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    HILOG_INFO("NAPI_GetType, main event thread complete end.");
}

void GetTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetType,  main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, gettypeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    HILOG_INFO("NAPI_GetType,  main event thread complete end.");
}

napi_value NAPI_GetFileTypes(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperGetFileTypesCB *getfiletypesCB = new (std::nothrow) DAHelperGetFileTypesCB;
    if (getfiletypesCB == nullptr) {
        HILOG_ERROR("%{public}s, getfiletypesCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    getfiletypesCB->cbBase.cbInfo.env = env;
    getfiletypesCB->cbBase.asyncWork = nullptr;
    getfiletypesCB->cbBase.deferred = nullptr;
    getfiletypesCB->cbBase.ability = nullptr;

    napi_value ret = GetFileTypesWrap(env, info, getfiletypesCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (getfiletypesCB != nullptr) {
            delete getfiletypesCB;
            getfiletypesCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value GetFileTypesWrap(napi_env env, napi_callback_info info, DAHelperGetFileTypesCB *getfiletypesCB)
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
        getfiletypesCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, getfiletypesCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        getfiletypesCB->mimeTypeFilter = NapiValueToStringUtf8(env, args[PARAM1]);
        HILOG_INFO("%{public}s,mimeTypeFilter=%{public}s", __func__, getfiletypesCB->mimeTypeFilter.c_str());
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    getfiletypesCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = GetFileTypesAsync(env, args, ARGS_TWO, getfiletypesCB);
    } else {
        ret = GetFileTypesPromise(env, getfiletypesCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}
napi_value GetFileTypesAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperGetFileTypesCB *getfiletypesCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || getfiletypesCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &getfiletypesCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetFileTypesExecuteCB,
            GetFileTypesAsyncCompleteCB,
            (void *)getfiletypesCB,
            &getfiletypesCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getfiletypesCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetFileTypesPromise(napi_env env, DAHelperGetFileTypesCB *getfiletypesCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (getfiletypesCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    getfiletypesCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetFileTypesExecuteCB,
            GetFileTypesPromiseCompleteCB,
            (void *)getfiletypesCB,
            &getfiletypesCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, getfiletypesCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void GetFileTypesExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetFileTypes, worker pool thread execute.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    if (getfiletypesCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(getfiletypesCB->uri);
        HILOG_INFO("NAPI_GetFileTypes, uri:%{public}s", uri.ToString().c_str());
        HILOG_INFO("NAPI_GetFileTypes, mimeTypeFilter:%{public}s", getfiletypesCB->mimeTypeFilter.c_str());
        getfiletypesCB->result = getfiletypesCB->dataAbilityHelper->GetFileTypes(uri, getfiletypesCB->mimeTypeFilter);
    } else {
        HILOG_INFO("NAPI_GetFileTypes, dataAbilityHelper == nullptr.");
    }
    HILOG_INFO("NAPI_GetFileTypes, worker pool thread execute end.");
}

void GetFileTypesAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetFileTypes, main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, getfiletypesCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapGetFileTypesCB(env, *getfiletypesCB);

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (getfiletypesCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, getfiletypesCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    HILOG_INFO("NAPI_GetFileTypes, main event thread complete end.");
}

napi_value WrapGetFileTypesCB(napi_env env, const DAHelperGetFileTypesCB &getfiletypesCB)
{
    HILOG_INFO("WrapGetFileTypesCB, called.");
    HILOG_INFO("NAPI_GetFileTypes, result.size:%{public}zu", getfiletypesCB.result.size());
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        HILOG_INFO("NAPI_GetFileTypes, result[%{public}zu]:%{public}s", i, getfiletypesCB.result.at(i).c_str());
    }
    napi_value proValue = nullptr;

    napi_value jsArrayresult = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayresult));
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, getfiletypesCB.result.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayresult, i, proValue));
    }
    HILOG_INFO("WrapGetFileTypesCB, end.");
    return jsArrayresult;
}

void GetFileTypesPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetFileTypes,  main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value result = nullptr;

    result = WrapGetFileTypesCB(env, *getfiletypesCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, getfiletypesCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    HILOG_INFO("NAPI_GetFileTypes,  main event thread complete end.");
}

napi_value NAPI_NormalizeUri(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperNormalizeUriCB *normalizeuriCB = new (std::nothrow) DAHelperNormalizeUriCB;
    if (normalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, normalizeuriCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    normalizeuriCB->cbBase.cbInfo.env = env;
    normalizeuriCB->cbBase.asyncWork = nullptr;
    normalizeuriCB->cbBase.deferred = nullptr;
    normalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = NormalizeUriWrap(env, info, normalizeuriCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (normalizeuriCB != nullptr) {
            delete normalizeuriCB;
            normalizeuriCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value NormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
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
        normalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, normalizeuriCB->uri.c_str());
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    normalizeuriCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = NormalizeUriAsync(env, args, ARGS_ONE, normalizeuriCB);
    } else {
        ret = NormalizeUriPromise(env, normalizeuriCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}
napi_value NormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || normalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &normalizeuriCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NormalizeUriExecuteCB,
            NormalizeUriAsyncCompleteCB,
            (void *)normalizeuriCB,
            &normalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, normalizeuriCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value NormalizeUriPromise(napi_env env, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (normalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    normalizeuriCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NormalizeUriExecuteCB,
            NormalizeUriPromiseCompleteCB,
            (void *)normalizeuriCB,
            &normalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, normalizeuriCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void NormalizeUriExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_NormalizeUri, worker pool thread execute.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    Uri uriValue(normalizeuriCB->uri);
    if (normalizeuriCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(normalizeuriCB->uri);
        uriValue = normalizeuriCB->dataAbilityHelper->NormalizeUri(uri);
        normalizeuriCB->result = uriValue.ToString();
    } else {
        HILOG_INFO("NAPI_NormalizeUri, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_NormalizeUri, worker pool thread execute end.");
}

void NormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_NormalizeUri, main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, normalizeuriCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (normalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, normalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    HILOG_INFO("NAPI_NormalizeUri, main event thread complete end.");
}

void NormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_NormalizeUri,  main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, normalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    HILOG_INFO("NAPI_NormalizeUri,  main event thread complete end.");
}

napi_value NAPI_DenormalizeUri(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperDenormalizeUriCB *denormalizeuriCB = new (std::nothrow) DAHelperDenormalizeUriCB;
    if (denormalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, denormalizeuriCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    denormalizeuriCB->cbBase.cbInfo.env = env;
    denormalizeuriCB->cbBase.asyncWork = nullptr;
    denormalizeuriCB->cbBase.deferred = nullptr;
    denormalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = DenormalizeUriWrap(env, info, denormalizeuriCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (denormalizeuriCB != nullptr) {
            delete denormalizeuriCB;
            denormalizeuriCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value DenormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
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
        denormalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, denormalizeuriCB->uri.c_str());
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    denormalizeuriCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = DenormalizeUriAsync(env, args, ARGS_ONE, denormalizeuriCB);
    } else {
        ret = DenormalizeUriPromise(env, denormalizeuriCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}
napi_value DenormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || denormalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &denormalizeuriCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DenormalizeUriExecuteCB,
            DenormalizeUriAsyncCompleteCB,
            (void *)denormalizeuriCB,
            &denormalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value DenormalizeUriPromise(napi_env env, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (denormalizeuriCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    denormalizeuriCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DenormalizeUriExecuteCB,
            DenormalizeUriPromiseCompleteCB,
            (void *)denormalizeuriCB,
            &denormalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void DenormalizeUriExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_DenormalizeUri, worker pool thread execute.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    Uri uriValue(denormalizeuriCB->uri);
    if (denormalizeuriCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(denormalizeuriCB->uri);
        uriValue = denormalizeuriCB->dataAbilityHelper->DenormalizeUri(uri);
        denormalizeuriCB->result = uriValue.ToString();
    } else {
        HILOG_ERROR("NAPI_DenormalizeUri, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_DenormalizeUri, worker pool thread execute end.");
}

void DenormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_DenormalizeUri, main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, denormalizeuriCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (denormalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, denormalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    HILOG_INFO("NAPI_DenormalizeUri, main event thread complete end.");
}

void DenormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_DenormalizeUri,  main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, denormalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    HILOG_INFO("NAPI_DenormalizeUri,  main event thread complete end.");
}

void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value)
{
    auto tempPredicates = DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(env, value);
    if (tempPredicates == nullptr) {
        HILOG_ERROR("%{public}s, GetNativePredicates retval Marshalling failed.", __func__);
        return;
    }
    predicates = *tempPredicates;
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Delete(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperDeleteCB *deleteCB = new (std::nothrow) DAHelperDeleteCB;
    if (deleteCB == nullptr) {
        HILOG_ERROR("%{public}s, deleteCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    deleteCB->cbBase.cbInfo.env = env;
    deleteCB->cbBase.asyncWork = nullptr;
    deleteCB->cbBase.deferred = nullptr;
    deleteCB->cbBase.ability = nullptr;

    napi_value ret = DeleteWrap(env, info, deleteCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (deleteCB != nullptr) {
            delete deleteCB;
            deleteCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
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
napi_value DeleteWrap(napi_env env, napi_callback_info info, DAHelperDeleteCB *deleteCB)
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
        deleteCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, deleteCB->uri.c_str());
    }

    UnwrapDataAbilityPredicates(deleteCB->predicates, env, args[PARAM1]);
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    deleteCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = DeleteAsync(env, args, ARGS_TWO, deleteCB);
    } else {
        ret = DeletePromise(env, deleteCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value DeleteAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperDeleteCB *deleteCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || deleteCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &deleteCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DeleteExecuteCB,
            DeleteAsyncCompleteCB,
            (void *)deleteCB,
            &deleteCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, deleteCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value DeletePromise(napi_env env, DAHelperDeleteCB *deleteCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (deleteCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    deleteCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DeleteExecuteCB,
            DeletePromiseCompleteCB,
            (void *)deleteCB,
            &deleteCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, deleteCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void DeleteExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Delete, worker pool thread execute.");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    if (deleteCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(deleteCB->uri);
        deleteCB->result = deleteCB->dataAbilityHelper->Delete(uri, deleteCB->predicates);
    } else {
        HILOG_ERROR("NAPI_Delete, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_Delete, worker pool thread execute end.");
}

void DeleteAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Delete, main event thread complete.");
    DAHelperDeleteCB *DeleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, DeleteCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_int32(env, DeleteCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (DeleteCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, DeleteCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, DeleteCB->cbBase.asyncWork));
    delete DeleteCB;
    DeleteCB = nullptr;
    HILOG_INFO("NAPI_Delete, main event thread complete end.");
}

void DeletePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Delete,  main event thread complete.");
    DAHelperDeleteCB *DeleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, DeleteCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, DeleteCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, DeleteCB->cbBase.asyncWork));
    delete DeleteCB;
    DeleteCB = nullptr;
    HILOG_INFO("NAPI_Delete,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Update(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperUpdateCB *updateCB = new (std::nothrow) DAHelperUpdateCB;
    if (updateCB == nullptr) {
        HILOG_ERROR("%{public}s, updateCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    updateCB->cbBase.cbInfo.env = env;
    updateCB->cbBase.asyncWork = nullptr;
    updateCB->cbBase.deferred = nullptr;
    updateCB->cbBase.ability = nullptr;

    napi_value ret = UpdateWrap(env, info, updateCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (updateCB != nullptr) {
            delete updateCB;
            updateCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
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
napi_value UpdateWrap(napi_env env, napi_callback_info info, DAHelperUpdateCB *updateCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_FOUR;
    const size_t argcPromise = ARGS_THREE;
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
        updateCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, updateCB->uri.c_str());
    }

    updateCB->valueBucket.Clear();
    AnalysisValuesBucket(updateCB->valueBucket, env, args[PARAM1]);
    UnwrapDataAbilityPredicates(updateCB->predicates, env, args[PARAM2]);
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    updateCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = UpdateAsync(env, args, ARGS_THREE, updateCB);
    } else {
        ret = UpdatePromise(env, updateCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value UpdateAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperUpdateCB *updateCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || updateCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &updateCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            UpdateExecuteCB,
            UpdateAsyncCompleteCB,
            (void *)updateCB,
            &updateCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, updateCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value UpdatePromise(napi_env env, DAHelperUpdateCB *updateCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (updateCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    updateCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            UpdateExecuteCB,
            UpdatePromiseCompleteCB,
            (void *)updateCB,
            &updateCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, updateCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void UpdateExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Update, worker pool thread execute.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    if (updateCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(updateCB->uri);
        updateCB->result = updateCB->dataAbilityHelper->Update(uri, updateCB->valueBucket, updateCB->predicates);
    } else {
        HILOG_ERROR("NAPI_Update, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_Update, worker pool thread execute end.");
}

void UpdateAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Update, main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, updateCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_int32(env, updateCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (updateCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, updateCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    HILOG_INFO("NAPI_Update, main event thread complete end.");
}

void UpdatePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Update,  main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, updateCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, updateCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    HILOG_INFO("NAPI_Update,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_OpenFile(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperOpenFileCB *openFileCB = new (std::nothrow) DAHelperOpenFileCB;
    if (openFileCB == nullptr) {
        HILOG_ERROR("%{public}s, openFileCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    openFileCB->cbBase.cbInfo.env = env;
    openFileCB->cbBase.asyncWork = nullptr;
    openFileCB->cbBase.deferred = nullptr;
    openFileCB->cbBase.ability = nullptr;

    napi_value ret = OpenFileWrap(env, info, openFileCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (openFileCB != nullptr) {
            delete openFileCB;
            openFileCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
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
napi_value OpenFileWrap(napi_env env, napi_callback_info info, DAHelperOpenFileCB *openFileCB)
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
        openFileCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, openFileCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        openFileCB->mode = NapiValueToStringUtf8(env, args[PARAM1]);
        HILOG_INFO("%{public}s,mode=%{public}s", __func__, openFileCB->mode.c_str());
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    openFileCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = OpenFileAsync(env, args, ARGS_TWO, openFileCB);
    } else {
        ret = OpenFilePromise(env, openFileCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value OpenFileAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperOpenFileCB *openFileCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || openFileCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &openFileCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            OpenFileExecuteCB,
            OpenFileAsyncCompleteCB,
            (void *)openFileCB,
            &openFileCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, openFileCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value OpenFilePromise(napi_env env, DAHelperOpenFileCB *openFileCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (openFileCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    openFileCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            OpenFileExecuteCB,
            OpenFilePromiseCompleteCB,
            (void *)openFileCB,
            &openFileCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, openFileCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void OpenFileExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_OpenFile, worker pool thread execute.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    if (OpenFileCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(OpenFileCB->uri);
        OpenFileCB->result = OpenFileCB->dataAbilityHelper->OpenFile(uri, OpenFileCB->mode);
    } else {
        HILOG_ERROR("NAPI_OpenFile, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_OpenFile, worker pool thread execute end.");
}

void OpenFileAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_OpenFile, main event thread complete.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, OpenFileCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_int32(env, OpenFileCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (OpenFileCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, OpenFileCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, OpenFileCB->cbBase.asyncWork));
    delete OpenFileCB;
    OpenFileCB = nullptr;
    HILOG_INFO("NAPI_OpenFile, main event thread complete end.");
}

void OpenFilePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_OpenFileCB,  main event thread complete.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, OpenFileCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, OpenFileCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, OpenFileCB->cbBase.asyncWork));
    delete OpenFileCB;
    OpenFileCB = nullptr;
    HILOG_INFO("NAPI_OpenFileCB,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_BatchInsert(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperBatchInsertCB *BatchInsertCB = new (std::nothrow) DAHelperBatchInsertCB;
    if (BatchInsertCB == nullptr) {
        HILOG_ERROR("%{public}s, BatchInsertCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    BatchInsertCB->cbBase.cbInfo.env = env;
    BatchInsertCB->cbBase.asyncWork = nullptr;
    BatchInsertCB->cbBase.deferred = nullptr;
    BatchInsertCB->cbBase.ability = nullptr;

    napi_value ret = BatchInsertWrap(env, info, BatchInsertCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (BatchInsertCB != nullptr) {
            delete BatchInsertCB;
            BatchInsertCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

std::vector<NativeRdb::ValuesBucket> NapiValueObject(napi_env env, napi_value param)
{
    HILOG_INFO("%{public}s,called", __func__);
    std::vector<NativeRdb::ValuesBucket> result;
    UnwrapArrayObjectFromJS(env, param, result);
    return result;
}

bool UnwrapArrayObjectFromJS(napi_env env, napi_value param, std::vector<NativeRdb::ValuesBucket> &value)
{
    HILOG_INFO("%{public}s,called", __func__);
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        HILOG_INFO("%{public}s, IsArrayForNapiValue is false", __func__);
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            HILOG_INFO("%{public}s, napi_get_element is false", __func__);
            return false;
        }

        NativeRdb::ValuesBucket valueBucket;
        valueBucket.Clear();
        AnalysisValuesBucket(valueBucket, env, jsValue);

        value.push_back(valueBucket);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return true;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value BatchInsertWrap(napi_env env, napi_callback_info info, DAHelperBatchInsertCB *batchInsertCB)
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
        batchInsertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, batchInsertCB->uri.c_str());
    }

    batchInsertCB->values = NapiValueObject(env, args[PARAM1]);

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    batchInsertCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = BatchInsertAsync(env, args, ARGS_TWO, batchInsertCB);
    } else {
        ret = BatchInsertPromise(env, batchInsertCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value BatchInsertAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperBatchInsertCB *batchInsertCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || batchInsertCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &batchInsertCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            BatchInsertExecuteCB,
            BatchInsertAsyncCompleteCB,
            (void *)batchInsertCB,
            &batchInsertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, batchInsertCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value BatchInsertPromise(napi_env env, DAHelperBatchInsertCB *batchInsertCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (batchInsertCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    batchInsertCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            BatchInsertExecuteCB,
            BatchInsertPromiseCompleteCB,
            (void *)batchInsertCB,
            &batchInsertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, batchInsertCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void BatchInsertExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_BatchInsert, worker pool thread execute.");
    DAHelperBatchInsertCB *batchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    if (batchInsertCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(batchInsertCB->uri);
        batchInsertCB->result = batchInsertCB->dataAbilityHelper->BatchInsert(uri, batchInsertCB->values);
    } else {
        HILOG_ERROR("NAPI_BatchInsert, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_BatchInsert, worker pool thread execute end.");
}

void BatchInsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_BatchInsert, main event thread complete.");
    DAHelperBatchInsertCB *BatchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, BatchInsertCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_int32(env, BatchInsertCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (BatchInsertCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, BatchInsertCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, BatchInsertCB->cbBase.asyncWork));
    delete BatchInsertCB;
    BatchInsertCB = nullptr;
    HILOG_INFO("NAPI_BatchInsert, main event thread complete end.");
}

void BatchInsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_BatchInsertCB,  main event thread complete.");
    DAHelperBatchInsertCB *BatchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, BatchInsertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, BatchInsertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, BatchInsertCB->cbBase.asyncWork));
    delete BatchInsertCB;
    BatchInsertCB = nullptr;
    HILOG_INFO("NAPI_BatchInsertCB,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Query(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperQueryCB *QueryCB = new (std::nothrow) DAHelperQueryCB;
    if (QueryCB == nullptr) {
        HILOG_ERROR("%{public}s, QueryCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    QueryCB->cbBase.cbInfo.env = env;
    QueryCB->cbBase.asyncWork = nullptr;
    QueryCB->cbBase.deferred = nullptr;
    QueryCB->cbBase.ability = nullptr;

    napi_value ret = QueryWrap(env, info, QueryCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (QueryCB != nullptr) {
            delete QueryCB;
            QueryCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
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
napi_value QueryWrap(napi_env env, napi_callback_info info, DAHelperQueryCB *queryCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_FOUR;
    const size_t argcPromise = ARGS_THREE;
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
        queryCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, queryCB->uri.c_str());
    }

    std::vector<std::string> result;
    bool arrayStringbool = false;
    arrayStringbool = NapiValueToArrayStringUtf8(env, args[PARAM1], result);
    if (arrayStringbool == false) {
        HILOG_ERROR("%{public}s, The return value of arraystringbool is false", __func__);
        return nullptr;
    }
    queryCB->columns = result;
    for (size_t i = 0; i < queryCB->columns.size(); i++) {
        HILOG_INFO("%{public}s,columns=%{public}s", __func__, queryCB->columns.at(i).c_str());
    }

    UnwrapDataAbilityPredicates(queryCB->predicates, env, args[PARAM2]);
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    queryCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = QueryAsync(env, args, ARGS_THREE, queryCB);
    } else {
        ret = QueryPromise(env, queryCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value QueryAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperQueryCB *queryCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || queryCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &queryCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            QueryExecuteCB,
            QueryAsyncCompleteCB,
            (void *)queryCB,
            &queryCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, queryCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value QueryPromise(napi_env env, DAHelperQueryCB *queryCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (queryCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    queryCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            QueryExecuteCB,
            QueryPromiseCompleteCB,
            (void *)queryCB,
            &queryCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, queryCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void QueryPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_DAHelperQueryCB,  main event thread complete.");
    DAHelperQueryCB *QueryCB = static_cast<DAHelperQueryCB *>(data);
    napi_value result = nullptr;
    result = WrapResultSet(env, QueryCB->result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, QueryCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, QueryCB->cbBase.asyncWork));
    delete QueryCB;
    QueryCB = nullptr;
    HILOG_INFO("NAPI_DAHelperQueryCB,  main event thread complete end.");
}

void QueryExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Query, worker pool thread execute.");
    DAHelperQueryCB *queryCB = static_cast<DAHelperQueryCB *>(data);
    if (queryCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(queryCB->uri);
        auto resultset = queryCB->dataAbilityHelper->Query(uri, queryCB->columns, queryCB->predicates);
        if (resultset != nullptr) {
            queryCB->result = resultset;
        } else {
            HILOG_INFO("NAPI_Query, resultset == nullptr.");
        }
    } else {
        HILOG_ERROR("NAPI_Query, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_Query, worker pool thread execute end.");
}

void QueryAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Query, main event thread complete.");
    DAHelperQueryCB *queryCB = static_cast<DAHelperQueryCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, queryCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    result[PARAM1] = WrapResultSet(env, queryCB->result);

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (queryCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, queryCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, queryCB->cbBase.asyncWork));
    delete queryCB;
    queryCB = nullptr;
    HILOG_INFO("NAPI_Query, main event thread complete end.");
}

napi_value WrapResultSet(napi_env env, const std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (resultSet == nullptr) {
        HILOG_ERROR("%{public}s, input parameter resultSet is nullptr", __func__);
        return WrapVoidToJS(env);
    }

    return RdbJsKit::ResultSetProxy::NewInstance(env, resultSet);
}

napi_value NAPI_Release(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    DAHelperReleaseCB *releaseCB = new (std::nothrow) DAHelperReleaseCB;
    if (releaseCB == nullptr) {
        HILOG_ERROR("%{public}s, releaseCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    releaseCB->cbBase.cbInfo.env = env;
    releaseCB->cbBase.asyncWork = nullptr;
    releaseCB->cbBase.deferred = nullptr;

    napi_value ret = ReleaseWrap(env, info, releaseCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (releaseCB != nullptr) {
            delete releaseCB;
            releaseCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value ReleaseWrap(napi_env env, napi_callback_info info, DAHelperReleaseCB *releaseCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("DataAbilityHelper ReleaseWrap objectInfo = %{public}p", objectInfo);
    releaseCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = ReleaseAsync(env, args, ARGS_ONE, releaseCB);
    } else {
        ret = ReleasePromise(env, releaseCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value ReleaseAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperReleaseCB *releaseCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || releaseCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &releaseCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ReleaseExecuteCB,
            ReleaseAsyncCompleteCB,
            (void *)releaseCB,
            &releaseCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, releaseCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value ReleasePromise(napi_env env, DAHelperReleaseCB *releaseCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (releaseCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    releaseCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ReleaseExecuteCB,
            ReleasePromiseCompleteCB,
            (void *)releaseCB,
            &releaseCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, releaseCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void ReleaseExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_Release, worker pool thread execute.");
    DAHelperReleaseCB *releaseCB = static_cast<DAHelperReleaseCB *>(data);
    if (releaseCB->dataAbilityHelper != nullptr) {
        releaseCB->result = releaseCB->dataAbilityHelper->Release();
    } else {
        HILOG_ERROR("NAPI_Release, dataAbilityHelper == nullptr");
    }
    HILOG_INFO("NAPI_Release, worker pool thread execute end.");
}

void ReleaseAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Release, main event thread complete.");
    DAHelperReleaseCB *releaseCB = static_cast<DAHelperReleaseCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, releaseCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_get_boolean(env, releaseCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (releaseCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, releaseCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, releaseCB->cbBase.asyncWork));
    delete releaseCB;
    releaseCB = nullptr;
    HILOG_INFO("NAPI_Release, main event thread complete end.");
}

void ReleasePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_Release,  main event thread complete.");
    DAHelperReleaseCB *releaseCB = static_cast<DAHelperReleaseCB *>(data);
    napi_value result = nullptr;
    napi_get_boolean(env, releaseCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, releaseCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, releaseCB->cbBase.asyncWork));
    delete releaseCB;
    releaseCB = nullptr;
    HILOG_INFO("NAPI_Release,  main event thread complete end.");
}

napi_value NAPI_ExecuteBatch(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,start", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = new (std::nothrow) DAHelperExecuteBatchCB;
    if (executeBatchCB == nullptr) {
        HILOG_ERROR("%{public}s, executeBatchCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    executeBatchCB->cbBase.cbInfo.env = env;
    executeBatchCB->cbBase.asyncWork = nullptr;
    executeBatchCB->cbBase.deferred = nullptr;
    executeBatchCB->cbBase.ability = nullptr;

    napi_value ret = ExecuteBatchWrap(env, info, executeBatchCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (executeBatchCB != nullptr) {
            delete executeBatchCB;
            executeBatchCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

bool UnwrapArrayOperationFromJS(
    napi_env env, napi_callback_info info, napi_value param, std::vector<std::shared_ptr<DataAbilityOperation>> &result)
{
    HILOG_INFO("%{public}s,called", __func__);
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        HILOG_ERROR("%{public}s, Wrong argument type ", __func__);
        return false;
    }
    HILOG_INFO("%{public}s, param size:%{public}d ", __func__, arraySize);
    result.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            HILOG_ERROR("%{public}s, get element failed, index:%{public}d ", __func__, i);
            return false;
        }
        std::shared_ptr<DataAbilityOperation> operation = nullptr;
        UnwrapDataAbilityOperation(operation, env, jsValue);
        HILOG_INFO("%{public}s, UnwrapDataAbilityOperation, index:%{public}d ", __func__, i);
        result.push_back(operation);
    }
    return true;
}

napi_value ExecuteBatchWrap(napi_env env, napi_callback_info info, DAHelperExecuteBatchCB *executeBatchCB)
{
    HILOG_INFO("%{public}s,start", __func__);
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
        executeBatchCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        HILOG_INFO("%{public}s,uri=%{public}s", __func__, executeBatchCB->uri.c_str());
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    UnwrapArrayOperationFromJS(env, info, args[PARAM1], operations);
    HILOG_INFO("%{public}s,operations size=%{public}zu", __func__, operations.size());
    executeBatchCB->operations = operations;

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    HILOG_INFO("%{public}s,DataAbilityHelper objectInfo = %{public}p", __func__, objectInfo);
    executeBatchCB->dataAbilityHelper = objectInfo;

    if (argcAsync > argcPromise) {
        ret = ExecuteBatchAsync(env, args, argcAsync, argcPromise, executeBatchCB);
    } else {
        ret = ExecuteBatchPromise(env, executeBatchCB);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

napi_value ExecuteBatchAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperExecuteBatchCB *executeBatchCB)
{
    HILOG_INFO("%{public}s, asyncCallback start.", __func__);
    if (args == nullptr || executeBatchCB == nullptr) {
        HILOG_ERROR("%{public}s, param is nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &executeBatchCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ExecuteBatchExecuteCB,
            ExecuteBatchAsyncCompleteCB,
            (void *)executeBatchCB,
            &executeBatchCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, executeBatchCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value ExecuteBatchPromise(napi_env env, DAHelperExecuteBatchCB *executeBatchCB)
{
    HILOG_INFO("%{public}s, promise start.", __func__);
    if (executeBatchCB == nullptr) {
        HILOG_ERROR("%{public}s, param is nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    executeBatchCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ExecuteBatchExecuteCB,
            ExecuteBatchPromiseCompleteCB,
            (void *)executeBatchCB,
            &executeBatchCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, executeBatchCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

void ExecuteBatchExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s,NAPI_ExecuteBatch, worker pool thread execute start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    if (executeBatchCB->dataAbilityHelper != nullptr) {
        OHOS::Uri uri(executeBatchCB->uri);
        executeBatchCB->result = executeBatchCB->dataAbilityHelper->ExecuteBatch(uri, executeBatchCB->operations);
        HILOG_INFO("%{public}s, dataAbilityHelper is not nullptr. %{public}zu",
            __func__, executeBatchCB->result.size());
    }
    HILOG_INFO("%{public}s,NAPI_ExecuteBatch, worker pool thread execute end.", __func__);
}

void ExecuteBatchAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, main event thread complete start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, executeBatchCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_array(env, &result[PARAM1]);
    GetDataAbilityResultForResult(env, executeBatchCB->result, result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (executeBatchCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, executeBatchCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeBatchCB->cbBase.asyncWork));
    delete executeBatchCB;
    executeBatchCB = nullptr;
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, main event thread complete end.", __func__);
}

void ExecuteBatchPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, main event thread complete start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetDataAbilityResultForResult(env, executeBatchCB->result, result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, executeBatchCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeBatchCB->cbBase.asyncWork));
    delete executeBatchCB;
    executeBatchCB = nullptr;
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, main event thread complete end.", __func__);
}

void GetDataAbilityResultForResult(
    napi_env env, const std::vector<std::shared_ptr<DataAbilityResult>> dataAbilityResult, napi_value result)
{
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, getDataAbilityResultForResult start. %{public}zu",
        __func__, dataAbilityResult.size());
    int32_t index = 0;
    std::vector<std::shared_ptr<DataAbilityResult>> entities = dataAbilityResult;
    for (const auto &item : entities) {
        napi_value objDataAbilityResult;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objDataAbilityResult));

        napi_value uri;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item->GetUri().ToString().c_str(), NAPI_AUTO_LENGTH, &uri));
        HILOG_INFO("%{public}s, NAPI_ExecuteBatch, uri = [%{public}s]", __func__, item->GetUri().ToString().c_str());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "uri", uri));

        napi_value count;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item->GetCount(), &count));
        HILOG_INFO("%{public}s, NAPI_ExecuteBatch, count = [%{public}d]", __func__, item->GetCount());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "count", count));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objDataAbilityResult));
        index++;
    }
    HILOG_INFO("%{public}s, NAPI_ExecuteBatch, getDataAbilityResultForResult end.", __func__);
}

}  // namespace AppExecFwk
}  // namespace OHOS
