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

#include "ability_stack_manager.h"

#include <singleton.h>
#include <map>

#include "hilog_wrapper.h"
#include "ability_util.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "app_scheduler.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AAFwk {
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
}

AbilityStackManager::~AbilityStackManager()
{}

int AbilityStackManager::StartAbility(const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Start ability.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    auto currentTopAbilityRecord = GetCurrentTopAbility();
    if (!CanStartInLockMissionState(abilityRequest, currentTopAbilityRecord)) {
        SendUnlockMissionMessage();
        return LOCK_MISSION_STATE_DENY_REQUEST;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    auto type = abilityInfo.type;
    if (abilityInfo.applicationInfo.isLauncherApp && type == AppExecFwk::AbilityType::PAGE && currentTopAbilityRecord &&
        AbilityUtil::IsSystemDialogAbility(
        currentTopAbilityRecord->GetAbilityInfo().bundleName, currentTopAbilityRecord->GetAbilityInfo().name)) {
        HILOG_ERROR("Page ability is dialog type, cannot return to luncher.");
        return ERR_INVALID_VALUE;
    }

    if (!waittingAbilityQueue_.empty()) {
        HILOG_INFO("Waiting queue is not empty, so enqueue ability for waiting.");
        EnqueueWaittingAbility(abilityRequest);
        return START_ABILITY_WAITING;
    }

    if (currentTopAbilityRecord != nullptr) {
        std::string element = currentTopAbilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("current top: %{public}s", element.c_str());
        if (currentTopAbilityRecord->GetAbilityState() != ACTIVE) {
            HILOG_INFO("Top ability is not active, so enqueue ability for waiting.");
            EnqueueWaittingAbility(abilityRequest);
            return START_ABILITY_WAITING;
        }
    }

    // need to start ability as special
    if (abilityRequest.startSetting && !abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        auto windowkey = static_cast<AbilityWindowConfiguration>(
            std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
        HILOG_DEBUG("Start ability with settings ...");
        if (windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING ||
            windowkey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY) {
            return StartAbilityAsSpecialLocked(currentTopAbilityRecord, abilityRequest);
        }
    }

    return StartAbilityLocked(currentTopAbilityRecord, abilityRequest);
}

int AbilityStackManager::StartAbilityLocked(
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
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);

    // 4. start processing ability lifecycle
    if (currentTopAbility == nullptr) {
        // top ability is null, then launch the first Ability.
        targetAbilityRecord->SetLauncherRoot();
        return targetAbilityRecord->LoadAbility();
    } else {
        // complete ability background if needed.
        return StartAbilityLifeCycle(lastTopAbility, currentTopAbility, targetAbilityRecord);
    }
}

int AbilityStackManager::StartAbilityLifeCycle(std::shared_ptr<AbilityRecord> lastTopAbility,
    std::shared_ptr<AbilityRecord> currentTopAbility, std::shared_ptr<AbilityRecord> targetAbility)
{
    CHECK_POINTER_AND_RETURN(targetAbility, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentTopAbility, ERR_INVALID_VALUE);
    enum ChangeType { T_ACTIVE, T_CHANGE, T_DEFAULT } changeType;

    bool isMissionChanged = currentTopAbility->GetMissionRecordId() != targetAbility->GetMissionRecordId();
    bool isStackChanged = currentTopAbility->GetMissionStackId() != targetAbility->GetMissionStackId();
    bool isCurrentFull = IsFullScreenStack(currentTopAbility->GetMissionStackId());
    bool isTargetFull = IsFullScreenStack(targetAbility->GetMissionStackId());

    std::shared_ptr<AbilityRecord> needBackgroundAbility;

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

int AbilityStackManager::StartAbilityAsSpecialLocked(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Start ability as special locked.");
    CHECK_POINTER_AND_RETURN(currentTopAbility, INNER_ERR);
    CHECK_RET_RETURN_RET(
        CheckMultiWindowCondition(currentTopAbility, abilityRequest), "Check multiwindow condition is failed.");

    // 1. choose target mission stack
    auto targetStack = GetTargetMissionStack(abilityRequest);
    CHECK_POINTER_AND_RETURN(targetStack, CREATE_MISSION_STACK_FAILED);
    auto lastTopAbility = targetStack->GetTopAbilityRecord();

    // 2. get mission record and ability recode
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;
    GetMissionRecordAndAbilityRecord(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
    if (targetAbilityRecord == nullptr || targetMissionRecord == nullptr) {
        HILOG_ERROR("Failed to get ability record or mission record.");
        return ERR_INVALID_VALUE;
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);

    MissionOption option;
    option.userId = userId_;
    option.missionId = targetMissionRecord->GetMissionRecordId();
    option.winModeKey = static_cast<AbilityWindowConfiguration>(
        std::atoi(abilityRequest.startSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str()));
    targetMissionRecord->SetMissionOption(option);

    // 3. first create mission record or stack is not changed,
    // just load target ability and inactive the current top ability.
    if (targetMissionRecord->GetMissionStack() == nullptr ||
        targetMissionRecord->GetMissionStack()->GetMissionStackId() == targetStack->GetMissionStackId()) {
        MoveMissionStackToTop(targetStack);
        MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);
        HILOG_DEBUG("First create mission record ,missionId:%{public}d", targetMissionRecord->GetMissionRecordId());
        return StartAbilityLifeCycle(lastTopAbility, currentTopAbility, targetAbilityRecord);
    }

    // 4. mission stack is changed, move mission to target stack.
    isMultiWinMoving_ = true;
    CompleteMoveMissionToStack(targetMissionRecord, targetStack);
    MoveMissionAndAbility(currentTopAbility, targetAbilityRecord, targetMissionRecord);
    HILOG_DEBUG("Mission stack is changed, move mission to target stack ,missionId:%{public}d",
        targetMissionRecord->GetMissionRecordId());
    CHECK_RET_RETURN_RET(DispatchLifecycle(currentTopAbility, targetAbilityRecord, false), "Dispatch lifecycle error.");
    // Judging target system window mode, and notify event.
    auto willWinMode = JudgingTargetSystemWindowMode(option.winModeKey);
    auto targetWinMode = GetTargetSystemWindowMode(willWinMode);
    NotifyWindowModeChanged(targetWinMode);
    return ERR_OK;
}

void AbilityStackManager::MoveMissionAndAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    std::shared_ptr<AbilityRecord> &targetAbilityRecord, std::shared_ptr<MissionRecord> &targetMissionRecord)
{
    HILOG_INFO("Move mission and ability.");
    CHECK_POINTER(targetAbilityRecord);
    CHECK_POINTER(targetMissionRecord);

    // set relationship of mission record and ability record
    if (currentTopAbility != nullptr) {
        targetAbilityRecord->SetPreAbilityRecord(currentTopAbility);
        currentTopAbility->SetNextAbilityRecord(targetAbilityRecord);
        // move mission to end, don't set pre mission
        auto targetPreMission = targetMissionRecord->GetPreMissionRecord();
        if (targetPreMission) {
            currentTopAbility->GetMissionRecord()->SetPreMissionRecord(targetPreMission);
        }
        targetMissionRecord->SetPreMissionRecord(currentTopAbility->GetMissionRecord());
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
    HILOG_INFO("Terminate ability.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    // if ability was already in terminate list, don't do anything but wait.
    CHECK_TRUE_RETURN_RET(abilityRecord->IsTerminating(), ERR_OK, "Ability is on terminating.");
    // check if ability is in stack to avoid user create fake token.
    CHECK_POINTER_AND_RETURN_LOG(
        GetAbilityRecordByToken(token), INNER_ERR, "Ability is not in stack, nor in terminating list.");

    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER_AND_RETURN(missionRecord, INNER_ERR);
    if (abilityRecord->IsLauncherAbility() && abilityRecord->IsLauncherRoot()) {
        HILOG_WARN("Don't allow terminate root launcher");
        return TERMINATE_LAUNCHER_DENIED;
    }

    if (!CanStopInLockMissionState(abilityRecord)) {
        SendUnlockMissionMessage();
        return LOCK_MISSION_STATE_DENY_REQUEST;
    }

    HILOG_INFO("Schedule normal terminate process.");
    abilityRecord->SetTerminatingState();
    return TerminateAbilityLocked(abilityRecord, resultCode, resultWant);
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

int AbilityStackManager::TerminateAbilityLocked(
    const std::shared_ptr<AbilityRecord> &abilityRecord, int resultCode, const Want *resultWant)
{
    HILOG_INFO("%{public}s, called", __func__);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is invalid");
        return ERR_INVALID_VALUE;
    }

    if (abilityRecord->IsRestarting()) {
        HILOG_ERROR("abilityRecord is restarting, deny terminate.");
        return ERR_INVALID_VALUE;
    }
    // save result to caller AbilityRecord
    if (resultWant != nullptr) {
        abilityRecord->SaveResultToCallers(resultCode, resultWant);
    }
    // common case, ability terminate at active and at top position
    if (abilityRecord->IsAbilityState(AbilityState::ACTIVE) ||
        abilityRecord->IsAbilityState(AbilityState::ACTIVATING) ||
        abilityRecord->IsAbilityState(AbilityState::INITIAL)) {
        RemoveTerminatingAbility(abilityRecord);
        abilityRecord->Inactivate();
        return ERR_OK;
    }
    // it's not common case when ability terminate at non-active state and non-top position.
    if (abilityRecord->IsAbilityState(AbilityState::INACTIVE)) {
        // ability on inactive, remove AbilityRecord out of stack and then schedule to background.
        RemoveTerminatingAbility(abilityRecord);
        abilityRecord->SendResultToCallers();
        MoveToBackgroundTask(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::BACKGROUND)) {
        // ability on background, remove AbilityRecord out of stack and then schedule to terminate.
        RemoveTerminatingAbility(abilityRecord);
        abilityRecord->SendResultToCallers();
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            HILOG_WARN("Disconnect ability terminate timeout.");
            self->CompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    } else if (abilityRecord->IsAbilityState(AbilityState::INACTIVATING) ||
               abilityRecord->IsAbilityState(AbilityState::MOVING_BACKGROUND)) {
        // ability on inactivating or moving to background.
        // remove AbilityRecord out of stack and waiting for ability(kit) AbilityTransitionDone.
        RemoveTerminatingAbility(abilityRecord);
        abilityRecord->SendResultToCallers();
    } else {
        HILOG_WARN("Ability state is invalid.");
    }
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

    std::shared_ptr<AbilityRecord> activeAbility;
    if (topAbility->IsAbilityState(ACTIVE) || topAbility->IsAbilityState(ACTIVATING)) {
        activeAbility = topAbility;
    }

    std::vector<AbilityRecordInfo> abilityInfos;
    missionRecord->GetAllAbilityInfo(abilityInfos);
    for (auto &ability : abilityInfos) {
        auto abilityRecord = missionRecord->GetAbilityRecordById(ability.id);
        if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
            HILOG_WARN("Ability record is not exist or is on terminating.");
            continue;
        }

        if (abilityRecord == activeAbility) {
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
        auto ret = TerminateAbilityLocked(abilityRecord, DEFAULT_INVAL_VALUE, nullptr);
        if (ret != ERR_OK) {
            HILOG_ERROR("Remove mission error: %{public}d.", ret);
            return REMOVE_MISSION_FAILED;
        }
    }

    if (activeAbility) {
        activeAbility->SetTerminatingState();
        auto ret = TerminateAbilityLocked(activeAbility, DEFAULT_INVAL_VALUE, nullptr);
        if (ret != ERR_OK) {
            HILOG_ERROR("Remove mission error: %{public}d.", ret);
            return REMOVE_MISSION_FAILED;
        }
    }

    return ERR_OK;
}

int AbilityStackManager::RemoveStack(int stackId)
{
    HILOG_DEBUG("stackId : %{public}d", stackId);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (stackId > MAX_MISSION_STACK_ID || stackId < MIN_MISSION_STACK_ID) {
        HILOG_ERROR("stackId:%{public}d is invalid.", stackId);
        return ERR_INVALID_VALUE;
    }

    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }

    return RemoveStackLocked(stackId);
}

int AbilityStackManager::RemoveStackLocked(int stackId)
{
    HILOG_DEBUG("stackId : %{public}d", stackId);
    CHECK_TRUE_RETURN_RET(missionStackList_.empty(), MISSION_STACK_LIST_IS_EMPTY, "Mission stack list is empty.");

    // don't allow remove launcher mission stack.
    CHECK_TRUE_RETURN_RET(stackId == LAUNCHER_MISSION_STACK_ID,
        REMOVE_STACK_LAUNCHER_DENIED,
        "Don't allow remove launcher mission stack.");

    auto isExist = [stackId](
                       const std::shared_ptr<MissionStack> &stack) { return stackId == stack->GetMissionStackId(); };
    auto iter = std::find_if(missionStackList_.begin(), missionStackList_.end(), isExist);
    CHECK_TRUE_RETURN_RET(iter == missionStackList_.end(), REMOVE_STACK_ID_NOT_EXIST, "Remove stack id is not exist.");
    // remove mission record from mission stack.
    if (*iter != nullptr) {
        std::vector<MissionRecordInfo> missionInfos;
        (*iter)->GetAllMissionInfo(missionInfos);
        for (auto &mission : missionInfos) {
            int result = RemoveMissionByIdLocked(mission.id);
            if (result != ERR_OK) {
                HILOG_ERROR("Remove mission failed, mission id : %{public}d", mission.id);
                return result;
            }
        }
    }

    return ERR_OK;
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

/**
 * remove AbilityRecord from stack to terminate list.
 * update MissionStack to prepare next top ability.
 */
void AbilityStackManager::RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    auto missionRecord = abilityRecord->GetMissionRecord();
    CHECK_POINTER(missionRecord);
    auto currentStack = missionRecord->GetMissionStack();
    CHECK_POINTER(currentStack);

    auto isActive = (abilityRecord->IsAbilityState(AbilityState::ACTIVE) ||
                     abilityRecord->IsAbilityState(AbilityState::ACTIVATING) ||
                     abilityRecord->IsAbilityState(AbilityState::INITIAL));

    missionRecord->RemoveAbilityRecord(abilityRecord);
    terminateAbilityRecordList_.push_back(abilityRecord);

    std::shared_ptr<AbilityRecord> needTopAbility;

    if (!missionRecord->IsEmpty()) {
        needTopAbility = missionRecord->GetTopAbilityRecord();
        CHECK_POINTER(needTopAbility);
        abilityRecord->SetNextAbilityRecord(needTopAbility);
        return;
    }

    if (!isActive) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(currentStack);
        return;
    }

    if (currentStack->IsEqualStackId(FLOATING_MISSION_STACK_ID)) {
        RemoveMissionRecordById(missionRecord->GetMissionRecordId());
        JudgingIsRemoveMultiScreenStack(currentStack);
        MoveMissionStackToTop(GetTopFullScreenStack());
        needTopAbility = GetCurrentTopAbility();
        abilityRecord->SetNextAbilityRecord(needTopAbility);
        return;
    }

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
        abilityRecord->SetNextAbilityRecord(needTopAbility);
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
                return GetOrCreateMissionStack(SPLIT_SCREEN_MISSION_STACK_ID, true);
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

    std::shared_ptr<AbilityEventHandler> handler =
        DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());

    abilityRecord->SetScheduler(scheduler);
    if (abilityRecord->IsRestarting()) {
        abilityRecord->RestoreAbilityState();
    }
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token);

    return ERR_OK;
}

int AbilityStackManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
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

    if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
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
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    if (info.state == AppState::TERMINATED || info.state == AppState::END) {
        for (auto &ability : terminateAbilityRecordList_) {
            if (!ability) {
                continue;
            }
            if (ability->GetApplicationInfo().name == info.appName &&
                (info.processName == ability->GetAbilityInfo().process ||
                info.processName == ability->GetApplicationInfo().bundleName)) {
                ability->SetAppState(info.state);
            }
        }
    } else {
        for (auto &stack : missionStackList_) {
            std::vector<MissionRecordInfo> missions;
            stack->GetAllMissionInfo(missions);
            for (auto &missionInfo : missions) {
                auto mission = stack->GetMissionRecordById(missionInfo.id);
                if (!mission) {
                    HILOG_ERROR("Mission is nullptr.");
                    continue;
                }
                std::vector<AbilityRecordInfo> abilitys;
                mission->GetAllAbilityInfo(abilitys);
                for (auto &abilityInfo : abilitys) {
                    auto ability = mission->GetAbilityRecordById(abilityInfo.id);
                    if (!ability) {
                        HILOG_ERROR("Ability is nullptr.");
                        continue;
                    }

                    if (ability->GetApplicationInfo().name == info.appName &&
                        (info.processName == ability->GetAbilityInfo().process ||
                        info.processName == ability->GetApplicationInfo().bundleName)) {
                        ability->SetAppState(info.state);
                    }
                }
            }
        }
    }
}

void AbilityStackManager::CompleteActive(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    // update top active ability
    UpdateFocusAbilityRecord(abilityRecord);

    // multi window moving, complete state.
    if (abilityRecord->GetInMovingState()) {
        // top ability active at last, finish moving.
        if (abilityRecord == GetCurrentTopAbility()) {
            HILOG_DEBUG("Complete multi window moving,target state is active.");
            abilityRecord->SetInMovingState(false);
            isMultiWinMoving_ = false;
            return;
        }
        // background to active state.
        abilityRecord->SetInMovingState(false);
        ContinueLifecycle();
        return;
    }

    DelayedSingleton<AbilityManagerService>::GetInstance()->NotifyBmsAbilityLifeStatus(
        abilityRecord->GetAbilityInfo().bundleName,
        abilityRecord->GetAbilityInfo().name,
        AbilityUtil::UTCTimeSeconds());
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
    if (abilityRecord->GetPowerState()) {
        CHECK_POINTER(powerStorage_);
        bool isActiveAbility = false;
        auto powerActiveStorages = powerStorage_->GetPowerOffActiveRecord();
        for (auto &powerActiveStorage : powerActiveStorages) {
            auto storageActiveAbility = powerActiveStorage.ability.lock();
            if (abilityRecord == storageActiveAbility) {
                isActiveAbility = true;
            }
        }
        HILOG_DEBUG("isActiveAbility value %{public}d", static_cast<int>(isActiveAbility));
        if (isActiveAbility && abilityRecord == GetCurrentTopAbility()) {
            HILOG_DEBUG("Top ability, complete active.");
            abilityRecord->SetPowerState(false);
            handler->PostTask(startWaittingAbilityTask, "startWaittingAbility");
            powerStorage_.reset();
            return;
        }
        if (isActiveAbility) {
            abilityRecord->SetPowerState(false);
            return;
        }
        HILOG_DEBUG("Not top ability, need complete inactive.");
        abilityRecord->ProcessInactivate();
        return;
    }

    if (abilityRecord->IsToEnd()) {
        abilityRecord->SetToEnd(false);
    }

    /* PostTask to trigger start Ability from waiting queue */
    handler->PostTask(startWaittingAbilityTask, "startWaittingAbility");

    auto preAbilityRecord = abilityRecord->GetPreAbilityRecord();
    // 1. preAbility must be inactive when start ability.
    // move preAbility to background only if it was inactive.
    if (preAbilityRecord) {
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
    if (nextAbilityRecord != nullptr && nextAbilityRecord->IsAbilityState(AbilityState::INACTIVE) &&
        nextAbilityRecord->IsTerminating()) {
        std::string nextElement = nextAbilityRecord->GetWant().GetElement().GetURI();
        HILOG_INFO("Next ability record : %{public}s", nextElement.c_str());
        MoveToBackgroundTask(nextAbilityRecord);
    }

    // 3. when the mission ends and returns to lanucher directly, the next and back are inconsistent.
    // shoukd move back ability to background and then terminate.
    std::shared_ptr<AbilityRecord> backAbilityRecord = abilityRecord->GetBackAbilityRecord();
    if (backAbilityRecord != nullptr && backAbilityRecord->IsAbilityState(AbilityState::INACTIVE) &&
        backAbilityRecord->IsTerminating() &&
        (nextAbilityRecord == nullptr || nextAbilityRecord->GetRecordId() != backAbilityRecord->GetRecordId())) {
        std::string backElement = backAbilityRecord->GetWant().GetElement().GetURI();
        HILOG_INFO("Back ability record: %{public}s", backElement.c_str());
        MoveToBackgroundTask(backAbilityRecord);
    }
    if (powerOffing_ && waittingAbilityQueue_.empty()) {
        HILOG_INFO("Wait for the ability life cycle to complete and execute poweroff.");
        PowerOffLocked();
    }
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
        self->CompleteBackground(abilityRecord);
    };
    abilityRecord->MoveToBackground(task);
}

void AbilityStackManager::CompleteInactive(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);

    // 0. multi window moving , complete lifecycle.
    if (abilityRecord->GetInMovingState()) {
        if (abilityRecord == GetCurrentTopAbility()) {
            ContinueLifecycle();
            return;
        }
        MoveToBackgroundTask(abilityRecord);
        abilityRecord->SetInMovingState(false);
        // to process next active ability
        ContinueLifecycle();
        return;
    }

    if (abilityRecord->IsToEnd()) {
        auto nextAbility = abilityRecord->GetNextAbilityRecord();
        if (!nextAbility || (nextAbility && !nextAbility->IsToEnd())) {
            abilityRecord->SetToEnd(false);
            MoveToBackgroundTask(abilityRecord);
            return;
        }
    }

    // ability state is inactive
    if (abilityRecord->GetPowerState()) {
        CHECK_POINTER(powerStorage_);
        auto powerActiveStorages = powerStorage_->GetPowerOffActiveRecord();
        for (auto &powerActiveStorage : powerActiveStorages) {
            auto storageActiveAbility = powerActiveStorage.ability.lock();
            if (abilityRecord == storageActiveAbility) {
                abilityRecord->SetPowerState(false);
                MoveToBackgroundTask(abilityRecord);
                return;
            }
        }

        HILOG_DEBUG("Complete ,target state is inactive.");
        abilityRecord->SetPowerState(false);
        auto powerInActiveStorages = powerStorage_->GetPowerOffInActiveRecord();
        for (auto &powerInActiveStorage : powerInActiveStorages) {
            auto storageInActiveAbility = powerInActiveStorage.ability.lock();
            CHECK_POINTER_CONTINUE(storageInActiveAbility);
            if (storageInActiveAbility->GetPowerState()) {
                HILOG_DEBUG("Wait other ability to complete lifecycle. Ability: %{public}s.",
                    storageInActiveAbility->GetAbilityInfo().name.c_str());
                return;
            }
        }
        if (ChangedPowerStorageAbilityToActive(powerStorage_) != ERR_OK) {
            HILOG_ERROR("ChangedPowerStorageAbilityToActive Fail");
            return;
        }
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
        if (nextActiveAbility->IsAbilityState(ACTIVE)) {
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

    // 2. it may be callback of restart ability.
    if (abilityRecord->IsRestarting()) {
        HILOG_INFO("%{public}s, back ability record: %{public}s", __func__, element.c_str());
        MoveToBackgroundTask(abilityRecord);
        return;
    }
    // 3. it may be callback of start ability.
    // if next ability has been launched and is in bottom of mission, just resume other than loading ability.
    auto nextAbilityRecord = abilityRecord->GetNextAbilityRecord();
    CHECK_POINTER_LOG(nextAbilityRecord, "Failed to get next ability record.");

    std::string nextElement = nextAbilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Next ability record: %{public}s", nextElement.c_str());
    nextAbilityRecord->ProcessActivate();
}

void AbilityStackManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
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
    // destroy abilityRecord
    auto windowInfo = abilityRecord->GetWindowInfo();
    if (windowInfo != nullptr) {
        windowTokenToAbilityMap_.erase(windowInfo->windowToken_);
    }

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

void AbilityStackManager::GetAllStackInfo(StackInfo &stackInfo)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto missionStack : missionStackList_) {
        MissionStackInfo missionStackInfo;
        missionStackInfo.id = missionStack->GetMissionStackId();
        missionStack->GetAllMissionInfo(missionStackInfo.missionRecords);
        stackInfo.missionStackInfos.emplace_back(missionStackInfo);
    }
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

    if (!topAbility->IsAbilityState(ACTIVE)) {
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

    // The singleInstance start mode, mission name is #bundleName:abilityName.
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        GetRecordBySingleton(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
        // The standard start mode, mission name is bundle name by default.
    } else {
        GetRecordByStandard(abilityRequest, currentTopAbility, targetAbilityRecord, targetMissionRecord);
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

int AbilityStackManager::GetRecentMissions(
    const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
{
    HILOG_INFO("Get recent missions.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (numMax < 0) {
        HILOG_ERROR("numMax is invalid");
        return ERR_INVALID_VALUE;
    }
    if (flags < RECENT_WITH_EXCLUDED || flags > RECENT_IGNORE_UNAVAILABLE) {
        HILOG_ERROR("flags is invalid");
        return ERR_INVALID_VALUE;
    }

    return GetRecentMissionsLocked(numMax, flags, recentList);
}

int AbilityStackManager::GetRecentMissionsLocked(
    const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
{
    HILOG_INFO("Get recent missions locked.");
    CHECK_POINTER_AND_RETURN(defaultMissionStack_, ERR_NO_INIT);

    bool withExcluded = (static_cast<uint32_t>(flags) & RECENT_WITH_EXCLUDED) != 0;
    std::vector<MissionRecordInfo> missionInfos;
    defaultMissionStack_->GetAllMissionInfo(missionInfos);
    for (auto &mission : missionInfos) {
        if (static_cast<int>(recentList.size()) >= numMax) {
            break;
        }
        // flags is RECENT_IGNORE_UNAVAILABLE,
        // You need to determine the mission optimized by the process
        // Then continue
        if (!withExcluded) {
            auto missionRecord = defaultMissionStack_->GetMissionRecordById(mission.id);
            if (!missionRecord) {
                HILOG_ERROR("Mission is nullptr, continue.");
                continue;
            }
            auto ability = missionRecord->GetTopAbilityRecord();
            if (!ability) {
                HILOG_ERROR("Ability is nullptr, continue.");
                continue;
            }
            if (ability->IsAbilityState(AbilityState::INITIAL)) {
                HILOG_INFO("Flag is RECENT_IGNORE_UNAVAILABLE, ability state: INITIAL, continue.");
                continue;
            }
        }
        AbilityMissionInfo recentMissionInfo;
        CreateRecentMissionInfo(mission, recentMissionInfo);
        recentList.emplace_back(recentMissionInfo);
    }

    return ERR_OK;
}

void AbilityStackManager::CreateRecentMissionInfo(
    const MissionRecordInfo &mission, AbilityMissionInfo &recentMissionInfo)
{
    HILOG_INFO("Create recent mission info.");
    recentMissionInfo.id = mission.id;
    recentMissionInfo.runingState = DEFAULT_INVAL_VALUE;

    auto missionRecord = GetMissionRecordFromAllStacks(mission.id);
    CHECK_POINTER_LOG(missionRecord, "Mission record is not exist.");
    auto parentStack = missionRecord->GetMissionStack();
    recentMissionInfo.missionStackId = parentStack->GetMissionStackId();
    auto baseAbility = missionRecord->GetBottomAbilityRecord();
    if (baseAbility != nullptr) {
        recentMissionInfo.baseWant = baseAbility->GetWant();
        OHOS::AppExecFwk::ElementName baseElement(baseAbility->GetAbilityInfo().deviceId,
            baseAbility->GetAbilityInfo().bundleName,
            baseAbility->GetAbilityInfo().name);
        recentMissionInfo.baseAbility = baseElement;
    }

    auto topAbility = missionRecord->GetTopAbilityRecord();
    if (topAbility != nullptr) {
        OHOS::AppExecFwk::ElementName topElement(topAbility->GetAbilityInfo().deviceId,
            topAbility->GetAbilityInfo().bundleName,
            topAbility->GetAbilityInfo().name);
        recentMissionInfo.topAbility = topElement;

        MissionDescriptionInfo missionDescription;
        missionDescription.label = topAbility->GetApplicationInfo().label;
        missionDescription.iconPath = topAbility->GetApplicationInfo().iconPath;
        recentMissionInfo.missionDescription = missionDescription;
    }

    if (auto desc = missionRecord->GetMissionDescriptionInfo()) {
        recentMissionInfo.missionDescription = *desc;
    }

    recentMissionInfo.size = missionRecord->GetAbilityRecordCount();
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

int AbilityStackManager::UpdateConfiguration(const DummyConfiguration &config)
{
    HILOG_INFO("%{public}s called", __FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    // update all stack configuration.
    for (auto &stack : missionStackList_) {
        CHECK_POINTER_AND_RETURN(stack, ERR_INVALID_VALUE);
        HILOG_DEBUG("stack id : %{public}d", stack->GetMissionStackId());
        std::shared_ptr<DummyConfiguration> configSptr = std::make_shared<DummyConfiguration>(config);
        stack->UpdateConfiguration(configSptr);
    }

    return ProcessConfigurationChange();
}

int AbilityStackManager::ProcessConfigurationChange()
{
    HILOG_INFO("%{public}s called.", __FUNCTION__);

    // all active ability check whether process onconfigurationchanged
    for (auto &stack : missionStackList_) {
        CHECK_POINTER_AND_RETURN(stack, ERR_INVALID_VALUE);
        HILOG_DEBUG("stack id : %{public}d", stack->GetMissionStackId());
        std::vector<MissionRecordInfo> missionInfos;
        stack->GetAllMissionInfo(missionInfos);
        for (auto &mission : missionInfos) {
            auto missionRecord = stack->GetMissionRecordById(mission.id);
            if (!missionRecord) {
                continue;
            }
            for (auto &it : mission.abilityRecordInfos) {
                auto abilityRecord = missionRecord->GetAbilityRecordById(it.id);
                if (abilityRecord && abilityRecord->IsAbilityState(AbilityState::ACTIVE)) {
                    abilityRecord->ProcessConfigurationChange();
                }
            }
        }
    }

    return ERR_OK;
}

void AbilityStackManager::RestartAbility(const std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("%{public}s called", __FUNCTION__);
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetRestarting(true);
    if (abilityRecord->IsAbilityState(AbilityState::ACTIVE) ||
        abilityRecord->IsAbilityState(AbilityState::ACTIVATING)) {
        abilityRecord->Inactivate();
    } else if (abilityRecord->IsAbilityState(AbilityState::INACTIVE) ||
               abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        MoveToBackgroundTask(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::BACKGROUND) ||
               abilityRecord->IsAbilityState(AbilityState::MOVING_BACKGROUND)) {
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
    HILOG_INFO("Move mission to end locked by mission id.");

    if (isMultiWinMoving_) {
        HILOG_ERROR("System is moving multi window state, request deny.");
        return MOVE_MISSION_TO_STACK_MOVING_DENIED;
    }

    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }
    auto requestMission = GetMissionRecordFromAllStacks(missionId);
    CHECK_POINTER_AND_RETURN(requestMission, MOVE_MISSION_FAILED);
    auto requestAbility = requestMission->GetTopAbilityRecord();
    CHECK_POINTER_AND_RETURN(requestAbility, MOVE_MISSION_FAILED);
    auto requestStack = requestMission->GetMissionStack();
    CHECK_POINTER_AND_RETURN(requestStack, MOVE_MISSION_FAILED);

    if (!requestAbility->IsAbilityState(AbilityState::ACTIVE) ||
        requestAbility->GetAbilityInfo().applicationInfo.isLauncherApp) {
        HILOG_ERROR("Ability is not active, or ability is launcher, can't move mission to end.");
        return MOVE_MISSION_FAILED;
    }
    // current top ability hold focus
    auto currentTopAbility = GetCurrentTopAbility();
    CHECK_POINTER_AND_RETURN(currentTopAbility, MOVE_MISSION_FAILED);
    auto currentMission = currentTopAbility->GetMissionRecord();
    CHECK_POINTER_AND_RETURN(currentMission, MOVE_MISSION_FAILED);
    auto currentStack = currentMission->GetMissionStack();
    CHECK_POINTER_AND_RETURN(currentStack, MOVE_MISSION_FAILED);

    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;
    std::shared_ptr<MissionStack> targetMissionStack;

    if (IsFullScreenStack(currentStack->GetMissionStackId()) &&
        requestStack->GetMissionStackId() == FLOATING_MISSION_STACK_ID) {
        requestStack->MoveMissionRecordToBottom(requestMission);
        requestAbility->SetToEnd(true);
        requestAbility->Inactivate();
        return ERR_OK;
    }

    if (currentStack->GetMissionStackId() == FLOATING_MISSION_STACK_ID &&
        requestStack->GetMissionStackId() == FLOATING_MISSION_STACK_ID) {
        requestStack->MoveMissionRecordToBottom(requestMission);
        requestAbility->SetToEnd(true);
        requestAbility->Inactivate();
        // floating hold focus, shift focus to full screen stack
        if (currentTopAbility == requestAbility) {
            auto fullScreenStack = GetTopFullScreenStack();
            CHECK_POINTER_AND_RETURN(fullScreenStack, MOVE_MISSION_FAILED);
            MoveMissionStackToTop(fullScreenStack);
            auto fullScreenTopAbility = fullScreenStack->GetTopAbilityRecord();
            CHECK_POINTER_AND_RETURN(fullScreenTopAbility, MOVE_MISSION_FAILED);
            UpdateFocusAbilityRecord(fullScreenTopAbility, true);
        }
        return ERR_OK;
    }

    auto isActiveLauncher = (requestMission->IsLauncherCreate() ||
                             requestMission->GetMissionStack()->GetBottomMissionRecord() == requestMission);
    requestStack->MoveMissionRecordToBottom(requestMission);
    requestMission->SetIsLauncherCreate();

    if (isActiveLauncher) {
        MoveMissionStackToTop(launcherMissionStack_);
        targetMissionStack = launcherMissionStack_;
        targetMissionRecord = launcherMissionStack_->GetTopMissionRecord();
        targetAbilityRecord = targetMissionRecord->GetTopAbilityRecord();
    } else {
        targetMissionStack = requestStack;
        targetMissionRecord = targetMissionStack->GetTopMissionRecord();
        targetAbilityRecord = targetMissionRecord->GetTopAbilityRecord();
    }

    targetAbilityRecord->SetPreAbilityRecord(requestAbility);
    requestAbility->SetNextAbilityRecord(targetAbilityRecord);

    targetAbilityRecord->SetToEnd(true);
    requestAbility->SetToEnd(true);
    requestAbility->Inactivate();
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

    // If the launcher ability is dead, delete the record
    if (abilityRecord->IsLauncherAbility()) {
        OnAbilityDiedByLauncher(abilityRecord);
        return;
    }

    OnAbilityDiedByDefault(abilityRecord);
}

void AbilityStackManager::OnAbilityDiedByLauncher(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("On ability died by launcher.");
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(launcherMissionStack_);
    auto mission = abilityRecord->GetMissionRecord();
    if (!mission || !launcherMissionStack_->IsExistMissionRecord(mission->GetMissionRecordId()) ||
        !mission->IsExistAbilityRecord(abilityRecord->GetRecordId())) {
        HILOG_ERROR("Mission or ability record is not in launcher stack.");
        return;
    }

    if (!abilityRecord->IsUninstallAbility()) {
        HILOG_INFO("Save mission");
        resumeMissionContainer_->Save(mission);
    } else {
        resumeMissionContainer_->Remove(mission->GetMissionRecordId());
    }

    // Terminate launcher ability on the top of dead ability
    std::vector<AbilityRecordInfo> abilityInfos;
    mission->GetAllAbilityInfo(abilityInfos);
    for (auto &it : abilityInfos) {
        auto ability = mission->GetAbilityRecordById(it.id);
        if (!ability) {
            HILOG_WARN("Ability is nullptr.");
            continue;
        }
        if (ability == abilityRecord) {
            break;
        }
        if (ability->IsTerminating()) {
            HILOG_ERROR("Ability is terminating.");
            continue;
        }
        HILOG_DEBUG("Terminate launcher ability.");
        ability->SetTerminatingState();
        TerminateAbilityLocked(ability, DEFAULT_INVAL_VALUE, nullptr);
    }

    // Process the dead ability record
    if (mission->GetBottomAbilityRecord() == abilityRecord && abilityRecord->IsLauncherRoot()) {
        HILOG_DEBUG("Root launcher ability died, set state: INITIAL.");
        abilityRecord->SetAbilityState(AbilityState::INITIAL);
    } else {
        mission->RemoveAbilityRecord(abilityRecord);
        if (mission->IsEmpty()) {
            launcherMissionStack_->RemoveMissionRecord(mission->GetMissionRecordId());
        }
    }

    BackToLauncher();
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

void AbilityStackManager::OnAbilityDiedByDefault(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("On ability died by default.");
    CHECK_POINTER(abilityRecord);
    auto mission = abilityRecord->GetMissionRecord();
    CHECK_POINTER(mission);
    if (!mission->IsExistAbilityRecord(abilityRecord->GetRecordId())) {
        HILOG_ERROR("Mission is nullptr or not exist.");
        return;
    }

    if (abilityRecord->IsAbilityState(AbilityState::ACTIVATING) ||
        abilityRecord->IsAbilityState(AbilityState::INITIAL)) {
        auto preAbility = abilityRecord->GetPreAbilityRecord();
        if (preAbility && preAbility->IsAbilityState(AbilityState::INACTIVE)) {
            MoveToBackgroundTask(preAbility);
        }
    }

    if (!abilityRecord->IsUninstallAbility()) {
        HILOG_INFO("Save mission");
        resumeMissionContainer_->Save(mission);
    } else {
        resumeMissionContainer_->Remove(mission->GetMissionRecordId());
    }

    auto topAbility = mission->GetTopAbilityRecord();
    CHECK_POINTER(topAbility);
    auto stack = mission->GetMissionStack();
    CHECK_POINTER(stack);
    auto isFloatingFocusDied =
        ((topAbility == GetCurrentTopAbility()) && stack->IsEqualStackId(FLOATING_MISSION_STACK_ID) &&
            (topAbility->IsAbilityState(ACTIVE) || topAbility->IsAbilityState(ACTIVATING)));

    auto isFullStackActiveDied = (IsFullScreenStack(stack->GetMissionStackId()) &&
                                  (topAbility->IsAbilityState(ACTIVE) || topAbility->IsAbilityState(ACTIVATING)));

    std::vector<AbilityRecordInfo> abilityInfos;
    mission->GetAllAbilityInfo(abilityInfos);
    for (auto &it : abilityInfos) {
        auto ability = mission->GetAbilityRecordById(it.id);
        if (!ability) {
            HILOG_ERROR("ability is nullptr,%{public}d", __LINE__);
            continue;
        }
        if (ability == abilityRecord) {
            break;
        }
        if (ability->IsTerminating()) {
            HILOG_ERROR("ability is terminating, %{public}d", __LINE__);
            continue;
        }
        HILOG_INFO("Terminate ability, %{public}d", __LINE__);
        ability->SetForceTerminate(true);
        ability->SetTerminatingState();
        TerminateAbilityLocked(ability, DEFAULT_INVAL_VALUE, nullptr);
    }

    if (abilityRecord->IsUninstallAbility()) {
        HILOG_INFO("Ability uninstall,%{public}d", __LINE__);
        mission->RemoveAbilityRecord(abilityRecord);
        if (mission->IsEmpty()) {
            RemoveMissionRecordById(mission->GetMissionRecordId());
            JudgingIsRemoveMultiScreenStack(stack);
        }
    } else {
        if (mission->GetBottomAbilityRecord() == abilityRecord) {
            HILOG_INFO("Ability died, state: INITIAL, %{public}d", __LINE__);
            abilityRecord->SetAbilityState(AbilityState::INITIAL);
        } else {
            HILOG_INFO("Ability died, remove record, %{public}d", __LINE__);
            mission->RemoveAbilityRecord(abilityRecord);
        }
    }

    if (isFloatingFocusDied) {
        HILOG_INFO("floating focus ability died, back to full stack");
        auto fullScreenStack = GetTopFullScreenStack();
        CHECK_POINTER(fullScreenStack);
        auto topFullScreenAbility = fullScreenStack->GetTopAbilityRecord();
        CHECK_POINTER(fullScreenStack);
        MoveMissionStackToTop(fullScreenStack);
        if (topFullScreenAbility->IsAbilityState(AbilityState::ACTIVE)) {
            UpdateFocusAbilityRecord(topFullScreenAbility, true);
        } else {
            topFullScreenAbility->ProcessActivate();
        }
        return;
    }

    if (isFullStackActiveDied) {
        HILOG_INFO("full stack active ability died, back to launcher.");
        BackToLauncher();
    }
}

void AbilityStackManager::BackToLauncher()
{
    HILOG_INFO("Back to launcher.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    CHECK_POINTER(defaultMissionStack_);
    CHECK_POINTER(launcherMissionStack_);

    auto fullScreenStack = GetTopFullScreenStack();
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
    if (!launcherMissionStack_) {
        HILOG_ERROR("Mission stack is invalid.");
        return nullptr;
    }
    std::vector<MissionRecordInfo> missionInfos;
    launcherMissionStack_->GetAllMissionInfo(missionInfos);
    for (auto &mission : missionInfos) {
        auto missionRecord = launcherMissionStack_->GetMissionRecordById(mission.id);
        if (!missionRecord) {
            continue;
        }
        for (auto &it : mission.abilityRecordInfos) {
            auto abilityRecord = missionRecord->GetAbilityRecordById(it.id);
            if (abilityRecord && abilityRecord->IsLauncherRoot()) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

void AbilityStackManager::UninstallApp(const std::string &bundleName)
{
    HILOG_INFO("Uninstall app, bundleName: %{public}s", bundleName.c_str());
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto task = [bundleName, this]() { AddUninstallTags(bundleName); };
    handler->PostTask(task);
}

void AbilityStackManager::AddUninstallTags(const std::string &bundleName)
{
    HILOG_INFO("Add uninstall tags, bundleName: %{public}s", bundleName.c_str());
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    for (auto &stack : missionStackList_) {
        std::vector<MissionRecordInfo> missions;
        stack->GetAllMissionInfo(missions);
        for (auto &missionInfo : missions) {
            auto mission = stack->GetMissionRecordById(missionInfo.id);
            if (!mission) {
                HILOG_ERROR("Mission is nullptr.");
                continue;
            }
            std::vector<AbilityRecordInfo> abilitys;
            mission->GetAllAbilityInfo(abilitys);
            for (auto &abilityInfo : abilitys) {
                auto ability = mission->GetAbilityRecordById(abilityInfo.id);
                if (!ability) {
                    HILOG_ERROR("Ability is nullptr.");
                    continue;
                }

                if (ability->GetAbilityInfo().bundleName == bundleName) {
                    if (ability->IsAbilityState(AbilityState::INITIAL)) {
                        mission->RemoveAbilityRecord(ability);
                        if (mission->IsEmpty()) {
                            stack->RemoveMissionRecord(mission->GetMissionRecordId());
                        }
                        CHECK_POINTER(resumeMissionContainer_);
                        resumeMissionContainer_->Remove(mission->GetMissionRecordId());
                        if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
                            if (lockMissionContainer_->IsSameLockedMission(mission->GetName())) {
                                lockMissionContainer_->ReleaseLockedMission(mission, -1, true);
                            }
                        }
                        continue;
                    }
                    ability->SetIsUninstallAbility();
                }
            }
        }
    }
    // Multi screen stack may be empty
    auto floatingStack = GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, false);
    JudgingIsRemoveMultiScreenStack(floatingStack);
    auto splitScreenStack = GetOrCreateMissionStack(SPLIT_SCREEN_MISSION_STACK_ID, false);
    JudgingIsRemoveMultiScreenStack(splitScreenStack);
}

std::shared_ptr<AbilityRecord> AbilityStackManager::GetAbilityRecordByEventId(int64_t eventId) const
{
    HILOG_DEBUG("Get ability record by eventId.");
    for (auto &stack : missionStackList_) {
        std::vector<MissionRecordInfo> missionInfos;
        stack->GetAllMissionInfo(missionInfos);
        for (auto &mission : missionInfos) {
            auto missionRecord = stack->GetMissionRecordById(mission.id);
            if (!missionRecord) {
                continue;
            }
            for (auto &it : mission.abilityRecordInfos) {
                auto abilityRecord = missionRecord->GetAbilityRecordById(it.id);
                if (abilityRecord && abilityRecord->GetEventId() == eventId) {
                    return abilityRecord;
                }
            }
        }
    }
    return nullptr;
}

void AbilityStackManager::OnTimeOut(uint32_t msgId, int64_t eventId)
{
    HILOG_DEBUG("On timeout.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    auto abilityRecord = GetAbilityRecordByEventId(eventId);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("stack manager on time out event: ability record is nullptr.");
        return;
    }

    // release mission when locked.
    auto mission = abilityRecord->GetMissionRecord();
    if (mission && lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        if (lockMissionContainer_->IsSameLockedMission(mission->GetName())) {
            lockMissionContainer_->ReleaseLockedMission(mission, -1, true);
        }
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
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }
    if (missionOption.missionId == DEFAULT_INVAL_VALUE) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_DATA;
    }
    if (JudgingTargetSystemWindowMode(missionOption.winModeKey) != SystemWindowMode::FLOATING_WINDOW_MODE) {
        HILOG_ERROR("The requested mode is error.");
        return ERR_INVALID_DATA;
    }
    std::list<MissionOption> missionOptions;
    MissionOption option = missionOption;
    missionOptions.push_back(option);

    return MoveMissionsToStackLocked(missionOptions);
}

int AbilityStackManager::MoveMissionToSplitScreenStack(const MissionOption &missionOption)
{
    HILOG_DEBUG("Move mission to split screen stack, missionId:%{public}d", missionOption.missionId);
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (lockMissionContainer_ && lockMissionContainer_->IsLockedMissionState()) {
        HILOG_ERROR("current is lock mission state, refusing to operate other mission.");
        return ERR_INVALID_VALUE;
    }
    if (missionOption.missionId == DEFAULT_INVAL_VALUE) {
        HILOG_ERROR("mission id is invalid.");
        return ERR_INVALID_DATA;
    }
    if (JudgingTargetSystemWindowMode(missionOption.winModeKey) != SystemWindowMode::SPLITSCREEN_WINDOW_MODE) {
        HILOG_ERROR("The requested mode is error.");
        return ERR_INVALID_DATA;
    }
    std::list<MissionOption> missionOptions;
    MissionOption option = missionOption;
    missionOptions.push_back(option);

    return MoveMissionsToStackLocked(missionOptions);
}

int AbilityStackManager::MoveMissionsToStackLocked(const std::list<MissionOption> &missionOptions)
{
#if BINDER_IPC_32BIT
    HILOG_DEBUG("Request mission option size :%{public}u", missionOptions.size());
#else
    HILOG_DEBUG("Request mission option size :%{public}lu", missionOptions.size());
#endif

    // check condition whether can enter or exit multiwindow mode.
    CHECK_RET_RETURN_RET(CheckMultiWindowCondition(missionOptions), "check multiwindow condition is failed.");

    // complete mission stack moving and notify ability window mode has changed.
    auto lastTopAbility = GetCurrentTopAbility();
    isMultiWinMoving_ = true;
    for (auto &it : missionOptions) {
        auto sourceMission = GetMissionRecordFromAllStacks(it.missionId);
        CheckMissionRecordIsResume(sourceMission);
        CHECK_POINTER_AND_RETURN(sourceMission, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
        CHECK_RET_RETURN_RET(CompleteMissionMoving(sourceMission, JudgingTargetStackId(it.winModeKey)),
            "complete mission moving failed.");
        sourceMission->SetMissionOption(it);
    }

    // to do ,split srceen mode ,change second screen missionoption.

    // schulde ability lifescycle (all stack)
    auto currentTopAbility = GetCurrentTopAbility();
    bool isFullScreen =
        missionOptions.front().winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    // top ability active and display fullscreen, others background state.
    CHECK_RET_RETURN_RET(
        DispatchLifecycle(lastTopAbility, currentTopAbility, isFullScreen), "Dispatch lifecycle error.");

    // Judging target system window mode, and notify event.
    auto willWinMode = JudgingTargetSystemWindowMode(missionOptions.front().winModeKey);
    auto targetWinMode = GetTargetSystemWindowMode(willWinMode);
    NotifyWindowModeChanged(targetWinMode);

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
    HILOG_INFO("Continue lifecycle.");
    for (auto &stack : missionStackList_) {
        CHECK_POINTER(stack);
        std::vector<MissionRecordInfo> missionInfos;
        stack->GetAllMissionInfo(missionInfos);
        for (auto &it : missionInfos) {
            std::shared_ptr<MissionRecord> missionRecord = stack->GetMissionRecordById(it.id);
            CHECK_POINTER(missionRecord);
            auto topAbility = missionRecord->GetTopAbilityRecord();
            CHECK_POINTER(topAbility);
            if (topAbility->GetInMovingState() && topAbility != GetCurrentTopAbility()) {
                HILOG_DEBUG("Wait other ability to complete lifecycle. Ability: %{public}s.",
                    topAbility->GetAbilityInfo().name.c_str());
                return;
            }
        }
    }

    auto currentTopAbility = GetCurrentTopAbility();
    CHECK_POINTER(currentTopAbility);
    HILOG_DEBUG("At last, complete top ability lifecycle, target state is active.");
    if (currentTopAbility->IsAbilityState(AbilityState::ACTIVE)) {
        UpdateFocusAbilityRecord(currentTopAbility, true);
        currentTopAbility->SetInMovingState(false);
        isMultiWinMoving_ = false;
    } else {
        currentTopAbility->ProcessActivate();
    }
}

int AbilityStackManager::DispatchLifecycle(const std::shared_ptr<AbilityRecord> &lastTopAbility,
    const std::shared_ptr<AbilityRecord> &currentTopAbility, bool isTopFullScreen)
{
    CHECK_POINTER_AND_RETURN(lastTopAbility, ERR_INVALID_DATA);
    CHECK_POINTER_AND_RETURN(currentTopAbility, ERR_INVALID_DATA);

    auto lastMission = lastTopAbility->GetMissionRecord();
    CHECK_POINTER_AND_RETURN(lastMission, ERR_INVALID_DATA);
    auto lastMissionStack = lastMission->GetMissionStack();
    CHECK_POINTER_AND_RETURN(lastMissionStack, ERR_INVALID_DATA);

    auto topFullStack = GetTopFullScreenStack();
    CHECK_POINTER_AND_RETURN(topFullStack, ERR_INVALID_DATA);

    for (auto &stack : missionStackList_) {
        CHECK_POINTER_AND_RETURN(stack, ERR_INVALID_DATA);
        bool isSyncVisual = SupportSyncVisualByStackId(stack->GetMissionStackId());

        HILOG_DEBUG("Processing stack lifescycle: stack id: %{public}d, isSyncVisual: %{public}d",
            stack->GetMissionStackId(),
            isSyncVisual);

        std::vector<MissionRecordInfo> missionInfos;
        stack->GetAllMissionInfo(missionInfos);
        for (auto &it : missionInfos) {
            std::shared_ptr<MissionRecord> missionRecord = stack->GetMissionRecordById(it.id);
            CHECK_POINTER_AND_RETURN(missionRecord, ERR_INVALID_DATA);
            auto topAbility = missionRecord->GetTopAbilityRecord();
            CHECK_POINTER_AND_RETURN(topAbility, ERR_INVALID_DATA);
            // last top active ability , need to inactive.
            if (topAbility == lastTopAbility || topAbility == currentTopAbility) {
                HILOG_DEBUG("Last top active ability or Current top ability, lifecycle at last.");
                topAbility->SetInMovingState(true);
                continue;
            }
            // top ability active , others need to background state.
            if (isTopFullScreen) {
                HILOG_DEBUG("Top ability will full screen, others need to background.");
                topAbility->ProcessInactivateInMoving();
                continue;
            }
            // Except the top mission of the stack, all the others are in the background state,
            // When this mission stack not support sync visual.
            if (!stack->IsTopMissionRecord(missionRecord)) {
                if (!isSyncVisual) {
                    HILOG_DEBUG("This mission stack not support sync visual. need to background.");
                    topAbility->ProcessInactivateInMoving();
                }
                continue;
            }
            // process top mission.
            // fullscreen stack, only the top ability is active state, others keep background state.
            if (stack == topFullStack) {
                HILOG_DEBUG("Fullscreen stack ,top abiltiy need to active.");
                topAbility->ProcessActivateInMoving();
                continue;
            }
            // others force background state.
            HILOG_DEBUG("Others keep background state.");
            topAbility->ProcessInactivateInMoving();
        }
    }

    bool isNotTopInFullScreen =
        IsFullScreenStack(lastTopAbility->GetMissionStackId()) &&
        (!lastMissionStack->IsTopMissionRecord(lastMission) || lastMissionStack != topFullStack);
    bool isNotTopInMultiWin = !IsFullScreenStack(lastTopAbility->GetMissionStackId()) &&
                              !SupportSyncVisualByStackId(lastTopAbility->GetMissionStackId()) &&
                              (!lastMissionStack->IsTopMissionRecord(lastMission) ||
                                  (lastMission->GetMissionRecordId() == (currentTopAbility->GetMissionRecordId())));
    if (lastTopAbility == currentTopAbility) {
        HILOG_DEBUG("Lasttopability is equal to currenttopability.");
    } else if (isNotTopInFullScreen || isNotTopInMultiWin || isTopFullScreen) {
        HILOG_DEBUG("Last top active ability , need to inactive.");
        lastTopAbility->ProcessInactivateInMoving();
    } else {
        lastTopAbility->SetInMovingState(false);
    }

    // if there is no ability to wait for processing active, just active top ability immediately
    ContinueLifecycle();

    return ERR_OK;
}

SystemWindowMode AbilityStackManager::GetTargetSystemWindowMode(const SystemWindowMode &willWinMode)
{
    if (curSysWindowMode_ == willWinMode) {
        return curSysWindowMode_;
    }

    bool isCrossFlag = curSysWindowMode_ == SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE;
    switch (willWinMode) {
        case SystemWindowMode::SPLITSCREEN_WINDOW_MODE:
        case SystemWindowMode::FLOATING_WINDOW_MODE:
            return (isCrossFlag || curSysWindowMode_ != SystemWindowMode::DEFAULT_WINDOW_MODE) ? curSysWindowMode_
                                                                                               : willWinMode;
        default:
            break;
    }

    // will win mode is default mode ,need to check current mission stack list.
    bool isFloating = GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, false) != nullptr;
    bool isSplitScreen = GetOrCreateMissionStack(SPLIT_SCREEN_MISSION_STACK_ID, false) != nullptr;

    if (isFloating && isSplitScreen) {
        return SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE;
    } else if (isFloating) {
        return SystemWindowMode::FLOATING_WINDOW_MODE;
    } else if (isSplitScreen) {
        return SystemWindowMode::SPLITSCREEN_WINDOW_MODE;
    } else {
        return willWinMode;
    }
}

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
    for (auto &it : missionOptions) {
        if (!it.IsSameWindowMode(lastOption.winModeKey)) {
            HILOG_ERROR("Mission options are not in same win mode.");
            return MOVE_MISSION_TO_STACK_NOT_SAME_WIN_MODE;
        }
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
    }

    // check whether target mission stack will be overflow.
    if (!CheckMissionStackWillOverflow(missionOptions)) {
        HILOG_ERROR("Mission stack will overflow, refuse to move mission.");
        return MOVE_MISSION_TO_STACK_TARGET_STACK_OVERFLOW;
    }

    return ERR_OK;
}

int AbilityStackManager::CheckMultiWindowCondition(
    const std::shared_ptr<AbilityRecord> &currentTopAbility, const AbilityRequest &abilityRequest) const
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
    if (!CheckMissionStackWillOverflow(list)) {
        HILOG_ERROR("Mission stack will overflow.");
        return START_ABILITY_SETTING_NOT_SUPPORT_MULTI_WIN;
    }

    return ERR_OK;
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
    if (stackId > MAX_MISSION_STACK_ID || stackId < MIN_MISSION_STACK_ID) {
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
    CHECK_POINTER_AND_RETURN(sourceStack, ERR_INVALID_DATA);

    bool isTop = sourceMission->IsLauncherCreate() && sourceMission == GetTopMissionRecord();

    HILOG_DEBUG("Mission reparent : src stack id:%{public}d, target stack id:%{public}d, isTop:%{public}d",
        sourceStack->GetMissionStackId(),
        targetStack->GetMissionStackId(),
        isTop);

    // remove mission record from source stack.
    sourceStack->RemoveMissionRecord(sourceMission->GetMissionRecordId());
    // add mission to target mission stack top.
    targetStack->AddMissionRecordToTop(sourceMission);
    targetStack->MoveMissionRecordToTop(sourceMission);
    sourceMission->SetMissionStack(targetStack, targetStack->GetMissionStackId());
    if ((sourceStack->IsEmpty() || isTop) && targetStack != defaultMissionStack_) {
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

    if (preAbility && preAbility->IsAbilityState(INACTIVE)) {
        HILOG_INFO("Load timeout, restart pre ability.");
        MoveMissionStackToTop(preAbility->GetMissionRecord()->GetMissionStack());
        preAbility->ProcessActivate();
    } else {
        auto topFullScreenStack = GetTopFullScreenStack();
        CHECK_POINTER(topFullScreenStack);
        auto topFullScreenAbility = topFullScreenStack->GetTopAbilityRecord();
        if (topFullScreenAbility && topFullScreenAbility->IsAbilityState(INACTIVE)) {
            HILOG_INFO("Load timeout, top full screen ability restart.");
            MoveMissionStackToTop(topFullScreenStack);
            topFullScreenAbility->ProcessActivate();
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
    for (auto &it : stackSettings_) {
        if (it.stackId == stackId && (!IsFullScreenStack(stackId))) {
            return it.isSyncVisual;
        }
    }
    return false;
}

SystemWindowMode AbilityStackManager::JudgingTargetSystemWindowMode(AbilityWindowConfiguration config) const
{
    switch (config) {
        // to do, split multi screen ,add key
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
        // to do, split multi screen ,add key
        case AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY:
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

int AbilityStackManager::PowerOff()
{
    HILOG_INFO("Power off.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    return PowerOffLocked();
}

int AbilityStackManager::PowerOffLocked()
{
    HILOG_INFO("Power off locked.");
    auto currentTopAbility = GetCurrentTopAbility();
    if ((currentTopAbility && !currentTopAbility->IsAbilityState(AbilityState::ACTIVE)) ||
        !waittingAbilityQueue_.empty()) {
        HILOG_WARN("Current top ability is not active, waiting ability lifecycle complete");
        // In CompleteActive,waiting ability lifecycle complete,execute PowerOffLocked again
        powerOffing_ = true;
        return POWER_OFF_WAITING;
    }

    powerStorage_ = std::make_shared<PowerStorage>();
    CHECK_POINTER_AND_RETURN(powerStorage_, POWER_OFF_FAILED);
    for (auto &stack : missionStackList_) {
        std::vector<MissionRecordInfo> missionInfos;
        stack->GetAllMissionInfo(missionInfos);
        auto checkAbility = [&](const MissionRecordInfo &missionInfo) {
            auto missionRecord = stack->GetMissionRecordById(missionInfo.id);
            CHECK_POINTER(missionRecord);
            auto abilityRecord = missionRecord->GetTopAbilityRecord();
            CHECK_POINTER(abilityRecord);
            if (abilityRecord->IsAbilityState(AbilityState::ACTIVE)) {
                powerStorage_->SetPowerOffActiveRecord(abilityRecord);
                abilityRecord->SetPowerState(true);
                abilityRecord->ProcessInactivate();
            }
            if (abilityRecord->IsAbilityState(AbilityState::INACTIVE)) {
                powerStorage_->SetPowerOffInActiveRecord(abilityRecord);
                MoveToBackgroundTask(abilityRecord);
            }
        };
        for_each(missionInfos.begin(), missionInfos.end(), checkAbility);
    }
    return ERR_OK;
}

int AbilityStackManager::PowerOn()
{
    HILOG_INFO("Power on.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    return PowerOnLocked();
}

int AbilityStackManager::PowerOnLocked()
{
    HILOG_INFO("Power on locked.");
    powerOffing_ = false;

    CHECK_POINTER_AND_RETURN(powerStorage_, POWER_ON_FAILED);
    auto powerInActiveStorages = powerStorage_->GetPowerOffInActiveRecord();
    if (powerInActiveStorages.empty()) {
        if (ChangedPowerStorageAbilityToActive(powerStorage_) != ERR_OK) {
            HILOG_ERROR("ChangedPowerStorageAbilityToActive Fail");
            return POWER_ON_FAILED;
        }
        return ERR_OK;
    }

    for (auto &powerInActiveStorage : powerInActiveStorages) {
        auto storageInActiveAbility = powerInActiveStorage.ability.lock();
        CHECK_POINTER_CONTINUE(storageInActiveAbility);
        storageInActiveAbility->SetPowerState(true);
        storageInActiveAbility->ProcessActivate();
    }
    return ERR_OK;
}

int AbilityStackManager::ChangedPowerStorageAbilityToActive(std::shared_ptr<PowerStorage> &powerStorage)
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER_AND_RETURN(powerStorage, ERR_INVALID_VALUE);
    auto powerActiveStorages = powerStorage->GetPowerOffActiveRecord();
    // Start the ability except the top of the stack
    for (auto &powerActiveStorage : powerActiveStorages) {
        auto storageActiveAbility = powerActiveStorage.ability.lock();
        CHECK_POINTER_CONTINUE(storageActiveAbility);
        storageActiveAbility->SetPowerState(true);
        if (storageActiveAbility == GetCurrentTopAbility()) {
            HILOG_DEBUG("There is no ability in active state except the top of the stack");
            continue;
        }
        storageActiveAbility->ProcessActivate();
    }
    // Finally,Start stack top ability
    auto currentTopAbility = GetCurrentTopAbility();
    CHECK_POINTER_AND_RETURN(currentTopAbility, ERR_INVALID_VALUE);
    currentTopAbility->ProcessActivate();
    HILOG_DEBUG("start the ability at the top of the stack");
    return ERR_OK;
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
    HILOG_INFO("Get floating missions.");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    auto floatingStack = GetOrCreateMissionStack(FLOATING_MISSION_STACK_ID, false);
    CHECK_POINTER_AND_RETURN_LOG(
        floatingStack, GET_FLOATING_STACK_FAILED, "Floating stack is not exist, get floating missions failed.");

    std::vector<MissionRecordInfo> missionInfos;
    floatingStack->GetAllMissionInfo(missionInfos);
    for (auto &mission : missionInfos) {
        AbilityMissionInfo recentMissionInfo;
        CreateRecentMissionInfo(mission, recentMissionInfo);
        list.emplace_back(recentMissionInfo);
    }

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

        auto targetWinMode = GetTargetSystemWindowMode(SystemWindowMode::DEFAULT_WINDOW_MODE);
        NotifyWindowModeChanged(targetWinMode);
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
    }
    focusAbilityRecordMap_.emplace(displayId, focusAbility);
}

void AbilityStackManager::CheckMissionRecordIsResume(const std::shared_ptr<MissionRecord> &mission)
{
    if (resumeMissionContainer_ && resumeMissionContainer_->IsResume(mission->GetMissionRecordId())) {
        resumeMissionContainer_->Resume(mission);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
