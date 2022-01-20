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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_PROXY_H
#define OHOS_AAFWK_ABILITY_MANAGER_PROXY_H

#include "ability_manager_interface.h"
#include "hilog_wrapper.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityManagerProxy
 * AbilityManagerProxy.
 */
class AbilityManagerProxy : public IRemoteProxy<IAbilityManager> {
public:
    explicit AbilityManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityManager>(impl)
    {}

    virtual ~AbilityManagerProxy()
    {}

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override;

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param callerToken, caller ability token.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode) override;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token) override;

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;

    /**
     * DisconnectAbility, connect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param isKill, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool isKill, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityScheduler, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) override;

    /**
     * AddWindowInfo, add windowToken to AbilityRecord.
     *
     * @param token, the token of the ability.
     * @param windowToken, window id of the ability.
     */
    virtual void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken) override;

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThread(
        const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override;

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override;

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) override;

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
    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId) override;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want) override;

    /**
     * Obtains information about ability stack that are running on the device.
     *
     * @param stackInfo Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAllStackInfo(StackInfo &stackInfo) override;

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
    virtual int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList) override;

    /**
     * Get mission snapshot by mission id
     *
     * @param missionId the id of the mission to retrieve the sAutoapshots
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionSnapshot(const int32_t missionId, MissionPixelMap &missionPixelMap) override;

    /**
     * Ask that the mission associated with a given mission ID be moved to the
     * front of the stack, so it is now visible to the user.
     *
     * @param missionId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToTop(int32_t missionId) override;

    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, bool nonFirst) override;

    /**
     * Remove the specified mission from the stack by missionid
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RemoveMission(int id) override;

    /**
     * Remove the specified mission stack by stack id
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RemoveStack(int id) override;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName) override;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int ClearUpApplicationData(const std::string &bundleName) override;

    /**
     * Uninstall app
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName) override;

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param missionOption, target mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToFloatingStack(const MissionOption &missionOption) override;

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param primary, display primary mission option
     * @param secondary, display secondary mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary) override;

    /**
     * Change the focus of ability in the mission stack.
     * @param lostToken, the token of lost focus ability
     * @param getToken, the token of get focus ability
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ChangeFocusAbility(
        const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken) override;

    /**
     * minimize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeMultiWindow(int missionId) override;

    /**
     * maximize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MaximizeMultiWindow(int missionId) override;

    /**
     * get missions info of floating mission stack.
     * @param list, mission info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetFloatingMissions(std::vector<AbilityMissionInfo> &list) override;

    /**
     * close multiwindow by mission id.
     * @param missionId, the id of target mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseMultiWindow(int missionId) override;

    /**
     * set special mission stack default settings.
     * @param stackSetting, mission stack default settings.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetMissionStackSetting(const StackSetting &stackSetting) override;

    /**
     * @brief Checks whether this ability is the first ability in a mission.
     *
     * @return Returns true is first in Mission.
     */
    virtual bool IsFirstInMission(const sptr<IRemoteObject> &token) override;

    /**
     * Checks whether a specified permission has been granted to the process identified by pid and uid
     *
     * @param permission Indicates the permission to check.
     * @param pid Indicates the ID of the process to check.
     * @param uid Indicates the UID of the process to check.
     * @param message Describe success or failure
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message) override;

    /**
     * Save the top ability States and move them to the background
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PowerOff() override;

    /**
     * Restore the state before top ability poweroff
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PowerOn() override;

    /**
     * Sets the application to start its ability in lock mission mode.
     * @param missionId luck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LockMission(int missionId) override;

    /**
     * Unlocks this ability by exiting the lock mission mode.
     * @param missionId unluck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnlockMission(int missionId) override;

    /**
     * Sets description information about the mission containing this ability.
     *
     * @param description Indicates the object containing information about the
     *                    mission. This parameter cannot be null.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description) override;

    /**
     * get current system mission lock mode state.
     *
     * @return Returns 0: LOCK_MISSION_STATE_NONE, 1: LOCK_MISSION_STATE_LOCKED
     */
    virtual int GetMissionLockModeState() override;

    /**
     * Updates the configuration by modifying the configuration.
     *
     * @param config Indicates the new configuration
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateConfiguration(const AppExecFwk::Configuration &config) override;

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override;

    virtual int SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo) override;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) override;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) override;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) override;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual void UnregisterCancelListener(
        const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override;
    /**
     * set lock screen white list
     *
     * @param isAllow whether to allow startup on lock screen.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetShowOnLockScreen(bool isAllow) override;

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo) override;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId) override;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    virtual int NotifyContinuationResult(int32_t missionId, const int32_t result) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfos) override;

    virtual int CleanMission(int32_t missionId) override;

    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    virtual int StartUser(int userId) override;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;

    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;

    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;

    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;

    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() override;

    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId, MissionSnapshot& snapshot) override;

private:
    template <typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);

private:
    static inline BrokerDelegator<AbilityManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
