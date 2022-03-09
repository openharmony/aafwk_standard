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

#include "ability_manager_client.h"

#include "string_ex.h"
#include "ability_manager_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "shared_memory.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::instance_ = nullptr;
std::mutex AbilityManagerClient::mutex_;

#define CHECK_REMOTE_OBJECT(object)                        \
    if (!object) {                                         \
        if (ERR_OK != Connect()) {                         \
            HILOG_ERROR("ability service can't connect."); \
            return;                                        \
        }                                                  \
    }

#define CHECK_REMOTE_OBJECT_AND_RETURN(object, value)      \
    if (!object) {                                         \
        if (ERR_OK != Connect()) {                         \
            HILOG_ERROR("ability service can't connect."); \
            return value;                                  \
        }                                                  \
    }

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
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
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ScheduleCommandAbilityDone(token);
}

#ifdef SUPPORT_GRAPHICS
void AbilityManagerClient::AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
{
    CHECK_REMOTE_OBJECT(remoteObject_);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->AddWindowInfo(token, windowToken);
}
#endif

ErrCode AbilityManagerClient::StartAbility(const Want &want, int requestCode, int32_t userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    HILOG_INFO("%{public}s called, bundleName=%{public}s, abilityName=%{public}s, userId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartAbility(want, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    HILOG_INFO("%{public}s called, bundleName=%{public}s, abilityName=%{public}s",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
    HILOG_INFO("%{public}s called, userId=%{public}d, windowMode_=%{public}d, displayId_=%{public}d",
        __func__, userId, startOptions.GetWindowMode(), startOptions.GetDisplayID());
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartAbility(want, startOptions, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::TerminateAbility(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->TerminateAbilityByCaller(callerToken, requestCode);
}

ErrCode AbilityManagerClient::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->TerminateAbilityResult(token, startId);
}

ErrCode AbilityManagerClient::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->CloseAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    HILOG_INFO("%{public}s called, fromUser=%{public}d", __func__, fromUser);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MinimizeAbility(token, fromUser);
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    HILOG_INFO("%{public}s called, bundleName=%{public}s, abilityName=%{public}s, userId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ConnectAbility(want, connect, callerToken, userId);
}

ErrCode AbilityManagerClient::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DisconnectAbility(connect);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, nullptr);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}

ErrCode AbilityManagerClient::DumpState(const std::string &args, std::vector<std::string> &state)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->DumpSysState(args, state, isClient, isUserID, UserID);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObject_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get registry.");
        return GET_ABILITY_SERVICE_FAILED;
    }
    remoteObject_ = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return GET_ABILITY_SERVICE_FAILED;
    }
    HILOG_DEBUG("Connect ability manager service success.");
    return ERR_OK;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::GetAllStackInfo(StackInfo &stackInfo)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetAllStackInfo(stackInfo);
}
#endif

ErrCode AbilityManagerClient::StopServiceAbility(const Want &want)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StopServiceAbility(want);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::GetRecentMissions(
    const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetRecentMissions(numMax, flags, recentList);
}

ErrCode AbilityManagerClient::GetMissionSnapshot(const int32_t missionId, MissionSnapshot &missionSnapshot)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    MissionPixelMap missionPixelMap;
    int ret = abms->GetMissionSnapshot(missionId, missionPixelMap);
    if (ret == ERR_OK) {
        HILOG_INFO("missionPixelMap.imageInfo.shmKey: %{public}d", missionPixelMap.imageInfo.shmKey);
        if (missionPixelMap.imageInfo.size == 0) {
            HILOG_INFO("size is 0.");
            return -1;
        }
        void *data = SharedMemory::PopSharedMemory(missionPixelMap.imageInfo.shmKey, missionPixelMap.imageInfo.size);
        if (!data) {
            HILOG_INFO("SharedMemory::PopSharedMemory return value is nullptr.");
            return -1;
        }
        Media::InitializationOptions mediaOption;
        mediaOption.size.width = missionPixelMap.imageInfo.width;
        mediaOption.size.height = missionPixelMap.imageInfo.height;
        mediaOption.pixelFormat = Media::PixelFormat::BGRA_8888;
        mediaOption.editable = true;
        auto pixel = Media::PixelMap::Create((const uint32_t *)data,
            missionPixelMap.imageInfo.size / sizeof(uint32_t), mediaOption);
        if (!pixel) {
            HILOG_INFO(" Media::PixelMap::Create return value is nullptr.");
            return -1;
        }
        HILOG_INFO("width = [%{public}d]", pixel->GetWidth());
        HILOG_INFO("height = [%{public}d]", pixel->GetHeight());
        HILOG_INFO("size = [%{public}d]", missionPixelMap.imageInfo.size);
        missionSnapshot.topAbility = missionPixelMap.topAbility;
        missionSnapshot.snapshot = std::move(pixel);
    }
    return ret;
}

ErrCode AbilityManagerClient::MoveMissionToTop(int32_t missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToTop(missionId);
}

ErrCode AbilityManagerClient::MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToEnd(token, nonFirst);
}

ErrCode AbilityManagerClient::RemoveMissions(std::vector<int> missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    int error = ERR_OK;
    for (auto it : missionId) {
        error = abms->RemoveMission(it);
        if (error != ERR_OK) {
            HILOG_ERROR("Failed, error:%{private}d", error);
            break;
        }
    }

    return error;
}

ErrCode AbilityManagerClient::RemoveStack(int id)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->RemoveStack(id);
}
#endif

ErrCode AbilityManagerClient::KillProcess(const std::string &bundleName)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->KillProcess(bundleName);
}

ErrCode AbilityManagerClient::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ForceTimeoutForTest(abilityName, state);
}

ErrCode AbilityManagerClient::ClearUpApplicationData(const std::string &bundleName)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ClearUpApplicationData(bundleName);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::IsFirstInMission(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_ERROR("token is nullptr");
        return ERR_NULL_OBJECT;
    }
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    if (!(abms->IsFirstInMission(token))) {
        return NO_FIRST_IN_MISSION;
    }
    return ERR_OK;
}
#endif

ErrCode AbilityManagerClient::CompelVerifyPermission(
    const std::string &permission, int pid, int uid, std::string &message)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->CompelVerifyPermission(permission, pid, uid, message);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::MoveMissionToFloatingStack(const MissionOption &missionOption)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToFloatingStack(missionOption);
}

ErrCode AbilityManagerClient::MoveMissionToSplitScreenStack(
    const MissionOption &primary, const MissionOption &secondary)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToSplitScreenStack(primary, secondary);
}

ErrCode AbilityManagerClient::MinimizeMultiWindow(int missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MinimizeMultiWindow(missionId);
}

ErrCode AbilityManagerClient::MaximizeMultiWindow(int missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MaximizeMultiWindow(missionId);
}
#endif

ErrCode AbilityManagerClient::ChangeFocusAbility(
    const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ChangeFocusAbility(lostFocusToken, getFocusToken);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::GetFloatingMissions(std::vector<AbilityMissionInfo> &list)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetFloatingMissions(list);
}

ErrCode AbilityManagerClient::CloseMultiWindow(int missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->CloseMultiWindow(missionId);
}

ErrCode AbilityManagerClient::SetMissionStackSetting(const StackSetting &stackSetting)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SetMissionStackSetting(stackSetting);
}

ErrCode AbilityManagerClient::PowerOff()
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->PowerOff();
}

ErrCode AbilityManagerClient::PowerOn()
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->PowerOn();
}

ErrCode AbilityManagerClient::LockMission(int missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->LockMission(missionId);
}

ErrCode AbilityManagerClient::UnlockMission(int missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->UnlockMission(missionId);
}

ErrCode AbilityManagerClient::SetMissionDescriptionInfo(
    const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SetMissionDescriptionInfo(token, description);
}

ErrCode AbilityManagerClient::GetMissionLockModeState()
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetMissionLockModeState();
}
#endif

ErrCode AbilityManagerClient::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->UpdateConfiguration(config);
}

sptr<IWantSender> AbilityManagerClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, nullptr);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetWantSender(wantSenderInfo, callerToken);
}

ErrCode AbilityManagerClient::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SendWantSender(target, senderInfo);
}

void AbilityManagerClient::CancelWantSender(const sptr<IWantSender> &sender)
{
    CHECK_REMOTE_OBJECT(remoteObject_);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->CancelWantSender(sender);
}

ErrCode AbilityManagerClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    uid = abms->GetPendingWantUid(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    userId = abms->GetPendingWantUserId(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    bundleName = abms->GetPendingWantBundleName(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    code = abms->GetPendingWantCode(target);
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetPendingWantType(const sptr<IWantSender> &target, int32_t &type)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    type = abms->GetPendingWantType(target);
    type < 0 ? type = 0 : type;
    return ERR_OK;
}

void AbilityManagerClient::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    CHECK_REMOTE_OBJECT(remoteObject_);
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->RegisterCancelListener(sender, recevier);
}

void AbilityManagerClient::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    CHECK_REMOTE_OBJECT(remoteObject_);
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->UnregisterCancelListener(sender, recevier);
}

ErrCode AbilityManagerClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetPendingRequestWant(target, want);
}

ErrCode AbilityManagerClient::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetWantSenderInfo(target, info);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::SetShowOnLockScreen(bool isAllow)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SetShowOnLockScreen(isAllow);
}
#endif

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AbilityManagerClient::GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo)
{
    CHECK_REMOTE_OBJECT(remoteObject_);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->GetSystemMemoryAttr(memoryInfo);
    return;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        HILOG_ERROR("srcDeviceId or dstDeviceId or callback is null!");
        return ERR_INVALID_VALUE;
    }
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    int result = abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    return result;
}
#endif

ErrCode AbilityManagerClient::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken,
    int32_t status)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    int result = abms->StartContinuation(want, abilityToken, status);
    return result;
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    CHECK_REMOTE_OBJECT(remoteObject_);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    abms->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::LockMissionForCleanup(int32_t missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->LockMissionForCleanup(missionId);
}

ErrCode AbilityManagerClient::UnlockMissionForCleanup(int32_t missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->UnlockMissionForCleanup(missionId);
}

ErrCode AbilityManagerClient::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->RegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->UnRegisterMissionListener(listener);
}

ErrCode AbilityManagerClient::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, REGISTER_REMOTE_MISSION_LISTENER_FAIL);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->RegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, UNREGISTER_REMOTE_MISSION_LISTENER_FAIL);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->UnRegisterMissionListener(deviceId, listener);
}

ErrCode AbilityManagerClient::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetMissionInfos(deviceId, numMax, missionInfos);
}

ErrCode AbilityManagerClient::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetMissionInfo(deviceId, missionId, missionInfo);
}

ErrCode AbilityManagerClient::CleanMission(int32_t missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->CleanMission(missionId);
}

ErrCode AbilityManagerClient::CleanAllMissions()
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->CleanAllMissions();
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToFront(missionId);
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->MoveMissionToFront(missionId, startOptions);
}

ErrCode AbilityManagerClient::GetMissionIdByToken(const sptr<IRemoteObject> &token, int32_t &missionId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    if (!abms) {
        HILOG_ERROR("ability manager service connect failed!");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    missionId = abms->GetMissionIdByToken(token);
    if (missionId < 0) {
        HILOG_ERROR("get missionid by token failed!");
        return -1;
    }
    return ERR_OK;
}
#endif

ErrCode AbilityManagerClient::StartAbilityByCall(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callToken)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    HILOG_DEBUG("AbilityManagerClient::StartAbilityByCall called.");
    return abms->StartAbilityByCall(want, connect, callToken);
}

ErrCode AbilityManagerClient::ReleaseAbility(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->ReleaseAbility(connect, element);
}

ErrCode AbilityManagerClient::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetAbilityRunningInfos(info);
}

ErrCode AbilityManagerClient::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetExtensionRunningInfos(upperLimit, info);
}

ErrCode AbilityManagerClient::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetProcessRunningInfos(info);
}

#ifdef SUPPORT_GRAPHICS
/**
 * Start synchronizing remote device mission
 * @param devId, deviceId.
 * @param fixConflict, resolve synchronizing conflicts flag.
 * @param tag, call tag.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}
/**
 * Stop synchronizing remote device mission
 * @param devId, deviceId.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string &devId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StopSyncRemoteMissions(devId);
}
#endif

ErrCode AbilityManagerClient::StartUser(int accountId)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartUser(accountId);
}
ErrCode AbilityManagerClient::StopUser(int accountId, const sptr<IStopUserCallback> &callback)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StopUser(accountId, callback);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->RegisterSnapshotHandler(handler);
}

ErrCode AbilityManagerClient::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetMissionSnapshot(deviceId, missionId, snapshot);
}
#endif

ErrCode AbilityManagerClient::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->StartUserTest(want, observer);
}

ErrCode AbilityManagerClient::FinishUserTest(
    const std::string &msg, const int &resultCode, const std::string &bundleName)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->FinishUserTest(msg, resultCode, bundleName);
}

ErrCode AbilityManagerClient::GetCurrentTopAbility(sptr<IRemoteObject> &token)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->GetCurrentTopAbility(token);
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityManagerClient::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DelegatorDoAbilityForeground(token);
}

ErrCode AbilityManagerClient::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DelegatorDoAbilityBackground(token);
}

ErrCode AbilityManagerClient::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string& label)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);
    auto abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SetMissionLabel(token, label);
}

ErrCode AbilityManagerClient::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DoAbilityForeground(token, flag);
}

ErrCode AbilityManagerClient::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->DoAbilityBackground(token, flag);
}
#endif

ErrCode AbilityManagerClient::SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SetAbilityController(abilityController, imAStabilityTest);
}

ErrCode AbilityManagerClient::SendANRProcessID(int pid)
{
    CHECK_REMOTE_OBJECT_AND_RETURN(remoteObject_, ABILITY_SERVICE_NOT_CONNECTED);

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(remoteObject_);
    return abms->SendANRProcessID(pid);
}
}  // namespace AAFwk
}  // namespace AAFwk
