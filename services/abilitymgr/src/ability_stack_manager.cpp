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

    resumeMissionContainer_ = std::make_shared<ResumeMissionContainer>(
        DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler());
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
    SetMissionStackSetting(floatSetting);
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
#ifdef SUPPORT_GRAPHICS
    CHECK_RET_RETURN_RET(CheckMultiWindowCondition(currentTopAbility, topFullAbility, abilityRequest),
        "Check multiwindow condition is failed.");
#endif
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

    std::list<MissionOption> missionOptions;
    MissionOption option;
    option.userId = userId_;
    option.missionId = targetMissionRecord->GetMissionRecordId();
    option.winModeKey = static_cast<AbilityWindowConfiguration>(
        std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
    targetMissionRecord->SetMissionOption(option);
    missionOptions.push_front(option);

    if (IsSplitScreenStack(targetStack->GetMissionStackId())) {
        GenerateMissinOptionsOfSplitScreen(missionOptions, targetStack, targetMissionRecord);
    }

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

    // 4. mission stack is changed, move mission to target stack.
    isMultiWinMoving_ = true;
    for (auto &it : missionOptions) {
        if (it.missionId == targetMissionRecord->GetMissionRecordId()) {
            CompleteMoveMissionToStack(targetMissionRecord, targetStack);
            MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);
            HILOG_DEBUG("Mission stack is changed, move mission to target stack ,missionId:%{public}d",
                targetMissionRecord->GetMissionRecordId());
            continue;
        }
        auto secondMission = GetMissionRecordFromAllStacks(it.missionId);
        if (secondMission) {
            CompleteMoveMissionToStack(secondMission, targetStack);
            HILOG_DEBUG("Mission stack is changed, move mission to target stack ,missionId:%{public}d",
                targetMissionRecord->GetMissionRecordId());
        }
    }

    CHECK_RET_RETURN_RET(DispatchLifecycle(currentTopAbility, targetAbilityRecord, false), "Dispatch lifecycle error.");
    // Judging target system window mode, and notify event.
    NotifyWindowModeChanged(GetLatestSystemWindowMode());
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
    // check mission window mode.
    if (targetAbilityRecord->GetMissionRecord() == nullptr) {
        auto option = targetMissionRecord->GetMissionOption();
        if (option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING ||
            option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
            auto setting = AbilityStartSetting::GetEmptySetting();
            setting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, std::to_string(option.winModeKey));
            targetAbilityRecord->SetStartSetting(setting);
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
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (missionId < 0) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }
    return RemoveMissionByIdLocked(missionId);
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

void AbilityStackManager::SetMissionStackSetting(const StackSetting &stackSetting)
{
    HILOG_DEBUG("Set mission stack setting, stackId : %{public}d", stackSetting.stackId);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return;
    }
    auto isExist = [stackId = stackSetting.stackId](const StackSetting &it) { return stackId == it.stackId; };
    auto iter = std::find_if(stackSettings_.begin(), stackSettings_.end(), isExist);
    if (iter != stackSettings_.end()) {
        stackSettings_.erase(iter);
    }
    StackSetting settings = stackSetting;
    stackSettings_.emplace_back(settings);
}

void AbilityStackManager::RemoveTerminatingAbility(
    const std::shared_ptr<AbilityRecord> &abilityRecord, const std::shared_ptr<AbilityRecord> &lastActiveAbility)
{
    HILOG_DEBUG("Remove terminating ability called.");
    CHECK_POINTER(abilityRecord);
    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER(missionRecord);
    auto currentStack = missionRecord->GetMissionStack();
    CHECK_POINTER(currentStack);

    auto isActive = (abilityRecord->IsActiveState() ||
                     abilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
                     abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW));

    missionRecord->RemoveAbilityRecord(abilityRecord);
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken());
    terminateAbilityRecordList_.push_back(abilityRecord);

    std::shared_ptr<AbilityRecord> needTopAbility;
    // clear next ability record
    abilityRecord->SetNextAbilityRecord(nullptr);

    if (IsLockScreenState() && abilityRecord->GetLockScreenState()) {
        abilityRecord->SetLockScreenState(false);
        if (!missionRecord->IsEmpty()) {
            missionRecord->GetBottomAbilityRecord()->SetLockScreenState(true);
        }
    }

    if (!missionRecord->IsEmpty()) {
        if (abilityRecord == lastActiveAbility) {
            needTopAbility = missionRecord->GetTopAbilityRecord();
            abilityRecord->SetNextAbilityRecord(needTopAbility);
        }
        return;
    }

    if (IsLockScreenState() && currentStack->GetMissionStackId() == LOCK_SCREEN_STACK_ID) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        if (currentStack->IsEmpty()) {
            missionStackList_.remove(lockScreenMissionStack_);
            currentMissionStack_ = missionStackList_.front();
            lockScreenMissionStack_.reset();
        } else {
            needTopAbility = currentStack->GetTopAbilityRecord();
            CHECK_POINTER(needTopAbility);
            abilityRecord->SetNextAbilityRecord(needTopAbility);
        }
        return;
    }

    // mission is empty, ability is background state, remove mission record
    if (!isActive) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        if (IsSplitScreenStack(abilityRecord->GetMissionStackId())) {
            auto topMission = currentStack->GetTopMissionRecord();
            CHECK_POINTER(topMission);

            UpdateMissionOption(
                topMission, defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);

            currentStack->RemoveMissionRecord(topMission->GetMissionRecordId());
            defaultMissionStack_->AddMissionRecordToEnd(topMission);
        }
        JudgingIsRemoveMultiScreenStack(currentStack);
        return;
    }

    // mission is empty and ability is active
    // current stack is floating stack
    if (currentStack->IsEqualStackId(FLOATING_MISSION_STACK_ID)) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(currentStack);
        MoveMissionStackToTop(GetTopFullScreenStackIncludeSplitScreen());
        if (abilityRecord == lastActiveAbility) {
            needTopAbility = GetCurrentTopAbility();
            abilityRecord->SetNextAbilityRecord(needTopAbility);
        }
        return;
    }

    // current stack is split screen stack
    if (IsSplitScreenStack(abilityRecord->GetMissionStackId())) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(currentStack);
        auto topMission = currentStack->GetTopMissionRecord();
        if (!topMission) {
            // split screen is empty, back to launcher
            auto launcherAbility = GetLauncherRootAbility();
            CHECK_POINTER(launcherAbility);
            auto launcherMission = launcherAbility->GetMissionRecord();
            CHECK_POINTER(launcherMission);
            MoveMissionStackToTop(launcherMissionStack_);
            launcherMissionStack_->MoveMissionRecordToTop(launcherMission);
            if (abilityRecord == lastActiveAbility) {
                abilityRecord->SetNextAbilityRecord(launcherAbility);
            }
        }

        if (topMission && abilityRecord == lastActiveAbility) {
            auto topAbility = topMission->GetTopAbilityRecord();
            RemoveMissionRecordById(topMission->GetMissionRecordId());
            JudgingIsRemoveMultiScreenStack(currentStack);
            UpdateMissionOption(
                topMission, defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);

            MoveMissionStackToTop(defaultMissionStack_);
            abilityRecord->SetNextAbilityRecord(topAbility);
        }
        return;
    }

    // current stack is default stack or launcher stack
    if (IsFullScreenStack(currentStack->GetMissionStackId())) {
        auto isExist = (!missionRecord->IsLauncherCreate() && missionRecord->GetPreMissionRecord() != nullptr &&
                        launcherMissionStack_->IsExistMissionRecord(
                            missionRecord->GetPreMissionRecord()->GetMissionRecordId()));
        if ((missionRecord->IsLauncherCreate()) || (missionRecord == missionStackList_.back()->GetTopMissionRecord()) ||
            isExist || (missionRecord == missionStackList_.front()->GetBottomMissionRecord())) {
            RemoveMissionRecordById(missionRecord->GetMissionRecordId());
            MoveMissionStackToTop(launcherMissionStack_);
        } else {
            RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        }
        auto fullScreenStack = GetTopFullScreenStack();
        CHECK_POINTER(fullScreenStack);
        needTopAbility = fullScreenStack->GetTopAbilityRecord();
        CHECK_POINTER(needTopAbility);
        if (abilityRecord == lastActiveAbility) {
            abilityRecord->SetNextAbilityRecord(needTopAbility);
        }
    }
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

void AbilityStackManager::AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_DEBUG("Add window id.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    // create WindowInfo and add to its AbilityRecord
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetWindowInfo()) {
        HILOG_DEBUG("WindowInfo is already added. Can't add again.");
        return;
    }

    if (!abilityRecord->IsAbilityState(ACTIVATING)) {
        HILOG_INFO("Add windowInfo at state: %{public}d.", abilityRecord->GetAbilityState());
    }
    if (windowTokenToAbilityMap_[windowToken] != nullptr) {
        // It shouldn't happen. Possible reasons for this case:
        // 1. windowmanager generates same window token.
        // 2. abilityms doesn't destroy ability in terminate process.
        HILOG_ERROR("Window token has been added to other AbilityRecord. ability name: %{private}s",
            abilityRecord->GetAbilityInfo().name.c_str());
    } else {
        abilityRecord->AddWindowInfo(windowToken);
        windowTokenToAbilityMap_[windowToken] = abilityRecord;
        HILOG_INFO("Add windowInfo complete, ability:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    }
#endif
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

void AbilityStackManager::DumpWaittingAbilityQueue(std::string &result)
{
    std::queue<AbilityRequest> copyQueue;
    {
        std::lock_guard<std::recursive_mutex> guard(stackLock_);
        if (waittingAbilityQueue_.empty()) {
            result = "The waitting ability queue is empty.";
            return;
        }
        copyQueue = waittingAbilityQueue_;
    }

    result = "User ID #" + std::to_string(userId_) + LINE_SEPARATOR;
    while (!copyQueue.empty()) {
        auto ability = copyQueue.front();
        std::vector<std::string> state;
        ability.Dump(state);

        for (auto it : state) {
            result += it;
            result += LINE_SEPARATOR;
        }
        copyQueue.pop();
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

int AbilityStackManager::SetMissionDescriptionInfo(
    const std::shared_ptr<AbilityRecord> &abilityRecord, const MissionDescriptionInfo &description)
{
    HILOG_DEBUG("%{public}s called", __FUNCTION__);
    CHECK_POINTER_AND_RETURN(abilityRecord, SET_MISSION_INFO_FAILED);

    auto mission = abilityRecord->GetMissionRecord();
    CHECK_POINTER_AND_RETURN(mission, SET_MISSION_INFO_FAILED);
    auto ptr = std::make_shared<MissionDescriptionInfo>(description);
    mission->SetMissionDescriptionInfo(ptr);

    return ERR_OK;
}

int AbilityStackManager::GetMissionLockModeState()
{
    HILOG_DEBUG("%{public}s called", __FUNCTION__);
    if (!lockMissionContainer_) {
        return LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_NONE;
    }

    return lockMissionContainer_->GetLockedMissionState();
}

void AbilityStackManager::RestartAbility(const std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("%{public}s called", __FUNCTION__);
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetRestarting(true);
    if (abilityRecord->IsAbilityState(AbilityState::ACTIVE) ||
        abilityRecord->IsAbilityState(AbilityState::ACTIVATING) ||
        abilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
        abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        abilityRecord->Inactivate();
    } else if (abilityRecord->IsAbilityState(AbilityState::INACTIVE) ||
               abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        MoveToBackgroundTask(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::BACKGROUND) ||
               abilityRecord->IsAbilityState(AbilityState::MOVING_BACKGROUND) ||
               abilityRecord->IsAbilityState(AbilityState::BACKGROUND_NEW) ||
               abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING_NEW)) {
        auto self(shared_from_this());
        auto timeoutTask = [abilityRecord, self]() {
            HILOG_WARN("disconnect ability terminate timeout.");
            self->CompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(timeoutTask);
    } else {
        HILOG_WARN("target ability can't be restarted.");
    }
}

int AbilityStackManager::MoveMissionToTop(int32_t missionId)
{
    HILOG_INFO("Move mission to top.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (missionId < 0) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }

    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }

    return MoveMissionToTopLocked(missionId);
}

int AbilityStackManager::MoveMissionToTopLocked(int32_t missionId)
{
    HILOG_INFO("Move mission to top locked.");

    if (isMultiWinMoving_) {
        HILOG_ERROR("System is moving multi window state, request deny.");
        return MOVE_MISSION_TO_STACK_MOVING_DENIED;
    }

    CHECK_TRUE_RETURN_RET((!defaultMissionStack_ || !launcherMissionStack_), ERR_NO_INIT, "Mission stack is invalid.");
    auto currentTopAbility = GetCurrentTopAbility();
    CHECK_POINTER_AND_RETURN(currentTopAbility, MOVE_MISSION_FAILED);

    auto currentStack = currentTopAbility->GetMissionRecord()->GetMissionStack();
    CHECK_POINTER_AND_RETURN(currentStack, MOVE_MISSION_FAILED);

    auto requestMissionRecord = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN(requestMissionRecord, MOVE_MISSION_FAILED);

    CheckMissionRecordIsResume(requestMissionRecord);

    auto requestAbilityRecord = requestMissionRecord->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN(requestAbilityRecord, MOVE_MISSION_FAILED);

    auto requestStack = requestMissionRecord->GetMissionStack();
    CHECK_POINTER_AND_RETURN(requestStack, MOVE_MISSION_FAILED);

    // request ability active, current ability active, change focus
    if (requestAbilityRecord->IsAbilityState(ACTIVE)) {
        MoveMissionStackToTop(requestStack);
        requestStack->MoveMissionRecordToTop(requestMissionRecord);
        UpdateFocusAbilityRecord(requestAbilityRecord, true);
        return ERR_OK;
    }

    // current stack is split screen stack or request stack is split screen stack.
    // Multiple split stacks are not supported
    if (IsSplitScreenStack(requestStack->GetMissionStackId()) ||
        IsSplitScreenStack(currentStack->GetMissionStackId())) {
        MoveMissionStackToTop(requestStack);
        requestStack->MoveMissionRecordToTop(requestMissionRecord);
        DispatchLifecycle(currentTopAbility, requestAbilityRecord, false);
        return ERR_OK;
    }

    if (currentStack->IsEqualStackId(FLOATING_MISSION_STACK_ID) &&
        IsFullScreenStack(requestStack->GetMissionStackId())) {
        auto fullScreenStack = GetTopFullScreenStack();
        CHECK_POINTER_AND_RETURN(fullScreenStack, MOVE_MISSION_FAILED);
        currentTopAbility = fullScreenStack->GetTopAbilityRecord();
    }

    if (IsFullScreenStack(currentStack->GetMissionStackId()) &&
        requestStack->IsEqualStackId(FLOATING_MISSION_STACK_ID)) {
        auto floatingStack = GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, false);
        auto topAbility = floatingStack->GetTopAbilityRecord();
        if (SupportSyncVisualByStackId(FLOATING_MISSION_STACK_ID) ||
            (!SupportSyncVisualByStackId(FLOATING_MISSION_STACK_ID) && requestAbilityRecord == topAbility)) {
            MoveMissionStackToTop(requestStack);
            requestAbilityRecord->ProcessActivate();
            return ERR_OK;
        }
        currentTopAbility = topAbility;
    }

    MoveMissionStackToTop(requestStack);
    MoveMissionAndAbility(currentTopAbility, requestAbilityRecord, requestMissionRecord);

    currentTopAbility->Inactivate();
    return ERR_OK;
}

int AbilityStackManager::MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }

    if (isMultiWinMoving_) {
        HILOG_ERROR("System is moving multi window state, request deny.");
        return MOVE_MISSION_TO_STACK_MOVING_DENIED;
    }

    return MoveMissionToEndLocked(token, nonFirst);
}

int AbilityStackManager::MoveMissionToEndLocked(const sptr<IRemoteObject> &token, const bool nonFirst)
{
    HILOG_INFO("Move mission to end locked.");

    CHECK_POINTER_AND_RETURN(token, MOVE_MISSION_FAILED);

    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, MOVE_MISSION_FAILED);

    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER_AND_RETURN(missionRecord, MOVE_MISSION_FAILED);

    if (!nonFirst) {
        if (missionRecord->GetBottomAbilityRecord() != abilityRecord) {
            HILOG_ERROR("nonFirst is false, it's not the bottom of the mission, can't move mission to end.");
            return MOVE_MISSION_FAILED;
        }
    }

    return MoveMissionToEndLocked(missionRecord->GetMissionRecordId());
}

int AbilityStackManager::MoveMissionToEndLocked(int missionId)
{
    return ERR_OK;
}

void AbilityStackManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("On ability died.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(launcherMissionStack_);
    CHECK_POINTER(defaultMissionStack_);

    if (abilityRecord->GetAbilityInfo().type != AbilityType::PAGE) {
        HILOG_ERROR("Ability type is not page.");
        return;
    }
    // release mission when locked.
    auto mission = abilityRecord->GetMissionRecord();
    if (mission && lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        if (lockMissionContainer_->IsSameLockedMission(mission->GetName())) {
            lockMissionContainer_->ReleaseLockedMission(mission, -1, true);
        }
    }
    HandleAbilityDied(abilityRecord);
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

void AbilityStackManager::UpdateMissionOption(const std::shared_ptr<MissionRecord> &mission,
    const std::shared_ptr<MissionStack> &moveToStack, const AbilityWindowConfiguration winModeKey)
{
    MissionOption option;
    option.missionId = mission->GetMissionRecordId();
    option.winModeKey = winModeKey;
    moveToStack->AddMissionRecordToTop(mission);
    moveToStack->MoveMissionRecordToTop(mission);
    mission->SetMissionStack(moveToStack, moveToStack->GetMissionStackId());
    mission->SetMissionOption(option);
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
    HILOG_DEBUG("On timeout.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto abilityRecord = GetAbilityRecordByEventId(eventId);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("stack manager on time out event: ability record is nullptr.");
        BackToLauncher();
        return;
    }

    // release mission when locked.
    auto mission = abilityRecord->GetMissionRecord();
    if (mission && lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        if (lockMissionContainer_->IsSameLockedMission(mission->GetName())) {
            lockMissionContainer_->ReleaseLockedMission(mission, -1, true);
        }
    }

    // release mission when lock screen.
    auto stack = mission->GetMissionStack();
    CHECK_POINTER(stack);
    if (stack->GetMissionStackId() == LOCK_SCREEN_STACK_ID && (msgId == AbilityManagerService::LOAD_TIMEOUT_MSG ||
        msgId == AbilityManagerService::ACTIVE_TIMEOUT_MSG)) {
        ActiveTopAbility(abilityRecord);
        return;
    }
    CHECK_POINTER_LOG(abilityRecord, "stack manager on time out event: ability record is nullptr.");

    HILOG_DEBUG("Ability timeout ,msg:%{public}d,name:%{public}s", msgId, abilityRecord->GetAbilityInfo().name.c_str());

    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            ActiveTopAbility(abilityRecord);
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            HandleActiveTimeout(abilityRecord);
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
        case AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG:
            CompleteInactive(abilityRecord);
            break;
        default:
            break;
    }
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

int AbilityStackManager::MoveMissionToFloatingStack(const MissionOption &missionOption)
{
    HILOG_DEBUG("Move mission to floating stack, missionId: %{public}d", missionOption.missionId);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_TRUE_RETURN_RET(lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState(),
        ERR_INVALID_DATA,
        "current is lock mission state, refusing to operate other mission.");
    CHECK_TRUE_RETURN_RET(missionOption.missionId == DEFAULT_INVAL_VALUE, ERR_INVALID_DATA, "mission id is invalid.");
    CHECK_TRUE_RETURN_RET(
        JudgingTargetSystemWindowMode(missionOption.winModeKey) != SystemWindowMode::FLOATING_WINDOW_MODE,
        ERR_INVALID_DATA,
        "The requested mode is error.");

    std::list<MissionOption> missionOptions;
    MissionOption option = missionOption;
    missionOptions.push_back(option);

    return MoveMissionsToStackLocked(missionOptions);
}

int AbilityStackManager::MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary)
{
    HILOG_DEBUG("primary:%{public}d, winMode:%{public}d, secondary:%{public}d, "
                "winMode:%{public}d",
        primary.missionId,
        primary.winModeKey,
        secondary.missionId,
        secondary.winModeKey);

    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_TRUE_RETURN_RET(lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState(),
        ERR_INVALID_DATA,
        "current is lock mission state, refusing to operate other mission.");
    CHECK_TRUE_RETURN_RET(primary.missionId == DEFAULT_INVAL_VALUE, ERR_INVALID_DATA, "mission id is invalid.");
    CHECK_TRUE_RETURN_RET(
        JudgingTargetSystemWindowMode(primary.winModeKey) != SystemWindowMode::SPLITSCREEN_WINDOW_MODE,
        ERR_INVALID_DATA,
        "The requested mode is error.");

    std::list<MissionOption> missionOptions;
    CHECK_RET_RETURN_RET(
        GenerateMissinOptionsOfSplitScreen(primary, secondary, missionOptions), "generate mission option error.");

    return MoveMissionsToStackLocked(missionOptions);
}

int AbilityStackManager::GenerateMissinOptionsOfSplitScreen(std::list<MissionOption> &missionOptions,
    const std::shared_ptr<MissionStack> &targetStack, std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    HILOG_DEBUG("generate missin options of split screen stack.");
    // first create splitscreen stack
    if (targetStack->IsEmpty()) {
        auto topAbility = GetTopAbilityOfFullScreen();
        CHECK_POINTER_AND_RETURN(topAbility, ERR_INVALID_DATA);
        auto topMission = topAbility->GetMissionRecord();
        CHECK_POINTER_AND_RETURN(topMission, ERR_INVALID_DATA);
        MissionOption optionSecondary;
        optionSecondary.winModeKey = (targetMissionRecord->GetMissionOption().winModeKey ==
                                         AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY)
                                         ? AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY
                                         : AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
        optionSecondary.missionId = topAbility->GetMissionRecordId();
        optionSecondary.userId = userId_;
        topMission->SetMissionOption(optionSecondary);
        missionOptions.push_front(optionSecondary);
        HILOG_DEBUG("splitscreen stack will add two missions together.");
    } else {
        auto missions = targetStack->GetMissionRecordByWinMode(targetMissionRecord->GetMissionOption().winModeKey);
        for (auto &it : missions) {
            if (it->GetMissionRecordId() != targetMissionRecord->GetMissionRecordId() && missions.size() > 1) {
                MissionOption optionSecondary;
                optionSecondary.winModeKey = (targetMissionRecord->GetMissionOption().winModeKey ==
                                                 AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY)
                                                 ? AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY
                                                 : AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
                optionSecondary.missionId = it->GetMissionRecordId();
                optionSecondary.userId = userId_;
                it->SetMissionOption(optionSecondary);
                missionOptions.push_front(optionSecondary);
                HILOG_DEBUG("splitscreen stack will swap.");
                break;
            }
        }
    }
    return ERR_OK;
}

int AbilityStackManager::GenerateMissinOptionsOfSplitScreen(
    const MissionOption &primary, const MissionOption &secondary, std::list<MissionOption> &missionOptions)
{
    MissionOption optionPrimary = primary;
    missionOptions.push_front(optionPrimary);
    if (secondary.missionId > DEFAULT_INVAL_VALUE) {
        if (JudgingTargetSystemWindowMode(secondary.winModeKey) != SystemWindowMode::SPLITSCREEN_WINDOW_MODE ||
            primary.winModeKey == secondary.winModeKey || primary.missionId == secondary.missionId) {
            HILOG_ERROR("mission option  is invalid.");
            return ERR_INVALID_DATA;
        }
        MissionOption optionSecondary = secondary;
        missionOptions.push_front(optionSecondary);
        HILOG_DEBUG("splitscreen stack will add two missions together.");
    } else {
        // select top mission of full screen as secondary when second parameter is not set.
        if (!IsExistSplitScreenStack()) {
            auto topAbility = GetTopAbilityOfFullScreen();
            CHECK_TRUE_RETURN_RET(topAbility && primary.missionId == topAbility->GetMissionRecordId(),
                ERR_INVALID_DATA,
                "mission option  is invalid.");
            MissionOption optionSecondary;
            optionSecondary.winModeKey =
                (primary.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY)
                    ? AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY
                    : AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
            optionSecondary.missionId = topAbility->GetMissionRecordId();
            optionSecondary.userId = userId_;
            missionOptions.push_front(optionSecondary);
            HILOG_DEBUG("splitscreen stack will add two missions together.");
        } else {
            HILOG_DEBUG("splitscreen stack will emplace mission, winmode: %{public}d", primary.winModeKey);
        }
    }
    return ERR_OK;
}

int AbilityStackManager::MoveMissionsToStackLocked(const std::list<MissionOption> &missionOptions)
{
#if BINDER_IPC_32BIT
    HILOG_DEBUG("Request mission option size :%{public}u", missionOptions.size());
#else
    HILOG_DEBUG("Request mission option size :%{public}lu", missionOptions.size());
#endif
#ifdef SUPPORT_GRAPHICS
    // check condition whether can enter or exit multiwindow mode.
    CHECK_RET_RETURN_RET(CheckMultiWindowCondition(missionOptions), "check multiwindow condition is failed.");
#endif
    // complete mission stack moving and notify ability window mode has changed.
    auto lastTopAbility = GetCurrentTopAbility();
    isMultiWinMoving_ = true;

    auto targetStackId = JudgingTargetStackId(missionOptions.front().winModeKey);
    // new mission stack
    if (targetStackId == SPLIT_SCREEN_MISSION_STACK_ID) {
        if (IsExistSplitScreenStack() && !InFrontOfFullScreenStack()) {
            targetStackId = ++splitScreenStackId;
        }
    }
    for (auto &it : missionOptions) {
        auto sourceMission = GetMissionRecordFromAllStacks(it.missionId);
        CheckMissionRecordIsResume(sourceMission);
        CHECK_POINTER_AND_RETURN(sourceMission, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
        sourceMission->SetMissionOption(it);
        CHECK_RET_RETURN_RET(CompleteMissionMoving(sourceMission, targetStackId), "complete mission moving failed.");
    }

    // schulde ability lifescycle (all stack)
    auto currentTopAbility = GetCurrentTopAbility();
    bool isFullScreen =
        missionOptions.front().winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    // top ability active and display fullscreen, others background state.
    CHECK_RET_RETURN_RET(
        DispatchLifecycle(lastTopAbility, currentTopAbility, isFullScreen), "Dispatch lifecycle error.");

    // Judging target system window mode, and notify event.
    NotifyWindowModeChanged(GetLatestSystemWindowMode());

    return ERR_OK;
}

int AbilityStackManager::MaximizeMultiWindow(int missionId)
{
    HILOG_DEBUG("Maximize multi window, missionId:%{public}d", missionId);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }

    auto missionRecord = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN(missionRecord, MAXIMIZE_MULTIWINDOW_NOT_EXIST);
    auto parentStack = missionRecord->GetMissionStack();
    CHECK_POINTER_AND_RETURN(parentStack, ERR_INVALID_DATA);
    if (!parentStack->IsEqualStackId(FLOATING_MISSION_STACK_ID)) {
        HILOG_ERROR("This mission is not exist in multiwindow stack.");
        return MAXIMIZE_MULTIWINDOW_NOT_EXIST;
    }
    auto topAbility = missionRecord->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN(topAbility, ERR_INVALID_DATA);
    if (!topAbility->IsAbilityState(AbilityState::INACTIVE) && !topAbility->IsAbilityState(AbilityState::ACTIVE)) {
        HILOG_ERROR("This mission is not visible, maximize failed.");
        return MAXIMIZE_MULTIWINDOW_FAILED;
    }
    // construct mission option, request to move mission to default stack.
    MissionOption option;
    option.missionId = missionId;
    option.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    std::list<MissionOption> missionOptions;
    missionOptions.push_back(option);

    return MoveMissionsToStackLocked(missionOptions);
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

#ifdef SUPPORT_GRAPHICS
int AbilityStackManager::CheckMultiWindowCondition(const std::list<MissionOption> &missionOptions) const
{
    HILOG_DEBUG("Check multi window condition.");
    if (isMultiWinMoving_) {
        HILOG_ERROR("System is moving multi window state, request deny.");
        return MOVE_MISSION_TO_STACK_MOVING_DENIED;
    }

    auto currentTopAbilityRecord = GetCurrentTopAbility();
    if (currentTopAbilityRecord &&
        AbilityUtil::IsSystemDialogAbility(
        currentTopAbilityRecord->GetAbilityInfo().bundleName, currentTopAbilityRecord->GetAbilityInfo().name)) {
        HILOG_ERROR("Top page ability is dialog type, cannot return to launcher");
        return MOVE_MISSION_TO_STACK_MOVING_DENIED;
    }

    if (missionOptions.empty() || missionOptions.size() > MAX_CAN_MOVE_MISSIONS) {
        HILOG_ERROR("Moving missions has been out of size, Maximum:%{public}d.", MAX_CAN_MOVE_MISSIONS);
        return MOVE_MISSION_TO_STACK_OUT_OF_SIZE;
    }

    // check request missions exist, and has same window mode.
    auto lastOption = missionOptions.front();
    bool isFirst = true;
    for (auto &it : missionOptions) {
        if (!isFirst && !it.IsSameWindowMode(lastOption.winModeKey)) {
            HILOG_ERROR("Mission options are not in same win mode.");
            return MOVE_MISSION_TO_STACK_NOT_SAME_WIN_MODE;
        }
        isFirst = false;
        auto targetMission = GetMissionRecordFromAllStacks(it.missionId);
        if (!targetMission) {
            HILOG_ERROR("Moving mission record is not exist.");
            return MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION;
        }
        auto parentStack = targetMission->GetMissionStack();
        if (parentStack && parentStack->GetMissionStackId() == JudgingTargetStackId(it.winModeKey)) {
            HILOG_ERROR("Refuse moving mission to its own stack.");
            return MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION;
        }
        if (!targetMission->SupportMultWindow()) {
            HILOG_ERROR("Moving mission record is not support multi window.");
            return MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN;
        }
        if (IsExistSplitScreenStack() && !InFrontOfFullScreenStack() &&
            (it.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            it.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY)) {
            HILOG_ERROR("split screen stack is exist, but not at the top, replacing misiion or creating a new split "
                        "stack is not supported.");
            return MOVE_MISSION_TO_STACK_MOVING_DENIED;
        }
    }

    // check whether target mission stack will be overflow.
    if (!CheckMissionStackWillOverflow(missionOptions)) {
        HILOG_ERROR("Mission stack will overflow, refuse to move mission.");
        return MOVE_MISSION_TO_STACK_TARGET_STACK_OVERFLOW;
    }

    return ERR_OK;
}

int AbilityStackManager::CheckMultiWindowCondition(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &topFullAbility, const AbilityRequest &abilityRequest) const
{
    HILOG_DEBUG("Check multi window condition.");
    CHECK_POINTER_AND_RETURN_LOG(abilityRequest.startSetting, START_ABILITY_SETTING_FAILED, "startsetting is nullptr.");

    if (isMultiWinMoving_) {
        HILOG_ERROR("System is moving multi window state, request deny.");
        return START_ABILITY_SETTING_FAILED;
    }

    if (currentTopAbility &&
        AbilityUtil::IsSystemDialogAbility(
            currentTopAbility->GetAbilityInfo().bundleName, currentTopAbility->GetAbilityInfo().name)) {
        HILOG_ERROR("Top page ability is dialog type, cannot return to launcher.");
        return START_ABILITY_SETTING_FAILED;
    }
    // check whether target mission stack will be overflow.
    MissionOption option;
    option.winModeKey = static_cast<AbilityWindowConfiguration>(
        std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
    std::list<MissionOption> list;
    list.push_back(option);

    // check splitscreen stack request.
    if (option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        if (!CheckSplitSrceenCondition(abilityRequest, topFullAbility)) {
            HILOG_ERROR("check splitscreen condition failed.");
            return START_ABILITY_SETTING_FAILED;
        }
    }

    if (!CheckMissionStackWillOverflow(list)) {
        HILOG_ERROR("Mission stack will overflow.");
        return START_ABILITY_SETTING_NOT_SUPPORT_MULTI_WIN;
    }

    return ERR_OK;
}
#endif

bool AbilityStackManager::CheckSplitSrceenCondition(
    const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &topFullAbility) const
{
    if (IsExistSplitScreenStack()) {
        if (!InFrontOfFullScreenStack()) {
            HILOG_ERROR("split screen stack is exist, but not at the top, replacing misiion or creating a new split "
                        "stack is not supported.");
            return false;
        }
    } else {
        // will create a new splitscreen stack, check current condition.
        if (topFullAbility->IsLauncherAbility()) {
            HILOG_ERROR("can't move launcher to splitscreen stack.");
            return false;
        } else {
            if (topFullAbility->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON) {
                if (topFullAbility->GetAbilityInfo().bundleName == abilityRequest.abilityInfo.bundleName &&
                    topFullAbility->GetAbilityInfo().name == abilityRequest.abilityInfo.name) {
                    HILOG_ERROR("can't only move full screen ability to splitscreen stack.");
                    return false;
                }
            } else {
                if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON &&
                    topFullAbility->GetAbilityInfo().name == abilityRequest.abilityInfo.name) {
                    HILOG_ERROR("can't only move full screen ability to splitscreen stack.");
                    return false;
                }
            }
        }
    }

    return true;
}

bool AbilityStackManager::CheckMissionStackWillOverflow(const std::list<MissionOption> &missionOptions) const
{
    std::map<int, int> calcMap;
    for (auto &it : missionStackList_) {
        calcMap.emplace(it->GetMissionStackId(), it->GetMissionRecordCount());
    }

    auto isOverFlow = [&](int stackId, std::map<int, int> &calcMap) {
        calcMap[stackId]++;
        if (GetMaxHoldMissionsByStackId(stackId) > 0 && calcMap[stackId] > GetMaxHoldMissionsByStackId(stackId)) {
            HILOG_ERROR(
                "Over limit of holding mission, stackId:%{public}d , size:%{public}d", stackId, calcMap[stackId]);
            return false;
        }
        return true;
    };

    // check whether splitscreen and floating stack will overflow.
    for (auto &option : missionOptions) {
        auto stackId = JudgingTargetStackId(option.winModeKey);
        if (!isOverFlow(stackId, calcMap)) {
            return false;
        }
    }

    return true;
}

int AbilityStackManager::CompleteMissionMoving(std::shared_ptr<MissionRecord> &sourceMission, int stackId)
{
    CHECK_POINTER_AND_RETURN(sourceMission, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
    auto targetStack = GetOrCreateMissionStack(stackId, true);
    CHECK_POINTER_AND_RETURN(targetStack, ERR_INVALID_DATA);

    CHECK_RET_RETURN_RET(CompleteMoveMissionToStack(sourceMission, targetStack), "Moving mission record failed.");

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

int AbilityStackManager::CompleteMoveMissionToStack(
    const std::shared_ptr<MissionRecord> &sourceMission, const std::shared_ptr<MissionStack> &targetStack)
{
    CHECK_POINTER_AND_RETURN(sourceMission, ERR_INVALID_DATA);
    CHECK_POINTER_AND_RETURN(targetStack, ERR_INVALID_DATA);
    auto sourceStack = sourceMission->GetMissionStack();
    if (sourceStack == nullptr) {
        // new create mission, directly move to target mission stack
        EmplaceMissionToStack(sourceMission, targetStack);
        sourceMission->SetMissionStack(targetStack, targetStack->GetMissionStackId());
        MoveMissionStackToTop(targetStack);
        return ERR_OK;
    }

    bool isTop = sourceMission->IsLauncherCreate() && sourceMission == GetTopMissionRecord();
    HILOG_DEBUG("Mission reparent : src stack id:%{public}d, target stack id:%{public}d, isTop:%{public}d",
        sourceStack->GetMissionStackId(),
        targetStack->GetMissionStackId(),
        isTop);

    // remove mission record from source stack.
    if (IsSplitScreenStack(sourceStack->GetMissionStackId())) {
        auto oldMission = GetFriendMissionBySplitScreen(sourceStack, sourceMission->GetMissionRecordId());
        if (oldMission != nullptr) {
            // put the removed mission into the default stack.
            sourceStack->RemoveMissionRecord(oldMission->GetMissionRecordId());
            UpdateMissionOption(
                oldMission, defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
            MoveMissionStackToTop(defaultMissionStack_);
        }
    }

    sourceStack->RemoveMissionRecord(sourceMission->GetMissionRecordId());
    EmplaceMissionToStack(sourceMission, targetStack);

    sourceMission->SetMissionStack(targetStack, targetStack->GetMissionStackId());
    if ((sourceStack->IsEmpty() || isTop) && targetStack != defaultMissionStack_ &&
        sourceStack == defaultMissionStack_) {
        MoveMissionStackToTop(launcherMissionStack_);
    }
    if (sourceStack->IsEmpty() && (!IsFullScreenStack(sourceStack->GetMissionStackId()))) {
        HILOG_DEBUG("Remove stack when mission count is zero. stack:%{public}d.", sourceStack->GetMissionStackId());
        missionStackList_.remove(sourceStack);
    }
    // move target mission stack to top.
    MoveMissionStackToTop(targetStack);

    return ERR_OK;
}

void AbilityStackManager::EmplaceMissionToStack(
    const std::shared_ptr<MissionRecord> &sourceMission, const std::shared_ptr<MissionStack> &targetStack)
{
    if (IsSplitScreenStack(targetStack->GetMissionStackId())) {
        // emplace mission to splitscreen stack.
        auto oldMission =
            targetStack->EmplaceMissionRecord(sourceMission->GetMissionOption().winModeKey, sourceMission);
        if (oldMission != nullptr) {
            // put the removed mission into the default stack.
            UpdateMissionOption(
                oldMission, defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);
        }
        targetStack->MoveMissionRecordToTop(sourceMission);
    } else {
        // add mission to target mission stack top.
        targetStack->AddMissionRecordToTop(sourceMission);
        targetStack->MoveMissionRecordToTop(sourceMission);
    }
}

void AbilityStackManager::ActiveTopAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_INFO("Active top ability.");
    CHECK_POINTER(abilityRecord);

    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());

    if (abilityRecord->IsLauncherRoot()) {
        HILOG_INFO("Launcher root load timeout, restart.");
        BackToLauncher();
        return;
    }
    if (abilityRecord->IsLockScreenRoot()) {
        HILOG_INFO("lock screen root load timeout, restart.");
        DelayedStartLockScreenApp();
        return;
    }

    // Pre ability does not need to judge null
    auto preAbility = abilityRecord->GetPreAbilityRecord();
    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER(missionRecord);
    auto stack = missionRecord->GetMissionStack();
    CHECK_POINTER(stack);

    // remove ability and mission and stack
    missionRecord->RemoveAbilityRecord(abilityRecord);
    if (missionRecord->IsEmpty()) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(stack);
    }

    if (preAbility) {
        HILOG_INFO("Load timeout, restart pre ability.");
        if (preAbility->IsAbilityState(ACTIVE)) {
            auto preMission = preAbility->GetMissionRecord();
            CHECK_POINTER(preMission);
            auto preStack = preMission->GetMissionStack();
            CHECK_POINTER(preStack);
            if (IsSplitScreenStack(preStack->GetMissionStackId())) {
                preStack->RemoveMissionRecord(preMission->GetMissionRecordId());
                JudgingIsRemoveMultiScreenStack(preStack);
                UpdateMissionOption(
                    preMission, defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);

                MoveMissionStackToTop(defaultMissionStack_);
            } else {
                MoveMissionStackToTop(preStack);
            }
            UpdateFocusAbilityRecord(preAbility, true);
        }
        if (preAbility->IsAbilityState(INACTIVE)) {
            MoveMissionStackToTop(preAbility->GetMissionRecord()->GetMissionStack());
            preAbility->ProcessActivate();
        }
    } else {
        auto topFullScreenStack = GetTopFullScreenStackIncludeSplitScreen();
        CHECK_POINTER(topFullScreenStack);
        auto topFullScreenAbility = topFullScreenStack->GetTopAbilityRecord();
        if (topFullScreenAbility) {
            HILOG_INFO("Load timeout, top full screen ability restart.");
            MoveMissionStackToTop(topFullScreenStack);
            if (topFullScreenAbility->IsAbilityState(ACTIVE)) {
                UpdateFocusAbilityRecord(preAbility, true);
            }
            if (topFullScreenAbility->IsAbilityState(INACTIVE)) {
                topFullScreenAbility->ProcessActivate();
            }
        } else {
            HILOG_INFO("Load timeout, back to launcher.");
            BackToLauncher();
        }
    }
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

bool AbilityStackManager::IsFirstInMission(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Is first in mission.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_POINTER_RETURN_BOOL(token);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_RETURN_BOOL(abilityRecord);

    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER_RETURN_BOOL(missionRecord);

    return (missionRecord->GetBottomAbilityRecord() == abilityRecord);
}

int AbilityStackManager::StartLockMission(int uid, int missionId, bool isSystemApp, int isLock)
{
    HILOG_INFO("%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    if (lockMissionContainer_ == nullptr) {
        lockMissionContainer_ = std::make_shared<LockMissionContainer>();
        CHECK_POINTER_AND_RETURN(lockMissionContainer_, INNER_ERR);
    }

    std::shared_ptr<MissionRecord> missionRecord;
    int lockUid = -1;
    if (!CheckLockMissionCondition(uid, missionId, isLock, isSystemApp, missionRecord, lockUid)) {
        HILOG_ERROR("check lock mission condition failed.");
        return LOCK_MISSION_DENY_FAILED;
    }

    // lock mission
    if (isLock) {
        if (!lockMissionContainer_->SetLockedMission(missionRecord, lockUid, isSystemApp)) {
            HILOG_ERROR("lock mission error.");
            return LOCK_MISSION_DENY_FAILED;
        }
        // notify wms mission locked state.

        // move lock mission to top
        return MoveMissionToTopLocked(missionRecord->GetMissionRecordId());
    }

    // unlock mission
    if (!lockMissionContainer_->ReleaseLockedMission(missionRecord, uid, isSystemApp)) {
        HILOG_ERROR("unlock mission error.");
        return UNLOCK_MISSION_DENY_FAILED;
    }
    return ERR_OK;
}

bool AbilityStackManager::CheckLockMissionCondition(
    int uid, int missionId, int isLock, bool isSystemApp, std::shared_ptr<MissionRecord> &mission, int &lockUid)
{
    CHECK_POINTER_RETURN_BOOL(lockMissionContainer_);
    // lock or unlock mission by uid and missionId.
    if ((isLock && lockMissionContainer_->IsLockedMissionState()) ||
        (!isLock && !lockMissionContainer_->IsLockedMissionState())) {
        return false;
    }

    mission = GetMissionRecordFromAllStacks(missionId);
    if (missionId < 0 || mission == nullptr) {
        HILOG_ERROR("target mission is not exist.");
        return false;
    }

    auto fullScreenStack = IsFullScreenStack(mission->GetMissionStack()->GetMissionStackId());
    if (!fullScreenStack) {
        HILOG_ERROR("Multi-window active,lock mission failed.");
        return false;
    }

    auto topability = mission->GetTopAbilityRecord();
    auto bottomability = mission->GetBottomAbilityRecord();
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_RETURN_BOOL(topability);
    CHECK_POINTER_RETURN_BOOL(bottomability);
    CHECK_POINTER_RETURN_BOOL(abilityManagerService);
    lockUid = abilityManagerService->GetUidByBundleName(bottomability->GetAbilityInfo().bundleName);
    HILOG_INFO("target mission uid :%{public}d", lockUid);

    if (isLock && !isSystemApp) {
        // caller and locking ability must be same uid, and forground state.
        return (lockUid == uid) &&
               (topability->IsAbilityState(AbilityState::ACTIVE) || topability->IsAbilityState(AbilityState::INACTIVE));
    }

    return true;
}

bool AbilityStackManager::CanStartInLockMissionState(
    const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &currentTopAbility) const
{
    if (currentTopAbility == nullptr || lockMissionContainer_ == nullptr) {
        return true;
    }

    if (!lockMissionContainer_->IsLockedMissionState()) {
        return true;
    }

    // current ability singeton mode
    if (currentTopAbility->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
            std::string bundleName = AbilityConfig::MISSION_NAME_MARK_HEAD + abilityRequest.abilityInfo.bundleName +
                                     AbilityConfig::MISSION_NAME_SEPARATOR + abilityRequest.abilityInfo.name;
            return lockMissionContainer_->IsSameLockedMission(bundleName);
        }
        return false;
    }
    // current ability standard mode
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        bool isStackNotChanged = (currentTopAbility->IsLauncherAbility() && IsLauncherAbility(abilityRequest)) ||
                                 (!currentTopAbility->IsLauncherAbility() && !IsLauncherAbility(abilityRequest));
        // ability request will add to same mission.
        return isStackNotChanged;
    }

    return false;
}

bool AbilityStackManager::CanStopInLockMissionState(const std::shared_ptr<AbilityRecord> &terminateAbility) const
{
    if (lockMissionContainer_ == nullptr) {
        return true;
    }

    if (!lockMissionContainer_->IsLockedMissionState()) {
        return true;
    }

    auto mission = terminateAbility->GetMissionRecord();
    auto bottom = mission->GetBottomAbilityRecord();
    CHECK_POINTER_RETURN_BOOL(mission);
    CHECK_POINTER_RETURN_BOOL(bottom);
    return (lockMissionContainer_->IsSameLockedMission(mission->GetName()) && terminateAbility != bottom);
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

int AbilityStackManager::MinimizeMultiWindow(int missionId)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }
    return MinimizeMultiWindowLocked(missionId);
}

int AbilityStackManager::MinimizeMultiWindowLocked(int missionId)
{
    HILOG_INFO("Minimize multi window locked.");
    auto missionRecord = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN(missionRecord, MINIMIZE_MULTI_WINDOW_FAILED);

    auto stack = missionRecord->GetMissionStack();
    CHECK_POINTER_AND_RETURN(stack, MINIMIZE_MULTI_WINDOW_FAILED);

    if (!stack->IsEqualStackId(FLOATING_MISSION_STACK_ID)) {
        HILOG_ERROR("Mission is not in the float stack, can't be minimized.");
        return MINIMIZE_MULTI_WINDOW_FAILED;
    }

    return MoveMissionToEndLocked(missionRecord->GetMissionRecordId());
}

int AbilityStackManager::ChangeFocusAbility(
    const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
{
    HILOG_INFO("Change focus ability.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER_AND_RETURN(lostFocusToken, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(getFocusToken, ERR_INVALID_VALUE);
    if (getFocusToken == lostFocusToken) {
        HILOG_WARN("get token is equal to lost token.");
        return CHANGE_FOCUS_ABILITY_FAILED;
    }

    auto currentAbility = GetCurrentTopAbility();
    CHECK_POINTER_AND_RETURN(currentAbility, CHANGE_FOCUS_ABILITY_FAILED);
    if (!currentAbility->IsAbilityState(AbilityState::ACTIVE) || !waittingAbilityQueue_.empty()) {
        HILOG_WARN("Top ability is not active or waiting queue is not empty, change focus failed");
        return CHANGE_FOCUS_ABILITY_FAILED;
    }

    auto targetAbility = Token::GetAbilityRecordByToken(getFocusToken);
    CHECK_POINTER_AND_RETURN(targetAbility, CHANGE_FOCUS_ABILITY_FAILED);
    return ChangeFocusAbilityLocked(targetAbility);
}

int AbilityStackManager::ChangeFocusAbilityLocked(const std::shared_ptr<AbilityRecord> &targetAbility)
{
    HILOG_INFO("Change focus ability locked.");
    CHECK_POINTER_AND_RETURN(targetAbility, ERR_INVALID_VALUE);

    auto currentAbility = GetCurrentTopAbility();
    CHECK_POINTER_AND_RETURN(currentAbility, CHANGE_FOCUS_ABILITY_FAILED);

    if (targetAbility == currentAbility || !targetAbility->IsAbilityState(ACTIVE)) {
        HILOG_ERROR("Target ability is current ability, or target ability is not active, can't change focus.");
        return CHANGE_FOCUS_ABILITY_FAILED;
    }

    auto targetMission = targetAbility->GetMissionRecord();
    CHECK_POINTER_AND_RETURN_LOG(
        targetMission, CHANGE_FOCUS_ABILITY_FAILED, " TargetMission is  nullptr, change focus failed.");

    return MoveMissionToTop(targetMission->GetMissionRecordId());
}

int AbilityStackManager::GetFloatingMissions(std::vector<AbilityMissionInfo> &list)
{
    return ERR_OK;
}

int AbilityStackManager::CloseMultiWindow(int missionId)
{
    HILOG_INFO("Close multi window.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }

    auto mission = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN(mission, CLOSE_MULTI_WINDOW_FAILED);

    auto stack = mission->GetMissionStack();
    CHECK_POINTER_AND_RETURN(stack, CLOSE_MULTI_WINDOW_FAILED);

    if (IsFullScreenStack(stack->GetMissionStackId())) {
        HILOG_ERROR("Full screen stack is not close.");
        return CLOSE_MULTI_WINDOW_FAILED;
    }

    return RemoveMissionByIdLocked(missionId);
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
    CHECK_POINTER(abilityRecord);
    auto getMissionOptionDisplayId = [target = abilityRecord]() {
        auto missionRecord = target->GetMissionRecord();
        return missionRecord ? missionRecord->GetMissionOption().displayKey : DISPLAY_DEFAULT_ID;
    };
    auto setting = abilityRecord->GetStartSetting();
    int displayId = setting ? std::atoi(setting->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str())
                            : getMissionOptionDisplayId();
    UpdateFocusAbilityRecord(displayId, abilityRecord, isNotify);
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
    if (resumeMissionContainer_ && resumeMissionContainer_->IsResume(mission->GetMissionRecordId())) {
        resumeMissionContainer_->Resume(mission);
    }
}

int AbilityStackManager::GetMissionSnapshot(int32_t missionId, MissionPixelMap &missionPixelMap)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("Get mission snapshot.");

    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    auto missionRecord = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN_LOG(missionRecord, REMOVE_MISSION_ID_NOT_EXIST, "mission is invalid.");
    auto topAbilityRecord = missionRecord->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN_LOG(topAbilityRecord, REMOVE_MISSION_ID_NOT_EXIST, "top ability is invalid.");
    auto windowInfo = topAbilityRecord->GetWindowInfo();
    int windowID = 0;
    if (windowInfo) {
        windowID = windowInfo->windowToken_;
        HILOG_INFO("windowID is %{public}d", windowID);
    }
    screenshotHandler_->StartScreenshot(missionId, windowID);
    auto topAbility = missionRecord->GetTopAbilityRecord();
    if (topAbility) {
        OHOS::AppExecFwk::ElementName topElement(topAbility->GetAbilityInfo().deviceId,
            topAbility->GetAbilityInfo().bundleName,
            topAbility->GetAbilityInfo().name);
        missionPixelMap.topAbility = topElement;
    }

    auto imageInfo = screenshotHandler_->GetImageInfo(missionId);
    screenshotHandler_->RemoveImageInfo(missionId);
    HILOG_INFO("width : %{public}d, height: %{public}d", imageInfo.width, imageInfo.height);
    missionPixelMap.imageInfo.width = imageInfo.width;
    missionPixelMap.imageInfo.height = imageInfo.height;
    missionPixelMap.imageInfo.format = imageInfo.format;
    missionPixelMap.imageInfo.size = imageInfo.size;
    missionPixelMap.imageInfo.shmKey = SharedMemory::PushSharedMemory(imageInfo.data, imageInfo.size);
#endif
    return ERR_OK;
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
    HILOG_INFO("RestoreMissionRecordOnLockScreen Start.");
    CHECK_POINTER(missionRecord);
    auto missionIndexInfo = missionRecord->GetMissionIndexInfo();
    auto targetStack = GetStackById(missionIndexInfo.GetStackId());
    CHECK_POINTER(targetStack);
    auto sourceStack = missionRecord->GetMissionStack();
    if (sourceStack) {
        sourceStack->RemoveMissionRecord(missionRecord->GetMissionRecordId());
    }
    targetStack->AddMissionRecordByIndex(missionRecord, missionIndexInfo.GetMissionIndex());
    missionRecord->SetMissionStack(targetStack, targetStack->GetMissionStackId());
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
    subscriber_ = std::make_shared<LockScreenEventSubscriber>(subscribeInfo, handler);
    return EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AbilityStackManager::UpdateLockScreenState(bool isLockScreen)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    isLockScreen_ = isLockScreen;
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
