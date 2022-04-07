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

#include "ability_stack_manager.h"

#include <map>
#include <fstream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "bytrace.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "shared_memory.h"

namespace OHOS {
namespace AAFwk {
int64_t AbilityStackManager::splitScreenStackId = SPLIT_SCREEN_MISSION_STACK_ID;
const std::map<SystemWindowMode, std::string> AbilityStackManager::windowModeToStrMap_ = {
    {SystemWindowMode::DEFAULT_WINDOW_MODE, "default window mode"},
    {SystemWindowMode::SPLITSCREEN_WINDOW_MODE, "split screen window mode"},
    {SystemWindowMode::FLOATING_WINDOW_MODE, "floating window mode"},
    {SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE, "floating and split screen window mode"}
};
AbilityStackManager::AbilityStackManager(int userId) : userId_(userId)
{}

void AbilityStackManager::Init()
{
    launcherMissionStack_ = std::make_shared<MissionStack>(LAUNCHER_MISSION_STACK_ID, userId_);
    missionStackList_.push_back(launcherMissionStack_);
    defaultMissionStack_ = std::make_shared<MissionStack>(DEFAULT_MISSION_STACK_ID, userId_);
    missionStackList_.push_back(defaultMissionStack_);
    currentMissionStack_ = launcherMissionStack_;

#ifdef SUPPORT_GRAPHICS
    screenshotHandler_ = std::make_shared<ScreenshotHandler>();
#endif
    if (!SubscribeEvent()) {
        HILOG_ERROR("SubscribeEvent Error.");
    }

    // complete floating mission stack setting init.
    StackSetting floatSetting;
    floatSetting.userId = userId_;
    floatSetting.stackId = FLOATING_MISSION_STACK_ID;
    floatSetting.maxHoldMission = FLOATING_MAX_STACK_MISSION_NUM;
    floatSetting.isSyncVisual = true;
}

AbilityStackManager::~AbilityStackManager()
{}

int AbilityStackManager::StartAbility(const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Start ability.");
    return 0;
}

int AbilityStackManager::StartAbilityLocked(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // lock screen state
    if (IsLockScreenState()) {
        HILOG_DEBUG("Start ability with Permit list...");
        if (!lockScreenMissionStack_) {
            return StartAbilityByAllowListLocked(nullptr, abilityRequest);
        } else {
            auto lockTopAbilityRecord = lockScreenMissionStack_->GetTopAbilityRecord();
            if (lockTopAbilityRecord && lockTopAbilityRecord->GetAbilityState() != ACTIVE) {
                HILOG_INFO("lock screen stack top ability not active(%{public}s), enqueue(%{public}s) for waiting.",
                    lockTopAbilityRecord->GetAbilityInfo().name.c_str(), abilityRequest.abilityInfo.name.c_str());
                EnqueueWaittingAbility(abilityRequest);
                return START_ABILITY_WAITING;
            }
            return StartAbilityByAllowListLocked(lockTopAbilityRecord, abilityRequest);
        }
    }
    // start multi window
    if (abilityRequest.startSetting && !abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        auto windowkey = static_cast<AbilityWindowConfiguration>(
            std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
        if (windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING ||
            windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
            HILOG_INFO("Start ability AsMultiWindow: %{public}d", windowkey);
            return StartAbilityAsMultiWindowLocked(currentTopAbility, abilityRequest);
        }
    }
    HILOG_INFO("Start ability AsDefault.");
    return StartAbilityAsDefaultLocked(currentTopAbility, abilityRequest);
}

int AbilityStackManager::StartAbilityAsDefaultLocked(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Start ability locked.");
    CHECK_POINTER_AND_RETURN(currentMissionStack_, INNER_ERR);
    // 1. choose target mission stack
    std::shared_ptr<MissionStack> targetStack = GetTargetMissionStack(abilityRequest);
    CHECK_POINTER_AND_RETURN(targetStack, CREATE_MISSION_STACK_FAILED);
    auto lastTopAbility = targetStack->GetTopAbilityRecord();

    // 2. move target mission stack to top, currentMissionStack will be changed.
    MoveMissionStackToTop(targetStack);

    // 3. get mission record and ability recode
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;
    GetMissionRecordAndAbilityRecord(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
    if (targetAbilityRecord == nullptr || targetMissionRecord == nullptr) {
        HILOG_ERROR("Failed to get ability record or mission record.");
        MoveMissionStackToTop(lastMissionStack_);
        return ERR_INVALID_VALUE;
    }
    if (abilityRequest.IsContinuation()) {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
    } else {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);

    // 4. start processing ability lifecycle
    if (currentTopAbility == nullptr) {
        HILOG_INFO("top ability is null, then launch the first Ability.");
        targetAbilityRecord->SetLauncherRoot();
        return targetAbilityRecord->LoadAbility();
    } else {
        HILOG_INFO("top ability not null, complete ability background if needed.");
        return StartAbilityLifeCycle(lastTopAbility, currentTopAbility, targetAbilityRecord);
    }
}

int AbilityStackManager::StartAbilityLifeCycle(std::shared_ptr<AbilityRecord> lastTopAbility,
    std::shared_ptr<AbilityRecord> currentTopAbility, std::shared_ptr<AbilityRecord> targetAbility)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(targetAbility, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentTopAbility, ERR_INVALID_VALUE);
    enum ChangeType { T_ACTIVE = 0, T_CHANGE, T_DEFAULT } changeType;

    bool isMissionChanged = currentTopAbility->GetMissionRecordId() != targetAbility->GetMissionRecordId();
    bool isStackChanged = currentTopAbility->GetMissionStackId() != targetAbility->GetMissionStackId();
    bool isCurrentFull = IsFullScreenStack(currentTopAbility->GetMissionStackId());
    bool isTargetFull = IsFullScreenStack(targetAbility->GetMissionStackId());

    if (isStackChanged && ((IsSplitScreenStack(currentTopAbility->GetMissionStackId()) && isTargetFull) ||
        IsSplitScreenStack(targetAbility->GetMissionStackId()))) {
        return DispatchLifecycle(currentTopAbility, targetAbility, false);
    }

    std::shared_ptr<AbilityRecord> needBackgroundAbility;
    changeType = static_cast<ChangeType>(GetTargetChangeType(isMissionChanged,
        isStackChanged,
        isCurrentFull,
        isTargetFull,
        lastTopAbility,
        targetAbility,
        needBackgroundAbility));

    needBackgroundAbility = (changeType == T_DEFAULT) ? currentTopAbility : needBackgroundAbility;
    changeType = needBackgroundAbility ? changeType : T_ACTIVE;

    HILOG_DEBUG("ChangeType: %{public}d, needBackAbility : %{public}s",
        changeType,
        needBackgroundAbility ? needBackgroundAbility->GetAbilityInfo().name.c_str() : "none");

    // deal ability lifecycle.
    switch (changeType) {
        // 1. last top ability don't need inactive , target ability active directly.
        case T_ACTIVE: {
            targetAbility->ProcessActivate();
            break;
        }
        // 2. change inactive ability, add pre and next flag.
        case T_CHANGE: {
            targetAbility->SetPreAbilityRecord(needBackgroundAbility);
            needBackgroundAbility->SetNextAbilityRecord(targetAbility);
            needBackgroundAbility->Inactivate();
            break;
        }
        // 3. usually, last top ability need inactive, target ability need active.
        case T_DEFAULT:
        default: {
            needBackgroundAbility->Inactivate();
            break;
        }
    }

    return ERR_OK;
}

int AbilityStackManager::GetTargetChangeType(bool isMissionChanged, bool isStackChanged, bool isCurrentFull,
    bool isTargetFull, std::shared_ptr<AbilityRecord> lastTopAbility, std::shared_ptr<AbilityRecord> targetAbility,
    std::shared_ptr<AbilityRecord> &needBackgroundAbility)
{
    enum ChangeType { T_ACTIVE = 0, T_CHANGE, T_DEFAULT } changeType;
    // set target changeType
    if (isMissionChanged) {
        if (isStackChanged) {
            if (isCurrentFull && isTargetFull) {
                changeType = T_DEFAULT;
            } else if (!isCurrentFull && isTargetFull) {
                auto needAbility = (targetAbility->GetMissionStackId() == DEFAULT_MISSION_STACK_ID)
                                       ? launcherMissionStack_->GetTopAbilityRecord()
                                       : defaultMissionStack_->GetTopAbilityRecord();
                changeType = T_CHANGE;
                needBackgroundAbility =
                    (needAbility && needAbility->IsAbilityState(AbilityState::ACTIVE)) ? needAbility : lastTopAbility;
            } else {
                auto self(shared_from_this());
                auto IsChanged = [&lastTopAbility, &targetAbility, self]() {
                    bool isSameMission = lastTopAbility->GetMissionRecordId() == targetAbility->GetMissionRecordId();
                    bool isSyncVisual = self->SupportSyncVisualByStackId(targetAbility->GetMissionStackId());
                    return (isSameMission || (!isSameMission && !isSyncVisual)) &&
                                   lastTopAbility->IsAbilityState(AbilityState::ACTIVE)
                               ? T_CHANGE
                               : T_ACTIVE;
                };
                changeType = lastTopAbility ? IsChanged() : T_ACTIVE;
                needBackgroundAbility = (changeType == T_CHANGE) ? lastTopAbility : nullptr;
            }
        } else {
            if (SupportSyncVisualByStackId(targetAbility->GetMissionStackId())) {
                auto targetMission = targetAbility->GetMissionRecord();
                auto secondAbility = targetMission ? targetMission->GetLastTopAbility() : nullptr;
                if (secondAbility && secondAbility->IsAbilityState(AbilityState::ACTIVE)) {
                    needBackgroundAbility = secondAbility;
                    changeType = T_CHANGE;
                } else {
                    changeType = T_ACTIVE;
                }
            } else {
                changeType = T_DEFAULT;
            }
        }
    } else {
        changeType = T_DEFAULT;
    }

    return changeType;
}

int AbilityStackManager::StartAbilityAsMultiWindowLocked(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability as special locked.");

    CHECK_POINTER_AND_RETURN(currentTopAbility, INNER_ERR);
    auto topFullStack = GetTopFullScreenStack();
    CHECK_POINTER_AND_RETURN(topFullStack, ERR_INVALID_DATA);
    auto topFullAbility = topFullStack->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN(topFullAbility, ERR_INVALID_DATA);
    // 1. choose target mission stack
    auto targetStack = GetTargetMissionStack(abilityRequest);
    CHECK_POINTER_AND_RETURN(targetStack, CREATE_MISSION_STACK_FAILED);
    auto lastTopAbility = targetStack->GetTopAbilityRecord();

    // 2. get mission record and ability recode
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;
    GetMissionRecordAndAbilityRecord(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
    CHECK_POINTER_AND_RETURN(targetAbilityRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(targetMissionRecord, ERR_INVALID_VALUE);

    if (abilityRequest.IsContinuation()) {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
    } else {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);

    // 3. first create mission record or stack is not changed,
    // just load target ability and inactive the current top ability.
    if ((!IsSplitScreenStack(targetStack->GetMissionStackId()) &&
        (targetMissionRecord->GetMissionStack() == nullptr ||
        targetMissionRecord->GetMissionStack()->GetMissionStackId() == targetStack->GetMissionStackId())) ||
        (IsSplitScreenStack(targetStack->GetMissionStackId()) &&
        targetStack->GetMissionRecordById(targetMissionRecord->GetMissionRecordId()))) {
        MoveMissionStackToTop(targetStack);
        MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);
        HILOG_DEBUG("First create mission record ,missionId:%{public}d", targetMissionRecord->GetMissionRecordId());
        CHECK_RET_RETURN_RET(StartAbilityLifeCycle(lastTopAbility, currentTopAbility, targetAbilityRecord),
            "Start ability lifecycle error.");
        NotifyWindowModeChanged(GetLatestSystemWindowMode());
        return ERR_OK;
    }

    return ERR_OK;
}

int AbilityStackManager::StartAbilityByAllowListLocked(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability as allow list locked.");
    // 1. lockscreen stack is target mission stack
    std::shared_ptr<MissionStack> targetStack = GetTargetMissionStack(abilityRequest);
    CHECK_POINTER_AND_RETURN(targetStack, CREATE_MISSION_STACK_FAILED);
    HILOG_DEBUG("StartAbilityLocked targetStack id %{public}d", targetStack->GetMissionStackId());
    // 2. move target mission stack to top, currentMissionStack will be changed.
    currentMissionStack_ = targetStack;
    CHECK_POINTER_AND_RETURN(currentMissionStack_, INNER_ERR);
    // 3. get mission record and ability recode
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;
    GetMissionRecordAndAbilityRecord(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
    if (targetAbilityRecord == nullptr || targetMissionRecord == nullptr) {
        HILOG_ERROR("Failed to get ability record or mission record.");
        if (lockScreenMissionStack_ && lockScreenMissionStack_->IsEmpty()) {
            missionStackList_.remove(lockScreenMissionStack_);
            currentMissionStack_ = missionStackList_.front();
            lockScreenMissionStack_.reset();
        }
        return ERR_INVALID_VALUE;
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);
    // 4. start processing ability lifecycle
    if (currentTopAbility == nullptr) {
        // top ability is null
        if (targetAbilityRecord->GetAbilityState() == INITIAL) {
            targetAbilityRecord->SetLockScreenRoot();
            return targetAbilityRecord->LoadAbility();
        }
        if (targetAbilityRecord->GetAbilityState() == BACKGROUND) {
            targetAbilityRecord->ProcessActivate();
        }
    } else {
        // complete ability background if needed.
        currentTopAbility->Inactivate();
    }
    return ERR_OK;
}

void AbilityStackManager::SortPreMission(
    const std::shared_ptr<MissionRecord> &mission, const std::shared_ptr<MissionRecord> &nextMission)
{
    CHECK_POINTER(mission);
    CHECK_POINTER(nextMission);
    auto targetPreMission = nextMission->GetPreMissionRecord();
    if (targetPreMission) {
        mission->SetPreMissionRecord(targetPreMission);
    }
    nextMission->SetPreMissionRecord(mission);
}

void AbilityStackManager::MoveMissionAndAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    std::shared_ptr<AbilityRecord> &targetAbilityRecord, std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Move mission and ability.");
    CHECK_POINTER(targetAbilityRecord);
    CHECK_POINTER(targetMissionRecord);

    // set relationship of mission record and ability record
    if (currentTopAbility != nullptr) {
        targetAbilityRecord->SetPreAbilityRecord(currentTopAbility);
        currentTopAbility->SetNextAbilityRecord(targetAbilityRecord);
        // move mission to end, don't set pre mission
        SortPreMission(currentTopAbility->GetMissionRecord(), targetMissionRecord);
        if (currentTopAbility->IsLauncherAbility()) {
            targetMissionRecord->SetIsLauncherCreate();
        }
    }

    // add caller record
    targetAbilityRecord->SetMissionRecord(targetMissionRecord);
    // reparent mission record, currentMissionStack is the target mission stack.
    targetMissionRecord->SetMissionStack(currentMissionStack_, currentMissionStack_->GetMissionStackId());
    targetAbilityRecord->SetMissionStackId(currentMissionStack_->GetMissionStackId());
    // add ability record to mission record.
    // if this ability record exist this mission record, do not add.
    targetMissionRecord->AddAbilityRecordToTop(targetAbilityRecord);
    // add mission record to mission stack.
    // if this mission record exist this mission stack, do not add.
    currentMissionStack_->AddMissionRecordToTop(targetMissionRecord);
    // move mission record to top
    // if this mission exist at top, do not move.
    currentMissionStack_->MoveMissionRecordToTop(targetMissionRecord);
}

int AbilityStackManager::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    return 0;
}

void AbilityStackManager::MakeTerminatingAbility(TerminatingAbility &unit,
    const std::shared_ptr<AbilityRecord> &abilityRecord, int resultCode, const Want *resultWant)
{
    unit.abilityRecord = abilityRecord;
    unit.resultCode = resultCode;
    unit.resultWant = resultWant;
}

int AbilityStackManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    HILOG_INFO("Terminate ability.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    std::shared_ptr<AbilityRecord> targetAbility = nullptr;
    int result = static_cast<int>(ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    for (auto &stack : missionStackList_) {
        targetAbility = stack->GetAbilityRecordByCaller(caller, requestCode);
        if (targetAbility) {
            result = AbilityUtil::JudgeAbilityVisibleControl(targetAbility->GetAbilityInfo());
            break;
        }
    }

    if (!targetAbility) {
        HILOG_ERROR("%{public}s, Can't find target ability", __func__);
        return NO_FOUND_ABILITY_BY_CALLER;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    if (!CanStopInLockMissionState(targetAbility)) {
        SendUnlockMissionMessage();
        return LOCK_MISSION_STATE_DENY_REQUEST;
    }

    return TerminateAbility(targetAbility->GetToken(), DEFAULT_INVAL_VALUE, nullptr);
}

void AbilityStackManager::SortAndGetLastActiveAbility(
    std::list<TerminatingAbility> &terminateLists, std::shared_ptr<AbilityRecord> &lastActiveAbility)
{
    TerminatingAbility lastTerminateAbility;
    for (auto &it : terminateLists) {
        if (it.abilityRecord &&
            (it.abilityRecord->IsActiveState() ||
             it.abilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
             it.abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW))) {
            lastActiveAbility = it.abilityRecord;
            lastTerminateAbility = it;
        }
    }

    terminateLists.remove_if(
        [&lastActiveAbility](const TerminatingAbility &unit) { return unit.abilityRecord == lastActiveAbility; });
    terminateLists.emplace_back(lastTerminateAbility);
}

int AbilityStackManager::TerminateAbilityLocked(std::list<TerminatingAbility> &terminateLists)
{
    HILOG_INFO("Terminate ability locked called. size : %{public}d", static_cast<int>(terminateLists.size()));
    std::shared_ptr<AbilityRecord> lastActiveAbility;
    SortAndGetLastActiveAbility(terminateLists, lastActiveAbility);

    if (lastActiveAbility) {
        HILOG_INFO("last active ability name : %{public}s", lastActiveAbility->GetAbilityInfo().name.c_str());
    }

    for (auto &unit : terminateLists) {
        auto &ability = unit.abilityRecord;
        if (!ability) {
            HILOG_ERROR("ability is invalid");
            continue;
        }

        HILOG_INFO("terminate ability name : %{public}s", ability->GetAbilityInfo().name.c_str());

        if (ability->IsRestarting()) {
            HILOG_ERROR("abilityRecord is restarting, deny terminate.");
            continue;
        }

        if (unit.resultWant != nullptr) {
            ability->SaveResultToCallers(unit.resultCode, unit.resultWant);
        }
        if (ability->IsAbilityState(AbilityState::INITIAL) ||
            ability->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
            ability->IsAbilityState(AbilityState::FOREGROUNDING_NEW) ||
            ability->IsActiveState()) {
            // ability on initial, remove AbilityRecord out of stack.
            RemoveTerminatingAbility(ability, lastActiveAbility);
            ability->Inactivate();
            continue;
        }
        // it's not common case when ability terminate at non-active state and non-top position.
        if (ability->IsAbilityState(AbilityState::INACTIVE)) {
            // ability on inactive, remove AbilityRecord out of stack and then schedule to background.
            RemoveTerminatingAbility(ability, lastActiveAbility);
            ability->SendResultToCallers();
            MoveToBackgroundTask(ability);
        }
        if (ability->IsAbilityState(AbilityState::BACKGROUND) ||
            ability->IsAbilityState(AbilityState::BACKGROUND_NEW)) {
            // ability on background, remove AbilityRecord out of stack and then schedule to terminate.
            RemoveTerminatingAbility(ability, lastActiveAbility);
            ability->SendResultToCallers();
            auto task = [ability, stackManager = shared_from_this()]() {
                HILOG_WARN("Disconnect ability terminate timeout.");
                stackManager->CompleteTerminate(ability);
            };
            ability->Terminate(task);
        }
        if (ability->IsAbilityState(AbilityState::INACTIVATING) ||
            ability->IsAbilityState(AbilityState::MOVING_BACKGROUND)) {
            // ability on inactivating or moving to background.
            // remove AbilityRecord out of stack and waiting for ability(kit) AbilityTransitionDone.
            RemoveTerminatingAbility(ability, lastActiveAbility);
            ability->SendResultToCallers();
        }
    }

    return ERR_OK;
}

int AbilityStackManager::MinimizeAbility(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Terminate ability.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    // check if ability is in stack to avoid user create fake token.
    CHECK_POINTER_AND_RETURN_LOG(
        GetAbilityRecordByToken(token), INNER_ERR, "Ability is not in stack.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    return MinimizeAbilityLocked(abilityRecord);
}

int AbilityStackManager::MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_INFO("%{public}s, called", __func__);

    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        HILOG_WARN("Ability state is invalid.");
        return ERR_OK;
    }

    MoveToBackgroundTask(abilityRecord);
    return ERR_OK;
}

int AbilityStackManager::RemoveMissionById(int missionId)
{
    return 0;
}

int AbilityStackManager::RemoveMissionByIdLocked(int missionId)
{
    CHECK_POINTER_AND_RETURN_LOG(defaultMissionStack_, ERR_NO_INIT, "defaultMissionStack_ is invalid.");

    auto missionRecord = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN_LOG(missionRecord, REMOVE_MISSION_ID_NOT_EXIST, "Mission id is invalid.");
    CHECK_TRUE_RETURN_RET(IsLauncherMission(missionId),
        REMOVE_MISSION_LAUNCHER_DENIED,
        "Don't allow to terminate mission which has launcher ability.");

    auto currentStack = missionRecord->GetMissionStack();
    CHECK_POINTER_AND_RETURN_LOG(currentStack, REMOVE_MISSION_FAILED, "Current stack is nullptr.");

    auto topAbility = missionRecord->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN(topAbility, REMOVE_MISSION_FAILED);

    // Remove all missions of the current split screen
    if (IsSplitScreenStack(currentStack->GetMissionStackId())) {
        return RemoveStackLocked(currentStack->GetMissionStackId());
    }

    std::vector<AbilityRecordInfo> abilityInfos;
    missionRecord->GetAllAbilityInfo(abilityInfos);
    std::list<TerminatingAbility> terminateLists;
    for (auto &ability : abilityInfos) {
        auto abilityRecord = missionRecord->GetAbilityRecordById(ability.id);
        if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
            HILOG_WARN("Ability record is not exist or is on terminating.");
            continue;
        }

        if (abilityRecord->IsAbilityState(AbilityState::INITIAL)) {
            HILOG_INFO("Ability record state is INITIAL, remove ability, continue.");
            missionRecord->RemoveAbilityRecord(abilityRecord);
            if (missionRecord->IsEmpty()) {
                RemoveMissionRecordById(missionRecord->GetMissionRecordId());
                JudgingIsRemoveMultiScreenStack(currentStack);
            }
            continue;
        }

        abilityRecord->SetTerminatingState();
        TerminatingAbility unit;
        MakeTerminatingAbility(unit, abilityRecord, DEFAULT_INVAL_VALUE, nullptr);
        terminateLists.emplace_back(unit);
    }

    return TerminateAbilityLocked(terminateLists);
}

int AbilityStackManager::RemoveStackLocked(int stackId)
{
    return 0;
}

void AbilityStackManager::RemoveTerminatingAbility(
    const std::shared_ptr<AbilityRecord> &abilityRecord, const std::shared_ptr<AbilityRecord> &lastActiveAbility)
{
}

int AbilityStackManager::GetAbilityStackManagerUserId() const
{
    return userId_;
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetCurrentTopAbility() const
{
    std::shared_ptr<MissionStack> topMissionStack = missionStackList_.front();
    HILOG_DEBUG("Top mission stack id is %{public}d", topMissionStack->GetMissionStackId());
    return topMissionStack->GetTopAbilityRecord();
}

sptr<Token> AbilityStackManager::GetCurrentTopAbilityToken()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    sptr<Token> token = nullptr;
    std::shared_ptr<MissionStack> topMissionStack = missionStackList_.front();
    std::shared_ptr<AbilityRecord> abilityRecord = topMissionStack->GetTopAbilityRecord();
    if (abilityRecord != nullptr) {
        token = abilityRecord->GetToken();
    }
    return token;
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetAbilityRecordById(const int64_t recordId)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto missionStack : missionStackList_) {
        std::shared_ptr<AbilityRecord> abilityRecord = missionStack->GetAbilityRecordById(recordId);
        if (abilityRecord != nullptr) {
            return abilityRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetStackById(int stackId)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto missionStack : missionStackList_) {
        if (missionStack->GetMissionStackId() == stackId) {
            return missionStack;
        }
    }
    return nullptr;
}

std::shared_ptr<MissionRecord> AbilityStackManager::GetTopMissionRecord() const
{
    if (currentMissionStack_ == nullptr) {
        HILOG_ERROR("currentMissionStack_ is nullptr.");
        return nullptr;
    }
    return currentMissionStack_->GetTopMissionRecord();
}

std::shared_ptr<MissionRecord> AbilityStackManager::GetMissionRecordById(int id) const
{
    if (currentMissionStack_ == nullptr) {
        HILOG_ERROR("currentMissionStack_ is nullptr.");
        return nullptr;
    }
    return currentMissionStack_->GetMissionRecordById(id);
}

std::shared_ptr<MissionRecord> AbilityStackManager::GetMissionRecordFromAllStacks(int id) const
{
    for (auto missionStack : missionStackList_) {
        std::shared_ptr<MissionRecord> missionRecord = missionStack->GetMissionRecordById(id);
        if (missionRecord != nullptr) {
            return missionRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<MissionRecord> AbilityStackManager::GetMissionRecordByName(std::string name) const
{
    for (auto missionStack : missionStackList_) {
        std::shared_ptr<MissionRecord> missionRecord = missionStack->GetTargetMissionRecord(name);
        if (missionRecord != nullptr) {
            return missionRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto missionStack : missionStackList_) {
        std::shared_ptr<AbilityRecord> abilityRecord = missionStack->GetAbilityRecordByToken(token);
        if (abilityRecord != nullptr) {
            return abilityRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetAbilityFromTerminateList(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto abilityRecord : terminateAbilityRecordList_) {
        // token is type of IRemoteObject, abilityRecord->GetToken() is type of Token extending from IRemoteObject.
        if (abilityRecord && token == abilityRecord->GetToken()->AsObject()) {
            return abilityRecord;
        }
    }
    return nullptr;
}

bool AbilityStackManager::RemoveMissionRecordById(int id)
{
    for (auto missionstack : missionStackList_) {
        if (missionstack->RemoveMissionRecord(id)) {
            return true;
        }
    }
    return false;
}

void AbilityStackManager::MoveMissionStackToTop(const std::shared_ptr<MissionStack> &stack)
{
    if (stack == nullptr) {
        HILOG_ERROR("Stack is nullptr.");
        return;
    }
    if (missionStackList_.front() == stack) {
        HILOG_DEBUG("Stack is at the top of list, mission id: %d", stack->GetMissionStackId());
        return;
    }
    lastMissionStack_ = currentMissionStack_;
    missionStackList_.remove(stack);
    missionStackList_.push_front(stack);
    currentMissionStack_ = stack;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTargetMissionStack(const AbilityRequest &abilityRequest)
{
    // lockScreen : starting ability in lockScreen stack
    if (IsLockScreenState()) {
        if (!lockScreenMissionStack_) {
            lockScreenMissionStack_ = std::make_shared<MissionStack>(LOCK_SCREEN_STACK_ID, userId_);
            missionStackList_.remove(lockScreenMissionStack_);
            missionStackList_.push_front(lockScreenMissionStack_);
        }
        return lockScreenMissionStack_;
    }

    // priority : starting launcher ability .
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        return launcherMissionStack_;
    }

    // next: start ability for settings(multiwindow)
    if (abilityRequest.startSetting != nullptr) {
        return GetTargetMissionStackBySetting(abilityRequest);
    }

    // other: refer to the stack of the caller ability.
    return GetTargetMissionStackByDefault(abilityRequest);
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTargetMissionStackByDefault(const AbilityRequest &abilityRequest)
{
    bool isSingleton = abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON;
    std::shared_ptr<MissionRecord> requestMission = nullptr;
    if (isSingleton) {
        std::string bundleName = AbilityUtil::ConvertBundleNameSingleton(
            abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
        requestMission = GetMissionRecordByName(bundleName);
    } else {
        requestMission = GetMissionRecordByName(abilityRequest.abilityInfo.bundleName);
    }

    bool isExist = requestMission != nullptr;
    auto currentTop = GetCurrentTopAbility();
    if (currentTop) {
        // caller is launcher , request is not launcher, exist ability just restart.
        if (isExist && (currentTop->IsLauncherAbility() || (!currentTop->IsLauncherAbility() && isSingleton) ||
            (!currentTop->IsLauncherAbility() &&
            currentTop->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON))) {
            return requestMission->GetMissionStack();
        }
        // caller and request is not launcher, start ability at current mission stack.
        if (!currentTop->IsLauncherAbility() && !isSingleton &&
            currentTop->GetAbilityInfo().launchMode != AppExecFwk::LaunchMode::SINGLETON) {
            auto callerParent = currentTop->GetMissionRecord();
            return callerParent->GetMissionStack();
        }
    }

    return defaultMissionStack_;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTargetMissionStackBySetting(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.startSetting != nullptr) {
        AbilityWindowConfiguration windowMode = static_cast<AbilityWindowConfiguration>(
            std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
        switch (windowMode) {
            case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING:
                return GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, true);
            case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY:
            case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY: {
                if (IsExistSplitScreenStack() && !InFrontOfFullScreenStack()) {
                    return GetOrCreateMissionStack(++splitScreenStackId, true);
                }
                return GetOrCreateMissionStack(SPLIT_SCREEN_MISSION_STACK_ID, true);
            }
            default:
                break;
        }
    }

    return defaultMissionStack_;
}

int AbilityStackManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());

    abilityRecord->SetScheduler(scheduler);
    if (abilityRecord->IsRestarting()) {
        abilityRecord->RestoreAbilityState();
    }
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token);

    return ERR_OK;
}

int AbilityStackManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        HILOG_INFO("Ability record may in terminate list.");
        abilityRecord = GetAbilityFromTerminateList(token);
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    }

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    HILOG_INFO("ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int AbilityStackManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    HILOG_DEBUG("Dispatch state.");
    switch (state) {
        case AbilityState::ACTIVE: {
            return DispatchActive(abilityRecord, state);
        }
        case AbilityState::INACTIVE: {
            return DispatchInactive(abilityRecord, state);
        }
        case AbilityState::BACKGROUND: {
            return DispatchBackground(abilityRecord, state);
        }
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord, state);
        }
        case AbilityState::FOREGROUND_NEW: {
            return DispatchForegroundNew(abilityRecord, state);
        }
        case AbilityState::BACKGROUND_NEW: {
            return DispatchBackgroundNew(abilityRecord, state);
        }
        default: {
            HILOG_WARN("Don't support transiting state: %d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int AbilityStackManager::DispatchActive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::ACTIVATING)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::ACTIVATING,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, abilityRecord->GetEventId());
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteActive(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

int AbilityStackManager::DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING) && !abilityRecord->IsNewVersion()) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::INACTIVATING,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecord->GetEventId());
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteInactive(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

int AbilityStackManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::MOVING_BACKGROUND)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::MOVING_BACKGROUND,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }

    // remove background timeout task.
    handler->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

int AbilityStackManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::TERMINATING)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::TERMINATING,
            abilityRecord->GetAbilityState(),
            state);
        return INNER_ERR;
    }

    // remove terminate timeout task.
    handler->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void AbilityStackManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HILOG_DEBUG("Ability request app state %{public}d done.", state);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    AppAbilityState abilitState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilitState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("ability: %{public}s", element.c_str());
        abilityRecord->Activate();
    }
}

void AbilityStackManager::OnAppStateChanged(const AppInfo &info)
{
}

void AbilityStackManager::CompleteActive(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
}

void AbilityStackManager::MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetIsNewWant(false);
    std::string backElement = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("Ability record: %{public}s", backElement.c_str());
    auto self(shared_from_this());
    auto task = [abilityRecord, self]() {
        HILOG_WARN("Stack manager move to background timeout.");
        if (abilityRecord->IsNewVersion()) {
            self->CompleteBackgroundNew(abilityRecord);
        } else {
            self->CompleteBackground(abilityRecord);
        }
    };
    abilityRecord->MoveToBackground(task);
}

void AbilityStackManager::CompleteInactive(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
}

void AbilityStackManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    sptr<Token> token = abilityRecord->GetToken();
    HILOG_INFO("ability: %{public}s", element.c_str());

    if (abilityRecord->GetAbilityState() == ACTIVATING || abilityRecord->GetAbilityState() == ACTIVE ||
        abilityRecord->GetAbilityState() == INITIAL) {
        HILOG_ERROR("Ability may be activing or active, it can't complete background.");
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(token);
    // Abilities ahead of the one started with SingleTask mode were put in terminate list, we need to terminate
    // them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityRecordList_) {
        if (terminateAbility->IsAbilityState(AbilityState::BACKGROUND)) {
            auto timeoutTask = [terminateAbility, self]() {
                HILOG_WARN("Disconnect ability terminate timeout.");
                self->CompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }

    if (abilityRecord->IsRestarting() && abilityRecord->IsAbilityState(AbilityState::BACKGROUND)) {
        auto timeoutTask = [abilityRecord, self]() {
            HILOG_WARN("disconnect ability terminate timeout.");
            self->CompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(timeoutTask);
    }
}

void AbilityStackManager::CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    if (!abilityRecord->IsAbilityState(AbilityState::TERMINATING)) {
        HILOG_ERROR("%{public}s, ability is not terminating.", __func__);
        return;
    }

    // notify AppMS terminate
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        // Don't return here
        HILOG_ERROR("AppMS fail to terminate ability.");
    }
#ifdef SUPPORT_GRAPHICS
    // destroy abilityRecord
    auto windowInfo = abilityRecord->GetWindowInfo();
    if (windowInfo != nullptr) {
        windowTokenToAbilityMap_.erase(windowInfo->windowToken_);
    }
#endif

    if (abilityRecord->IsRestarting()) {
        abilityRecord->SetAbilityState(AbilityState::INITIAL);
        abilityRecord->SetScheduler(nullptr);
        abilityRecord->LoadAbility();
    }
    for (auto it : terminateAbilityRecordList_) {
        if (it == abilityRecord) {
            terminateAbilityRecordList_.remove(it);
            HILOG_DEBUG("Destroy ability record count %ld", abilityRecord.use_count());
            break;
        }
        HILOG_WARN("Can't find ability in terminate list.");
    }
}

void AbilityStackManager::Dump(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (auto missionStack : missionStackList_) {
        missionStack->Dump(info);
    }
}

void AbilityStackManager::DumpStack(int missionStackId, std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (auto missionStack : missionStackList_) {
        if (missionStackId == missionStack->GetMissionStackId()) {
            missionStack->Dump(info);
            return;
        }
    }
    info.push_back("Invalid stack number, please see ability dump stack-list.");
    return;
}

void AbilityStackManager::DumpStackList(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (auto missionStack : missionStackList_) {
        missionStack->DumpStackList(info);
    }
}

void AbilityStackManager::DumpFocusMap(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (auto iter : focusAbilityRecordMap_) {
        std::string displayInfo = "  Display ID #" + std::to_string(iter.first);
        info.push_back(displayInfo);
        auto ability = iter.second;
        if (ability.lock()) {
            ability.lock()->Dump(info);
        }
    }
}

void AbilityStackManager::DumpWindowMode(std::vector<std::string> &info)
{
    auto dumpInfo = "window mode : " + AbilityStackManager::ConvertWindowModeState(curSysWindowMode_);
    info.push_back(dumpInfo);
}

void AbilityStackManager::DumpMission(int missionId, std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (auto missionStack : missionStackList_) {
        auto mission = missionStack->GetMissionRecordById(missionId);
        if (mission != nullptr) {
            mission->Dump(info);
            return;
        }
    }
    info.push_back("error: invalid mission number, please see 'ability dump --stack-list'.");
    return;
}

void AbilityStackManager::DumpTopAbility(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    auto topAbility = GetCurrentTopAbility();
    if (topAbility) {
        topAbility->Dump(info);
    }
    return;
}

void AbilityStackManager::EnqueueWaittingAbility(const AbilityRequest &abilityRequest)
{
    waittingAbilityQueue_.push(abilityRequest);
    return;
}

void AbilityStackManager::StartWaittingAbility()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto topAbility = GetCurrentTopAbility();
    CHECK_POINTER(topAbility);

    if (!(topAbility->IsAbilityState(ACTIVE) || topAbility->IsAbilityState(FOREGROUND_NEW))) {
        HILOG_INFO("Top ability is not active, must return for start waiting again.");
        return;
    }

    if (!waittingAbilityQueue_.empty()) {
        AbilityRequest abilityRequest = waittingAbilityQueue_.front();
        waittingAbilityQueue_.pop();
        StartAbilityLocked(topAbility, abilityRequest);
        return;
    }
}

void AbilityStackManager::GetMissionRecordAndAbilityRecord(const AbilityRequest &abilityRequest,
    const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &targetAbilityRecord,
    std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    HILOG_DEBUG("Get mission record and ability record.");
    CHECK_POINTER(currentMissionStack_);

    if (abilityRequest.startSetting) {
        auto windowkey = static_cast<AbilityWindowConfiguration>(
            std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
        HILOG_DEBUG("Start ability with settings ...");
        if (windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
            // start ability at splitcreen win mode
            GetRecordBySplitScreenMode(abilityRequest, targetAbilityRecord, targetMissionRecord);
            return;
        }
    }

    // The singleInstance start mode, mission name is #bundleName:abilityName.
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        GetRecordBySingleton(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
        // The standard start mode, mission name is bundle name by default.
    } else {
        GetRecordByStandard(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
    }
}

void AbilityStackManager::GetRecordBySplitScreenMode(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetAbilityRecord, std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    std::string bundleName = abilityRequest.abilityInfo.bundleName;
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        bundleName = AbilityUtil::ConvertBundleNameSingleton(
            abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    }
    auto topFullStack = GetTopFullScreenStackIncludeSplitScreen();
    CHECK_POINTER(topFullStack);
    auto topFullAbility = topFullStack->GetTopAbilityRecord();
    CHECK_POINTER(topFullStack);

    auto missionRecord = GetMissionRecordByName(bundleName);
    if (missionRecord == nullptr) {
        targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        targetMissionRecord = std::make_shared<MissionRecord>(bundleName);
    } else {
        CheckMissionRecordIsResume(missionRecord);
        if ((abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETOP &&
            missionRecord->IsTopAbilityRecordByName(abilityRequest.abilityInfo.name)) ||
            abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON ||
            !IsSplitScreenStack(missionRecord->GetMissionRecordId())) {
            targetAbilityRecord = missionRecord->GetTopAbilityRecord();
            targetAbilityRecord->SetWant(abilityRequest.want);
            targetAbilityRecord->SetIsNewWant(true);
        } else {
            targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        }
        targetMissionRecord = missionRecord;
    }
}

void AbilityStackManager::GetRecordBySingleton(const AbilityRequest &abilityRequest,
    const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &targetAbilityRecord,
    std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    std::string bundleName =
        AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    auto missionRecord = GetMissionRecordByName(bundleName);
    if (missionRecord == nullptr) {
        targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        targetMissionRecord = std::make_shared<MissionRecord>(bundleName);
    } else {
        targetAbilityRecord = missionRecord->GetTopAbilityRecord();
        if (targetAbilityRecord != nullptr) {
            targetAbilityRecord->SetWant(abilityRequest.want);
            targetAbilityRecord->SetIsNewWant(true);
            CheckMissionRecordIsResume(missionRecord);
            if (IsLockScreenState() && !DeleteMissionRecordInStackOnLockScreen(missionRecord)) {
                targetAbilityRecord = nullptr;
                targetMissionRecord = nullptr;
                return;
            }
        }
        targetMissionRecord = missionRecord;
    }
}

void AbilityStackManager::GetRecordByStandard(const AbilityRequest &abilityRequest,
    const std::shared_ptr<AbilityRecord> &currentTopAbility, std::shared_ptr<AbilityRecord> &targetAbilityRecord,
    std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    bool isStackChanged = false;
    if (currentTopAbility) {
        isStackChanged = (currentTopAbility->IsLauncherAbility() && !IsLauncherAbility(abilityRequest)) ||
                         (!currentTopAbility->IsLauncherAbility() && IsLauncherAbility(abilityRequest));
    }

    if (currentTopAbility == nullptr || (currentTopAbility && isStackChanged) ||
        (currentTopAbility && currentTopAbility->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON)) {
        // first get target mission record by bundleName
        auto missionRecord = GetMissionRecordByName(abilityRequest.abilityInfo.bundleName);
        if (missionRecord == nullptr) {
            targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
            targetMissionRecord = std::make_shared<MissionRecord>(abilityRequest.abilityInfo.bundleName);
        } else {
            /* If current top ability is singleton mode, target mission record will be changed.
             *  Check whether the requested ability is not at the top of the stack of the target mission,
             *  True: Need to create a new one . Other: Restart the top ability of this mission.
             */
            CheckMissionRecordIsResume(missionRecord);
            if (IsLockScreenState() && !DeleteMissionRecordInStackOnLockScreen(missionRecord)) {
                targetAbilityRecord = nullptr;
                targetMissionRecord = nullptr;
                return;
            }

            if (currentTopAbility && (!isStackChanged) &&
                (currentTopAbility->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON) &&
                ((!missionRecord->IsTopAbilityRecordByName(abilityRequest.abilityInfo.name)) ||
                (missionRecord->IsTopAbilityRecordByName(abilityRequest.abilityInfo.name) &&
                abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD))) {
                targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
            } else {
                targetAbilityRecord = missionRecord->GetTopAbilityRecord();
                if (targetAbilityRecord != nullptr) {
                    // The third-party app is called back from the desktop,
                    // and the specified slice route does not take effect.
                    targetAbilityRecord->SetWant(abilityRequest.want);
                    targetAbilityRecord->SetIsNewWant(true);
                }
            }
            targetMissionRecord = missionRecord;
        }
    } else if (currentTopAbility && (!isStackChanged)) {
        // The requested ability is already top ability. Reuse top ability.
        targetMissionRecord = currentTopAbility->GetMissionRecord();
        CheckMissionRecordIsResume(targetMissionRecord);
        if (IsLockScreenState() && !DeleteMissionRecordInStackOnLockScreen(targetMissionRecord)) {
            targetAbilityRecord = nullptr;
            targetMissionRecord = nullptr;
            return;
        }
        if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETOP &&
            currentTopAbility->GetMissionRecord()->IsTopAbilityRecordByName(abilityRequest.abilityInfo.name)) {
            targetAbilityRecord = currentTopAbility;
            targetAbilityRecord->SetWant(abilityRequest.want);
            targetAbilityRecord->SetIsNewWant(true);
        } else {
            targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        }
    }
}

bool AbilityStackManager::IsLauncherAbility(const AbilityRequest &abilityRequest) const
{
    return abilityRequest.abilityInfo.applicationInfo.isLauncherApp;
}

bool AbilityStackManager::IsLauncherMission(int id)
{
    if (launcherMissionStack_ == nullptr || launcherMissionStack_->GetMissionRecordById(id) == nullptr) {
        return false;
    }
    return true;
}

void AbilityStackManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
}

void AbilityStackManager::DelayedStartLauncher()
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto timeoutTask = [stackManager = shared_from_this()]() {
        HILOG_DEBUG("The launcher needs to be restarted.");
        stackManager->BackToLauncher();
    };
    handler->PostTask(timeoutTask, "Launcher_Restart", AbilityManagerService::RESTART_TIMEOUT);
}

std::shared_ptr<MissionRecord> AbilityStackManager::GetFriendMissionBySplitScreen(
    const std::shared_ptr<MissionStack> &stack, const int missionId)
{
    return nullptr;
}

void AbilityStackManager::HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
}

void AbilityStackManager::BackToLauncher()
{
    HILOG_INFO("Back to launcher.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER(defaultMissionStack_);
    CHECK_POINTER(launcherMissionStack_);

    auto fullScreenStack = GetTopFullScreenStackIncludeSplitScreen();
    CHECK_POINTER(fullScreenStack);
    auto currentTopAbility = fullScreenStack->GetTopAbilityRecord();
    if (currentTopAbility && (currentTopAbility->IsAbilityState(AbilityState::ACTIVE))) {
        HILOG_WARN("Current top ability is active, no need to start launcher.");
        return;
    }
    auto launcherAbility = GetLauncherRootAbility();
    CHECK_POINTER_LOG(launcherAbility, "There is no root launcher ability record, back to launcher failed.");

    MoveMissionStackToTop(launcherMissionStack_);

    auto missionRecord = launcherAbility->GetMissionRecord();
    CHECK_POINTER_LOG(missionRecord, "Can't get root launcher ability record's mission, back to launcher failed.");

    CheckMissionRecordIsResume(missionRecord);

    if (currentTopAbility) {
        launcherAbility->SetPreAbilityRecord(currentTopAbility);
        currentTopAbility->SetNextAbilityRecord(launcherAbility);

        missionRecord->SetPreMissionRecord(currentTopAbility->GetMissionRecord());
        if (currentTopAbility->IsLauncherAbility()) {
            missionRecord->SetIsLauncherCreate();
        }
    }
    launcherMissionStack_->MoveMissionRecordToTop(missionRecord);
    launcherAbility->ProcessActivate();
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetLauncherRootAbility() const
{
    return nullptr;
}

void AbilityStackManager::UninstallApp(const std::string &bundleName)
{
    HILOG_INFO("Uninstall app, bundleName: %{public}s", bundleName.c_str());
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto task = [bundleName, self = shared_from_this()]() { self->AddUninstallTags(bundleName); };
    handler->PostTask(task);
}

void AbilityStackManager::AddUninstallTags(const std::string &bundleName)
{
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetAbilityRecordByEventId(int64_t eventId) const
{
    return nullptr;
}

void AbilityStackManager::OnTimeOut(uint32_t msgId, int64_t eventId)
{
}

void AbilityStackManager::HandleActiveTimeout(const std::shared_ptr<AbilityRecord> &ability)
{
    HILOG_DEBUG("Handle active timeout");
    CHECK_POINTER(ability);
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());

    if (ability->IsLauncherRoot()) {
        HILOG_INFO("Launcher root load timeout, restart.");
        BackToLauncher();
        return;
    }

    auto missionRecord = ability->GetMissionRecord();
    CHECK_POINTER(missionRecord);
    auto stack = missionRecord->GetMissionStack();
    CHECK_POINTER(stack);
    missionRecord->RemoveAbilityRecord(ability);
    if (missionRecord->IsEmpty()) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(stack);
    }

    BackToLauncher();
}

void AbilityStackManager::ContinueLifecycle()
{
}

int AbilityStackManager::DispatchLifecycle(const std::shared_ptr<AbilityRecord> &lastTopAbility,
    const std::shared_ptr<AbilityRecord> &currentTopAbility, bool isTopFullScreen)
{
    return ERR_OK;
}

SystemWindowMode AbilityStackManager::GetLatestSystemWindowMode()
{
    bool isFloating = GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, false) != nullptr;
    bool isSplitScreen = IsExistSplitScreenStack();
    if (isFloating && isSplitScreen) {
        return SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE;
    } else if (isFloating) {
        return SystemWindowMode::FLOATING_WINDOW_MODE;
    } else if (isSplitScreen) {
        return SystemWindowMode::SPLITSCREEN_WINDOW_MODE;
    } else {
        return SystemWindowMode::DEFAULT_WINDOW_MODE;
    }
}

int AbilityStackManager::CompleteMissionMoving(std::shared_ptr<MissionRecord> &sourceMission, int stackId)
{
    CHECK_POINTER_AND_RETURN(sourceMission, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
    auto targetStack = GetOrCreateMissionStack(stackId, true);
    CHECK_POINTER_AND_RETURN(targetStack, ERR_INVALID_DATA);

    return ERR_OK;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetOrCreateMissionStack(int stackId, bool isCreateFlag)
{
    HILOG_DEBUG("Target stack id:%{public}d", stackId);
    if (stackId < MIN_MISSION_STACK_ID) {
        HILOG_ERROR("stack id:%{public}d is not defined.", stackId);
        return nullptr;
    }
    auto isExist = [stackId](
                       const std::shared_ptr<MissionStack> &stack) { return stackId == stack->GetMissionStackId(); };
    auto iter = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    if (iter != missionStackList_.end()) {
        HILOG_DEBUG("Get target mission stack success.");
        return *iter;
    }

    if (isCreateFlag) {
        HILOG_DEBUG("Target mission stack is not exist, need to create.");
        std::shared_ptr<MissionStack> missionStack = std::make_shared<MissionStack>(stackId, userId_);
        missionStackList_.push_back(missionStack);
#if BINDER_IPC_32BIT
        HILOG_DEBUG("Add stackId(%{public}d) to missionStackList, mission stack size:%{public}u",
            stackId,
            missionStackList_.size());
#else
        HILOG_DEBUG("Add stackId(%{public}d) to missionStackList, mission stack size:%{public}lu",
            stackId,
            missionStackList_.size());
#endif
        return missionStack;
    }

    HILOG_DEBUG("Target mission stack is not exist.");
    return nullptr;
}

int AbilityStackManager::GetMaxHoldMissionsByStackId(int stackId) const
{
    for (auto &it : stackSettings_) {
        if (it.stackId == stackId) {
            return it.maxHoldMission;
        }
    }
    return DEFAULT_INVAL_VALUE;
}

bool AbilityStackManager::SupportSyncVisualByStackId(int stackId) const
{
    if (stackId >= SPLIT_SCREEN_MISSION_STACK_ID) {
        return true;
    }

    for (auto &it : stackSettings_) {
        if (it.stackId == stackId && (stackId == FLOATING_MISSION_STACK_ID)) {
            return it.isSyncVisual;
        }
    }

    return false;
}

SystemWindowMode AbilityStackManager::JudgingTargetSystemWindowMode(AbilityWindowConfiguration config) const
{
    switch (config) {
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY:
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY:
            return SystemWindowMode::SPLITSCREEN_WINDOW_MODE;
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING:
            return SystemWindowMode::FLOATING_WINDOW_MODE;
        default:
            return SystemWindowMode::DEFAULT_WINDOW_MODE;
    }
}

int AbilityStackManager::JudgingTargetStackId(AbilityWindowConfiguration config) const
{
    switch (config) {
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY:
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY:
            return SPLIT_SCREEN_MISSION_STACK_ID;
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING:
            return FLOATING_MISSION_STACK_ID;
        default:
            return DEFAULT_MISSION_STACK_ID;
    }
}

int AbilityStackManager::StartLockMission(int uid, int missionId, bool isSystemApp, int isLock)
{
    return ERR_OK;
}

bool AbilityStackManager::CheckLockMissionCondition(
    int uid, int missionId, int isLock, bool isSystemApp, std::shared_ptr<MissionRecord> &mission, int &lockUid)
{
    return true;
}

bool AbilityStackManager::CanStartInLockMissionState(
    const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &currentTopAbility) const
{
    return false;
}

bool AbilityStackManager::CanStopInLockMissionState(const std::shared_ptr<AbilityRecord> &terminateAbility) const
{
    return true;
}

void AbilityStackManager::SendUnlockMissionMessage()
{
    HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
}

bool AbilityStackManager::IsTopInMission(const std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    CHECK_POINTER_RETURN_BOOL(abilityRecord);
    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER_RETURN_BOOL(missionRecord);

    return (missionRecord->GetTopAbilityRecord() == abilityRecord);
}

bool AbilityStackManager::IsFrontInAllStack(const std::shared_ptr<MissionStack> &stack) const
{
    return stack == missionStackList_.front();
}

bool AbilityStackManager::IsFullScreenStack(int stackId) const
{
    return (stackId == DEFAULT_MISSION_STACK_ID || stackId == LAUNCHER_MISSION_STACK_ID);
}

bool AbilityStackManager::IsLockScreenStack(int stackId) const
{
    return (stackId == LOCK_SCREEN_STACK_ID);
}

bool AbilityStackManager::IsSplitScreenStack(int stackId) const
{
    return (stackId >= SPLIT_SCREEN_MISSION_STACK_ID);
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTopFullScreenStack()
{
    auto isExist = [&](const std::shared_ptr<MissionStack> &stack) {
        return IsFullScreenStack(stack->GetMissionStackId());
    };
    auto iter = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    if (iter != missionStackList_.end()) {
        return (*iter);
    }
    return nullptr;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTopSplitScreenStack()
{
    auto isExist = [&](const std::shared_ptr<MissionStack> &stack) {
        return IsSplitScreenStack(stack->GetMissionStackId());
    };
    auto iter = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    if (iter != missionStackList_.end()) {
        return (*iter);
    }
    return nullptr;
}

std::shared_ptr<MissionStack> AbilityStackManager::GetTopFullScreenStackIncludeSplitScreen()
{
    auto isExist = [&](const std::shared_ptr<MissionStack> &stack) {
        return (IsFullScreenStack(stack->GetMissionStackId()) || IsSplitScreenStack(stack->GetMissionStackId()));
    };
    auto iter = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    if (iter != missionStackList_.end()) {
        return (*iter);
    }
    return nullptr;
}

bool AbilityStackManager::InFrontOfFullScreenStack() const
{
    auto isExist = [&](const std::shared_ptr<MissionStack> &stack) {
        return (stack->GetMissionStackId() == DEFAULT_MISSION_STACK_ID ||
                stack->GetMissionStackId() == LAUNCHER_MISSION_STACK_ID ||
                IsSplitScreenStack(stack->GetMissionStackId()));
    };
    auto iterExist = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    if (iterExist != missionStackList_.end()) {
        return IsSplitScreenStack((*iterExist)->GetMissionStackId());
    }

    return false;
}

bool AbilityStackManager::IsExistSplitScreenStack() const
{
    auto isExist = [&](const std::shared_ptr<MissionStack> &stack) {
        return IsSplitScreenStack(stack->GetMissionStackId());
    };
    auto iterExist = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    return iterExist != missionStackList_.end();
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetTopAbilityOfFullScreen()
{
    auto topFullStack = GetTopFullScreenStack();
    CHECK_POINTER_AND_RETURN(topFullStack, nullptr);
    return topFullStack->GetTopAbilityRecord();
}

void AbilityStackManager::JudgingIsRemoveMultiScreenStack(std::shared_ptr<MissionStack> &stack)
{
    HILOG_INFO("Judging is remove multi screen stack.");

    if (stack && !IsFullScreenStack(stack->GetMissionStackId()) && stack->IsEmpty()) {
        HILOG_DEBUG("Current stack is empty, remove.");
        missionStackList_.remove(stack);
        NotifyWindowModeChanged(GetLatestSystemWindowMode());
    }
}

void AbilityStackManager::NotifyWindowModeChanged(const SystemWindowMode &windowMode)
{
    HILOG_INFO("Notify window mode changed, will window mode: %{public}d", windowMode);
    if (curSysWindowMode_ == windowMode) {
        HILOG_ERROR("change mode is current mode");
        return;
    }

    HILOG_INFO("current mode : %{public}d , target window mode : %{public}d", curSysWindowMode_, windowMode);

    curSysWindowMode_ = windowMode;

    std::string data;
    switch (windowMode) {
        case SystemWindowMode::SPLITSCREEN_WINDOW_MODE:
            data = "SPLITSCREEN_WINDOW_MODE";
            break;
        case SystemWindowMode::FLOATING_WINDOW_MODE:
            data = "FLOATING_WINDOW_MODE";
            break;
        case SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE:
            data = "FLOATING_AND_SPLITSCREEN_WINDOW_MODE";
            break;
        default:
            data = "DEFAULT_WINDOW_MODE";
            break;
    }

    HILOG_INFO("Publish common event : system window mode changed");
    EventFwk::CommonEventData commonData;
    Want want;
    want.SetAction(AbilityConfig::EVENT_SYSTEM_WINDOW_MODE_CHANGED);
    commonData.SetWant(want);
    commonData.SetCode(AbilityConfig::EVENT_CODE_SYSTEM_WINDOW_MODE_CHANGED);
    commonData.SetData(data);
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void AbilityStackManager::UpdateFocusAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isNotify)
{
}

void AbilityStackManager::UpdateFocusAbilityRecord(
    int displayId, const std::shared_ptr<AbilityRecord> &focusAbility, bool isNotify)
{
    CHECK_POINTER(focusAbility);
    auto iter = focusAbilityRecordMap_.find(displayId);
    if (iter != focusAbilityRecordMap_.end()) {
        auto loseFocusAbility = iter->second.lock();
        if (loseFocusAbility && isNotify) {
            loseFocusAbility->TopActiveAbilityChanged(false);
        }
        focusAbilityRecordMap_.erase(iter);
    }

    HILOG_INFO("top active ability changed, displayId:%{public}d, name:%{public}s",
        displayId,
        focusAbility->GetAbilityInfo().name.c_str());

    if (isNotify) {
        focusAbility->TopActiveAbilityChanged(true);
        auto focusMission = focusAbility->GetMissionRecord();
        if (focusMission) {
            focusMission->UpdateActiveTimestamp();
        }
    }
    focusAbilityRecordMap_.emplace(displayId, focusAbility);
}

void AbilityStackManager::CheckMissionRecordIsResume(const std::shared_ptr<MissionRecord> &mission)
{
}

bool AbilityStackManager::IsLockScreenState()
{
    HILOG_INFO("Is Lock Screen State.");
    return isLockScreen_;
}

bool AbilityStackManager::DeleteMissionRecordInStackOnLockScreen(const std::shared_ptr<MissionRecord> &missionRecord)
{
    return true;
}

void AbilityStackManager::RestoreMissionRecordOnLockScreen(const std::shared_ptr<MissionRecord> &missionRecord)
{
}

std::string AbilityStackManager::ConvertWindowModeState(const SystemWindowMode &mode)
{
    auto it = windowModeToStrMap_.find(mode);
    if (it != windowModeToStrMap_.end()) {
        return it->second;
    }
    return "none";
}

void AbilityStackManager::DelayedStartLockScreenApp()
{
    HILOG_INFO("DelayedStartLockScreenApp.");
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto timeoutTask = [stackManager = shared_from_this()]() {
        HILOG_DEBUG("The lockscreen app needs to be restarted.");
        stackManager->BackToLockScreen();
    };
    handler->PostTask(timeoutTask, "LockScreen_Restart", AbilityManagerService::RESTART_TIMEOUT);
}

void AbilityStackManager::BackToLockScreen()
{
    HILOG_INFO("Back to lockScreen.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER(lockScreenMissionStack_);

    auto currentTopAbility = lockScreenMissionStack_->GetTopAbilityRecord();
    if (currentTopAbility && (currentTopAbility->IsAbilityState(AbilityState::ACTIVE) ||
        currentTopAbility->IsAbilityState(AbilityState::ACTIVATING))) {
        HILOG_WARN("Current top ability is active, no need to start launcher.");
        return;
    }
    auto lockscreenAbility = GetLockScreenRootAbility();
    CHECK_POINTER_LOG(lockscreenAbility, "There is no root lock screen ability record, back to lock screen failed.");

    MoveMissionStackToTop(lockScreenMissionStack_);

    auto missionRecord = lockscreenAbility->GetMissionRecord();
    CHECK_POINTER_LOG(
        missionRecord, "Can't get root lock screen ability record's mission, back to lock screen failed.");

    CheckMissionRecordIsResume(missionRecord);

    lockScreenMissionStack_->MoveMissionRecordToTop(missionRecord);
    lockscreenAbility->ProcessActivate();
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetLockScreenRootAbility() const
{
    return nullptr;
}

bool AbilityStackManager::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        AbilityConfig::LOCK_SCREEN_EVENT_NAME,
    };
    EventFwk::MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    return true;
}

void AbilityStackManager::ProcessInactivateInMoving(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->IsAbilityState(AbilityState::ACTIVE) ||
        abilityRecord->IsAbilityState(AbilityState::ACTIVATING)) {
        HILOG_DEBUG("ProcessInactivateInMoving...");
        abilityRecord->ProcessInactivateInMoving();
        return;
    }
    if (abilityRecord->IsAbilityState(AbilityState::INACTIVE) ||
        abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        HILOG_DEBUG("MoveToBackgroundTask...");
        MoveToBackgroundTask(abilityRecord);
        return;
    }
}

void AbilityStackManager::CompleteInactiveByNewVersion(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (!abilityRecord->IsNewVersion()) {
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    if (abilityRecord->IsSwitchingPause()) {
        MoveToBackgroundTask(abilityRecord);
        abilityRecord->SetSwitchingPause(false);
        return;
    }

    // 1. it may be inactive callback of terminate ability.
    if (abilityRecord->IsTerminating()) {
        abilityRecord->SendResultToCallers();
        if (abilityRecord->IsForceTerminate()) {
            HILOG_WARN("Ability is forced to terminate.");
            MoveToBackgroundTask(abilityRecord);
            return;
        }
        auto nextActiveAbility = abilityRecord->GetNextAbilityRecord();
        CHECK_POINTER_LOG(nextActiveAbility, "No top ability! Jump to launcher.");
        if (nextActiveAbility->IsAbilityState(ACTIVE) ||
            nextActiveAbility->IsAbilityState(FOREGROUND_NEW) ||
            nextActiveAbility->IsAbilityState(FOREGROUNDING_NEW)) {
            MoveToBackgroundTask(abilityRecord);
            UpdateFocusAbilityRecord(nextActiveAbility, true);
            return;
        }
        // need to specify the back ability as the current ability
        nextActiveAbility->SetBackAbilityRecord(abilityRecord);
        // top ability.has been pushed into stack, but haven't load.
        // so we need load it first
        nextActiveAbility->ProcessActivate();
        return;
    }

    if (abilityRecord->IsSwitchingPause()) {
        MoveToBackgroundTask(abilityRecord);
        return;
    }

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    auto nextAbilityRecord = abilityRecord->GetNextAbilityRecord();
    CHECK_POINTER_LOG(nextAbilityRecord, "Failed to get next ability record.");

    std::string nextElement = nextAbilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Next ability record: %{public}s", nextElement.c_str());
    nextAbilityRecord->ProcessActivate();
}

int AbilityStackManager::DispatchForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::FOREGROUNDING_NEW,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, abilityRecord->GetEventId());
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteForegroundNew(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void AbilityStackManager::CompleteForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND_NEW);

    DelayedSingleton<AbilityManagerService>::GetInstance()->NotifyBmsAbilityLifeStatus(
        abilityRecord->GetAbilityInfo().bundleName,
        abilityRecord->GetAbilityInfo().name,
        AbilityUtil::UTCTimeSeconds(),
        abilityRecord->GetApplicationInfo().uid);
#if BINDER_IPC_32BIT
    HILOG_INFO("notify bms ability life status, bundle name:%{public}s, ability name:%{public}s, time:%{public}lld",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str(),
        AbilityUtil::UTCTimeSeconds());
#else
    HILOG_INFO("notify bms ability life status, bundle name:%{public}s, ability name:%{public}s, time:%{public}ld",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str(),
        AbilityUtil::UTCTimeSeconds());
#endif

    auto self(shared_from_this());
    auto startWaittingAbilityTask = [self]() { self->StartWaittingAbility(); };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");

    /* PostTask to trigger start Ability from waiting queue */
    handler->PostTask(startWaittingAbilityTask, "startWaittingAbility");

    auto preAbilityRecord = abilityRecord->GetPreAbilityRecord();
    // 1. preAbility must be inactive when start ability.
    // move preAbility to background only if it was inactive.
    if (preAbilityRecord && !preAbilityRecord->IsNewVersion()) {
        auto preStackId = preAbilityRecord->GetMissionStackId();
        auto currentStackId = abilityRecord->GetMissionStackId();
        auto preMissionId = preAbilityRecord->GetMissionRecordId();
        auto currentMissionId = abilityRecord->GetMissionRecordId();
        auto isBackground =
            (!IsTopInMission(preAbilityRecord)) || preAbilityRecord->IsToEnd() ||
            ((IsFullScreenStack(preStackId) && IsFullScreenStack(currentStackId)) ||
                ((preStackId == FLOATING_MISSION_STACK_ID) && (currentStackId == FLOATING_MISSION_STACK_ID) &&
                    preMissionId == currentMissionId) ||
                ((preStackId == FLOATING_MISSION_STACK_ID) && (currentStackId == FLOATING_MISSION_STACK_ID) &&
                    !SupportSyncVisualByStackId(FLOATING_MISSION_STACK_ID) && preMissionId != currentMissionId));

        if (isBackground && preAbilityRecord->IsAbilityState(AbilityState::INACTIVE) &&
            !AbilityUtil::IsSystemDialogAbility(
            abilityRecord->GetAbilityInfo().bundleName, abilityRecord->GetAbilityInfo().name)) {
            std::string preElement = preAbilityRecord->GetWant().GetElement().GetURI();
            HILOG_INFO("Pre ability record: %{public}s", preElement.c_str());
            // preAbility was inactive ,resume new want flag to false
            MoveToBackgroundTask(preAbilityRecord);
            // Flag completed move to end.
            if (preAbilityRecord->IsToEnd()) {
                preAbilityRecord->SetToEnd(false);
            }
        }
    }

    // 2. nextAbility was in terminate list when terminate ability.
    // should move to background and then terminate.
    std::shared_ptr<AbilityRecord> nextAbilityRecord = abilityRecord->GetNextAbilityRecord();
    if (nextAbilityRecord != nullptr &&
        nextAbilityRecord->IsTerminating() &&
        (nextAbilityRecord->IsAbilityState(AbilityState::INACTIVE) ||
        nextAbilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW))) {
        std::string nextElement = nextAbilityRecord->GetWant().GetElement().GetURI();
        HILOG_INFO("Next ability record : %{public}s", nextElement.c_str());
        MoveToBackgroundTask(nextAbilityRecord);
    }

    // 3. when the mission ends and returns to lanucher directly, the next and back are inconsistent.
    // shoukd move back ability to background and then terminate.
    std::shared_ptr<AbilityRecord> backAbilityRecord = abilityRecord->GetBackAbilityRecord();
    if (backAbilityRecord != nullptr &&
        backAbilityRecord->IsTerminating() &&
        (backAbilityRecord->IsAbilityState(AbilityState::INACTIVE) ||
        backAbilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW)) &&
        (nextAbilityRecord == nullptr || nextAbilityRecord->GetRecordId() != backAbilityRecord->GetRecordId())) {
        std::string backElement = backAbilityRecord->GetWant().GetElement().GetURI();
        HILOG_INFO("Back ability record: %{public}s", backElement.c_str());
        MoveToBackgroundTask(backAbilityRecord);
    }
}

int AbilityStackManager::DispatchBackgroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING_NEW)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::BACKGROUNDING_NEW,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }

    // remove background timeout task.
    handler->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackgroundNew(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void AbilityStackManager::CompleteBackgroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    sptr<Token> token = abilityRecord->GetToken();
    HILOG_INFO("ability: %{public}s", element.c_str());

    auto abilityState = abilityRecord->GetAbilityState();
    if (abilityState != AbilityState::BACKGROUNDING_NEW) {
        HILOG_ERROR("Ability state is %{public}d, it can't complete background.", abilityState);
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND_NEW);

    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(token);
    // Abilities ahead of the one started with SingleTask mode were put in terminate list, we need to terminate
    // them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityRecordList_) {
        if (terminateAbility->IsAbilityState(AbilityState::BACKGROUND_NEW)) {
            auto timeoutTask = [terminateAbility, self]() {
                HILOG_WARN("Disconnect ability terminate timeout.");
                self->CompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }

    if (abilityRecord->IsRestarting() && abilityRecord->IsAbilityState(AbilityState::BACKGROUND_NEW)) {
        auto timeoutTask = [abilityRecord, self]() {
            HILOG_WARN("disconnect ability terminate timeout.");
            self->CompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(timeoutTask);
    }
}

bool AbilityStackManager::IsStarted()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto lanucherRoot = GetLauncherRootAbility();
    return lanucherRoot != nullptr;
}

void AbilityStackManager::PauseManager()
{
    HILOG_INFO("PauseManager.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto currentTopAbility = GetCurrentTopAbility();
    if (!currentTopAbility) {
        HILOG_WARN("get top ability failed.");
        return;
    }

    currentTopAbility->SetSwitchingPause(true);
    if (currentTopAbility->IsAbilityState(AbilityState::ACTIVE)) {
        currentTopAbility->ProcessInactivate();
    }
    if (currentTopAbility->IsAbilityState(AbilityState::INACTIVE)) {
        MoveToBackgroundTask(currentTopAbility);
    }
}

void AbilityStackManager::ResumeManager()
{
    HILOG_INFO("ResumeManager, back to launcher.");
    BackToLauncher();
}
}  // namespace AAFwk
}  // namespace OHOS
