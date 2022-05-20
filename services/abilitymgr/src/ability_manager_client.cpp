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

#include "ability_manager_client.h"

#include "string_ex.h"
#include "ability_manager_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::instance_ = nullptr;
std::recursive_mutex AbilityManagerClient::mutex_;

#define CHECK_POINTER_RETURN(object)     \
    if (!object) {                       \
        HILOG_ERROR("proxy is nullptr"); \
        return;                          \
    }

#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityManagerClient>();
        }
    }
    return instance_;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ScheduleCommandAbilityDone(token);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, int requestCode, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Start ability come, ability:%{public}s, userId:%{public}d.",
        want.GetElement().GetAbilityName().c_str(), userId);
    return abms->StartAbility(want, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("%{public}s come, abilityName=%{public}s, userId=%{public}d.",
        __func__, want.GetElement().GetAbilityName().c_str(), userId);
    return abms->StartAbility(want, startOptions, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("%{public}s come, bundleName=%{public}s, abilityName=%{public}s, userId=%{public}d.",
        __func__, want.GetElement().GetAbilityName().c_str(), want.GetElement().GetBundleName().c_str(), userId);
    return abms->StartExtensionAbility(want, callerToken, userId, extensionType);
}

ErrCode AbilityManagerClient::StopExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("%{public}s come, bundleName=%{public}s, abilityName=%{public}s, userId=%{public}d.",
        __func__, want.GetElement().GetAbilityName().c_str(), want.GetElement().GetBundleName().c_str(), userId);
    return abms->StopExtensionAbility(want, callerToken, userId, extensionType);
}

ErrCode AbilityManagerClient::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Terminate ability come.");
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::TerminateAbility(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->TerminateAbilityByCaller(callerToken, requestCode);
}

ErrCode AbilityManagerClient::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->TerminateAbilityResult(token, startId);
}

ErrCode AbilityManagerClient::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Close ability come.");
    return abms->CloseAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Minimize ability, fromUser:%{public}d.", fromUser);
    return abms->MinimizeAbility(token, fromUser);
}

ErrCode AbilityManagerClient::ConnectAbility(const Want &want, const sptr<IAbilityConnection> &connect, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Connect ability called, bundleName:%{public}s, abilityName:%{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbility(want, connect, nullptr, userId);
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Connect ability called, bundleName:%{public}s, abilityName:%{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbility(want, connect, callerToken, userId);
}

ErrCode AbilityManagerClient::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("Disconnect ability begin.");
    return abms->DisconnectAbility(connect);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        return nullptr;
    }
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}

ErrCode AbilityManagerClient::DumpState(const std::string &args, std::vector<std::string> &state)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    abms->DumpSysState(args, state, isClient, isUserID, UserID);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get registry.");
        return GET_ABILITY_SERVICE_FAILED;
    }
    sptr<IRemoteObject> remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new AbilityMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("%{public}s :Failed to create AbilityMgrDeathRecipient!", __func__);
        return GET_ABILITY_SERVICE_FAILED;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("%{public}s :Add death recipient to AbilityManagerService failed.", __func__);
        return GET_ABILITY_SERVICE_FAILED;
    }

    proxy_ = iface_cast<IAbilityManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
    return ERR_OK;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want &want)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopServiceAbility(want);
}

ErrCode AbilityManagerClient::KillProcess(const std::string &bundleName)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->KillProcess(bundleName);
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerClient::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ForceTimeoutForTest(abilityName, state);
}
#endif

ErrCode AbilityManagerClient::ClearUpApplicationData(const std::string &bundleName)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ClearUpApplicationData(bundleName);
}

ErrCode AbilityManagerClient::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UpdateConfiguration(config);
}

sptr<IWantSender> AbilityManagerClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        return nullptr;
    }
    return abms->GetWantSender(wantSenderInfo, callerToken);
}

ErrCode AbilityManagerClient::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SendWantSender(target, senderInfo);
}

void AbilityManagerClient::CancelWantSender(const sptr<IWantSender> &sender)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->CancelWantSender(sender);
}

ErrCode AbilityManagerClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    uid = abms->GetPendingWantUid(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    userId = abms->GetPendingWantUserId(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    bundleName = abms->GetPendingWantBundleName(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    code = abms->GetPendingWantCode(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantType(const sptr<IWantSender> &target, int32_t &type)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    type = abms->GetPendingWantType(target);
    type < 0 ? type = 0 : type;
    return ERR_OK;
}

void AbilityManagerClient::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->RegisterCancelListener(sender, recevier);
}

void AbilityManagerClient::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->UnregisterCancelListener(sender, recevier);
}

ErrCode AbilityManagerClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetPendingRequestWant(target, want);
}

ErrCode AbilityManagerClient::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetWantSenderInfo(target, info);
}

void AbilityManagerClient::GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->GetSystemMemoryAttr(memoryInfo);
}

ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        HILOG_ERROR("srcDeviceId or dstDeviceId or callback is null!");
        return ERR_INVALID_VALUE;
    }

    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    return result;
}

ErrCode AbilityManagerClient::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken,
    int32_t status)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->StartContinuation(want, abilityToken, status);
    return result;
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

ErrCode AbilityManagerClient::LockMissionForCleanup(int32_t missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->LockMissionForCleanup(missionId);
}

ErrCode AbilityManagerClient::UnlockMissionForCleanup(int32_t missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnlockMissionForCleanup(missionId);
}

ErrCode AbilityManagerClient::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfos(deviceId, numMax, missionInfos);
}

ErrCode AbilityManagerClient::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfo(deviceId, missionId, missionInfo);
}

ErrCode AbilityManagerClient::CleanMission(int32_t missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanMission(missionId);
}

ErrCode AbilityManagerClient::CleanAllMissions()
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanAllMissions();
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId);
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId, startOptions);
}

ErrCode AbilityManagerClient::GetMissionIdByToken(const sptr<IRemoteObject> &token, int32_t &missionId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    missionId = abms->GetMissionIdByToken(token);
    if (missionId <= 0) {
        HILOG_ERROR("get missionid by token failed!");
        return MISSION_NOT_FOUND;
    }
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("AbilityManagerClient::StartAbilityByCall called.");
    return abms->StartAbilityByCall(want, connect, nullptr);
}

ErrCode AbilityManagerClient::StartAbilityByCall(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_DEBUG("AbilityManagerClient::StartAbilityByCall called.");
    return abms->StartAbilityByCall(want, connect, callToken);
}

ErrCode AbilityManagerClient::ReleaseAbility(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ReleaseAbility(connect, element);
}

ErrCode AbilityManagerClient::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetAbilityRunningInfos(info);
}

ErrCode AbilityManagerClient::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetExtensionRunningInfos(upperLimit, info);
}

ErrCode AbilityManagerClient::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetProcessRunningInfos(info);
}

/**
 * Start synchronizing remote device mission
 * @param devId, deviceId.
 * @param fixConflict, resolve synchronizing conflicts flag.
 * @param tag, call tag.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}
/**
 * Stop synchronizing remote device mission
 * @param devId, deviceId.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string &devId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopSyncRemoteMissions(devId);
}

ErrCode AbilityManagerClient::StartUser(int accountId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartUser(accountId);
}
ErrCode AbilityManagerClient::StopUser(int accountId, const sptr<IStopUserCallback> &callback)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopUser(accountId, callback);
}

ErrCode AbilityManagerClient::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterSnapshotHandler(handler);
}

ErrCode AbilityManagerClient::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionSnapshot(deviceId, missionId, snapshot);
}

ErrCode AbilityManagerClient::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartUserTest(want, observer);
}

ErrCode AbilityManagerClient::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->FinishUserTest(msg, resultCode, bundleName);
}

ErrCode AbilityManagerClient::GetCurrentTopAbility(sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetCurrentTopAbility(token);
}

ErrCode AbilityManagerClient::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DelegatorDoAbilityForeground(token);
}

ErrCode AbilityManagerClient::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DelegatorDoAbilityBackground(token);
}

ErrCode AbilityManagerClient::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string& label)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionLabel(token, label);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::SetMissionIcon(
    const sptr<IRemoteObject> &abilityToken, const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionIcon(abilityToken, icon);
}

ErrCode AbilityManagerClient::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterWindowManagerServiceHandler(handler);
}

void AbilityManagerClient::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN(abms);
    abms->CompleteFirstFrameDrawing(abilityToken);
}
#endif

ErrCode AbilityManagerClient::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DoAbilityForeground(token, flag);
}

ErrCode AbilityManagerClient::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DoAbilityBackground(token, flag);
}

ErrCode AbilityManagerClient::SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetAbilityController(abilityController, imAStabilityTest);
}

ErrCode AbilityManagerClient::SendANRProcessID(int pid)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SendANRProcessID(pid);
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerClient::BlockAmsService()
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAmsService();
}

ErrCode AbilityManagerClient::BlockAbility(int32_t abilityRecordId)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAbility(abilityRecordId);
}

ErrCode AbilityManagerClient::BlockAppService()
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->BlockAppService();
}
#endif

sptr<IAbilityManager> AbilityManagerClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }

    return proxy_;
}

void AbilityManagerClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void AbilityManagerClient::AbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_INFO("AbilityMgrDeathRecipient handle remote died.");
    AbilityManagerClient::GetInstance()->ResetProxy(remote);
}

ErrCode AbilityManagerClient::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->FreeInstallAbilityFromRemote(want, callback, userId, requestCode);
}

AppExecFwk::ElementName AbilityManagerClient::GetTopAbility()
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("[%{public}s] abms == nullptr", __FUNCTION__);
        return {};
    }

    return abms->GetTopAbility();
}

ErrCode AbilityManagerClient::DumpAbilityInfoDone(std::vector<std::string> &infos,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("DumpAbilityInfoDone begin.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->DumpAbilityInfoDone(infos, callerToken);
}
}  // namespace AAFwk
}  // namespace OHOS
