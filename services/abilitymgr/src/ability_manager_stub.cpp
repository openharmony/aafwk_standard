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
    requestFuncMap_[TERMINATE_ABILITY] = &AbilityManagerStub::TerminateAbilityInner;
    requestFuncMap_[TERMINATE_ABILITY_BY_CALLER] = &AbilityManagerStub::TerminateAbilityByCallerInner;
    requestFuncMap_[ATTACH_ABILITY_THREAD] = &AbilityManagerStub::AttachAbilityThreadInner;
    requestFuncMap_[ABILITY_TRANSITION_DONE] = &AbilityManagerStub::AbilityTransitionDoneInner;
    requestFuncMap_[CONNECT_ABILITY_DONE] = &AbilityManagerStub::ScheduleConnectAbilityDoneInner;
    requestFuncMap_[DISCONNECT_ABILITY_DONE] = &AbilityManagerStub::ScheduleDisconnectAbilityDoneInner;
    requestFuncMap_[ADD_WINDOW_INFO] = &AbilityManagerStub::AddWindowInfoInner;
    requestFuncMap_[TERMINATE_ABILITY_RESULT] = &AbilityManagerStub::TerminateAbilityResultInner;
    requestFuncMap_[LIST_STACK_INFO] = &AbilityManagerStub::GetAllStackInfoInner;
    requestFuncMap_[GET_RECENT_MISSION] = &AbilityManagerStub::GetRecentMissionsInner;
    requestFuncMap_[REMOVE_MISSION] = &AbilityManagerStub::RemoveMissionInner;
    requestFuncMap_[REMOVE_STACK] = &AbilityManagerStub::RemoveStackInner;
    requestFuncMap_[COMMAND_ABILITY_DONE] = &AbilityManagerStub::ScheduleCommandAbilityDoneInner;
    requestFuncMap_[GET_MISSION_SNAPSHOT] = &AbilityManagerStub::GetMissionSnapshotInner;
    requestFuncMap_[ACQUIRE_DATA_ABILITY] = &AbilityManagerStub::AcquireDataAbilityInner;
    requestFuncMap_[RELEASE_DATA_ABILITY] = &AbilityManagerStub::ReleaseDataAbilityInner;
    requestFuncMap_[MOVE_MISSION_TO_TOP] = &AbilityManagerStub::MoveMissionToTopInner;
    requestFuncMap_[MOVE_MISSION_TO_END] = &AbilityManagerStub::MoveMissionToEndInner;
    requestFuncMap_[KILL_PROCESS] = &AbilityManagerStub::KillProcessInner;
    requestFuncMap_[UNINSTALL_APP] = &AbilityManagerStub::UninstallAppInner;
    requestFuncMap_[START_ABILITY] = &AbilityManagerStub::StartAbilityInner;
    requestFuncMap_[START_ABILITY_ADD_CALLER] = &AbilityManagerStub::StartAbilityAddCallerInner;
    requestFuncMap_[CONNECT_ABILITY] = &AbilityManagerStub::ConnectAbilityInner;
    requestFuncMap_[DISCONNECT_ABILITY] = &AbilityManagerStub::DisconnectAbilityInner;
    requestFuncMap_[STOP_SERVICE_ABILITY] = &AbilityManagerStub::StopServiceAbilityInner;
    requestFuncMap_[DUMP_STATE] = &AbilityManagerStub::DumpStateInner;
    requestFuncMap_[IS_FIRST_IN_MISSION] = &AbilityManagerStub::IsFirstInMissionInner;
    requestFuncMap_[COMPEL_VERIFY_PERMISSION] = &AbilityManagerStub::CompelVerifyPermissionInner;
    requestFuncMap_[POWER_OFF] = &AbilityManagerStub::PowerOffInner;
    requestFuncMap_[POWER_ON] = &AbilityManagerStub::PowerOnInner;
    requestFuncMap_[LUCK_MISSION] = &AbilityManagerStub::LockMissionInner;
    requestFuncMap_[UNLUCK_MISSION] = &AbilityManagerStub::UnlockMissionInner;
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
    requestFuncMap_[SET_MISSION_INFO] = &AbilityManagerStub::SetMissionDescriptionInfoInner;
    requestFuncMap_[GET_MISSION_LOCK_MODE_STATE] = &AbilityManagerStub::GetMissionLockModeStateInner;
}

AbilityManagerStub::~AbilityManagerStub()
{
    requestFuncMap_.clear();
}

int AbilityManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("AbilityManagerStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
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
    HILOG_WARN("AbilityManagerStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilityManagerStub::TerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadParcelable<IRemoteObject>();
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    int32_t result = TerminateAbility(token, resultCode, resultWant);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityByCallerInner(MessageParcel &data, MessageParcel &reply)
{
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int requestCode = data.ReadInt32();
    int32_t result = TerminateAbility(callerToken, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadParcelable<IRemoteObject>());
    auto token = data.ReadParcelable<IRemoteObject>();
    int32_t result = AttachAbilityThread(scheduler, token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    int targetState = data.ReadInt32();
    int32_t result = AbilityTransitionDone(token, targetState);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    auto remoteObject = data.ReadParcelable<IRemoteObject>();
    int32_t result = ScheduleConnectAbilityDone(token, remoteObject);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    int32_t result = ScheduleDisconnectAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AddWindowInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadParcelable<IRemoteObject>();
    int windowToken = data.ReadInt32();
    AddWindowInfo(token, windowToken);
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityResultInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadParcelable<IRemoteObject>();
    int startId = data.ReadInt32();
    int32_t result = TerminateAbilityResult(token, startId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetAllStackInfoInner(MessageParcel &data, MessageParcel &reply)
{
    StackInfo stackInfo;
    int32_t result = GetAllStackInfo(stackInfo);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: GetAllStackInfo result error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&stackInfo)) {
        HILOG_ERROR("AbilityManagerStub: GetAllStackInfo error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetRecentMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int numMax = data.ReadInt32();
    int flags = data.ReadInt32();
    std::vector<AbilityMissionInfo> missionInfos;
    int32_t result = GetRecentMissions(numMax, flags, missionInfos);
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

int AbilityManagerStub::RemoveMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int id = data.ReadInt32();
    int32_t result = RemoveMission(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: remove mission error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RemoveStackInner(MessageParcel &data, MessageParcel &reply)
{
    int id = data.ReadInt32();
    int32_t result = RemoveStack(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    int32_t result = ScheduleCommandAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionSnapshotInner(MessageParcel &data, MessageParcel &reply)
{
    MissionSnapshotInfo snapshot;
    int32_t missionId = data.ReadInt32();
    int32_t result = GetMissionSnapshot(missionId, snapshot);
    if (!reply.WriteParcelable(&snapshot)) {
        HILOG_ERROR("AbilityManagerStub: GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: GetMissionSnapshot result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(new Uri(data.ReadString()));
    bool tryBind = data.ReadBool();
    sptr<IRemoteObject> callerToken = data.ReadParcelable<IRemoteObject>();
    sptr<IAbilityScheduler> result = AcquireDataAbility(*uri, tryBind, callerToken);
    HILOG_DEBUG("acquire data ability %{public}s", result ? "ok" : "failed");
    if (result) {
        reply.WriteParcelable(result->AsObject());
    } else {
        reply.WriteParcelable(nullptr);
    }
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadParcelable<IRemoteObject>());
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int32_t result = ReleaseDataAbility(scheduler, callerToken);
    HILOG_DEBUG("release data ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToTopInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = MoveMissionToTop(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: move mission to top error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToEndInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    auto nonFirst = data.ReadBool();
    int result = MoveMissionToEnd(token, nonFirst);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: move mission to top error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::KillProcessInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int result = KillProcess(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UninstallAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int result = UninstallApp(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilityManagerStub: want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilityManagerStub: want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilityManagerStub: want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callback = iface_cast<IAbilityConnection>(data.ReadParcelable<IRemoteObject>());
    auto token = data.ReadParcelable<IRemoteObject>();
    int32_t result = ConnectAbility(*want, callback, token);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<IAbilityConnection>(data.ReadParcelable<IRemoteObject>());
    int32_t result = DisconnectAbility(callback);
    HILOG_DEBUG("disconnect ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilityManagerStub: want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = StopServiceAbility(*want);
    reply.WriteInt32(result);
    delete want;
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

int AbilityManagerStub::IsFirstInMissionInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    auto result = IsFirstInMission(token);
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("%{public}s: error", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CompelVerifyPermissionInner(MessageParcel &data, MessageParcel &reply)
{
    auto permission = Str16ToStr8(data.ReadString16());
    auto pid = data.ReadInt32();
    auto uid = data.ReadInt32();
    std::string message;
    auto result = CompelVerifyPermission(permission, pid, uid, message);
    reply.WriteString16(Str8ToStr16(message));
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::PowerOffInner(MessageParcel &data, MessageParcel &reply)
{
    auto result = PowerOff();
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::PowerOnInner(MessageParcel &data, MessageParcel &reply)
{
    auto result = PowerOn();
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::LockMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = LockMission(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UnlockMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = UnlockMission(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionDescriptionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    auto missionInfo = data.ReadParcelable<MissionDescriptionInfo>();
    int result = SetMissionDescriptionInfo(token, *missionInfo);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: set mission info failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionLockModeStateInner(MessageParcel &data, MessageParcel &reply)
{
    int result = GetMissionLockModeState();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: get mission lock mode state failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    WantSenderInfo *wantSenderInfo = data.ReadParcelable<WantSenderInfo>();
    if (wantSenderInfo == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSenderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = data.ReadParcelable<IRemoteObject>();
    sptr<IWantSender> wantSender = GetWantSender(*wantSenderInfo, callerToken);
    if (!reply.WriteParcelable(wantSender->AsObject())) {
        HILOG_ERROR("failed to reply wantSender instance to client, for write parcel error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    SenderInfo *senderInfo = data.ReadParcelable<SenderInfo>();
    if (senderInfo == nullptr) {
        HILOG_ERROR("AbilityManagerStub: senderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendWantSender(wantSender, *senderInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CancelWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    CancelWantSender(wantSender);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t uid = GetPendingWantUid(wantSender);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = GetPendingWantUserId(wantSender);
    reply.WriteInt32(userId);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = GetPendingWantBundleName(wantSender);
    reply.WriteString16(Str8ToStr16(bundleName));
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t code = GetPendingWantCode(wantSender);
    reply.WriteInt32(code);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t type = GetPendingWantType(wantSender);
    reply.WriteInt32(type);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (sender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadParcelable<IRemoteObject>());
    if (receiver == nullptr) {
        HILOG_ERROR("AbilityManagerStub: receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    RegisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (sender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadParcelable<IRemoteObject>());
    if (receiver == nullptr) {
        HILOG_ERROR("AbilityManagerStub: receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    UnregisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    int32_t result = GetPendingRequestWant(wantSender, want);
    if (result != NO_ERROR) {
        HILOG_ERROR("AbilityManagerStub: GetPendingRequestWant is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(want.get());
    return NO_ERROR;
}

}  // namespace AAFwk
}  // namespace OHOS
