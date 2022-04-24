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
#include "ability_running_info.h"
#include "extension_running_info.h"
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
const int DELAY_LOCAL_FREE_INSTALL_TIMEOUT = 40000;
const int DELAY_REMOTE_FREE_INSTALL_TIMEOUT = 30000 + DELAY_LOCAL_FREE_INSTALL_TIMEOUT;
const std::string FROM_REMOTE_KEY = "freeInstallFromRemote";
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
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int GetAppMemorySize()
    {
        return 0;
    }

    virtual bool IsRamConstrainedDevice()
    {
        return false;
    }

    virtual AppExecFwk::ElementName GetTopAbility()
    {
        return {};
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
     * CloseAbility, close the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) = 0;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, the token of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) = 0;

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) = 0;

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
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID) = 0;

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
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName) = 0;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * force timeout ability.
     *
     * @param abilityName.
     * @param state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) = 0;
    #endif

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
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName, int32_t uid) = 0;

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
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo) = 0;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) = 0;

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) = 0;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) = 0;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) = 0;

    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) = 0;

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

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) = 0;

	/**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) = 0;

    /**
     * Release Ability, disconnect session with common ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseAbility(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) = 0;

    virtual int StartUser(int userId) = 0;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) = 0;

    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) = 0;

#ifdef SUPPORT_GRAPHICS
    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;
#endif

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

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) = 0;

    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) = 0;

    /**
     * GetCurrentTopAbility, get the token of current top ability.
     *
     * @param token, the token of current top ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetCurrentTopAbility(sptr<IRemoteObject> &token) = 0;

    /**
     * The delegator calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) = 0;

    /**
     * The delegator calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) = 0;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    /**
     * Send not response process ID to ability manager service.
     *
     * @param pid The not response process ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendANRProcessID(int pid) = 0;

    /**
     * Get mission id by ability token.
     *
     * @param token The token of ability.
     * @return Returns -1 if do not find mission, otherwise return mission id.
     */
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) = 0;

    virtual int RegisterWindowHandler(const sptr<IWindowHandler>& handler) = 0;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAmsService() = 0;

    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAbility(int32_t abilityRecordId) = 0;

    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() = 0;
    #endif

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    /**
     * Call free install from remote.
     *
     * @param want, the want of the ability to start.
     * @param callback, Callback from remote.
     * @param userId, Designation User ID.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

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

        // ipc id for set mission label (53)
        SET_MISSION_LABEL,

        // ipc id for ability foreground (54)
        DO_ABILITY_FOREGROUND,

        // ipc id for ability background (55)
        DO_ABILITY_BACKGROUND,

        // ipc id for move mission to front by options (56)
        MOVE_MISSION_TO_FRONT_BY_OPTIONS,

        // ipc for get mission id by ability token (57)
        GET_MISSION_ID_BY_ABILITY_TOKEN,

        // ipc id for set mission icon (58)
        SET_MISSION_ICON,

        // dump ability info done (59)
        DUMP_ABILITY_INFO_DONE,

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

        SEND_APP_NOT_RESPONSE_PROCESS_ID,

        // ipc id for starting ability by settings(1018)
        START_ABILITY_FOR_SETTINGS,

        GET_ABILITY_MISSION_SNAPSHOT,

        GET_SYSTEM_MEMORY_ATTR,

        GET_APP_MEMORY_SIZE,

        IS_RAM_CONSTRAINED_DEVICE,

        GET_ABILITY_RUNNING_INFO,

        GET_EXTENSION_RUNNING_INFO,

        GET_PROCESS_RUNNING_INFO,

        CLEAR_UP_APPLICATION_DATA,

        START_ABILITY_FOR_OPTIONS,

        BLOCK_AMS_SERVICE,

        BLOCK_ABILITY,

        BLOCK_APP_SERVICE,

        // ipc id for call ability
        START_CALL_ABILITY,

        RELEASE_CALL_ABILITY,

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

        // ipc id for user test(1120)
        START_USER_TEST = 1120,
        FINISH_USER_TEST = 1121,
        DELEGATOR_DO_ABILITY_FOREGROUND = 1122,
        DELEGATOR_DO_ABILITY_BACKGROUND = 1123,
        GET_CURRENT_TOP_ABILITY         = 1124,

        // ipc id 2001-3000 for tools
        // ipc id for dumping state (2001)
        DUMP_STATE = 2001,
        DUMPSYS_STATE = 2002,
        FORCE_TIMEOUT,

        REGISTER_WINDOW_HANDLER = 2500,

        GET_TOP_ABILITY = 3000,
        FREE_INSTALL_ABILITY_FROM_REMOTE = 3001,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_INTERFACE_H
