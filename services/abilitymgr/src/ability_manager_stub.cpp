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
    FirstStepInit();
    SecondStepInit();
}

AbilityManagerStub::~AbilityManagerStub()
{
    requestFuncMap_.clear();
}

void AbilityManagerStub::FirstStepInit()
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
    requestFuncMap_[START_ABILITY_FOR_SETTINGS] = &AbilityManagerStub::StartAbilityForSettingsInner;
    requestFuncMap_[MOVE_MISSION_TO_FLOATING_STACK] = &AbilityManagerStub::MoveMissionToFloatingStackInner;
    requestFuncMap_[MOVE_MISSION_TO_SPLITSCREEN_STACK] = &AbilityManagerStub::MoveMissionToSplitScreenStackInner;
    requestFuncMap_[CHANGE_FOCUS_ABILITY] = &AbilityManagerStub::ChangeFocusAbilityInner;
    requestFuncMap_[MINIMIZE_MULTI_WINDOW] = &AbilityManagerStub::MinimizeMultiWindowInner;
}

void AbilityManagerStub::SecondStepInit()
{
    requestFuncMap_[MAXIMIZE_MULTI_WINDOW] = &AbilityManagerStub::MaximizeMultiWindowInner;
    requestFuncMap_[GET_FLOATING_MISSIONS] = &AbilityManagerStub::GetFloatingMissionsInner;
    requestFuncMap_[CLOSE_MULTI_WINDOW] = &AbilityManagerStub::CloseMultiWindowInner;
    requestFuncMap_[SET_STACK_SETTING] = &AbilityManagerStub::SetMissionStackSettingInner;
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
    requestFuncMap_[GET_PENDING_WANT_SENDER_INFO] = &AbilityManagerStub::GetPendingRequestWantInner;
    requestFuncMap_[SET_MISSION_INFO] = &AbilityManagerStub::SetMissionDescriptionInfoInner;
    requestFuncMap_[GET_MISSION_LOCK_MODE_STATE] = &AbilityManagerStub::GetMissionLockModeStateInner;
    requestFuncMap_[UPDATE_CONFIGURATION] = &AbilityManagerStub::UpdateConfigurationInner;
    requestFuncMap_[START_ABILITY_AND_REQUESTUID] = &AbilityManagerStub::StartAbilityAddRequestUidInner;
    requestFuncMap_[SET_SHOW_ON_LOCK_SCREEN] = &AbilityManagerStub::SetShowOnLockScreenInner;
    requestFuncMap_[GET_SYSTEM_MEMORY_ATTR] = &AbilityManagerStub::GetSystemMemoryAttrInner;
}

int AbilityManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
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
    int32_t result = TerminateAbilityByCaller(callerToken, requestCode);
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
        HILOG_ERROR("GetAllStackInfo result error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&stackInfo)) {
        HILOG_ERROR("GetAllStackInfo error");
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
        HILOG_ERROR("remove mission error.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RemoveStackInner(MessageParcel &data, MessageParcel &reply)
{
    int id = data.ReadInt32();
    int32_t result = RemoveStack(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("remove stack error");
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
    MissionPixelMap missionPixelMap;
    int32_t missionId = data.ReadInt32();
    int32_t result = GetMissionSnapshot(missionId, missionPixelMap);
    if (!reply.WriteParcelable(&missionPixelMap)) {
        HILOG_ERROR("GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetMissionSnapshot result error");
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
        HILOG_ERROR("move mission to top error");
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
        HILOG_ERROR("move mission to top error");
        return ERR_INVALID_VALUE;
    }
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

int AbilityManagerStub::UninstallAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int uid = data.ReadInt32();
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
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAddRequestUidInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int32_t requestCode = data.ReadInt32();
    int32_t requestUid = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, requestCode, requestUid);
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
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto token = data.ReadParcelable<IRemoteObject>();
    int32_t result = StopServiceAbility(*want, token);
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
    auto callerToken = data.ReadParcelable<IRemoteObject>();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *abilityStartSetting, callerToken, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete abilityStartSetting;
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFloatingStackInner(MessageParcel &data, MessageParcel &reply)
{
    MissionOption *missionOption = data.ReadParcelable<MissionOption>();
    if (missionOption == nullptr) {
        HILOG_ERROR("missionOption is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto result = MoveMissionToFloatingStack(*missionOption);
    reply.WriteInt32(result);
    delete missionOption;
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToSplitScreenStackInner(MessageParcel &data, MessageParcel &reply)
{
    MissionOption *primary = data.ReadParcelable<MissionOption>();
    if (primary == nullptr) {
        HILOG_ERROR("missionOption is nullptr");
        return ERR_INVALID_VALUE;
    }
    MissionOption *secondary = data.ReadParcelable<MissionOption>();
    if (secondary == nullptr) {
        HILOG_ERROR("missionOption is nullptr");
        delete primary;
        return ERR_INVALID_VALUE;
    }
    auto result = MoveMissionToSplitScreenStack(*primary, *secondary);
    reply.WriteInt32(result);
    delete primary;
    delete secondary;
    return NO_ERROR;
}

int AbilityManagerStub::ChangeFocusAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto loseToken = data.ReadParcelable<IRemoteObject>();
    auto getToken = data.ReadParcelable<IRemoteObject>();
    auto result = ChangeFocusAbility(loseToken, getToken);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeMultiWindowInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    auto result = MinimizeMultiWindow(missionId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MaximizeMultiWindowInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    auto result = MaximizeMultiWindow(missionId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetFloatingMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityMissionInfo> missionInfos;
    auto result = GetFloatingMissions(missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto &it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CloseMultiWindowInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    auto result = CloseMultiWindow(missionId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionStackSettingInner(MessageParcel &data, MessageParcel &reply)
{
    StackSetting *stackSetting = data.ReadParcelable<StackSetting>();
    if (stackSetting == nullptr) {
        HILOG_ERROR("stackSetting is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto result = SetMissionStackSetting(*stackSetting);
    reply.WriteInt32(result);
    delete stackSetting;
    return NO_ERROR;
}

int AbilityManagerStub::IsFirstInMissionInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadParcelable<IRemoteObject>();
    auto result = IsFirstInMission(token);
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("reply write failed.");
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
    std::unique_ptr<MissionDescriptionInfo> missionInfo(data.ReadParcelable<MissionDescriptionInfo>());
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
    WantSenderInfo *wantSenderInfo = data.ReadParcelable<WantSenderInfo>();
    if (wantSenderInfo == nullptr) {
        HILOG_ERROR("wantSenderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = data.ReadParcelable<IRemoteObject>();
    sptr<IWantSender> wantSender = GetWantSender(*wantSenderInfo, callerToken);
    if (!reply.WriteParcelable((wantSender == nullptr ? nullptr : wantSender->AsObject()))) {
        HILOG_ERROR("failed to reply wantSender instance to client, for write parcel error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    SenderInfo *senderInfo = data.ReadParcelable<SenderInfo>();
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    CancelWantSender(wantSender);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadParcelable<IRemoteObject>());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    RegisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadParcelable<IRemoteObject>());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    UnregisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadParcelable<IRemoteObject>());
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

int AbilityManagerStub::SetShowOnLockScreenInner(MessageParcel &data, MessageParcel &reply)
{
    auto isAllow = data.ReadBool();
    int result = SetShowOnLockScreen(isAllow);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetShowOnLockScreen error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetSystemMemoryAttrInner(MessageParcel &data, MessageParcel &reply)
{
    AppExecFwk::SystemMemoryAttr memoryInfo;
    GetSystemMemoryAttr(memoryInfo);
    reply.WriteParcelable(&memoryInfo);
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
