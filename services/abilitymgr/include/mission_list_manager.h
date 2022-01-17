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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_H

#include <list>
#include <queue>
#include <memory>

#include "foundation/distributedhardware/devicemanager/interfaces/inner_kits/native_cpp/include/device_manager.h"
#include "mission_list.h"
#include "mission_listener_controller.h"
#include "mission_info.h"
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
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeAbility(const sptr<IRemoteObject> &token);

    int RegisterMissionListener(const sptr<IMissionListener> &listener);

    int UnRegisterMissionListener(const sptr<IMissionListener> &listener);

    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos);

    int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo);

    int MoveMissionToFront(int32_t missionId);

    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

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
     * @return int error code
     */
    int TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant);

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
     */
    void RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

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
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

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
     * @brief dump all abilities
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string> &info);

    /**
     * @brief dump mission list
     *
     * @param info dump result.
     */
    void DumpMissionList(std::vector<std::string> &info);

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
private:
    int StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest);
    int MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);
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

    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool RemoveMissionList(const std::list<std::shared_ptr<MissionList>> lists,
        const std::shared_ptr<MissionList> &list);
    int ClearMissionLocked(int missionId, std::shared_ptr<Mission> mission);
    int TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);
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
    bool IsPC();

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
