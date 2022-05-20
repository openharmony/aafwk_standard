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

#include "ability_manager_stub.h"

#include "errors.h"
#include "string_ex.h"

#include "ability_connect_callback_proxy.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_proxy.h"
#include "ability_scheduler_stub.h"

namespace OHOS {
namespace AAFwk {

AbilityManagerStub::AbilityManagerStub()
{
    FirstStepInit();
    SecondStepInit();
    ThirdStepInit();
}

AbilityManagerStub::~AbilityManagerStub()
{
    requestFuncMap_.clear();
}

void AbilityManagerStub::FirstStepInit()
{
    requestFuncMap_[TERMINATE_ABILITY] = &AbilityManagerStub::TerminateAbilityInner;
    requestFuncMap_[TERMINATE_ABILITY_BY_CALLER] = &AbilityManagerStub::TerminateAbilityByCallerInner;
    requestFuncMap_[MINIMIZE_ABILITY] = &AbilityManagerStub::MinimizeAbilityInner;
    requestFuncMap_[ATTACH_ABILITY_THREAD] = &AbilityManagerStub::AttachAbilityThreadInner;
    requestFuncMap_[ABILITY_TRANSITION_DONE] = &AbilityManagerStub::AbilityTransitionDoneInner;
    requestFuncMap_[CONNECT_ABILITY_DONE] = &AbilityManagerStub::ScheduleConnectAbilityDoneInner;
    requestFuncMap_[DISCONNECT_ABILITY_DONE] = &AbilityManagerStub::ScheduleDisconnectAbilityDoneInner;
    requestFuncMap_[TERMINATE_ABILITY_RESULT] = &AbilityManagerStub::TerminateAbilityResultInner;
    requestFuncMap_[COMMAND_ABILITY_DONE] = &AbilityManagerStub::ScheduleCommandAbilityDoneInner;
    requestFuncMap_[ACQUIRE_DATA_ABILITY] = &AbilityManagerStub::AcquireDataAbilityInner;
    requestFuncMap_[RELEASE_DATA_ABILITY] = &AbilityManagerStub::ReleaseDataAbilityInner;
    requestFuncMap_[KILL_PROCESS] = &AbilityManagerStub::KillProcessInner;
    requestFuncMap_[UNINSTALL_APP] = &AbilityManagerStub::UninstallAppInner;
    requestFuncMap_[START_ABILITY] = &AbilityManagerStub::StartAbilityInner;
    requestFuncMap_[START_ABILITY_ADD_CALLER] = &AbilityManagerStub::StartAbilityAddCallerInner;
    requestFuncMap_[CONNECT_ABILITY] = &AbilityManagerStub::ConnectAbilityInner;
    requestFuncMap_[DISCONNECT_ABILITY] = &AbilityManagerStub::DisconnectAbilityInner;
    requestFuncMap_[STOP_SERVICE_ABILITY] = &AbilityManagerStub::StopServiceAbilityInner;
    requestFuncMap_[DUMP_STATE] = &AbilityManagerStub::DumpStateInner;
    requestFuncMap_[DUMPSYS_STATE] = &AbilityManagerStub::DumpSysStateInner;
    requestFuncMap_[START_ABILITY_FOR_SETTINGS] = &AbilityManagerStub::StartAbilityForSettingsInner;
    requestFuncMap_[CONTINUE_MISSION] = &AbilityManagerStub::ContinueMissionInner;
    requestFuncMap_[CONTINUE_ABILITY] = &AbilityManagerStub::ContinueAbilityInner;
    requestFuncMap_[START_CONTINUATION] = &AbilityManagerStub::StartContinuationInner;
    requestFuncMap_[NOTIFY_COMPLETE_CONTINUATION] = &AbilityManagerStub::NotifyCompleteContinuationInner;
    requestFuncMap_[NOTIFY_CONTINUATION_RESULT] = &AbilityManagerStub::NotifyContinuationResultInner;
    requestFuncMap_[REGISTER_REMOTE_MISSION_LISTENER] = &AbilityManagerStub::RegisterRemoteMissionListenerInner;
    requestFuncMap_[UNREGISTER_REMOTE_MISSION_LISTENER] = &AbilityManagerStub::UnRegisterRemoteMissionListenerInner;
    requestFuncMap_[START_ABILITY_FOR_OPTIONS] = &AbilityManagerStub::StartAbilityForOptionsInner;
    requestFuncMap_[START_SYNC_MISSIONS] = &AbilityManagerStub::StartSyncRemoteMissionsInner;
    requestFuncMap_[STOP_SYNC_MISSIONS] = &AbilityManagerStub::StopSyncRemoteMissionsInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    requestFuncMap_[FORCE_TIMEOUT] = &AbilityManagerStub::ForceTimeoutForTestInner;
#endif
    requestFuncMap_[FREE_INSTALL_ABILITY_FROM_REMOTE] = &AbilityManagerStub::FreeInstallAbilityFromRemoteInner;
}

void AbilityManagerStub::SecondStepInit()
{
    requestFuncMap_[GET_PENDING_WANT_SENDER] = &AbilityManagerStub::GetWantSenderInner;
    requestFuncMap_[SEND_PENDING_WANT_SENDER] = &AbilityManagerStub::SendWantSenderInner;
    requestFuncMap_[CANCEL_PENDING_WANT_SENDER] = &AbilityManagerStub::CancelWantSenderInner;
    requestFuncMap_[GET_PENDING_WANT_UID] = &AbilityManagerStub::GetPendingWantUidInner;
    requestFuncMap_[GET_PENDING_WANT_USERID] = &AbilityManagerStub::GetPendingWantUserIdInner;
    requestFuncMap_[GET_PENDING_WANT_BUNDLENAME] = &AbilityManagerStub::GetPendingWantBundleNameInner;
    requestFuncMap_[GET_PENDING_WANT_CODE] = &AbilityManagerStub::GetPendingWantCodeInner;
    requestFuncMap_[GET_PENDING_WANT_TYPE] = &AbilityManagerStub::GetPendingWantTypeInner;
    requestFuncMap_[REGISTER_CANCEL_LISTENER] = &AbilityManagerStub::RegisterCancelListenerInner;
    requestFuncMap_[UNREGISTER_CANCEL_LISTENER] = &AbilityManagerStub::UnregisterCancelListenerInner;
    requestFuncMap_[GET_PENDING_REQUEST_WANT] = &AbilityManagerStub::GetPendingRequestWantInner;
    requestFuncMap_[GET_PENDING_WANT_SENDER_INFO] = &AbilityManagerStub::GetPendingRequestWantInner;
    requestFuncMap_[UPDATE_CONFIGURATION] = &AbilityManagerStub::UpdateConfigurationInner;
    requestFuncMap_[GET_SYSTEM_MEMORY_ATTR] = &AbilityManagerStub::GetSystemMemoryAttrInner;
    requestFuncMap_[GET_APP_MEMORY_SIZE] = &AbilityManagerStub::GetAppMemorySizeInner;
    requestFuncMap_[IS_RAM_CONSTRAINED_DEVICE] = &AbilityManagerStub::IsRamConstrainedDeviceInner;
    requestFuncMap_[CLEAR_UP_APPLICATION_DATA] = &AbilityManagerStub::ClearUpApplicationDataInner;
    requestFuncMap_[LOCK_MISSION_FOR_CLEANUP] = &AbilityManagerStub::LockMissionForCleanupInner;
    requestFuncMap_[UNLOCK_MISSION_FOR_CLEANUP] = &AbilityManagerStub::UnlockMissionForCleanupInner;
    requestFuncMap_[REGISTER_MISSION_LISTENER] = &AbilityManagerStub::RegisterMissionListenerInner;
    requestFuncMap_[UNREGISTER_MISSION_LISTENER] = &AbilityManagerStub::UnRegisterMissionListenerInner;
    requestFuncMap_[GET_MISSION_INFOS] = &AbilityManagerStub::GetMissionInfosInner;
    requestFuncMap_[GET_MISSION_INFO_BY_ID] = &AbilityManagerStub::GetMissionInfoInner;
    requestFuncMap_[CLEAN_MISSION] = &AbilityManagerStub::CleanMissionInner;
    requestFuncMap_[CLEAN_ALL_MISSIONS] = &AbilityManagerStub::CleanAllMissionsInner;
    requestFuncMap_[MOVE_MISSION_TO_FRONT] = &AbilityManagerStub::MoveMissionToFrontInner;
    requestFuncMap_[MOVE_MISSION_TO_FRONT_BY_OPTIONS] = &AbilityManagerStub::MoveMissionToFrontByOptionsInner;
    requestFuncMap_[START_CALL_ABILITY] = &AbilityManagerStub::StartAbilityByCallInner;
    requestFuncMap_[RELEASE_CALL_ABILITY] = &AbilityManagerStub::ReleaseInner;
    requestFuncMap_[SET_MISSION_LABEL] = &AbilityManagerStub::SetMissionLabelInner;
    requestFuncMap_[START_USER] = &AbilityManagerStub::StartUserInner;
    requestFuncMap_[STOP_USER] = &AbilityManagerStub::StopUserInner;
    requestFuncMap_[GET_ABILITY_RUNNING_INFO] = &AbilityManagerStub::GetAbilityRunningInfosInner;
    requestFuncMap_[GET_EXTENSION_RUNNING_INFO] = &AbilityManagerStub::GetExtensionRunningInfosInner;
    requestFuncMap_[GET_PROCESS_RUNNING_INFO] = &AbilityManagerStub::GetProcessRunningInfosInner;
    requestFuncMap_[SET_ABILITY_CONTROLLER] = &AbilityManagerStub::SetAbilityControllerInner;
    requestFuncMap_[GET_MISSION_SNAPSHOT_INFO] = &AbilityManagerStub::GetMissionSnapshotInfoInner;
    requestFuncMap_[IS_USER_A_STABILITY_TEST] = &AbilityManagerStub::IsRunningInStabilityTestInner;
    requestFuncMap_[SEND_APP_NOT_RESPONSE_PROCESS_ID] = &AbilityManagerStub::SendANRProcessIDInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    requestFuncMap_[BLOCK_ABILITY] = &AbilityManagerStub::BlockAbilityInner;
    requestFuncMap_[BLOCK_AMS_SERVICE] = &AbilityManagerStub::BlockAmsServiceInner;
    requestFuncMap_[BLOCK_APP_SERVICE] = &AbilityManagerStub::BlockAppServiceInner;
#endif
}

void AbilityManagerStub::ThirdStepInit()
{
    requestFuncMap_[START_USER_TEST] = &AbilityManagerStub::StartUserTestInner;
    requestFuncMap_[FINISH_USER_TEST] = &AbilityManagerStub::FinishUserTestInner;
    requestFuncMap_[GET_CURRENT_TOP_ABILITY] = &AbilityManagerStub::GetCurrentTopAbilityInner;
    requestFuncMap_[DELEGATOR_DO_ABILITY_FOREGROUND] = &AbilityManagerStub::DelegatorDoAbilityForegroundInner;
    requestFuncMap_[DELEGATOR_DO_ABILITY_BACKGROUND] = &AbilityManagerStub::DelegatorDoAbilityBackgroundInner;
    requestFuncMap_[DO_ABILITY_FOREGROUND] = &AbilityManagerStub::DoAbilityForegroundInner;
    requestFuncMap_[DO_ABILITY_BACKGROUND] = &AbilityManagerStub::DoAbilityBackgroundInner;
    requestFuncMap_[GET_MISSION_ID_BY_ABILITY_TOKEN] = &AbilityManagerStub::GetMissionIdByTokenInner;
    requestFuncMap_[GET_TOP_ABILITY] = &AbilityManagerStub::GetTopAbilityInner;
    requestFuncMap_[SET_MISSION_ICON] = &AbilityManagerStub::SetMissionIconInner;
    requestFuncMap_[DUMP_ABILITY_INFO_DONE] = &AbilityManagerStub::DumpAbilityInfoDoneInner;
    requestFuncMap_[START_EXTENSION_ABILITY] = &AbilityManagerStub::StartExtensionAbilityInner;
    requestFuncMap_[STOP_EXTENSION_ABILITY] = &AbilityManagerStub::StopExtensionAbilityInner;
#ifdef SUPPORT_GRAPHICS
    requestFuncMap_[REGISTER_WMS_HANDLER] = &AbilityManagerStub::RegisterWindowManagerServiceHandlerInner;
    requestFuncMap_[COMPLETEFIRSTFRAMEDRAWING] = &AbilityManagerStub::CompleteFirstFrameDrawingInner;
#endif
}

int AbilityManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AbilityManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilityManagerStub::GetTopAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    AppExecFwk::ElementName result = GetTopAbility();
    if (result.GetDeviceID().empty()) {
        HILOG_DEBUG("GetTopAbilityInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    bool flag = data.ReadBool();
    int32_t result;
    if (flag) {
        result = TerminateAbility(token, resultCode, resultWant);
    } else {
        result = CloseAbility(token, resultCode, resultWant);
    }
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityByCallerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int requestCode = data.ReadInt32();
    int32_t result = TerminateAbilityByCaller(callerToken, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    auto fromUser = data.ReadBool();
    int32_t result = MinimizeAbility(token, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    auto token = data.ReadRemoteObject();
    int32_t result = AttachAbilityThread(scheduler, token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int targetState = data.ReadInt32();
    std::unique_ptr<PacMap> saveData(data.ReadParcelable<PacMap>());
    if (!saveData) {
        HILOG_INFO("save data is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AbilityTransitionDone(token, targetState, *saveData);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        remoteObject = data.ReadRemoteObject();
    }
    int32_t result = ScheduleConnectAbilityDone(token, remoteObject);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleDisconnectAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityResultInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int startId = data.ReadInt32();
    int32_t result = TerminateAbilityResult(token, startId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleCommandAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(new Uri(data.ReadString()));
    bool tryBind = data.ReadBool();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    sptr<IAbilityScheduler> result = AcquireDataAbility(*uri, tryBind, callerToken);
    HILOG_DEBUG("acquire data ability %{public}s", result ? "ok" : "failed");
    if (result) {
        reply.WriteRemoteObject(result->AsObject());
    } else {
        reply.WriteParcelable(nullptr);
    }
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    auto callerToken = data.ReadRemoteObject();
    int32_t result = ReleaseDataAbility(scheduler, callerToken);
    HILOG_DEBUG("release data ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::KillProcessInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int result = KillProcess(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ClearUpApplicationDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int result = ClearUpApplicationData(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("ClearUpApplicationData error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UninstallAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t uid = data.ReadInt32();
    int result = UninstallApp(bundleName, uid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result = StartExtensionAbility(*want, callerToken, userId,
        static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StopExtensionAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    Want* want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result =
        StopExtensionAbility(*want, callerToken, userId, static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }

    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t result = ConnectAbility(*want, callback, token, userId);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    int32_t result = DisconnectAbility(callback);
    HILOG_DEBUG("disconnect ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int32_t result = StopServiceAbility(*want, userId);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::DumpSysStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;

    auto isClient = data.ReadBool();
    auto isUserID = data.ReadBool();
    auto UserID = data.ReadInt32();
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpSysState(args, result, isClient, isUserID, UserID);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpState(args, result);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    AbilityStartSetting *abilityStartSetting = data.ReadParcelable<AbilityStartSetting>();
    if (abilityStartSetting == nullptr) {
        HILOG_ERROR("abilityStartSetting is nullptr");
        delete want;
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *abilityStartSetting, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete abilityStartSetting;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr");
        delete want;
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *startOptions, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete startOptions;
    return NO_ERROR;
}

int AbilityManagerStub::UpdateConfigurationInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppExecFwk::Configuration> config(data.ReadParcelable<AppExecFwk::Configuration>());
    if (config == nullptr) {
        HILOG_ERROR("AbilityManagerStub: config is nullptr");
        return ERR_INVALID_VALUE;
    }
    int result = UpdateConfiguration(*config);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: update configuration failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<WantSenderInfo> wantSenderInfo(data.ReadParcelable<WantSenderInfo>());
    if (wantSenderInfo == nullptr) {
        HILOG_ERROR("wantSenderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    sptr<IWantSender> wantSender = GetWantSender(*wantSenderInfo, callerToken);
    if (!reply.WriteRemoteObject(((wantSender == nullptr) ? nullptr : wantSender->AsObject()))) {
        HILOG_ERROR("failed to reply wantSender instance to client, for write parcel error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<SenderInfo> senderInfo(data.ReadParcelable<SenderInfo>());
    if (senderInfo == nullptr) {
        HILOG_ERROR("senderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendWantSender(wantSender, *senderInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CancelWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    CancelWantSender(wantSender);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t uid = GetPendingWantUid(wantSender);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = GetPendingWantUserId(wantSender);
    reply.WriteInt32(userId);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = GetPendingWantBundleName(wantSender);
    reply.WriteString16(Str8ToStr16(bundleName));
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t code = GetPendingWantCode(wantSender);
    reply.WriteInt32(code);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t type = GetPendingWantType(wantSender);
    reply.WriteInt32(type);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    RegisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    UnregisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    int32_t result = GetPendingRequestWant(wantSender, want);
    if (result != NO_ERROR) {
        HILOG_ERROR("GetPendingRequestWant is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(want.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<WantSenderInfo> info(data.ReadParcelable<WantSenderInfo>());
    int32_t result = GetWantSenderInfo(wantSender, info);
    if (result != NO_ERROR) {
        HILOG_ERROR("GetWantSenderInfo is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(info.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetSystemMemoryAttrInner(MessageParcel &data, MessageParcel &reply)
{
    AppExecFwk::SystemMemoryAttr memoryInfo;
    GetSystemMemoryAttr(memoryInfo);
    reply.WriteParcelable(&memoryInfo);
    return NO_ERROR;
}

int AbilityManagerStub::GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetAppMemorySize();
    HILOG_INFO("GetAppMemorySizeInner result %{public}d", result);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetAppMemorySize error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsRamConstrainedDevice();
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ContinueMissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string srcDeviceId = data.ReadString();
    std::string dstDeviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("ContinueMissionInner callback readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        HILOG_ERROR("ContinueMissionInner wantParams readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, *wantParams);
    HILOG_INFO("ContinueMissionInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::ContinueAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    uint32_t versionCode = data.ReadUint32();
    int32_t result = ContinueAbility(deviceId, missionId, versionCode);
    HILOG_INFO("ContinueAbilityInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::StartContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("StartContinuationInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (abilityToken == nullptr) {
        HILOG_ERROR("Get abilityToken failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t status = data.ReadInt32();
    int32_t result = StartContinuation(*want, abilityToken, status);
    HILOG_INFO("StartContinuationInner result = %{public}d", result);

    return result;
}

int AbilityManagerStub::NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    std::string devId = data.ReadString();
    int32_t sessionId = data.ReadInt32();
    bool isSuccess = data.ReadBool();

    NotifyCompleteContinuation(devId, sessionId, isSuccess);
    HILOG_INFO("NotifyCompleteContinuationInner end");
    return NO_ERROR;
}

int AbilityManagerStub::NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t continuationResult = data.ReadInt32();

    int32_t result = NotifyContinuationResult(missionId, continuationResult);
    HILOG_INFO("StartContinuationInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = LockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = UnlockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: unlock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("stub register mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("stub unregister mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = UnRegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int numMax = data.ReadInt32();
    std::vector<MissionInfo> missionInfos;
    int32_t result = GetMissionInfos(deviceId, numMax, missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto &it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetMissionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    MissionInfo info;
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t missionId = data.ReadInt32();
    int result = GetMissionInfo(deviceId, missionId, info);
    if (!reply.WriteParcelable(&info)) {
        HILOG_ERROR("GetMissionInfo error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetMissionInfo result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int result = CleanMission(missionId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("CleanMission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int result = CleanAllMissions();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("CleanAllMissions failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int result = MoveMissionToFront(missionId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t missionId = GetMissionIdByToken(token);
    if (!reply.WriteInt32(missionId)) {
        HILOG_ERROR("GetMissionIdByToken write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    int result = MoveMissionToFront(missionId, *startOptions);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("AbilityManagerStub::StartAbilityByCallInner begin.");
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t result = StartAbilityByCall(*want, callback, callerToken);

    HILOG_DEBUG("resolve call ability ret = %d", result);

    reply.WriteInt32(result);
    delete want;

    HILOG_DEBUG("AbilityManagerStub::StartAbilityByCallInner end.");

    return NO_ERROR;
}

int AbilityManagerStub::ReleaseInner(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOG_ERROR("callback stub receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ReleaseAbility(callback, *element);

    HILOG_DEBUG("release call ability ret = %d", result);

    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::StartUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    int result = StartUser(userId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StartUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    sptr<IStopUserCallback> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IStopUserCallback>(data.ReadRemoteObject());
    }
    int result = StopUser(userId, callback);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StopUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionLabelInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("SetMissionLabelInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::string label = Str16ToStr8(data.ReadString16());
    int result = SetMissionLabel(token, label);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetMissionLabel failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionIconInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("SetMissionIconInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (!icon) {
        HILOG_ERROR("SetMissionIconInner read icon failed.");
        return ERR_NULL_OBJECT;
    }

    int result = SetMissionIcon(token, icon);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetMissionIcon failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
#else
    HILOG_ERROR("do not support SetMissionIcon.");
    return ERR_INVALID_VALUE;
#endif
}

int AbilityManagerStub::GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRunningInfo> abilityRunningInfos;
    auto result = GetAbilityRunningInfos(abilityRunningInfos);
    reply.WriteInt32(abilityRunningInfos.size());
    for (auto &it : abilityRunningInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    auto upperLimit = data.ReadInt32();
    std::vector<ExtensionRunningInfo> infos;
    auto result = GetExtensionRunningInfos(upperLimit, infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto result = GetProcessRunningInfos(infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    bool fixConflict = data.ReadBool();
    int64_t tag = data.ReadInt64();
    int result = StartSyncRemoteMissions(deviceId, fixConflict, tag);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StartSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int result = StopSyncRemoteMissions(data.ReadString());
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StopSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteMissionListenerInner deviceId empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteMissionListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterMissionListener(deviceId, listener);
    HILOG_INFO("AbilityManagerStub: RegisterRemoteMissionListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOG_ERROR("AbilityManagerStub: UnRegisterRemoteMissionListenerInner deviceId empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: UnRegisterRemoteMissionListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnRegisterMissionListener(deviceId, listener);
    HILOG_INFO("AbilityManagerStub: UnRegisterRemoteMissionListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<ISnapshotHandler> handler = iface_cast<ISnapshotHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        HILOG_ERROR("snapshot: AbilityManagerStub read snapshot handler failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterSnapshotHandler(handler);
    HILOG_INFO("snapshot: AbilityManagerStub register snapshot handler result = %{public}d", result);
    return result;
}

#ifdef SUPPORT_GRAPHICS
int AbilityManagerStub::RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWindowManagerServiceHandler> handler = iface_cast<IWindowManagerServiceHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        HILOG_ERROR("%{public}s read WMS handler failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    return RegisterWindowManagerServiceHandler(handler);
}

int AbilityManagerStub::CompleteFirstFrameDrawingInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<IRemoteObject> abilityToken = nullptr;
    if (data.ReadBool()) {
        HILOG_DEBUG("abilityToken is valid.");
        abilityToken = data.ReadObject<IRemoteObject>();
    }
    CompleteFirstFrameDrawing(abilityToken);
    return 0;
}
#endif

int AbilityManagerStub::GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    MissionSnapshot missionSnapshot;
    int32_t result = GetMissionSnapshot(deviceId, missionId, missionSnapshot);
    HILOG_INFO("snapshot: AbilityManagerStub get snapshot result = %{public}d", result);
    if (!reply.WriteParcelable(&missionSnapshot)) {
        HILOG_ERROR("GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetMissionSnapshot result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AppExecFwk::IAbilityController> controller =
        iface_cast<AppExecFwk::IAbilityController>(data.ReadRemoteObject());
    if (controller == nullptr) {
        HILOG_ERROR("AbilityManagerStub: setAbilityControllerInner controller readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    bool imAStabilityTest = data.ReadBool();
    int32_t result = SetAbilityController(controller, imAStabilityTest);
    HILOG_INFO("AbilityManagerStub: setAbilityControllerInner result = %{public}d", result);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("setAbilityControllerInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsRunningInStabilityTest();
    HILOG_INFO("AbilityManagerStub: IsRunningInStabilityTest result = %{public}d", result);
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("IsRunningInStabilityTest failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    int32_t result = StartUserTest(*want, observer);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::FinishUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int64_t resultCode = data.ReadInt64();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::GetCurrentTopAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token;
    auto result = GetCurrentTopAbility(token);
    if (!reply.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityForeground(token);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityBackground(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityForeground(token, flag);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityBackground(token, flag);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::SendANRProcessIDInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    int32_t result = SendANRProcessID(pid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerStub::ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string abilityName = Str16ToStr8(data.ReadString16());
    std::string state = Str16ToStr8(data.ReadString16());
    int result = ForceTimeoutForTest(abilityName, state);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("force ability timeout error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t abilityReocrdId = data.ReadInt32();
    int32_t result = BlockAbility(abilityReocrdId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAmsServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAmsService();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAppServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAppService();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int AbilityManagerStub::FreeInstallAbilityFromRemoteInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    want->SetParam(FROM_REMOTE_KEY, true);

    auto callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = data.ReadInt32();
    int32_t requestCode = data.ReadInt32();
    int32_t result = FreeInstallAbilityFromRemote(*want, callback, userId, requestCode);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpAbilityInfoDoneInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> infos;
    data.ReadStringVector(&infos);
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    int32_t result = DumpAbilityInfoDone(infos, callerToken);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
