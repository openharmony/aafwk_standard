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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H

#include <list>
#include <queue>
#include <memory>

#include "ability_running_info.h"
#include "foundation/distributedhardware/devicemanager/interfaces/inner_kits/native_cpp/include/device_manager.h"
#include "mission_list.h"
#include "mission_listener_controller.h"
#include "mission_info.h"
#include "mission_snapshot.h"
#include "snapshot.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MissionListManager : public std::enable_shared_from_this<MissionListManager> {
public:
    explicit MissionListManager(int userId);
    ~MissionListManager();

    /**
     * init ability mission manager.
     *
     */
    void Init();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(const AbilityRequest &abilityRequest);

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser);

    int RegisterMissionListener(const sptr<IMissionListener> &listener);

    int UnRegisterMissionListener(const sptr<IMissionListener> &listener);

    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos);

    int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo);

    int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr);

    int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher,
        std::shared_ptr<StartOptions> startOptions = nullptr);

    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    void OnAppStateChanged(const AppInfo &info);

    /**
     * attach ability thread ipc object.
     *
     * @param scheduler, ability thread ipc object.
     * @param token, the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * push waitting ability to queue.
     *
     * @param abilityRequest, the request of ability.
     */
    void EnqueueWaittingAbility(const AbilityRequest &abilityRequest);

    /**
     * push front waitting ability to queue.
     *
     * @param abilityRequest, the request of ability.
     */
    void EnqueueWaittingAbilityToFront(const AbilityRequest &abilityRequest);

    /**
     * start waitting ability.
     */
    void StartWaittingAbility();

    /**
     * @brief Get the Ability Record By Token object
     *
     * @param token the search token
     * @return std::shared_ptr<AbilityRecord> the AbilityRecord of the token
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;

    /**
     * @brief Get the Mission By Id object
     *
     * @param missionId the given missionId
     * @return the mission of the given id
     */
    std::shared_ptr<Mission> GetMissionById(int missionId) const;

    /**
     * @brief Terminate ability with the given abilityRecord
     *
     * @param abilityRecord the ability to terminate
     * @param resultCode the terminate data
     * @param resultWant the terminate data
     * @param flag mark terminate flag
     * @return int error code
     */
    int TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag);

    /**
     * @brief Terminate ability with caller
     *
     * @param caller the ability which start the ability
     * @param requestCode which ability to terminate
     * @return int error code
     */
    int TerminateAbility(const std::shared_ptr<AbilityRecord> &caller, int requestCode);

    /**
     * @brief remove the mission from the mission list
     *
     * @param abilityRecord the ability need to remove
     * @param flag mark is terminate or close
     */
    void RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag);

    /**
     * @brief remove the mission list from the mission list manager
     *
     * @param MissionList the mission list need to remove
     */
    void RemoveMissionList(const std::shared_ptr<MissionList> &MissionList);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param state the ability state
     * @param saveData the saved data
     * @return execute error code
     */
    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData);

    /**
     * @brief search the ability from terminating list
     *
     * @param token the ability token
     * @return the ability need to terminate
     */
    std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject> &token);

    /**
     * @brief clear the mission with the given id
     *
     * @param missionId the mission need to delete
     * @return int error code
     */
    int ClearMission(int missionId);

    /**
     * @brief clear all the missions
     *
     * @return int error code
     */
    int ClearAllMissions();

    void ClearAllMissionsLocked(std::list<std::shared_ptr<Mission>> &missionList,
        std::list<std::shared_ptr<Mission>> &foregroundAbilities, bool searchActive);

    /**
     * @brief Set the Mission Locked State object
     *
     * @param missionId the id of the mission
     * @return int error code
     */
    int SetMissionLockedState(int missionId, bool lockedState);

    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * @brief handle time out event
     *
     * @param msgId the msg id in ability record
     * @param eventId the event id in ability record
     */
    void OnTimeOut(uint32_t msgId, int64_t eventId);

    /**
     * @brief handle when ability died
     *
     * @param abilityRecord the died ability
     */
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId);

    /**
     * @brief handle when call contection died
     *
     * @param callRecord the died call contection
     */
    void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord);

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

    /**
     * Set mission label of this ability.
     *
     * @param abilityToken target ability token.
     * @param label target label.
     * @return Retun 0 if success.
     */
    int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label);

    /**
     * @brief dump all abilities
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string>& info);

    /**
     * @brief dump mission list
     *
     * @param info dump result.
     */
    void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    /**
     * @brief dump mission list by id with params
     *
     * @param info dump result.
     * @param params dump params.
     */
    void DumpMissionListByRecordId(
        std::vector<std::string>& info, bool isClient, int32_t abilityRecordId, const std::vector<std::string>& params);

    /**
     * @brief dump mission by id
     *
     * @param info dump result.
     */
    void DumpMission(int missionId, std::vector<std::string> &info);

    /**
     * @brief dump mission infos
     *
     * @param info dump result.
     */
    void DumpMissionInfos(std::vector<std::string> &info);

    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag);

    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want);
    /**
     * resolve the call ipc of ability for schudeling oncall.
     *
     * @param abilityRequest, target ability request.
     */
    int ResolveLocked(const AbilityRequest &abilityRequest);

    /**
     * release the connection of this call.
     *
     * @param connect, caller callback ipc.
     * @param element, target ability name.
     */
    int ReleaseLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element);
    /**
     * @brief register snapshotHandler
     * @param handler the snapshotHandler
     */
    void RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler);

    /**
     * @brief Get the Mission Snapshot object
     * @param missionId mission id
     * @param abilityToken abilityToken to get current mission snapshot
     * @param missionSnapshot result of snapshot
     * @return Returns true on success, false on failure.
     */
    bool GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
        MissionSnapshot& missionSnapshot);
    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);

    /**
     * @brief get current top ability by bundle name
     * @param bundleName the bundle name
     * @return the current top ability.
     */
    std::shared_ptr<AbilityRecord> GetCurrentTopAbility(const std::string &bundleName);

    void UninstallApp(const std::string &bundleName, int32_t uid);

    bool IsStarted();
    void PauseManager();
    void ResumeManager();
private:
    int StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    int StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    int MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser);
    std::shared_ptr<AbilityRecord> GetCurrentTopAbilityLocked() const;
    std::shared_ptr<MissionList> GetTargetMissionList(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionList> GetTargetMissionListByLauncher(const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionList> GetTargetMissionListByDefault(
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<Mission> GetReusedMission(const AbilityRequest &abilityRequest);
    void GetTargetMissionAndAbility(const AbilityRequest &abilityRequest,
        std::shared_ptr<Mission> &targetMission,
        std::shared_ptr<AbilityRecord> &targetRecord);
    void MoveMissionToTargetList(bool isCallFromLauncher,
        const std::shared_ptr<MissionList> &targetMissionList,
        const std::shared_ptr<Mission> &mission);
    void MoveMissionListToTop(const std::shared_ptr<MissionList> &missionList);
    void MoveNoneTopMissionToDefaultList(const std::shared_ptr<Mission> &mission);
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId);

    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminateAndUpdateMission(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool RemoveMissionList(const std::list<std::shared_ptr<MissionList>> lists,
        const std::shared_ptr<MissionList> &list);
    int ClearMissionLocked(int missionId, std::shared_ptr<Mission> mission);
    int TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag);
    std::shared_ptr<AbilityRecord> GetAbilityRecordByEventId(int64_t eventId) const;
    std::shared_ptr<AbilityRecord> GetAbilityRecordByCaller(
        const std::shared_ptr<AbilityRecord> &caller, int requestCode);
    std::shared_ptr<MissionList> GetTargetMissionList(int missionId, std::shared_ptr<Mission> &mission);
    void UpdateMissionTimeStamp(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void PostStartWaittingAbility();
    void HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);
    void HandleLauncherDied(std::shared_ptr<AbilityRecord> ability);
    void HandleAbilityDiedByDefault(std::shared_ptr<AbilityRecord> abilityRecord);
    void DelayedStartLauncher();
    void BackToLauncher();
    void GetAllForegroundAbilities(std::list<std::shared_ptr<AbilityRecord>>& foregroundList);
    void GetForegroundAbilities(const std::shared_ptr<MissionList>& missionList,
        std::list<std::shared_ptr<AbilityRecord>>& foregroundList);
    std::shared_ptr<Mission> GetMissionBySpecifiedFlag(const std::string &flag) const;
    
    // handle timeout event
    void HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void HandleForgroundNewTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &ability);
    void MoveToTerminateList(const std::shared_ptr<AbilityRecord> &ability);
    void DelayedResumeTimeout(const std::shared_ptr<AbilityRecord> &callerAbility);
    void BackToCaller(const std::shared_ptr<AbilityRecord> &callerAbility);

    // new version for call inner function.
    int ResolveAbility(const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest);
    std::shared_ptr<AbilityRecord> GetAbilityRecordByName(const AppExecFwk::ElementName &element);
    int CallAbilityLocked(const AbilityRequest &abilityRequest);
    void UpdateMissionSnapshot(const std::shared_ptr<AbilityRecord>& abilityRecord);

    void AddUninstallTags(const std::string &bundleName, int32_t uid);

private:
    int userId_;
    std::recursive_mutex managerLock_;
    // launcher list is also in currentMissionLists_
    std::list<std::shared_ptr<MissionList>> currentMissionLists_;
    // only manager the ability of standard in the default list
    std::shared_ptr<MissionList> defaultStandardList_;
    // only manager the ability of singleton in the default list for the fast search
    std::shared_ptr<MissionList> defaultSingleList_;
    std::shared_ptr<MissionList> launcherList_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;

    std::queue<AbilityRequest> waittingAbilityQueue_;
    std::shared_ptr<MissionListenerController> listenerController_;
    bool isPC_ = false;

private:
class MissionDmInitCallback : public DistributedHardware::DmInitCallback {
public:
    void OnRemoteDied() override;

public:
    static bool isInit_;
};
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
