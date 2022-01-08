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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_CLIENT_H
#define OHOS_AAFWK_ABILITY_MANAGER_CLIENT_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_interface.h"
#include "ability_manager_interface.h"
#include "want.h"

#include "iremote_object.h"
#include "system_memory_attr.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityManagerClient
 * AbilityManagerClient is used to access ability manager services.
 */
class AbilityManagerClient {
public:
    AbilityManagerClient();
    virtual ~AbilityManagerClient();
    static std::shared_ptr<AbilityManagerClient> GetInstance();

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData);

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token);

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token);

    /**
     * AddWindowInfo, add windowToken to AbilityRecord.
     *
     * @param token, the token of the ability.
     * @param windowToken, window id of the ability.
     */
    void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken);

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want Ability want.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want Ability want.
     * @param callerToken, caller ability token.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(
        const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE);

    /**
     * TerminateAbility with want, return want from ability manager service.
     *
     * @param token Ability token.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant);

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param callerToken, caller ability token.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode TerminateAbility(const sptr<IRemoteObject> &callerToken, int requestCode);

    /**
     * Destroys this Service ability if the number of times it
     * has been started equals the number represented by
     * the given startId.
     *
     * @param token ability's token.
     * @param startId is incremented by 1 every time this ability is started.
     * @return Returns true if the startId matches the number of startup times
     * and this Service ability will be destroyed; returns false otherwise.
     */
    ErrCode TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId);

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MinimizeAbility(const sptr<IRemoteObject> &token);

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken);

    /**
     * DisconnectAbility, disconnect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DisconnectAbility(const sptr<IAbilityConnection> &connect);

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken);

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    ErrCode ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken);

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode DumpState(const std::string &args, std::vector<std::string> &state);

    /**
     * Connect ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();

    /**
     * Get all stack info from ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetAllStackInfo(StackInfo &stackInfo);

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    ErrCode StopServiceAbility(const Want &want);

    /**
     * Get the list of the missions that the user has recently launched,
     * with the most recent being first and older ones after in order.
     *
     * @param recentList recent mission info
     * @param numMax The maximum number of entries to return in the list. The
     * actual number returned may be smaller, depending on how many tasks the
     * user has started and the maximum number the system can remember.
     * @param falgs Information about what to return.  May be any combination
     * of {@link #RECENT_WITH_EXCLUDED} and {@link #RECENT_IGNORE_UNAVAILABLE}.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetRecentMissions(const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList);

    /**
     * Get mission snapshot by mission id
     *
     * @param missionId the id of the mission to retrieve the sAutoapshots
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionSnapshot(const int32_t missionId, MissionSnapshot &missionSnapshot);

    /**
     * Ask that the mission associated with a given mission ID be moved to the
     * front of the stack, so it is now visible to the user.
     *
     * @param missionId.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToTop(int32_t missionId);

    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst);

    /**
     * Remove the specified mission from the stack by missionid
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RemoveMissions(std::vector<int> missionId);

    /**
     * Remove the specified mission stack by stack id
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RemoveStack(int id);

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode KillProcess(const std::string &bundleName);

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    ErrCode ClearUpApplicationData(const std::string &bundleName);

    /**
     * @brief Checks whether this ability is the first ability in a mission.
     *
     * @return Returns true is first in Mission.
     */
    ErrCode IsFirstInMission(const sptr<IRemoteObject> &token);

    ErrCode CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message);

    /**
     * Save the top ability States and move them to the background
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode PowerOff();

    /**
     * Restore the state before top ability poweroff
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode PowerOn();

    /**
     * Sets the application to start its ability in lock mission mode.
     * @param missionId luck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LockMission(int missionId);

    /**
     * Unlocks this ability by exiting the lock mission mode.
     * @param missionId unluck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnlockMission(int missionId);

    /**
     * Sets description information about the mission containing this ability.
     *
     * @param description Indicates the object containing information about the
     *                    mission. This parameter cannot be null.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description);

    /**
     * get current system mission lock mode state.
     *
     * @return Returns 0: LOCK_MISSION_STATE_NONE, 1: LOCK_MISSION_STATE_LOCKED
     */
    int GetMissionLockModeState();

    int UpdateConfiguration(const AppExecFwk::Configuration &config);

    sptr<IWantSender> GetWantSender(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken);

    ErrCode SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo);

    void CancelWantSender(const sptr<IWantSender> &sender);

    ErrCode GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid);

    ErrCode GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId);

    ErrCode GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName);

    ErrCode GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code);

    ErrCode GetPendingWantType(const sptr<IWantSender> &target, int32_t &type);

    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);

    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);

    ErrCode GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want);

    ErrCode GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info);

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param missionOption, target mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToFloatingStack(const MissionOption &missionOption);

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param primary, display primary mission option
     * @param secondary, display secondary mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary);

    /**
     * minimize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MinimizeMultiWindow(int missionId);

    /**
     * maximize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MaximizeMultiWindow(int missionId);

    /**
     * Change the focus of ability in the mission stack.
     * @param lostToken, the token of lost focus ability
     * @param getToken, the token of get focus ability
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ChangeFocusAbility(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken);

    /**
     * get missions info of floating mission stack.
     * @param list, mission info.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetFloatingMissions(std::vector<AbilityMissionInfo> &list);

    /**
     * close multiwindow by mission id.
     * @param missionId, the id of target mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CloseMultiWindow(int missionId);

    /**
     * set special mission stack default settings.
     * @param stackSetting, mission stack default settings.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetMissionStackSetting(const StackSetting &stackSetting);

    /**
     * set lock screen white list
     *
     * @param isAllow whether to allow startup on lock screen.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode SetShowOnLockScreen(bool isAllow);

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo);

    /**
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callback, AAFwk::WantParams &wantParams);

    /**
     * start continuation.
     * @param want, used to start a ability.
     * @param abilityToken, ability token.
     * @param status, continue status.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status);

    /**
     * notify continuation complete to dms.
     * @param deviceId, source device which start a continuation.
     * @param sessionId, represent a continuaion.
     * @param isSuccess, continuation result.
     * @return
     */
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess);

    /**
     * @brief Lock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode LockMissionForCleanup(int32_t missionId);

    /**
     * @brief Unlock specified mission.
     * @param missionId The id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnlockMissionForCleanup(int32_t missionId);

    /**
     * @brief Register mission listener to ams.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(const sptr<IMissionListener> &listener);

    /**
     * @brief UnRegister mission listener from ams.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const sptr<IMissionListener> &listener);

    /**
     * @brief Register mission listener to ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener);

    /**
     * @brief UnRegister mission listener from ability manager service.
     * @param deviceId The remote device Id.
     * @param listener The handler of listener.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener);

    /**
     * @brief Get mission infos from ams.
     * @param deviceId local or remote deviceid.
     * @param numMax max number of missions.
     * @param missionInfos mission info result.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfos(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos);

    /**
     * @brief Get mission info by id.
     * @param deviceId local or remote deviceid.
     * @param missionId Id of target mission.
     * @param missionInfo mision info of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode GetMissionInfo(const std::string& deviceId, int32_t missionId, MissionInfo &missionInfo);

    /**
     * @brief Clean mission by id.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanMission(int32_t missionId);

    /**
     * @brief Clean all missions in system.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode CleanAllMissions();

    /**
     * @brief Move a mission to front.
     * @param missionId Id of target mission.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode MoveMissionToFront(int32_t missionId);

    /**
     * Start synchronizing remote device mission
     * @param devId, deviceId.
     * @param fixConflict, resolve synchronizing conflicts flag.
     * @param tag, call tag.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag);

    /**
     * Stop synchronizing remote device mission
     * @param devId, deviceId.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode StopSyncRemoteMissions(const std::string& devId);
private:
    static std::mutex mutex_;
    static std::shared_ptr<AbilityManagerClient> instance_;
    sptr<IRemoteObject> remoteObject_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_H
