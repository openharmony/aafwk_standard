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

#include "ability_manager_proxy.h"

#include "errors.h"
#include "string_ex.h"

#include "ability_connect_callback_proxy.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_proxy.h"
#include "ability_scheduler_stub.h"

namespace OHOS {
namespace AAFwk {

bool AbilityManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

int AbilityManagerProxy::StartAbility(const Want &want, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("%{public}s fail, want write parcelable error", __func__);
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("%{public}s fail, requestCode write int32 error", __func__);
        return INNER_ERR;
    }

    error = Remote()->SendRequest(IAbilityManager::START_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("start ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("%{public}s fail, want write parcelable error", __func__);
        return INNER_ERR;
    }
    if (!data.WriteParcelable(callerToken)) {
        HILOG_ERROR("%{public}s fail, callerToken write parcelable error", __func__);
        return INNER_ERR;
    }
    if (!data.WriteInt32(requestCode)) {
        HILOG_ERROR("%{public}s fail, requestCode write int32 error", __func__);
        return INNER_ERR;
    }

    error = Remote()->SendRequest(IAbilityManager::START_ABILITY_ADD_CALLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("start ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token) || !data.WriteInt32(resultCode) || !data.WriteParcelable(resultWant)) {
        HILOG_ERROR("terminate ability. fail to write to parcel.");
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::TERMINATE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("terminate ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(callerToken) || !data.WriteInt32(requestCode)) {
        HILOG_ERROR("%{public}s. fail to write to parcel.", __func__);
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::TERMINATE_ABILITY_BY_CALLER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("terminate ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("connect ability fail, want error");
        return ERR_INVALID_VALUE;
    }
    if (connect == nullptr) {
        HILOG_ERROR("connect ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(connect->AsObject())) {
        HILOG_ERROR("connect ability fail, connect error");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(callerToken)) {
        HILOG_ERROR("connect ability fail, callerToken error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IAbilityManager::CONNECT_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("connect ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (connect == nullptr) {
        HILOG_ERROR("disconnect ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(connect->AsObject())) {
        HILOG_ERROR("disconnect ability fail, connect error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IAbilityManager::DISCONNECT_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("disconnect ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

sptr<IAbilityScheduler> AbilityManagerProxy::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!callerToken) {
        HILOG_ERROR("invalid parameters for acquire data ability.");
        return nullptr;
    }
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteString(uri.ToString()) || !data.WriteBool(tryBind) || !data.WriteParcelable(callerToken)) {
        HILOG_ERROR("failed to mashalling the acquires data ability.");
        return nullptr;
    }
    error = Remote()->SendRequest(IAbilityManager::ACQUIRE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed to send acquire data ability request, error: %{public}d", error);
        return nullptr;
    }

    return iface_cast<IAbilityScheduler>(reply.ReadParcelable<IRemoteObject>());
}

int AbilityManagerProxy::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!dataAbilityScheduler || !callerToken) {
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(dataAbilityScheduler->AsObject()) || !data.WriteParcelable(callerToken)) {
        HILOG_ERROR("failed to mashalling the release data ability.");
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::RELEASE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed to send release data ability request, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (scheduler == nullptr) {
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(scheduler->AsObject()) || !data.WriteParcelable(token)) {
        HILOG_ERROR("attach ability. fail to write to parcel.");
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::ATTACH_ABILITY_THREAD, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("attach ability fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token) || !data.WriteInt32(state)) {
        HILOG_ERROR("ability transaction done. fail to write to parcel.");
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::ABILITY_TRANSITION_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("ability transaction done fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token)) {
        HILOG_ERROR("schedule connect done fail, token error");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteParcelable(remoteObject)) {
        HILOG_ERROR("schedule connect done fail, remoteObject error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IAbilityManager::CONNECT_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("schedule connect done fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token)) {
        HILOG_ERROR("schedule disconnect done fail, token error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IAbilityManager::DISCONNECT_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("schedule disconnect done fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token)) {
        HILOG_ERROR("schedule command done fail, token error");
        return ERR_INVALID_VALUE;
    }

    error = Remote()->SendRequest(IAbilityManager::COMMAND_ABILITY_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("schedule command done fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(token) || !data.WriteInt32(windowToken)) {
        HILOG_ERROR("add window info write to parce fail.");
        return;
    }
    error = Remote()->SendRequest(ADD_WINDOW_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("add window info fail, error: %d", error);
    }
}

void AbilityManagerProxy::DumpState(const std::string &args, std::vector<std::string> &state)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteString16(Str8ToStr16(args));
    error = Remote()->SendRequest(IAbilityManager::DUMP_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("AbilityManagerProxy: SendRequest err %d", error);
        return;
    }
    int32_t stackNum = reply.ReadInt32();
    for (int i = 0; i < stackNum; i++) {
        std::string stac = Str16ToStr8(reply.ReadString16());
        state.emplace_back(stac);
    }
}

int AbilityManagerProxy::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token) || !data.WriteInt32(startId)) {
        HILOG_ERROR("terminate ability for result fail");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::TERMINATE_ABILITY_RESULT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("terminate ability for result fail, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::StopServiceAbility(const Want &want)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("stop service ability. fail to write to parcel.");
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::STOP_SERVICE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("stop service ability, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetAllStackInfo(StackInfo &stackInfo)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("GetAllStackInfo: remote is nullptr");
        return ERR_UNKNOWN_OBJECT;
    }
    error = remote->SendRequest(IAbilityManager::LIST_STACK_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetAllStackInfo: SendRequest err %{public}d", error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    if (result != ERR_OK) {
        HILOG_ERROR("GetAllStackInfo: ReadInt32 err %{public}d", result);
        return result;
    }
    std::unique_ptr<StackInfo> info(reply.ReadParcelable<StackInfo>());
    if (!info) {
        HILOG_ERROR("readParcelableInfo<StackInfo> failed");
        return ERR_UNKNOWN_OBJECT;
    }
    stackInfo = *info;
    return result;
}

template <typename T>
int AbilityManagerProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            HILOG_ERROR("Read Parcelable infos failed");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    HILOG_INFO("get parcelable infos success");
    return NO_ERROR;
}

int AbilityManagerProxy::GetRecentMissions(
    const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(numMax)) {
        HILOG_ERROR("get recent missions by numMax , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(flags)) {
        HILOG_ERROR("get recent missions by flags , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::GET_RECENT_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("get recent mission fail, error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<AbilityMissionInfo>(reply, recentList);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetMissionSnapshot(const int32_t missionId, MissionSnapshotInfo &snapshot)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("get recent missions by missionId , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::GET_MISSION_SNAPSHOT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("get mission snapshot fail, error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshotInfo> info(reply.ReadParcelable<MissionSnapshotInfo>());
    if (!info) {
        HILOG_ERROR("readParcelableInfo<MissionSnapshotInfo> failed");
        return ERR_UNKNOWN_OBJECT;
    }
    snapshot = *info;
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionToTop(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("move mission to top by missionId , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::MOVE_MISSION_TO_TOP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("move mission to top fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token) || data.WriteBool(nonFirst)) {
        HILOG_ERROR("move mission to end failed");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::MOVE_MISSION_TO_END, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("move mission to end fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RemoveMission(int id)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(id)) {
        HILOG_ERROR("remove mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::REMOVE_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("remove mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::RemoveStack(int id)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(id)) {
        HILOG_ERROR("remove stack by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::REMOVE_STACK, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("remove stack by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::KillProcess(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        HILOG_ERROR("%{public}s , WriteString16 fail.", __func__);
        return ERR_INVALID_VALUE;
    }
    int error = Remote()->SendRequest(IAbilityManager::KILL_PROCESS, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%s, error: %d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UninstallApp(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        HILOG_ERROR("%{public}s , WriteString16 fail.", __func__);
        return ERR_INVALID_VALUE;
    }
    int error = Remote()->SendRequest(IAbilityManager::UNINSTALL_APP, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%s, error: %d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

bool AbilityManagerProxy::IsFirstInMission(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token)) {
        HILOG_ERROR("%{public}s for result fail", __func__);
        return false;
    }
    auto error = Remote()->SendRequest(IAbilityManager::IS_FIRST_IN_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s for result fail, error: %d", __func__, error);
        return false;
    }
    return reply.ReadBool();
}

int AbilityManagerProxy::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(permission)) || !data.WriteInt32(pid) || !data.WriteInt32(uid)) {
        HILOG_ERROR("%{public}s, write failed", __func__);
        return INNER_ERR;
    }
    auto error = Remote()->SendRequest(IAbilityManager::COMPEL_VERIFY_PERMISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return error;
    }
    message = Str16ToStr8(reply.ReadString16());
    return reply.ReadInt32();
}

int AbilityManagerProxy::PowerOff()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    auto error = Remote()->SendRequest(IAbilityManager::POWER_OFF, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::PowerOn()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    auto error = Remote()->SendRequest(IAbilityManager::POWER_ON, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::LockMission(int missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("lock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::LUCK_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("lock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::UnlockMission(int missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("unlock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    error = Remote()->SendRequest(IAbilityManager::UNLUCK_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("unlock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::SetMissionDescriptionInfo(
    const sptr<IRemoteObject> &token, const MissionDescriptionInfo &missionDescriptionInfo)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(token)) {
        HILOG_ERROR("%{public}s for result fail", __func__);
        return false;
    }
    if (!data.WriteParcelable(&missionDescriptionInfo)) {
        HILOG_ERROR("%{public}s for result fail", __func__);
        return false;
    }
    error = Remote()->SendRequest(IAbilityManager::SET_MISSION_INFO, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("unlock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetMissionLockModeState()
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    error = Remote()->SendRequest(IAbilityManager::GET_MISSION_LOCK_MODE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("get mission luck mode state , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

sptr<IWantSender> AbilityManagerProxy::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteParcelable(&wantSenderInfo)) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return nullptr;
    }
    if (!data.WriteParcelable(callerToken)) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return nullptr;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return nullptr;
    }
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(reply.ReadParcelable<IRemoteObject>());
    if (!wantSender) {
        return nullptr;
    }
    return wantSender;
}

int AbilityManagerProxy::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return INNER_ERR;
    }

    auto error = Remote()->SendRequest(IAbilityManager::SEND_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::CancelWantSender(const sptr<IWantSender> &sender)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteParcelable(sender->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return;
    }
    // data.WriteString16(Str8ToStr16(bundleName));
    // data.WriteInt32(userId);
    auto error = Remote()->SendRequest(IAbilityManager::CANCEL_PENDING_WANT_SENDER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return;
    }
}

int AbilityManagerProxy::GetPendingWantUid(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return ERR_INVALID_VALUE;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_UID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return ERR_INVALID_VALUE;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_USERID, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

std::string AbilityManagerProxy::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return "";
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return "";
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_BUNDLENAME, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return "";
    }
    return Str16ToStr8(reply.ReadString16());
}

int AbilityManagerProxy::GetPendingWantCode(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return ERR_INVALID_VALUE;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_CODE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int AbilityManagerProxy::GetPendingWantType(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return ERR_INVALID_VALUE;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_WANT_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

void AbilityManagerProxy::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteParcelable(sender->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return;
    }
    if (receiver == nullptr || !data.WriteParcelable(receiver->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return;
    }
    auto error = Remote()->SendRequest(IAbilityManager::REGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return;
    }
}

void AbilityManagerProxy::UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (sender == nullptr || !data.WriteParcelable(sender->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return;
    }
    if (receiver == nullptr || !data.WriteParcelable(receiver->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return;
    }
    auto error = Remote()->SendRequest(IAbilityManager::UNREGISTER_CANCEL_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return;
    }
}

int AbilityManagerProxy::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (target == nullptr || !data.WriteParcelable(target->AsObject())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return INNER_ERR;
    }
    if (want == nullptr || !data.WriteParcelable(want.get())) {
        HILOG_ERROR("%{public}s WriteParcelable", __func__);
        return INNER_ERR;
    }
    auto error = Remote()->SendRequest(IAbilityManager::GET_PENDING_REQUEST_WANT, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("%{public}s send request error, error: %d", __func__, error);
        return error;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        HILOG_ERROR("readParcelableInfo failed");
        return INNER_ERR;
    }
    want = std::move(wantInfo);

    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
