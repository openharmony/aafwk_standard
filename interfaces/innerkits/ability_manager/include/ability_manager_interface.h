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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_INTERFACE_H
#define OHOS_AAFWK_ABILITY_MANAGER_INTERFACE_H

#include <vector>

#include <ipc_types.h>
#include <iremote_broker.h>

#include "ability_connect_callback_interface.h"
#include "ability_scheduler_interface.h"
#include "ability_start_setting.h"
#include "foundation/aafwk/standard/interfaces/innerkits/app_manager/include/appmgr/configuration.h"
#include "mission_snapshot.h"
#include "ability_mission_info.h"
#include "ability_running_info.h"
#include "extension_running_info.h"
#include "mission_option.h"
#include "stack_info.h"
#include "stack_setting.h"
#include "uri.h"
#include "want.h"
#include "want_sender_info.h"
#include "sender_info.h"
#include "want_sender_interface.h"
#include "want_receiver_interface.h"
#include "system_memory_attr.h"
#include "system_memory_attr.h"
#include "mission_listener_interface.h"
#include "mission_info.h"
#include "snapshot.h"
#include "start_options.h"
#include "stop_user_callback.h"
#include "running_process_info.h"
#include "remote_mission_listener_interface.h"
#include "iability_controller.h"

namespace OHOS {
namespace AAFwk {
const std::string ABILITY_MANAGER_SERVICE_NAME = "AbilityManagerService";
const int DEFAULT_INVAL_VALUE = -1;
/**
 * @class IAbilityManager
 * IAbilityManager interface is used to access ability manager services.
 */
class IAbilityManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityManager")

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(const Want &want, const StartOptions &startOptions, const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) = 0;

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param callerToken, caller ability token.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode) = 0;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, the token of the ability to minimize.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token) = 0;

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * DisconnectAbility, disconnect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect) = 0;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param authority, a string to identify a data ability, decoded from uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityScheduler, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * AddWindowInfo, add windowToken to AbilityRecord.
     *
     * @param token, the token of the ability.
     * @param windowToken, window id of the ability.
     */
    virtual void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken) = 0;

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) = 0;

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) = 0;

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) = 0;

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) = 0;

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) = 0;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) = 0;

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
    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId) = 0;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want) = 0;

    /**
     * Obtains information about ability stack that are running on the device.
     *
     * @param stackInfo Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAllStackInfo(StackInfo &stackInfo) = 0;

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
        const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList) = 0;

    /**
     * Get mission snapshot by mission id
     *
     * @param missionId the id of the mission to retrieve the sAutoapshots
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetMissionSnapshot(const int32_t missionId, MissionPixelMap &missionPixelMap) = 0;

    /**
     * Ask that the mission associated with a given mission ID be moved to the
     * front of the stack, so it is now visible to the user.
     *
     * @param missionId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToTop(int32_t missionId) = 0;

    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst) = 0;

    /**
     * Remove the specified mission from the stack by missionid
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RemoveMission(int id) = 0;

    /**
     * Remove the specified mission stack by stack id
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RemoveStack(int id) = 0;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName) = 0;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int ClearUpApplicationData(const std::string &bundleName) = 0;

    /**
     * Uninstall app
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName) = 0;

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param missionOption, target mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToFloatingStack(const MissionOption &missionOption) = 0;

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param primary, display primary mission option
     * @param secondary, display secondary mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary) = 0;

    /**
     * Change the focus of ability in the mission stack.
     * @param lostToken, the token of lost focus ability
     * @param getToken, the token of get focus ability
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ChangeFocusAbility(
        const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken) = 0;

    /**
     * minimize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeMultiWindow(int missionId) = 0;

    /**
     * maximize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MaximizeMultiWindow(int missionId) = 0;

    /**
     * get missions info of floating mission stack.
     * @param list, mission info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetFloatingMissions(std::vector<AbilityMissionInfo> &list) = 0;

    /**
     * close multiwindow by mission id.
     * @param missionId, the id of target mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseMultiWindow(int missionId) = 0;

    /**
     * set special mission stack default settings.
     * @param stackSetting, mission stack default settings.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetMissionStackSetting(const StackSetting &stackSetting) = 0;

    /** Checks whether this ability is the first ability in a mission.
     * @param lostToken, the token of ability
     * @return Returns true is first in Mission.
     */
    virtual bool IsFirstInMission(const sptr<IRemoteObject> &token) = 0;

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
    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message) = 0;

    /**
     * Save the top ability States and move them to the background
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PowerOff() = 0;

    /**
     * Restore the state before top ability poweroff
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PowerOn() = 0;

    /**
     * Sets the application to start its ability in lock mission mode.
     * @param missionId luck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int LockMission(int missionId) = 0;

    /**
     * Unlocks this ability by exiting the lock mission mode.
     * @param missionId unluck mission id
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnlockMission(int missionId) = 0;

    /**
     * Sets description information about the mission containing this ability.
     *
     * @param description Indicates the object containing information about the
     *                    mission. This parameter cannot be null.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description) = 0;

    /**
     * get current system mission lock mode state.
     *
     * @return Returns 0: LOCK_MISSION_STATE_NONE, 1: LOCK_MISSION_STATE_LOCKED
     */
    virtual int GetMissionLockModeState() = 0;

    /**
     * Updates the configuration by modifying the configuration.
     *
     * @param config Indicates the new configuration
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UpdateConfiguration(const AppExecFwk::Configuration &config) = 0;

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) = 0;

    virtual int SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo) = 0;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) = 0;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) = 0;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) = 0;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) = 0;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) = 0;
    /**
     * set lock screen white list
     *
     * @param isAllow whether to allow startup on lock screen.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetShowOnLockScreen(bool isAllow) = 0;

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo) = 0;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) = 0;

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId) = 0;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) = 0;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) = 0;

    virtual int NotifyContinuationResult(int32_t missionId, const int32_t result) = 0;

    virtual int LockMissionForCleanup(int32_t missionId) = 0;

    virtual int UnlockMissionForCleanup(int32_t missionId) = 0;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int GetMissionInfos(
        const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos) = 0;

    virtual int GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo) = 0;

    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId, MissionSnapshot& snapshot) = 0;

    virtual int CleanMission(int32_t missionId) = 0;

    virtual int CleanAllMissions() = 0;

    virtual int MoveMissionToFront(int32_t missionId) = 0;

    virtual int StartUser(int userId) = 0;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) = 0;

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) = 0;

    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) = 0;

    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) = 0;

    /**
     * Start synchronizing remote device mission
     * @param devId, deviceId.
     * @param fixConflict, resolve synchronizing conflicts flag.
     * @param tag, call tag.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag) = 0;

    /**
     * Stop synchronizing remote device mission
     * @param devId, deviceId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopSyncRemoteMissions(const std::string &devId) = 0;

    virtual int RegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener) = 0;

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) = 0;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) = 0;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() = 0;

    /**
     * @brief Register the snapshot handler
     * @param handler snapshot handler
     * @return int Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) = 0;

    enum {
        // ipc id 1-1000 for kit
        // ipc id for terminating ability (1)
        TERMINATE_ABILITY = 1,

        // ipc id for attaching ability thread (2)
        ATTACH_ABILITY_THREAD,

        // ipc id for ability transition done (3)
        ABILITY_TRANSITION_DONE,

        // ipc id for connecting ability done (4)
        CONNECT_ABILITY_DONE,

        // ipc id for disconnecting ability done (5)
        DISCONNECT_ABILITY_DONE,

        // ipc id for add window token (6)
        ADD_WINDOW_INFO,

        // ipc id for terminating ability for result (7)
        TERMINATE_ABILITY_RESULT,

        // ipc id for list stack info (8)
        LIST_STACK_INFO,

        // ipc id for get recent mission (9)
        GET_RECENT_MISSION,

        // ipc id for removing mission (10)
        REMOVE_MISSION,

        // ipc id for removing mission (11)
        REMOVE_STACK,

        // ipc id for removing mission (12)
        COMMAND_ABILITY_DONE,

        // ipc id for get mission snapshot (13)
        GET_MISSION_SNAPSHOT,

        // ipc id for acquire data ability (14)
        ACQUIRE_DATA_ABILITY,

        // ipc id for release data ability (15)
        RELEASE_DATA_ABILITY,

        // ipc id for move mission to top (16)
        MOVE_MISSION_TO_TOP,

        // ipc id for kill process (17)
        KILL_PROCESS,

        // ipc id for uninstall app (18)
        UNINSTALL_APP,

        // ipc id for terminate ability by callerToken and request code (19)
        TERMINATE_ABILITY_BY_CALLER,

        // ipc id for move mission to floating stack (20)
        MOVE_MISSION_TO_FLOATING_STACK,

        // ipc id for move mission to floating stack (21)
        MOVE_MISSION_TO_SPLITSCREEN_STACK,

        // ipc id for change focus ability (22)
        CHANGE_FOCUS_ABILITY,

        // ipc id for Minimize MultiWindow (23)
        MINIMIZE_MULTI_WINDOW,

        // ipc id for Maximize MultiWindow (24)
        MAXIMIZE_MULTI_WINDOW,

        // ipc id for get floating missions (25)
        GET_FLOATING_MISSIONS,

        // ipc id for get floating missions (26)
        CLOSE_MULTI_WINDOW,

        // ipc id for set mission stack setting (27)
        SET_STACK_SETTING,

        // ipc id for isfirstinmission app (28)
        IS_FIRST_IN_MISSION,

        // ipc id for move mission to end (29)
        MOVE_MISSION_TO_END,

        // ipc id for compel verify permission (30)
        COMPEL_VERIFY_PERMISSION,

        // ipc id for power off (31)
        POWER_OFF,

        // ipc id for power off (32)
        POWER_ON,

        // ipc id for luck mission (33)
        LUCK_MISSION,

        // ipc id for unluck mission (34)
        UNLUCK_MISSION,

        // ipc id for set mission info (35)
        SET_MISSION_INFO,

        // ipc id for get mission lock mode state (36)
        GET_MISSION_LOCK_MODE_STATE,

        // ipc id for update configuration (37)
        UPDATE_CONFIGURATION,

        // ipc id for minimize ability (38)
        MINIMIZE_ABILITY,

        // ipc id for lock mission for cleanup operation (39)
        LOCK_MISSION_FOR_CLEANUP,

        // ipc id for unlock mission for cleanup operation (40)
        UNLOCK_MISSION_FOR_CLEANUP,

        // ipc id for register mission listener (41)
        REGISTER_MISSION_LISTENER,

        // ipc id for unregister mission listener (42)
        UNREGISTER_MISSION_LISTENER,

        // ipc id for get mission infos (43)
        GET_MISSION_INFOS,

        // ipc id for get mission info by id (44)
        GET_MISSION_INFO_BY_ID,

        // ipc id for clean mission (45)
        CLEAN_MISSION,

        // ipc id for clean all missions (46)
        CLEAN_ALL_MISSIONS,

        // ipc id for move mission to front (47)
        MOVE_MISSION_TO_FRONT,

        // ipc id for get mission snap shot (48)
        GET_MISSION_SNAPSHOT_BY_ID,

        // ipc id for move mission to front (49)
        START_USER,

        // ipc id for move mission to front (50)
        STOP_USER,

        // ipc id for set ability controller (51)
        SET_ABILITY_CONTROLLER,

        // ipc id for get stability test flag (52)
        IS_USER_A_STABILITY_TEST,

        // ipc id 1001-2000 for DMS
        // ipc id for starting ability (1001)
        START_ABILITY = 1001,

        // ipc id for connecting ability (1002)
        CONNECT_ABILITY,

        // ipc id for disconnecting ability (1003)
        DISCONNECT_ABILITY,

        // ipc id for disconnecting ability (1004)
        STOP_SERVICE_ABILITY,

        // ipc id for starting ability by caller(1005)
        START_ABILITY_ADD_CALLER,

        GET_PENDING_WANT_SENDER,

        SEND_PENDING_WANT_SENDER,

        CANCEL_PENDING_WANT_SENDER,

        GET_PENDING_WANT_UID,

        GET_PENDING_WANT_BUNDLENAME,

        GET_PENDING_WANT_USERID,

        GET_PENDING_WANT_TYPE,

        GET_PENDING_WANT_CODE,

        REGISTER_CANCEL_LISTENER,

        UNREGISTER_CANCEL_LISTENER,

        GET_PENDING_REQUEST_WANT,

        GET_PENDING_WANT_SENDER_INFO,
        SET_SHOW_ON_LOCK_SCREEN,

        // ipc id for starting ability by settings(1018)
        START_ABILITY_FOR_SETTINGS,

        GET_ABILITY_MISSION_SNAPSHOT,

        GET_SYSTEM_MEMORY_ATTR,

        GET_ABILITY_RUNNING_INFO,

        GET_EXTENSION_RUNNING_INFO,

        GET_PROCESS_RUNNING_INFO,

        CLEAR_UP_APPLICATION_DATA,

        START_ABILITY_FOR_OPTIONS,

        // ipc id for continue ability(1101)
        START_CONTINUATION = 1101,

        NOTIFY_CONTINUATION_RESULT = 1102,

        NOTIFY_COMPLETE_CONTINUATION = 1103,

        CONTINUE_ABILITY = 1104,

        CONTINUE_MISSION = 1105,

        // ipc id for mission manager(1110)
        REGISTER_REMOTE_MISSION_LISTENER = 1110,
        UNREGISTER_REMOTE_MISSION_LISTENER = 1111,
        START_SYNC_MISSIONS = 1112,
        STOP_SYNC_MISSIONS = 1113,
        REGISTER_SNAPSHOT_HANDLER = 1114,
        GET_MISSION_SNAPSHOT_INFO = 1115,

        // ipc id 2001-3000 for tools
        // ipc id for dumping state (2001)
        DUMP_STATE = 2001,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_INTERFACE_H
