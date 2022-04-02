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

#ifndef OHOS_AAFWK_ABILITY_STACK_MANAGER_H
#define OHOS_AAFWK_ABILITY_STACK_MANAGER_H

#include <mutex>
#include <list>
#include <queue>
#include <unordered_map>
#include <vector>

#include "ability_info.h"
#include "ability_record.h"
#include "application_info.h"
#include "mission_record.h"
#include "mission_snapshot.h"
#include "mission_stack.h"
#include "mission_index_info.h"
#include "mission_option.h"
#include "ability_mission_info.h"
#include "lock_mission_container.h"
#include "lock_screen_event_subscriber.h"
#include "resume_mission_container.h"
#include "stack_info.h"
#include "power_storage.h"
#include "want.h"
#include "screenshot_handler.h"

namespace OHOS {
namespace AAFwk {
enum class SystemWindowMode {
    DEFAULT_WINDOW_MODE = 0,
    SPLITSCREEN_WINDOW_MODE,
    FLOATING_WINDOW_MODE,
    FLOATING_AND_SPLITSCREEN_WINDOW_MODE,
};

struct TerminatingAbility {
    std::shared_ptr<AbilityRecord> abilityRecord;
    int resultCode;
    const Want *resultWant;
};

/**
 * @class AbilityStackManager
 * AbilityStackManager provides a facility for managing page ability life cycle.
 */
class AbilityStackManager : public std::enable_shared_from_this<AbilityStackManager> {
public:
    explicit AbilityStackManager(int userId);
    ~AbilityStackManager();

    /**
     * init ability stack manager.
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
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant);

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param caller, caller ability record.
     * @param requestCode, abililty request code
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const std::shared_ptr<AbilityRecord> &caller, int requestCode);

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeAbility(const sptr<IRemoteObject> &token);

    /**
     * get ability stack manager's user id.
     *
     * @return Returns userId.
     */
    int GetAbilityStackManagerUserId() const;

    /**
     * get current working mission stack.
     *
     * @return current mission stack.
     */
    std::shared_ptr<MissionStack> GetCurrentMissionStack() const
    {
        return currentMissionStack_;
    }

    /**
     * get current top ability's token of stack.
     *
     * @return top ability record's token.
     */
    sptr<Token> GetCurrentTopAbilityToken();

    /**
     * get the ability record by token.
     *
     * @param recordId, ability record id.
     * @return ability record.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordById(const int64_t recordId);

    /**
     * get the stack by id.
     *
     * @param recordId, stack id.
     * @return MissionStack.
     */
    std::shared_ptr<MissionStack> GetStackById(int stackId);

    /**
     * get current top mission of stack.
     *
     * @return top mission record.
     */
    std::shared_ptr<MissionRecord> GetTopMissionRecord() const;

    /**
     * get the ability record by token.
     *
     * @param token, the token of ability.
     * @return ability record.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token);

    /**
     * get terminating ability from terminate list.
     *
     * @param token, the token of ability.
     */
    std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject> &token);

    /**
     * get the mission record by record id.
     *
     * @param id, the record id of mission.
     * @return mission record.
     */
    std::shared_ptr<MissionRecord> GetMissionRecordById(int id) const;

    /**
     * get the mission record by record name.
     *
     * @param name, the record name of mission.
     * @return mission record.
     */
    std::shared_ptr<MissionRecord> GetMissionRecordByName(std::string name) const;

    /**
     * get the mission record by record id from all stacks.
     *
     * @param id, the record id of mission.
     * @return mission record.
     */
    std::shared_ptr<MissionRecord> GetMissionRecordFromAllStacks(int id) const;

    /**
     * remove the mission record by record id.
     *
     * @param id, the record id of mission.
     * @return Returns true on success, false on failure.
     */
    bool RemoveMissionRecordById(int id);

    /**
     * attach ability thread ipc object.
     *
     * @param scheduler, ability thread ipc object.
     * @param token, the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @param saveData, save ability data
     * @return Returns ERR_OK on success, others on failure.
     */
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData);

    /**
     * AddWindowInfo, add windowToken to AbilityRecord.
     *
     * @param token, the token of the ability.
     * @param windowToken, window id of the ability.
     */
    void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken);

    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    void OnAppStateChanged(const AppInfo &info);

    /**
     * Remove the specified mission from the stack by mission id.
     *
     * @param missionId, target mission id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RemoveMissionById(int missionId);

    /**
     * move the mission stack to the top.
     *
     * @param stack, target mission stack.
     */
    void MoveMissionStackToTop(const std::shared_ptr<MissionStack> &stack);

    int GetMaxHoldMissionsByStackId(int stackId) const;
    bool SupportSyncVisualByStackId(int stackId) const;

    void SetMissionStackSetting(const StackSetting &stackSetting);

    /**
     * complete ability life cycle .
     *
     * @param abilityRecord.
     */
    void CompleteActive(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteInactive(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param info Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    void Dump(std::vector<std::string> &info);
    void DumpWaittingAbilityQueue(std::string &result);
    void DumpTopAbility(std::vector<std::string> &info);
    void DumpMission(int missionId, std::vector<std::string> &info);
    void DumpStack(int missionStackId, std::vector<std::string> &info);
    void DumpStackList(std::vector<std::string> &info);
    void DumpFocusMap(std::vector<std::string> &info);
    void DumpWindowMode(std::vector<std::string> &info);

    /**
     * get the target mission stack by want info.
     *
     * @param want , the want for starting ability.
     */
    std::shared_ptr<MissionStack> GetTargetMissionStack(const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionStack> GetTargetMissionStackByDefault(const AbilityRequest &abilityRequest);
    std::shared_ptr<MissionStack> GetTargetMissionStackBySetting(const AbilityRequest &abilityRequest);

    /**
     * Ask that the mission associated with a given mission ID be moved to the
     * front of the stack, so it is now visible to the user.
     *
     * @param missionId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToTop(int32_t missionId);

    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst);

    int MoveMissionToEndLocked(int missionId);

    /**
     * Ability detects death
     *
     * @param abilityRecord
     */
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * Uninstall app
     *
     * @param bundleName.
     */
    void UninstallApp(const std::string &bundleName);

    void OnTimeOut(uint32_t msgId, int64_t eventId);
    bool IsFirstInMission(const sptr<IRemoteObject> &token);
    bool IsFrontInAllStack(const std::shared_ptr<MissionStack> &stack) const;
    bool IsTopInMission(const std::shared_ptr<AbilityRecord> &abilityRecord) const;

    /**
     * Moving some missions to the specified stack by mission option(Enter splitscreen or floating window mode).
     * @param missionOption, target mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToFloatingStack(const MissionOption &missionOption);

    /**
     * Moving mission to the specified stack by mission option(Enter floating window mode).
     * @param primary, display primary mission option
     * @param secondary, display secondary mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary);

    /**
     * minimize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeMultiWindow(int missionId);
    /**
     * maximize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    int MaximizeMultiWindow(int missionId);

    /**
     * Change the focus of ability in the mission stack.
     * @param lostToken, the token of lost focus ability
     * @param getToken, the token of get focus ability
     * @return Returns ERR_OK on success, others on failure.
     */
    int ChangeFocusAbility(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken);

    /**
     * get missions info of floating mission stack.
     * @param list, mission info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int GetFloatingMissions(std::vector<AbilityMissionInfo> &list);

    /**
     * close multiwindow by mission id.
     * @param missionId, the id of target mission.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseMultiWindow(int missionId);

    void JudgingIsRemoveMultiScreenStack(std::shared_ptr<MissionStack> &stack);

    /**
     * Save the top ability States and move them to the background
     * @return Returns ERR_OK on success, others on failure.
     */
    int PowerOff();

    /**
     * Restore the state before top ability poweroff
     * @return Returns ERR_OK on success, others on failure.
     */
    int PowerOn();

    int StartLockMission(int uid, int missionId, bool isSystemApp, int isLock);
    int SetMissionDescriptionInfo(
        const std::shared_ptr<AbilityRecord> &abilityRecord, const MissionDescriptionInfo &description);
    int GetMissionLockModeState();

    void RestartAbility(const std::shared_ptr<AbilityRecord> abilityRecord);

    int GetMissionSnapshot(int32_t missionId, MissionPixelMap &missionPixelMap);

    void UpdateLockScreenState(bool isLockScreen);

    bool IsStarted();
    void PauseManager();
    void ResumeManager();
private:
    /**
     * dispatch ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchActive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchLifecycle(const std::shared_ptr<AbilityRecord> &lastTopAbility,
        const std::shared_ptr<AbilityRecord> &currentTopAbility, bool isTopFullScreen = false);
    void ContinueLifecycle();

    /**
     * get current top ability of stack.
     *
     * @return top ability record.
     */
    std::shared_ptr<AbilityRecord> GetCurrentTopAbility() const;

    /**
     * StartAbilityLocked.
     *
     * @param currentTopAbilityRecord, current top ability.
     * @param abilityRequest the request of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityLocked(
        const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest);

    int StartAbilityAsDefaultLocked(
        const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest);

    int StartAbilityAsMultiWindowLocked(
        const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest);

    int StartAbilityByAllowListLocked(
        const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest);

    /**
     * TerminateAbilityLocked.
     *
     * @param abilityRecord, target ability.
     * @param resultCode the result code of the ability to terminate.
     * @param resultWant the result Want of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbilityLocked(std::list<TerminatingAbility> &terminateLists);

    /**
     * MinimizeAbilityLocked.
     *
     * @param abilityRecord, target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * Remove the specified mission from the stack by mission id.
     *
     * @param missionId, target mission id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RemoveMissionByIdLocked(int missionId);

    /**
     * remove terminating ability from stack.
     *
     * @param abilityRecord, target ability.
     */
    void RemoveTerminatingAbility(
        const std::shared_ptr<AbilityRecord> &abilityRecord, const std::shared_ptr<AbilityRecord> &lastActiveAbility);

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
     * get tartget ability and mission by request and top ability.
     *
     * @param abilityRequest, the request of ability.
     * @param currentTopAbility, top ability.
     * @param tragetAbilityRecord, out param.
     * @param targetMissionRecord, out param.
     */
    void GetMissionRecordAndAbilityRecord(const AbilityRequest &abilityRequest,
        const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &tragetAbilityRecord,
        std::shared_ptr<MissionRecord> &targetMissionRecord);

    /**
     * check wheather the ability is launcher.
     *
     * @param abilityRequest, the abilityRequest fot starting ability.
     * @return Returns true on success, false on failure.
     */
    bool IsLauncherAbility(const AbilityRequest &abilityRequest) const;

    /**
     * check wheather the mission has launcher ability.
     *
     * @param id, mission id.
     * @return Returns true on success, false on failure.
     */
    bool IsLauncherMission(int id);

    void CreateRecentMissionInfo(const MissionRecordInfo &mission, AbilityMissionInfo &recentMissionInfo);

    /**
     * Ask that the mission associated with a given mission ID be moved to the
     * front of the stack, so it is now visible to the user.
     *
     * @param missionId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToTopLocked(int32_t missionId);

    /**
     * Requires that tasks associated with a given capability token be moved to the background
     *
     * @param token ability token
     * @param nonFirst If nonfirst is false and not the lowest ability of the mission, you cannot move mission to end
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionToEndLocked(const sptr<IRemoteObject> &token, const bool nonFirst);

    /**
     * Remove the specified mission stack by stack id
     *
     * @param id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RemoveStackLocked(int stackId);

    /**
     * Force return to launcher
     */
    void BackToLauncher();
    void DelayedStartLauncher();

    /**
     * Ability from default stack detects death
     *
     * @param abilityRecord
     */
    void HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * Add uninstall tags to ability
     *
     * @param bundleName
     */
    void AddUninstallTags(const std::string &bundleName);

    /**
     * Get target record by start mode.
     */
    void GetRecordBySingleton(const AbilityRequest &abilityRequest,
        const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &targetAbilityRecord,
        std::shared_ptr<MissionRecord> &targetMissionRecord);

    void GetRecordByStandard(const AbilityRequest &abilityRequest,
        const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &targetAbilityRecord,
        std::shared_ptr<MissionRecord> &targetMissionRecord);

    void GetRecordBySplitScreenMode(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord, std::shared_ptr<MissionRecord> &targetMissionRecord);

    /**
     * Get root ability from launcher mission stack.
     */
    std::shared_ptr<AbilityRecord> GetLauncherRootAbility() const;

    /**
     * Get ability record by event id.
     * @param eventId
     * @return Returns target record.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByEventId(int64_t eventId) const;

    /**
     * Get or Create mission stack by stack id,
     * @param stackId, target stack id
     * @param isCreateFlag, if the stack is not exist, decide whether or not to create a new mission stack,
     *                      default is fasle.
     * @return Returns target missionStack.
     */
    std::shared_ptr<MissionStack> GetOrCreateMissionStack(int stackId, bool isCreateFlag = false);

    /**
     * Moving some missions to the specified stack by mission option(Enter splitscreen or floating window mode).
     * @param missionOption, target mission option
     * @return Returns ERR_OK on success, others on failure.
     */
    int MoveMissionsToStackLocked(const std::list<MissionOption> &missionOptions);
#ifdef SUPPORT_GRAPHICS
    int CheckMultiWindowCondition(const std::list<MissionOption> &missionOptions) const;
    int CheckMultiWindowCondition(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        const std::shared_ptr<AbilityRecord> &topFullAbility, const AbilityRequest &abilityRequest) const;
#endif
    bool CheckSplitSrceenCondition(
        const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &topFullAbility) const;
    bool CheckMissionStackWillOverflow(const std::list<MissionOption> &missionOptions) const;
    int CompleteMoveMissionToStack(
        const std::shared_ptr<MissionRecord> &missionRecord, const std::shared_ptr<MissionStack> &stack);
    void EmplaceMissionToStack(
        const std::shared_ptr<MissionRecord> &sourceMission, const std::shared_ptr<MissionStack> &targetStack);
    int CompleteMissionMoving(std::shared_ptr<MissionRecord> &missionRecord, int stackId);
    SystemWindowMode JudgingTargetSystemWindowMode(AbilityWindowConfiguration config) const;
    SystemWindowMode GetLatestSystemWindowMode();
    int JudgingTargetStackId(AbilityWindowConfiguration config) const;
    int StartAbilityLifeCycle(std::shared_ptr<AbilityRecord> lastTopAbility,
        std::shared_ptr<AbilityRecord> currentTopAbility, std::shared_ptr<AbilityRecord> targetAbility);

    void ActiveTopAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void MoveMissionAndAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord, std::shared_ptr<MissionRecord> &targetMissionRecord);

    void SortPreMission(
        const std::shared_ptr<MissionRecord> &mission, const std::shared_ptr<MissionRecord> &nextMission);

    int PowerOffLocked();
    int PowerOnLocked();

    bool CheckLockMissionCondition(
        int uid, int missionId, int isLock, bool isSystemApp, std::shared_ptr<MissionRecord> &mission, int &lockUid);
    bool CanStartInLockMissionState(
        const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &currentTopAbility) const;
    bool CanStopInLockMissionState(const std::shared_ptr<AbilityRecord> &terminateAbility) const;
    void SendUnlockMissionMessage();
    std::shared_ptr<MissionStack> GetTopFullScreenStack();
    std::shared_ptr<MissionStack> GetTopSplitScreenStack();
    std::shared_ptr<MissionStack> GetTopFullScreenStackIncludeSplitScreen();
    bool InFrontOfFullScreenStack() const;
    bool IsExistSplitScreenStack() const;
    bool IsFullScreenStack(int stackId) const;
    bool IsLockScreenStack(int stackId) const;
    bool IsSplitScreenStack(int stackId) const;
    std::shared_ptr<AbilityRecord> GetTopAbilityOfFullScreen();
    int GenerateMissinOptionsOfSplitScreen(
        const MissionOption &primary, const MissionOption &secondary, std::list<MissionOption> &missionOptions);
    int GenerateMissinOptionsOfSplitScreen(std::list<MissionOption> &missionOptions,
        const std::shared_ptr<MissionStack> &targetStack, std::shared_ptr<MissionRecord> &targetMissionRecord);
    int GetTargetChangeType(bool isMissionChanged, bool isStackChanged, bool isCurrentFull, bool isTargetFull,
        std::shared_ptr<AbilityRecord> lastTopAbility, std::shared_ptr<AbilityRecord> targetAbility,
        std::shared_ptr<AbilityRecord> &needBackgroundAbility);

    /**
     * minimize multiwindow by mission id.
     * @param missionId, the id of target mission
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeMultiWindowLocked(int missionId);
    int ChangeFocusAbilityLocked(const std::shared_ptr<AbilityRecord> &targetAbility);

    void NotifyWindowModeChanged(const SystemWindowMode &windowMode);

    void UpdateFocusAbilityRecord(
        int displayId, const std::shared_ptr<AbilityRecord> &focusAbility, bool isNotify = false);
    void UpdateFocusAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isNotify = false);
    void CheckMissionRecordIsResume(const std::shared_ptr<MissionRecord> &mission);
    int ChangedPowerStorageAbilityToActive(std::shared_ptr<PowerStorage> &powerStorage,
        bool isPowerStateLockScreen = false);
    void HandleActiveTimeout(const std::shared_ptr<AbilityRecord> &ability);
    bool IsLockScreenState();
    bool DeleteMissionRecordInStackOnLockScreen(const std::shared_ptr<MissionRecord> &missionRecord);
    void RestoreMissionRecordOnLockScreen(const std::shared_ptr<MissionRecord> &missionRecord);
    void UpdatePowerOffRecord(int32_t missionId, const std::shared_ptr<AbilityRecord> &ability);
    void SetPowerOffRecordWhenLockScreen(std::shared_ptr<PowerStorage> &powerStorage);
    void DelayedStartLockScreenApp();
    void BackToLockScreen();
    std::shared_ptr<AbilityRecord> GetLockScreenRootAbility() const;
    void lockScreenStackAbilityDied(std::shared_ptr<MissionRecord> &mission);
    bool SubscribeEvent();

    void MakeTerminatingAbility(TerminatingAbility &unit, const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant);

    void SortAndGetLastActiveAbility(
        std::list<TerminatingAbility> &terminateLists, std::shared_ptr<AbilityRecord> &lastActiveAbility);

    std::shared_ptr<MissionRecord> GetFriendMissionBySplitScreen(
        const std::shared_ptr<MissionStack> &stack, const int missionId);

    void SortRecentMissions(std::vector<MissionRecordInfo> &missions);

    void UpdateMissionOption(const std::shared_ptr<MissionRecord> &mission,
        const std::shared_ptr<MissionStack> &moveToStack, const AbilityWindowConfiguration winModeKey);

    std::string ConvertWindowModeState(const SystemWindowMode &mode);

    void ProcessInactivateInMoving(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void CompleteInactiveByNewVersion(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    void CompleteForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);

    int DispatchBackgroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    void CompleteBackgroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord);

private:
    static constexpr int MIN_MISSION_STACK_ID = LAUNCHER_MISSION_STACK_ID;
    static constexpr int MAX_CAN_MOVE_MISSIONS = 2;
    static constexpr int FLOATING_MAX_STACK_MISSION_NUM = 5;

    int userId_;
    bool powerOffing_ = false;
    std::recursive_mutex stackLock_;
    std::shared_ptr<MissionStack> launcherMissionStack_;
    std::shared_ptr<MissionStack> defaultMissionStack_;
    std::shared_ptr<MissionStack> currentMissionStack_;
    std::shared_ptr<MissionStack> lastMissionStack_;
    std::shared_ptr<MissionStack> lockScreenMissionStack_ = nullptr;
    std::list<std::shared_ptr<MissionStack>> missionStackList_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityRecordList_;  // abilities on terminating put in this
                                                                            // list.
    std::queue<AbilityRequest> waittingAbilityQueue_;
    std::list<wptr<IRemoteObject>> focusWaitingList_;
    std::shared_ptr<PowerStorage> powerStorage_;
    // find AbilityRecord by windowToken. one windowToken has one and only one AbilityRecord.
    std::unordered_map<int, std::shared_ptr<AbilityRecord>> windowTokenToAbilityMap_;
    std::shared_ptr<LockMissionContainer> lockMissionContainer_ = nullptr;
    SystemWindowMode curSysWindowMode_ = SystemWindowMode::DEFAULT_WINDOW_MODE;
    bool isMultiWinMoving_ = false;
    std::vector<StackSetting> stackSettings_;
    std::map<int, std::weak_ptr<AbilityRecord>> focusAbilityRecordMap_;  // abilities has been focused ,
                                                                         // key : display id, value: focused ability
    std::shared_ptr<ResumeMissionContainer> resumeMissionContainer_;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<ScreenshotHandler> screenshotHandler_;
#endif
    static int64_t splitScreenStackId;
    const static std::map<SystemWindowMode, std::string> windowModeToStrMap_;
    std::shared_ptr<LockScreenEventSubscriber> subscriber_ = nullptr;
    bool isLockScreen_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_STACK_MANAGER_H
