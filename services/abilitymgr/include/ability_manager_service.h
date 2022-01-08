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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_SERVICE_H
#define OHOS_AAFWK_ABILITY_MANAGER_SERVICE_H

#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>

#include "ability_connect_manager.h"
#include "ability_event_handler.h"
#include "ability_manager_stub.h"
#include "ability_stack_manager.h"
#include "app_scheduler.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "data_ability_manager.h"
#include "distributed_sched_interface.h"
#include "distributed_sched_proxy.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "kernal_ability_manager.h"
#include "kernal_system_app_manager.h"
#include "mission_list_manager.h"
#include "system_ability.h"
#include "uri.h"
#include "ability_config.h"
#include "pending_want_manager.h"
#include "ams_configuration_parameter.h"

namespace OHOS {
namespace AAFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class PendingWantManager;
/**
 * @class AbilityManagerService
 * AbilityManagerService provides a facility for managing ability life cycle.
 */
class AbilityManagerService : public SystemAbility,
                              public AbilityManagerStub,
                              public AppStateCallback,
                              public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
    DECLEAR_SYSTEM_ABILITY(AbilityManagerService)
public:
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const;

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
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
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
    virtual int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;

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
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    /**
     * ContinueAbility, continue ability to ability.
     *
     * @param deviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId) override;

    /**
     * StartContinuation, continue ability to remote.
     *
     * @param want, Indicates the ability to start.
     * @param abilityToken, Caller ability token.
     * @param status, continue status.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    /**
     * NotifyCompleteContinuation, notify continuation complete to dms.
     * @param deviceId, source device which start a continuation.
     * @param sessionId, represent a continuaion.
     * @param isSuccess, continuation result.
     * @return
     */
    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    /**
     * NotifyContinuationResult, notify continue result to ability.
     *
     * @param missionId, Caller mission id.
     * @param result, continuation result.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyContinuationResult(int32_t missionId, const int32_t result) override;

    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
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
     * GetEventHandler, get the ability manager service's handler.
     *
     * @return Returns AbilityEventHandler ptr.
     */
    std::shared_ptr<AbilityEventHandler> GetEventHandler();

    /**
     * SetStackManager, set the user id of stack manager.
     *
     * @param userId, user id.
     */
    void SetStackManager(int userId);

    /**
     * GetStackManager, get the current stack manager.
     *
     * @return Returns AbilityStackManager ptr.
     */
    std::shared_ptr<AbilityStackManager> GetStackManager();

    /**
     * DumpWaittingAbilityQueue.
     *
     * @param result, result.
     */
    void DumpWaittingAbilityQueue(std::string &result);

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &info) override;

    /**
     * Obtains information about ability stack that are running on the device.
     *
     * @param stackInfo Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAllStackInfo(StackInfo &stackInfo) override;

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
    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst) override;

    /**
     * Remove the specified mission from the stack by mission id
     *
     * @param missionId.
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
     * @return ERR_OK, return back success, others fail.
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

    /**
     * remove all service record.
     *
     */
    void RemoveAllServiceRecord();

    /**
     * InitMissionListManager, set the user id of mission list manager.
     *
     * @param userId, user id.
     */
    void InitMissionListManager(int userId);

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

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override;

    virtual int CleanMission(int32_t missionId) override;

    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo) override;

    /**
     * get service record by element name.
     *
     */
    std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * wait for starting system ui.
     *
     */
    void StartSystemUi(const std::string name);

    void HandleLoadTimeOut(int64_t eventId);
    void HandleActiveTimeOut(int64_t eventId);
    void HandleInactiveTimeOut(int64_t eventId);
    void HandleForegroundNewTimeOut(int64_t eventId);
    void HandleBackgroundNewTimeOut(int64_t eventId);

    void RestartAbility(const sptr<IRemoteObject> &token);
    void NotifyBmsAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime);

    int StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode, int callerUid = -1);

    int CheckPermission(const std::string &bundleName, const std::string &permission);
    void UpdateLockScreenState(bool isLockScreen);

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration();

    int GetMissionSaveTime() const;

    /**
     * generate ability request.
     *
     */
    int GenerateAbilityRequest(
        const Want &want, int requestCode, AbilityRequest &request, const sptr<IRemoteObject> &callerToken);

    /**
     * Get mission id by target ability token.
     *
     * @param token target ability token.
     * @return the missionId of target mission.
     */
    int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * Get ability token by target mission id.
     *
     * @param missionId target missionId.
     * @return the ability token of target mission.
     */
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId);

    virtual int StartUser(int userId) override;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;

    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t ACTIVE_TIMEOUT_MSG = 1;
    static constexpr uint32_t INACTIVE_TIMEOUT_MSG = 2;
    static constexpr uint32_t BACKGROUND_TIMEOUT_MSG = 3;
    static constexpr uint32_t TERMINATE_TIMEOUT_MSG = 4;
    static constexpr uint32_t FOREGROUNDNEW_TIMEOUT_MSG = 5;
    static constexpr uint32_t BACKGROUNDNEW_TIMEOUT_MSG = 6;

    static constexpr uint32_t LOAD_TIMEOUT = 3000;            // ms
    static constexpr uint32_t ACTIVE_TIMEOUT = 5000;          // ms
    static constexpr uint32_t INACTIVE_TIMEOUT = 500;         // ms
    static constexpr uint32_t BACKGROUND_TIMEOUT = 10000;     // ms
    static constexpr uint32_t TERMINATE_TIMEOUT = 10000;      // ms
    static constexpr uint32_t CONNECT_TIMEOUT = 500;          // ms
    static constexpr uint32_t DISCONNECT_TIMEOUT = 500;       // ms
    static constexpr uint32_t COMMAND_TIMEOUT = 5000;         // ms
    static constexpr uint32_t SYSTEM_UI_TIMEOUT = 5000;       // ms
    static constexpr uint32_t RESTART_TIMEOUT = 5000;         // ms
    static constexpr uint32_t RESTART_ABILITY_TIMEOUT = 500;  // ms
    static constexpr uint32_t FOREGROUNDNEW_TIMEOUT = 5000;   // ms
    static constexpr uint32_t BACKGROUNDNEW_TIMEOUT = 3000;   // ms

    static constexpr uint32_t MIN_DUMP_ARGUMENT_NUM = 2;
    static constexpr uint32_t MAX_WAIT_SYSTEM_UI_NUM = 600;

    enum DumpKey {
        KEY_DUMP_ALL = 0,
        KEY_DUMP_STACK_LIST,
        KEY_DUMP_STACK,
        KEY_DUMP_MISSION,
        KEY_DUMP_TOP_ABILITY,
        KEY_DUMP_WAIT_QUEUE,
        KEY_DUMP_SERVICE,
        KEY_DUMP_DATA,
        KEY_DUMP_SYSTEM_UI,
        KEY_DUMP_FOCUS_ABILITY,
        KEY_DUMP_WINDOW_MODE,
        KEY_DUMP_MISSION_LIST,
        KEY_DUMP_MISSION_INFOS,
    };

    friend class AbilityStackManager;

protected:
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;
    int GetUidByBundleName(std::string bundleName);

    void OnAppStateChanged(const AppInfo &info) override;

private:
    /**
     * initialization of ability manager service.
     *
     */
    bool Init();
    /**
     * starting lanucher ability.
     *
     */
    void StartingLauncherAbility();
    /**
     * starting settings data ability.
     *
     */
    void StartingSettingsDataAbility();
    /**
     * starting phone service ability.
     *
     */
    void StartingPhoneServiceAbility();

    /**
     * starting system ui abilites.
     *
     */
    void StartingSystemUiAbility(const SatrtUiMode &mode);

    /**
     * starting contacts ability.
     *
     */
    void StartingContactsAbility();

    /**
     * starting mms ability.
     *
     */
    void StartingMmsAbility();

    /**
     * connet bms.
     *
     */
    void ConnectBmsService();
    /**
     * get the user id.
     *
     */
    int GetUserId();
    /**
     * Determine whether it is a system APP
     *
     */
    bool IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const;
    /**
     * Select to start the application according to the configuration file of AMS
     *
     */
    void StartSystemApplication();
    /**
     * Get parameters from the global
     *
     */
    void GetGlobalConfiguration();

    sptr<DistributedSchedule::IDistributedSched> GetDmsProxy();
    sptr<AppExecFwk::IBundleMgr> GetBundleManager();
    int StartRemoteAbility(const Want &want, int requestCode);
    int ConnectLocalAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken);
    int DisconnectLocalAbility(const sptr<IAbilityConnection> &connect);
    int ConnectRemoteAbility(const Want &want, const sptr<IRemoteObject> &connect);
    int DisconnectRemoteAbility(const sptr<IRemoteObject> &connect);
    int PreLoadAppDataAbilities(const std::string &bundleName);

    bool CheckIfOperateRemote(const Want &want);
    bool GetLocalDeviceId(std::string& localDeviceId);
    std::string AnonymizeDeviceId(const std::string& deviceId);
    bool VerificationToken(const sptr<IRemoteObject> &token);
    void RequestPermission(const Want *resultWant);

    bool CheckIsRemote(const std::string& deviceId);
    int GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos);
    int GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo);

    void DumpInner(const std::string &args, std::vector<std::string> &info);
    void DumpStackListInner(const std::string &args, std::vector<std::string> &info);
    void DumpStackInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInner(const std::string &args, std::vector<std::string> &info);
    void DumpTopAbilityInner(const std::string &args, std::vector<std::string> &info);
    void DumpWaittingAbilityQueueInner(const std::string &args, std::vector<std::string> &info);
    void DumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DataDumpStateInner(const std::string &args, std::vector<std::string> &info);
    void SystemDumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DumpFocusMapInner(const std::string &args, std::vector<std::string> &info);
    void DumpWindowModeInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionListInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info);
    void DumpFuncInit();
    bool CheckCallerIsSystemAppByIpc();
    bool IsExistFile(const std::string &path);
    using DumpFuncType = void (AbilityManagerService::*)(const std::string &args, std::vector<std::string> &info);
    std::map<uint32_t, DumpFuncType> dumpFuncMap_;

    constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;
    constexpr static int WAITING_BOOT_ANIMATION_TIMER = 5;

    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AbilityEventHandler> handler_;
    ServiceRunningState state_;
    std::unordered_map<int, std::shared_ptr<AbilityStackManager>> stackManagers_;
    std::shared_ptr<AbilityStackManager> currentStackManager_;
    std::shared_ptr<AbilityConnectManager> connectManager_;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::shared_ptr<AppScheduler> appScheduler_;
    std::shared_ptr<DataAbilityManager> dataAbilityManager_;
    std::shared_ptr<PendingWantManager> pendingWantManager_;
    std::shared_ptr<KernalSystemAppManager> systemAppManager_;
    std::shared_ptr<AmsConfigurationParameter> amsConfigResolver_;
    std::shared_ptr<AppExecFwk::Configuration> configuration_;
    const static std::map<std::string, AbilityManagerService::DumpKey> dumpMap;

    // new ams here
    std::unordered_map<int, std::shared_ptr<MissionListManager>> missionListManagers_;
    std::shared_ptr<MissionListManager> currentMissionListManager_;
    std::shared_ptr<KernalAbilityManager> kernalAbilityManager_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_SERVICE_H
