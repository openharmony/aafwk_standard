/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <string>

#include "distributed_mission_manager.h"

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "napi_common_data.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
const std::string TAG = "NAPIMissionRegistration";
constexpr size_t VALUE_BUFFER_SIZE = 128;

bool SetStartSyncMissionsContext(const napi_env &env, const napi_value &value,
    SyncRemoteMissionsContext* context)
{
    HILOG_INFO("%{public}s call.", __func__);
    bool isFixConflict = false;
    napi_has_named_property(env, value, "fixConflict", &isFixConflict);
    if (!isFixConflict) {
        HILOG_ERROR("%{public}s, Wrong argument name for fixConflict.", __func__);
        return false;
    }
    napi_value fixConflictValue = nullptr;
    napi_get_named_property(env, value, "fixConflict", &fixConflictValue);
    if (fixConflictValue == nullptr) {
        HILOG_ERROR("%{public}s, not find fixConflict.", __func__);
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, fixConflictValue, &valueType);
    if (valueType != napi_boolean) {
        HILOG_ERROR("%{public}s, fixConflict error type.", __func__);
        return false;
    }
    napi_get_value_bool(env, fixConflictValue, &context->fixConflict);
    bool isTag = false;
    napi_has_named_property(env, value, "tag", &isTag);
    if (!isTag) {
        HILOG_ERROR("%{public}s, Wrong argument name for tag.", __func__);
        return false;
    }
    napi_value tagValue = nullptr;
    napi_get_named_property(env, value, "tag", &tagValue);
    if (tagValue == nullptr) {
        HILOG_ERROR("%{public}s, not find tag.", __func__);
        return false;
    }
    napi_typeof(env, tagValue, &valueType);
    if (valueType != napi_number) {
        HILOG_ERROR("%{public}s, tag error type.", __func__);
        return false;
    }
    napi_get_value_int64(env, tagValue, &context->tag);
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool SetSyncRemoteMissionsContext(const napi_env &env, const napi_value &value,
    bool isStart, SyncRemoteMissionsContext* context)
{
    HILOG_INFO("%{public}s call.", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    napi_value deviceIdValue = nullptr;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    if (!isDeviceId) {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        return false;
    }
    napi_get_named_property(env, value, "deviceId", &deviceIdValue);
    if (deviceIdValue == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        return false;
    }
    napi_typeof(env, deviceIdValue, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, deviceId error type.", __func__);
        return false;
    }

    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, deviceIdValue, deviceId, VALUE_BUFFER_SIZE + 1, &context->valueLen);
    if (context->valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        return false;
    }
    context->deviceId = deviceId;

    if (isStart) {
        if (!SetStartSyncMissionsContext (env, value, context)) {
            HILOG_ERROR("%{public}s, Wrong argument for start sync.", __func__);
            return false;
        }
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool ProcessSyncInput(napi_env env, napi_callback_info info, bool isStart,
    SyncRemoteMissionsContext* syncContext)
{
    HILOG_INFO("%{public}s,called.", __func__);
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        HILOG_ERROR("%{public}s, argument size error.", __func__);
        return false;
    }
    syncContext->env = env;
    if (!SetSyncRemoteMissionsContext(env, argv[0], isStart, syncContext)) {
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        return false;
    }
    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[1], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            return false;
        }
        napi_create_reference(env, argv[1], 1, &syncContext->callbackRef);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return true;
}

void StartSyncRemoteMissionsAsyncWork(napi_env env, const napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StartSyncRemoteMissions(syncContext->deviceId,
                syncContext->fixConflict, syncContext->tag);
        },
        [](napi_env env, napi_status status, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { 0 };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, ("StartSyncRemoteMissions failed, error : " +
                    std::to_string(syncContext->result)).c_str(), NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        (void *)syncContext,
        &syncContext->work);
        napi_queue_async_work(env, syncContext->work);
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value NAPI_StartSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s, test1 called.", __func__);
    auto syncContext = new SyncRemoteMissionsContext();
    if (syncContext == nullptr) {
        HILOG_ERROR("%{public}s, syncContext is nullptr.", __func__);
        NAPI_ASSERT(env, false, "wrong arguments");
    }
    if (!ProcessSyncInput(env, info, true, syncContext)) {
        delete syncContext;
        syncContext = nullptr;
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        NAPI_ASSERT(env, false, "Wrong argument");
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StartSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

void StopSyncRemoteMissionsAsyncWork(napi_env env, napi_value resourceName,
    SyncRemoteMissionsContext* syncContext)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            syncContext->result = AbilityManagerClient::GetInstance()->
                StopSyncRemoteMissions(syncContext->deviceId);
        },
        [](napi_env env, napi_status status, void* data) {
            SyncRemoteMissionsContext* syncContext = (SyncRemoteMissionsContext*)data;
            // set result
            napi_value result[2] = { 0 };
            napi_get_undefined(env, &result[1]);
            if (syncContext->result == 0) {
                napi_get_undefined(env, &result[0]);
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, ("StopSyncRemoteMissions failed, error : " +
                    std::to_string(syncContext->result)).c_str(), NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
            }

            if (syncContext->callbackRef == nullptr) { // promise
                if (syncContext->result == 0) {
                    napi_resolve_deferred(env, syncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, syncContext->deferred, result[0]);
                }
            } else { // AsyncCallback
                napi_value callback = nullptr;
                napi_get_reference_value(env, syncContext->callbackRef, &callback);
                napi_value callResult;
                napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, syncContext->callbackRef);
            }
            napi_delete_async_work(env, syncContext->work);
            delete syncContext;
            syncContext = nullptr;
        },
        (void *)syncContext,
        &syncContext->work);
        napi_queue_async_work(env, syncContext->work);
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value NAPI_StopSyncRemoteMissions(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s, called.", __func__);
    auto syncContext = new SyncRemoteMissionsContext();
    if (syncContext == nullptr) {
        HILOG_ERROR("%{public}s, syncContext is nullptr.", __func__);
        NAPI_ASSERT(env, false, "wrong arguments");
    }
    if (!ProcessSyncInput(env, info, false, syncContext)) {
        delete syncContext;
        syncContext = nullptr;
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        NAPI_ASSERT(env, false, "Wrong argument");
    }
    napi_value result = nullptr;
    if (syncContext->callbackRef == nullptr) {
        napi_create_promise(env, &syncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    StopSyncRemoteMissionsAsyncWork(env, resourceName, syncContext);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

RegisterMissonCB *CreateRegisterMissonCBCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissonCB = new (std::nothrow) RegisterMissonCB;
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissonCB == nullptr", __func__);
        return nullptr;
    }
    registerMissonCB->cbBase.cbInfo.env = env;
    registerMissonCB->cbBase.asyncWork = nullptr;
    registerMissonCB->cbBase.deferred = nullptr;
    registerMissonCB->callbackRef = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
    return registerMissonCB;
}

void RegisterMissonExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissonCB = (RegisterMissonCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissonCB->deviceId);
    if (item != registration_.end()) {
        HILOG_INFO("registration exits.");
        registration = registration_[registerMissonCB->deviceId];
    } else {
        HILOG_INFO("registration not exits.");
        registration = new (std::nothrow) NAPIRemoteMissionListener();
    }
    registerMissonCB->missionRegistration = registration;
    if (registerMissonCB->missionRegistration == nullptr) {
        HILOG_ERROR("%{public}s missionRegistration == nullptr.", __func__);
        registerMissonCB->result = -1;
        return;
    }
    registerMissonCB->missionRegistration->SetEnv(env);
    registerMissonCB->missionRegistration->
        SetNotifyMissionsChangedCBRef(registerMissonCB->missionRegistrationCB.callback[0]);
    registerMissonCB->missionRegistration->
        SetNotifySnapshotCBRef(registerMissonCB->missionRegistrationCB.callback[1]);
    registerMissonCB->missionRegistration->
        SetNotifyNetDisconnectCBRef(registerMissonCB->
            missionRegistrationCB.callback[2]); // 2 refers the second argument
    HILOG_INFO("set callback success.");

    registerMissonCB->result =
        AbilityManagerClient::GetInstance()->
        RegisterMissionListener(registerMissonCB->deviceId,
        registerMissonCB->missionRegistration);
    if (registerMissonCB->result == NO_ERROR) {
        HILOG_INFO("add registration.");
        registration_[registerMissonCB->deviceId] = registration;
    }
    HILOG_INFO("%{public}s end.deviceId:%{public}d ", __func__, registerMissonCB->result);
}

void RegisterMissonCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissonCB = static_cast<RegisterMissonCB *>(data);
    // set result
    napi_value result[2] = { 0 };
    napi_get_undefined(env, &result[1]);
    if (registerMissonCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, ("registerMissionListener failed, error : " +
            std::to_string(registerMissonCB->result)).c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[0]);
    }

    if (registerMissonCB->callbackRef == nullptr) { // promise
        if (registerMissonCB->result == 0) {
            napi_resolve_deferred(env, registerMissonCB->cbBase.deferred, result[1]);
        } else {
            napi_reject_deferred(env, registerMissonCB->cbBase.deferred, result[0]);
        }
    } else { // AsyncCallback
        napi_value callback = nullptr;
        napi_get_reference_value(env, registerMissonCB->callbackRef, &callback);
        napi_value callResult;
        napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, registerMissonCB->callbackRef);
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, registerMissonCB->cbBase.asyncWork));
    delete registerMissonCB;
    registerMissonCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value RegisterMissonAsync(napi_env env, napi_value *args, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    if (registerMissonCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissonCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        RegisterMissonExecuteCB,
        RegisterMissonCallbackCompletedCB,
        (void *)registerMissonCB,
        &registerMissonCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissonCB->cbBase.asyncWork);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return result;
}

bool SetCallbackReference(napi_env env, const napi_value& value, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "notifyMissionsChanged", &isFirstCallback);
    bool isSecondCallback = false;
    napi_has_named_property(env, value, "notifySnapshot", &isSecondCallback);
    bool isThirdCallback = false;
    napi_has_named_property(env, value, "notifyNetDisconnect", &isThirdCallback);
    if (!isFirstCallback || !isSecondCallback || !isThirdCallback) {
        HILOG_ERROR("%{public}s, Wrong argument name for callback.", __func__);
        return false;
    }
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, value, "notifyMissionsChanged", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifyMissionsChanged.", __func__);
        return false;
    }
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifyMissionsChanged callback error type.", __func__);
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissonCB->missionRegistrationCB.callback[0]);

    napi_get_named_property(env, value, "notifySnapshot", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifySnapshot.", __func__);
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifySnapshot callback error type.", __func__);
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &registerMissonCB->missionRegistrationCB.callback[1]);

    napi_get_named_property(env, value, "notifyNetDisconnect", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback notifyNetDisconnect.", __func__);
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, notifyNetDisconnect callback error type.", __func__);
        return false;
    }
    napi_create_reference(env, jsMethod, 1,
        &registerMissonCB->missionRegistrationCB.callback[2]); // 2 refers the second argument
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CreateCallbackReference(napi_env env, const napi_value& value, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype == napi_object) {
        if (!SetCallbackReference(env, value, registerMissonCB)) {
            HILOG_ERROR("%{public}s, Wrong callback.", __func__);
            return false;
        }
    } else {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

napi_value RegisterMissonWrap(napi_env env, napi_callback_info info, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    HILOG_INFO("argcAsync is %{public}zu", argcAsync);
    if (argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }
    napi_value firstNApi = nullptr;
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, args[0], "deviceId", &isDeviceId);
    napi_typeof(env, args[0], &valueType);
    if (isDeviceId && valueType == napi_object) {
        napi_get_named_property(env, args[0], "deviceId", &firstNApi);
    } else {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        return nullptr;
    }
    if (firstNApi == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        return nullptr;
    }

    napi_typeof(env, firstNApi, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, deviceId error type.", __func__);
        return nullptr;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    size_t valueLen = 0;
    napi_get_value_string_utf8(env, firstNApi, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        return nullptr;
    }
    registerMissonCB->deviceId = deviceId;

    if (argcAsync > 1 && !CreateCallbackReference(env, args[1], registerMissonCB)) {
        HILOG_ERROR("%{public}s, Wrong arguments.", __func__);
        return nullptr;
    }

    if (argcAsync == ARGS_THREE) {
        napi_typeof(env, args[ARGS_TWO], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            return nullptr;
        }
        napi_create_reference(env, args[ARGS_TWO], 1, &registerMissonCB->callbackRef);
    }

    ret = RegisterMissonAsync(env, args, registerMissonCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

napi_value NAPI_RegisterMissionListener(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    RegisterMissonCB *registerMissonCB = CreateRegisterMissonCBCBInfo(env);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissonCB == nullptr", __func__);
        NAPI_ASSERT(env, false, "wrong arguments");
    }

    napi_value ret = RegisterMissonWrap(env, info, registerMissonCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete registerMissonCB;
        registerMissonCB = nullptr;
        NAPI_ASSERT(env, false, "wrong arguments");
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void NAPIMissionContinue::SetEnv(const napi_env &env)
{
    env_ = env;
}

NAPIRemoteMissionListener::~NAPIRemoteMissionListener()
{
    if (env_ == nullptr) {
        return;
    }
    if (notifyMissionsChangedRef_ != nullptr) {
        napi_delete_reference(env_, notifyMissionsChangedRef_);
        notifyMissionsChangedRef_ = nullptr;
    }
    if (notifySnapshotRef_ != nullptr) {
        napi_delete_reference(env_, notifySnapshotRef_);
        notifySnapshotRef_ = nullptr;
    }
    if (notifyNetDisconnectRef_ != nullptr) {
        napi_delete_reference(env_, notifyNetDisconnectRef_);
        notifyNetDisconnectRef_ = nullptr;
    }
}

void NAPIRemoteMissionListener::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIRemoteMissionListener::SetNotifyMissionsChangedCBRef(const napi_ref &ref)
{
    notifyMissionsChangedRef_ = ref;
}

void NAPIRemoteMissionListener::SetNotifySnapshotCBRef(const napi_ref &ref)
{
    notifySnapshotRef_ = ref;
}

void NAPIRemoteMissionListener::SetNotifyNetDisconnectCBRef(const napi_ref &ref)
{
    notifyNetDisconnectRef_ = ref;
}

void UvWorkNotifyMissionChanged(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifyMissionChanged, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifyMissionChanged, work is null");
        return;
    }
    RegisterMissonCB *registerMissonCB = static_cast<RegisterMissonCB *>(work->data);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("UvWorkNotifyMissionChanged, registerMissonCB is null");
        delete work;
        return;
    }
    napi_value result = nullptr;
    result =
        WrapString(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->deviceId.c_str(), "deviceId");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissonCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissonCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);
    delete registerMissonCB;
    registerMissonCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifyMissionChanged, uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyMissionsChanged(const std::string& deviceId)
{
    HILOG_INFO("%{public}s, called.", __func__);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    auto registerMissonCB = new (std::nothrow) RegisterMissonCB;
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissonCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissonCB->cbBase.cbInfo.env = env_;
    registerMissonCB->cbBase.cbInfo.callback = notifyMissionsChangedRef_;
    registerMissonCB->deviceId = deviceId;
    work->data = (void *)registerMissonCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyMissionChanged);
    if (rev != 0) {
        delete registerMissonCB;
        registerMissonCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UvWorkNotifySnapshot(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifySnapshot, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifySnapshot, work is null");
        return;
    }
    RegisterMissonCB *registerMissonCB = static_cast<RegisterMissonCB *>(work->data);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("UvWorkNotifySnapshot, registerMissonCB is null");
        delete work;
        return;
    }
    napi_value result[2] = {0};
    result[0] =
        WrapString(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->deviceId.c_str(), "deviceId");
    result[1] =
        WrapInt32(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->missionId, "missionId");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissonCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissonCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[0], &callResult);
    delete registerMissonCB;
    registerMissonCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifySnapshot, uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifySnapshot(const std::string& deviceId, int32_t missionId)
{
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    auto registerMissonCB = new (std::nothrow) RegisterMissonCB;
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissonCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissonCB->cbBase.cbInfo.env = env_;
    registerMissonCB->cbBase.cbInfo.callback = notifySnapshotRef_;
    registerMissonCB->deviceId = deviceId;
    registerMissonCB->missionId = missionId;
    work->data = (void *)registerMissonCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifySnapshot);
    if (rev != 0) {
        delete registerMissonCB;
        registerMissonCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UvWorkNotifyNetDisconnect(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkNotifyNetDisconnect, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkNotifyNetDisconnect, work is null");
        return;
    }
    RegisterMissonCB *registerMissonCB = static_cast<RegisterMissonCB *>(work->data);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("UvWorkNotifyNetDisconnect, registerMissonCB is null");
        delete work;
        return;
    }
    napi_value result[2] = {0};
    result[0] =
        WrapString(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->deviceId.c_str(), "deviceId");
    HILOG_INFO("UvWorkNotifyNetDisconnect, state = %{public}d", registerMissonCB->state);
    result[1] =
        WrapInt32(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->state, "state");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(registerMissonCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(registerMissonCB->cbBase.cbInfo.env, registerMissonCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(registerMissonCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[0], &callResult);
    delete registerMissonCB;
    registerMissonCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkNotifyNetDisconnect, uv_queue_work end");
}

void NAPIRemoteMissionListener::NotifyNetDisconnect(const std::string& deviceId, int32_t state)
{
    HILOG_INFO("%{public}s called. state = %{public}d", __func__, state);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    auto registerMissonCB = new (std::nothrow) RegisterMissonCB;
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s, registerMissonCB == nullptr.", __func__);
        delete work;
        return;
    }
    registerMissonCB->cbBase.cbInfo.env = env_;
    registerMissonCB->cbBase.cbInfo.callback = notifyNetDisconnectRef_;
    registerMissonCB->deviceId = deviceId;
    registerMissonCB->state = state;
    work->data = (void *)registerMissonCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkNotifyNetDisconnect);
    if (rev != 0) {
        delete registerMissonCB;
        registerMissonCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UnRegisterMissonExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissonCB = (RegisterMissonCB*)data;

    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<NAPIRemoteMissionListener> registration;
    auto item = registration_.find(registerMissonCB->deviceId);
    if (item != registration_.end()) {
        HILOG_INFO("registration exits.");
        registration = registration_[registerMissonCB->deviceId];
    } else {
        HILOG_INFO("registration not exits.");
        registerMissonCB->result = -1;
        return;
    }
    registerMissonCB->missionRegistration = registration;

    registerMissonCB->result =
        AbilityManagerClient::GetInstance()->
        UnRegisterMissionListener(registerMissonCB->deviceId,
        registerMissonCB->missionRegistration);
    if (registerMissonCB->result == NO_ERROR) {
        HILOG_INFO("remove registration.");
        registration_.erase(registerMissonCB->deviceId);
    }
    HILOG_INFO("%{public}s end.deviceId:%{public}d ", __func__, registerMissonCB->result);
}

void UnRegisterMissonPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto registerMissonCB = (RegisterMissonCB*)data;
    // set result
    napi_value result[2] = { 0 };
    napi_get_undefined(env, &result[1]);
    if (registerMissonCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, ("unRegisterMissionListener failed, error : " +
            std::to_string(registerMissonCB->result)).c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[0]);
    }

    if (registerMissonCB->callbackRef == nullptr) { // promise
        if (registerMissonCB->result == 0) {
            napi_resolve_deferred(env, registerMissonCB->cbBase.deferred, result[1]);
        } else {
            napi_reject_deferred(env, registerMissonCB->cbBase.deferred, result[0]);
        }
    } else { // AsyncCallback
        napi_value callback = nullptr;
        napi_get_reference_value(env, registerMissonCB->callbackRef, &callback);
        napi_value callResult;
        napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, registerMissonCB->callbackRef);
    }
    napi_delete_async_work(env, registerMissonCB->cbBase.asyncWork);
    delete registerMissonCB;
    registerMissonCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value UnRegisterMissonPromise(napi_env env, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value promise = nullptr;
    if (registerMissonCB->callbackRef == nullptr) {
        napi_create_promise(env, &registerMissonCB->cbBase.deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        UnRegisterMissonExecuteCB,
        UnRegisterMissonPromiseCompletedCB,
        (void *)registerMissonCB,
        &registerMissonCB->cbBase.asyncWork);
    napi_queue_async_work(env, registerMissonCB->cbBase.asyncWork);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return promise;
}

bool GetUnRegisterMissonDeviceId(napi_env env, const napi_value& value, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value firstNApi = nullptr;
    napi_valuetype valueType = napi_undefined;
    bool isDeviceId = false;
    napi_has_named_property(env, value, "deviceId", &isDeviceId);
    napi_typeof(env, value, &valueType);
    if (isDeviceId && valueType == napi_object) {
        napi_get_named_property(env, value, "deviceId", &firstNApi);
    } else {
        HILOG_ERROR("%{public}s, Wrong argument name for deviceId.", __func__);
        return false;
    }
    if (firstNApi == nullptr) {
        HILOG_ERROR("%{public}s, not find deviceId.", __func__);
        return false;
    }

    size_t valueLen = 0;
    napi_typeof(env, firstNApi, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    char deviceId[VALUE_BUFFER_SIZE + 1] = {0};
    napi_get_value_string_utf8(env, firstNApi, deviceId, VALUE_BUFFER_SIZE + 1, &valueLen);
    if (valueLen > VALUE_BUFFER_SIZE) {
        HILOG_ERROR("%{public}s, deviceId length not correct", __func__);
        return false;
    }
    registerMissonCB->deviceId = deviceId;
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

napi_value UnRegisterMissonWrap(napi_env env, napi_callback_info info, RegisterMissonCB *registerMissonCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = 2;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    HILOG_INFO("argc is %{public}zu", argc);
    if (argc != ARGS_ONE && argc != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (!GetUnRegisterMissonDeviceId(env, args[0], registerMissonCB)) {
        HILOG_ERROR("%{public}s, Wrong argument.", __func__);
        return nullptr;
    }

    if (argc == ARGS_TWO) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[1], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            return nullptr;
        }
        napi_create_reference(env, args[1], 1, &registerMissonCB->callbackRef);
    }
    ret = UnRegisterMissonPromise(env, registerMissonCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

napi_value NAPI_UnRegisterMissionListener(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    RegisterMissonCB *registerMissonCB = CreateRegisterMissonCBCBInfo(env);
    if (registerMissonCB == nullptr) {
        HILOG_ERROR("%{public}s registerMissonCB == nullptr", __func__);
        NAPI_ASSERT(env, false, "wrong arguments");
    }

    napi_value ret = UnRegisterMissonWrap(env, info, registerMissonCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete registerMissonCB;
        registerMissonCB = nullptr;
        NAPI_ASSERT(env, false, "wrong arguments");
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value WrapString(napi_env env, const std::string &param, const std::string &paramName)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);

    napi_value jsValue = nullptr;
    HILOG_INFO("%{public}s called. %{public}s = %{public}s", __func__, paramName.c_str(), param.c_str());
    napi_create_string_utf8(env, param.c_str(), NAPI_AUTO_LENGTH, &jsValue);
    napi_set_named_property(env, jsObject, paramName.c_str(), jsValue);

    return jsObject;
}

napi_value WrapInt32(napi_env env, int32_t num, const std::string &paramName)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);

    napi_value jsValue = nullptr;
    HILOG_INFO("%{public}s called. %{public}s = %{public}d", __func__, paramName.c_str(), num);
    napi_create_int32(env, num, &jsValue);
    napi_set_named_property(env, jsObject, paramName.c_str(), jsValue);

    return jsObject;
}

ContinueAbilityCB *CreateContinueAbilityCBCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s continueAbilityCB == nullptr", __func__);
        return nullptr;
    }
    continueAbilityCB->cbBase.cbInfo.env = env;
    continueAbilityCB->cbBase.asyncWork = nullptr;
    continueAbilityCB->cbBase.deferred = nullptr;
    continueAbilityCB->callbackRef = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
    return continueAbilityCB;
}

void ContinueAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    HILOG_INFO("create continueAbilityCB success.");
    sptr<NAPIMissionContinue> continuation(new (std::nothrow) NAPIMissionContinue());
    continueAbilityCB->abilityContinuation = continuation;
    if (continueAbilityCB->abilityContinuation == nullptr) {
        HILOG_ERROR("%{public}s abilityContinuation == nullptr.", __func__);
        return;
    }
    continueAbilityCB->abilityContinuation->SetContinueAbilityEnv(env);
    HILOG_INFO("set env success.");
    continueAbilityCB->abilityContinuation->
        SetContinueAbilityCBRef(continueAbilityCB->abilityContinuationCB.callback[0]);
    HILOG_INFO("set callback success.");
    continueAbilityCB->result = -1;
    continueAbilityCB->result = AAFwk::AbilityManagerClient::GetInstance()->
        ContinueMission(continueAbilityCB->srcDeviceId, continueAbilityCB->dstDeviceId,
        continueAbilityCB->missionId, continueAbilityCB->abilityContinuation,
        continueAbilityCB->wantParams);
    HILOG_INFO("%{public}s end. error:%{public}d ", __func__, continueAbilityCB->result);
}

void ContinueAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto continueAbilityCB = static_cast<ContinueAbilityCB *>(data);
    // set result
    napi_value result[2] = { 0 };
    napi_get_undefined(env, &result[1]);
    if (continueAbilityCB->result == 0) {
        napi_get_undefined(env, &result[0]);
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, ("ContinueAbility failed, error : " +
            std::to_string(continueAbilityCB->result)).c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[0]);
    }

    if (continueAbilityCB->callbackRef == nullptr) { // promise
        if (continueAbilityCB->result == 0) {
            napi_resolve_deferred(env, continueAbilityCB->cbBase.deferred, result[1]);
        } else {
            napi_reject_deferred(env, continueAbilityCB->cbBase.deferred, result[0]);
        }
    } else { // AsyncCallback
        napi_value callback = nullptr;
        napi_get_reference_value(env, continueAbilityCB->callbackRef, &callback);
        napi_value callResult;
        napi_call_function(env, nullptr, callback, ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env, continueAbilityCB->callbackRef);
    }
    napi_delete_async_work(env, continueAbilityCB->cbBase.asyncWork);
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value ContinueAbilityAsync(napi_env env, napi_value *args, ContinueAbilityCB *continueAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }

    napi_value result = nullptr;
    if (continueAbilityCB->callbackRef == nullptr) {
        napi_create_promise(env, &continueAbilityCB->cbBase.deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "ContinueAbilityAsyncForLauncher", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        ContinueAbilityExecuteCB,
        ContinueAbilityCallbackCompletedCB,
        (void *)continueAbilityCB,
        &continueAbilityCB->cbBase.asyncWork);
    napi_queue_async_work(env, continueAbilityCB->cbBase.asyncWork);
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return result;
}

bool CheckContinueKeyExist(napi_env env, const napi_value &value)
{
    bool isSrcDeviceId = false;
    napi_has_named_property(env, value, "srcDeviceId", &isSrcDeviceId);
    bool isDstDeviceId = false;
    napi_has_named_property(env, value, "dstDeviceId", &isDstDeviceId);
    bool isMissionId = false;
    napi_has_named_property(env, value, "missionId", &isMissionId);
    bool isWantParam = false;
    napi_has_named_property(env, value, "wantParam", &isWantParam);
    if (!isSrcDeviceId && !isDstDeviceId && !isMissionId && !isWantParam) {
        HILOG_ERROR("%{public}s, Wrong argument key.", __func__);
        return false;
    }
    return true;
}

bool CheckContinueFirstArgs(napi_env env, const napi_value &value, ContinueAbilityCB *continueAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!CheckContinueKeyExist(env, value)) {
        HILOG_ERROR("%{public}s, Wrong argument key.", __func__);
        return false;
    }
    napi_value firstNApi = nullptr;
    napi_value secondNApi = nullptr;
    napi_value thirdNApi = nullptr;
    napi_value fourthNApi = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    napi_get_named_property(env, value, "srcDeviceId", &firstNApi);
    napi_get_named_property(env, value, "dstDeviceId", &secondNApi);
    napi_get_named_property(env, value, "missionId", &thirdNApi);
    napi_get_named_property(env, value, "wantParam", &fourthNApi);
    if (firstNApi == nullptr || secondNApi == nullptr || thirdNApi == nullptr || fourthNApi == nullptr) {
        HILOG_ERROR("%{public}s, miss required parameters.", __func__);
        return false;
    }
    napi_typeof(env, firstNApi, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type srcDeviceId.", __func__);
        return false;
    }
    continueAbilityCB->srcDeviceId = AppExecFwk::UnwrapStringFromJS(env, firstNApi, "");
    napi_typeof(env, secondNApi, &valueType);
    if (valueType != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type dstDeviceId.", __func__);
        return false;
    }
    continueAbilityCB->dstDeviceId = AppExecFwk::UnwrapStringFromJS(env, secondNApi, "");
    napi_typeof(env, thirdNApi, &valueType);
    if (valueType != napi_number) {
        HILOG_ERROR("%{public}s, Wrong argument type missionId.", __func__);
        return false;
    }
    continueAbilityCB->missionId = AppExecFwk::UnwrapInt32FromJS(env, thirdNApi, -1);
    napi_typeof(env, fourthNApi, &valueType);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type wantParam.", __func__);
        return false;
    }
    AppExecFwk::UnwrapWantParams(env, fourthNApi, continueAbilityCB->wantParams);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

bool CheckContinueCallback(napi_env env, const napi_value &value, ContinueAbilityCB *continueAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value jsMethod = nullptr;
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }
    bool isFirstCallback = false;
    napi_has_named_property(env, value, "onContinueDone", &isFirstCallback);
    if (!isFirstCallback) {
        HILOG_ERROR("%{public}s, Wrong argument name for onContinueDone.", __func__);
        return false;
    }
    napi_get_named_property(env, value, "onContinueDone", &jsMethod);
    if (jsMethod == nullptr) {
        HILOG_ERROR("%{public}s, not find callback onContinueDone.", __func__);
        return false;
    }
    napi_typeof(env, jsMethod, &valuetype);
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, onContinueDone callback error type.", __func__);
        return false;
    }
    napi_create_reference(env, jsMethod, 1, &continueAbilityCB->abilityContinuationCB.callback[0]);
    HILOG_INFO("%{public}s called end.", __func__);
    return true;
}

napi_value ContinueAbilityWrap(napi_env env, napi_callback_info info, ContinueAbilityCB *continueAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 3;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr);
    HILOG_INFO("argcAsync is %{public}zu", argcAsync);
    if (argcAsync != ARGS_TWO && argcAsync != ARGS_THREE) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (!CheckContinueFirstArgs(env, args[0], continueAbilityCB)) {
        HILOG_ERROR("%{public}s, check the first argument failed.", __func__);
        return nullptr;
    }

    if (argcAsync > 1) {
        if (!CheckContinueCallback(env, args[1], continueAbilityCB)) {
            HILOG_ERROR("%{public}s, check callback failed.", __func__);
            return nullptr;
        }
    }

    if (argcAsync == ARGS_THREE) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, args[ARGS_TWO], &valueType);
        if (valueType != napi_function) {
            HILOG_ERROR("%{public}s, callback error type.", __func__);
            return nullptr;
        }
        napi_create_reference(env, args[ARGS_TWO], 1, &continueAbilityCB->callbackRef);
    }

    ret = ContinueAbilityAsync(env, args, continueAbilityCB);
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

napi_value NAPI_ContinueAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    ContinueAbilityCB *continueAbilityCB = CreateContinueAbilityCBCBInfo(env);
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s continueAbilityCB == nullptr", __func__);
        NAPI_ASSERT(env, false, "wrong arguments");
    }

    napi_value ret = ContinueAbilityWrap(env, info, continueAbilityCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        NAPI_ASSERT(env, false, "wrong arguments");
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void UvWorkOnContinueDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnCountinueDone, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnCountinueDone, work is null");
        return;
    }
    ContinueAbilityCB *continueAbilityCB = static_cast<ContinueAbilityCB *>(work->data);
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("UvWorkOnCountinueDone, continueAbilityCB is null");
        delete work;
        return;
    }
    napi_value result = nullptr;
    HILOG_INFO("UvWorkOnCountinueDone, resultCode = %{public}d", continueAbilityCB->resultCode);
    result =
        WrapInt32(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->resultCode, "resultCode");

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(continueAbilityCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(continueAbilityCB->cbBase.cbInfo.env,
        continueAbilityCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(continueAbilityCB->cbBase.cbInfo.env, undefined, callback, 1, &result, &callResult);
    if (continueAbilityCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(continueAbilityCB->cbBase.cbInfo.env, continueAbilityCB->cbBase.cbInfo.callback);
    }
    delete continueAbilityCB;
    continueAbilityCB = nullptr;
    delete work;
    HILOG_INFO("UvWorkOnCountinueDone, uv_queue_work end");
}

void NAPIMissionContinue::OnContinueDone(int32_t result)
{
    HILOG_INFO("%{public}s, called. result = %{public}d", __func__, result);
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    auto continueAbilityCB = new (std::nothrow) ContinueAbilityCB;
    if (continueAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, continueAbilityCB == nullptr.", __func__);
        delete work;
        return;
    }
    continueAbilityCB->cbBase.cbInfo.env = env_;
    continueAbilityCB->cbBase.cbInfo.callback = onContinueDoneRef_;
    continueAbilityCB->resultCode = result;
    work->data = (void *)continueAbilityCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnContinueDone);
    if (rev != 0) {
        delete continueAbilityCB;
        continueAbilityCB = nullptr;
        delete work;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void NAPIMissionContinue::SetContinueAbilityEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIMissionContinue::SetContinueAbilityCBRef(const napi_ref &ref)
{
    onContinueDoneRef_ = ref;
}

napi_value DistributedMissionManagerExport(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startSyncRemoteMissions", NAPI_StartSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("stopSyncRemoteMissions", NAPI_StopSyncRemoteMissions),
        DECLARE_NAPI_FUNCTION("registerMissionListener", NAPI_RegisterMissionListener),
        DECLARE_NAPI_FUNCTION("unRegisterMissionListener", NAPI_UnRegisterMissionListener),
        DECLARE_NAPI_FUNCTION("continueMission", NAPI_ContinueAbility),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties));
    return exports;
}

static napi_module missionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DistributedMissionManagerExport,
    .nm_modname = "distributedMissionManager",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&missionModule);
}
}
}