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

#include "mission_list_manager.h"

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "bytrace.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "mission_info_mgr.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr uint32_t NEXTABILITY_TIMEOUT = 1000;         // ms
std::string Time2str(time_t time)
{
    std::stringstream str;
    str << time;
    return str.str();
}
}
MissionListManager::MissionListManager(int userId) : userId_(userId) {}

MissionListManager::~MissionListManager() {}

void MissionListManager::Init()
{
    launcherList_ = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    defaultStandardList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    defaultSingleList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    currentMissionLists_.push_front(launcherList_);

    if (!listenerController_) {
        listenerController_ = std::make_shared<MissionListenerController>();
        listenerController_->Init();
    }

    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId_);
}

int MissionListManager::StartAbility(const AbilityRequest &abilityRequest)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto currentTopAbility = GetCurrentTopAbilityLocked();
    if (currentTopAbility) {
        std::string element = currentTopAbility->GetWant().GetElement().GetURI();
        auto state = currentTopAbility->GetAbilityState();
        HILOG_DEBUG("current top: %{public}s, state: %{public}s",
            element.c_str(), AbilityRecord::ConvertAbilityState(state).c_str());
        if (state != FOREGROUND_NEW) {
            HILOG_INFO("Top ability is not foreground, so enqueue ability for waiting.");
            EnqueueWaittingAbility(abilityRequest);
            return START_ABILITY_WAITING;
        }
    }

    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        std::string element = callerAbility->GetWant().GetElement().GetURI();
        auto state = callerAbility->GetAbilityState();
        HILOG_DEBUG("callerAbility is: %{public}s, state: %{public}s",
            element.c_str(), AbilityRecord::ConvertAbilityState(state).c_str());
        if (state != FOREGROUND_NEW) {
            HILOG_INFO("caller ability is not foreground, so enqueue ability for waiting.");
            EnqueueWaittingAbility(abilityRequest);
            return START_ABILITY_WAITING;
        }
    }

    return StartAbility(currentTopAbility, callerAbility, abilityRequest);
}

int MissionListManager::StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueWaittingAbility(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return 0;
    }

    return StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

int MissionListManager::MinimizeAbility(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Minimize ability.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    // check if ability is in list to avoid user create fake token.
    CHECK_POINTER_AND_RETURN_LOG(
        GetAbilityRecordByToken(token), INNER_ERR, "Ability is not in mission list.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    return MinimizeAbilityLocked(abilityRecord);
}

int MissionListManager::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listenerController_) {
        HILOG_ERROR("service not init, try again later.");
        return -1;
    }

    return listenerController_->AddMissionListener(listener);
}

int MissionListManager::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listenerController_) {
        HILOG_ERROR("service not init, try unregister again later.");
        return -1;
    }

    listenerController_->DelMissionListener(listener);
    return 0;
}

int MissionListManager::GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("Get mission infos.");
    if (numMax < 0) {
        HILOG_ERROR("numMax is invalid, numMax:%{public}d", numMax);
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionInfos(numMax, missionInfos);
}

int MissionListManager::GetMissionInfo(int32_t missionId, MissionInfo &missionInfo)
{
    HILOG_INFO("Get mission info by id:%{public}d.", missionId);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionInfoById(missionId, missionInfo);
}

int MissionListManager::MoveMissionToFront(int32_t missionId)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    return MoveMissionToFront(missionId, true);
}

int MissionListManager::MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher)
{
    HILOG_INFO("move mission to front:%{public}d.", missionId);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::shared_ptr<Mission> mission;
    auto targetMissionList = GetTargetMissionList(missionId, mission);
    if (!targetMissionList || !mission) {
        HILOG_ERROR("get target mission list failed, missionId: %{public}d", missionId);
        return MOVE_MISSION_FAILED;
    }

    MoveMissionToTargetList(isCallerFromLauncher, targetMissionList, mission);
    MoveMissionListToTop(targetMissionList);

    // update inner mission info time
    InnerMissionInfo innerMissionInfo;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(mission->GetMissionId(), innerMissionInfo);
    innerMissionInfo.missionInfo.time = Time2str(time(0));
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);

    auto targetAbilityRecord = mission->GetAbilityRecord();
    if (!targetAbilityRecord) {
        HILOG_ERROR("get target ability record failed, missionId: %{public}d", missionId);
        return MOVE_MISSION_FAILED;
    }

    // schedule target ability to foreground.
    targetAbilityRecord->ProcessForegroundAbility();
    HILOG_DEBUG("SetMovingState, missionId: %{public}d", missionId);
    mission->SetMovingState(true);
    return ERR_OK;
}

void MissionListManager::EnqueueWaittingAbility(const AbilityRequest &abilityRequest)
{
    waittingAbilityQueue_.push(abilityRequest);
    return;
}

void MissionListManager::StartWaittingAbility()
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto topAbility = GetCurrentTopAbilityLocked();
    CHECK_POINTER(topAbility);

    if (!topAbility->IsAbilityState(FOREGROUND_NEW)) {
        HILOG_INFO("Top ability is not foreground new, must return for start waiting again.");
        return;
    }

    if (!waittingAbilityQueue_.empty()) {
        AbilityRequest abilityRequest = waittingAbilityQueue_.front();
        waittingAbilityQueue_.pop();
        auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
        StartAbility(topAbility, callerAbility, abilityRequest);
        return;
    }
}

int MissionListManager::StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Start ability locked.");
    // 1. choose target mission list
    auto targetList = GetTargetMissionList(callerAbility, abilityRequest);
    CHECK_POINTER_AND_RETURN(targetList, CREATE_MISSION_STACK_FAILED);

    // 2. get target mission
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<Mission> targetMission;
    GetTargetMissionAndAbility(abilityRequest, targetMission, targetAbilityRecord);
    if (!targetMission || !targetAbilityRecord) {
        HILOG_ERROR("Failed to get mission or record.");
        return ERR_INVALID_VALUE;
    }
    if (abilityRequest.IsContinuation()) {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
    } else {
        targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);

    // 3. move mission to target list
    bool isCallerFromLauncher = (callerAbility && callerAbility->IsLauncherAbility());
    MoveMissionToTargetList(isCallerFromLauncher, targetList, targetMission);

    // 4. move target list to top
    MoveMissionListToTop(targetList);

    // ability is already foreground, process next ability, excepting for launcher's MainAbility.
    if ((abilityRequest.abilityInfo.bundleName != AbilityConfig::LAUNCHER_BUNDLE_NAME ||
        abilityRequest.abilityInfo.name != AbilityConfig::LAUNCHER_ABILITY_NAME) &&
        (targetAbilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
        targetAbilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW))) {
        PostStartWaittingAbility();
        return 0;
    }

    // 5. schedule target ability
    if (!currentTopAbility) {
        // top ability is null, then launch the first Ability.
        targetAbilityRecord->SetLauncherRoot();
        return targetAbilityRecord->LoadAbility();
    } else {
        // schedule target ability to foreground.
        targetAbilityRecord->ProcessForegroundAbility();
        return 0;
    }
}

void MissionListManager::GetTargetMissionAndAbility(const AbilityRequest &abilityRequest,
    std::shared_ptr<Mission> &targetMission,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (reUsedMission) {
        HILOG_DEBUG("find reused mission in running list.");
        targetMission = reUsedMission;
        targetRecord = targetMission->GetAbilityRecord();
        if (targetRecord) {
            targetRecord->SetWant(abilityRequest.want);
            targetRecord->SetIsNewWant(true);
        }
        return;
    }

    // no reused mission, create a new one.
    bool isSingleton = abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON;
    std::string missionName = isSingleton ? AbilityUtil::ConvertBundleNameSingleton(
        abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name) : abilityRequest.abilityInfo.bundleName;

    // try reuse mission info
    InnerMissionInfo info;
    bool findReusedMissionInfo = false;
    if (isSingleton && !abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        findReusedMissionInfo =
            DelayedSingleton<MissionInfoMgr>::GetInstance()->FindReusedSingletonMission(missionName, info);
    }
    findReusedMissionInfo = (findReusedMissionInfo && info.missionInfo.id > 0);
    HILOG_INFO("try find reused mission info. result:%{public}d", findReusedMissionInfo);

    info.missionName = missionName;
    info.isSingletonMode = isSingleton;
    info.missionInfo.runningState = 0;
    info.missionInfo.continuable = abilityRequest.abilityInfo.continuable;
    info.missionInfo.time = Time2str(time(0));
    info.missionInfo.iconPath = abilityRequest.appInfo.iconPath;
    info.missionInfo.want = abilityRequest.want;

    if (!findReusedMissionInfo) {
        info.missionInfo.label = abilityRequest.appInfo.label;
        if (!DelayedSingleton<MissionInfoMgr>::GetInstance()->GenerateMissionId(info.missionInfo.id)) {
            HILOG_DEBUG("failed to generate mission id.");
            return;
        }
    }
    targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    targetMission = std::make_shared<Mission>(info.missionInfo.id, targetRecord, missionName);
    targetRecord->SetUseNewMission();
    targetRecord->SetMission(targetMission);

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        targetRecord->SetSpecifiedFlag(abilityRequest.specifiedFlag);
    }

    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        return;
    }

    if (findReusedMissionInfo) {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
    } else {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->AddMissionInfo(info);
        if (listenerController_) {
            listenerController_->NotifyMissionCreated(info.missionInfo.id);
        }
    }
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionList(
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    // priority : starting launcher ability.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        HILOG_DEBUG("ability reques is launcher app.");
        return launcherList_;
    }

    // no caller ability, start ability from system service.
    if (!callerAbility) {
        HILOG_DEBUG("ability reques without caller.");
        std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
        return targetMissionList;
    }

    // caller is launcher, new or reuse list.
    if (callerAbility->IsLauncherAbility()) {
        HILOG_DEBUG("start ability, caller is launcher app.");
        return GetTargetMissionListByLauncher(abilityRequest);
    }

    // caller is not launcher: refer to the list of the caller ability.
    return GetTargetMissionListByDefault(callerAbility, abilityRequest);
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionListByLauncher(const AbilityRequest &abilityRequest)
{
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (reUsedMission) {
        HILOG_DEBUG("GetTargetMissionListByLauncher find reused mission list.");
        auto missionList = reUsedMission->GetMissionList();
        if (missionList && missionList != defaultSingleList_ && missionList != defaultStandardList_) {
            return missionList;
        }
    }

    HILOG_DEBUG("GetTargetMissionListByLauncher do not find reused mission list.");
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    return targetMissionList;
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionListByDefault(
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    if (!callerAbility) {
        return nullptr;
    }

    auto callerMission = callerAbility->GetMission();
    auto callerList = callerAbility->GetOwnedMissionList();
    if (!callerMission || !callerList) {
        return nullptr; // invalid status
    }

    // target mission should in caller mission list
    if (callerList != defaultStandardList_ && callerList != defaultSingleList_) {
        HILOG_DEBUG("GetTargetMissionListByDefault target is cller list.");
        return callerList;
    }

    // caller is default, need to start a new mission list
    HILOG_DEBUG("GetTargetMissionListByDefault target is default list.");
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    callerList->RemoveMission(callerMission);
    targetMissionList->AddMissionToTop(callerMission);

    return targetMissionList;
}

std::shared_ptr<Mission> MissionListManager::GetReusedMission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        return nullptr;
    }

    std::shared_ptr<Mission> reUsedMission = nullptr;
    std::string missionName = AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name);

    // find launcher first.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        if ((reUsedMission = launcherList_->GetSingletonMissionByName(missionName)) != nullptr) {
            return reUsedMission;
        }
    }

    // current
    for (auto missionList : currentMissionLists_) {
        if (missionList && (reUsedMission = missionList->GetSingletonMissionByName(missionName)) != nullptr) {
            return reUsedMission;
        }
    }

    // default single list
    if ((reUsedMission = defaultSingleList_->GetSingletonMissionByName(missionName)) != nullptr) {
        return reUsedMission;
    }

    return nullptr;
}

void MissionListManager::MoveMissionToTargetList(bool isCallFromLauncher,
    const std::shared_ptr<MissionList> &targetMissionList,
    const std::shared_ptr<Mission> &mission)
{
    auto missionList = mission->GetMissionList();
    // 1. new mission,move to target list.
    if (!missionList) {
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 2. launcher call launcher
    if (isCallFromLauncher && targetMissionList == launcherList_) {
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 3. reused mission is in default, move frome default to target list.
    if (missionList == defaultSingleList_ || missionList == defaultStandardList_) {
        missionList->RemoveMission(mission);
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 4. reused mission is in a valid list.
    bool isListChange = !(targetMissionList == missionList);
    if (isListChange) {
        // list change, pop above missions to default.
        MoveNoneTopMissionToDefaultList(mission);
        missionList->RemoveMission(mission);
    } else if (isCallFromLauncher) {
        // list not change, but call from launcher, pop above missions to default.
        MoveNoneTopMissionToDefaultList(mission);
    }
    targetMissionList->AddMissionToTop(mission);

    if (missionList->IsEmpty()) {
        currentMissionLists_.remove(missionList);
    }
}

void MissionListManager::MoveNoneTopMissionToDefaultList(const std::shared_ptr<Mission> &mission)
{
    auto missionList = mission->GetMissionList();
    if (!missionList) {
        return;
    }

    while (!missionList->IsEmpty()) {
        auto item = missionList->GetTopMission();
        if (item == mission) {
            break;
        }

        missionList->RemoveMission(item);
        if (mission->IsSingletonAbility()) {
            defaultSingleList_->AddMissionToTop(item);
        } else {
            defaultStandardList_->AddMissionToTop(item);
        }
    }
}

void MissionListManager::MoveMissionListToTop(const std::shared_ptr<MissionList> &missionList)
{
    if (!missionList) {
        HILOG_ERROR("mission list is nullptr.");
        return;
    }
    if (!currentMissionLists_.empty() && currentMissionLists_.front() == missionList) {
        HILOG_DEBUG("mission list is at the top of list");
        return;
    }

    currentMissionLists_.remove(missionList);
    currentMissionLists_.push_front(missionList);
}

int MissionListManager::MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_INFO("%{public}s, called", __func__);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND_NEW) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        HILOG_WARN("Ability state is invalid, not foregroundnew or foregerounding_new.");
        return ERR_OK;
    }

    MoveToBackgroundTask(abilityRecord);
    UpdateMissionTimeStamp(abilityRecord);
    return ERR_OK;
}

std::shared_ptr<AbilityRecord> MissionListManager::GetCurrentTopAbilityLocked() const
{
    if (currentMissionLists_.empty()) {
        return nullptr;
    }

    auto& topMissionList = currentMissionLists_.front();
    if (topMissionList) {
        return topMissionList->GetTopAbility();
    }
    return nullptr;
}

int MissionListManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());

    std::shared_ptr<AbilityEventHandler> handler =
        DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());

    abilityRecord->SetScheduler(scheduler);
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token);

    return ERR_OK;
}

void MissionListManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HILOG_DEBUG("Ability request app state %{public}d done.", state);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    AppAbilityState abilitState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilitState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("ability: %{public}s", element.c_str());
        abilityRecord->ForegroundAbility();
    }
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        return nullptr;
    }

    // first find in terminating list
    for (auto ability : terminateAbilityList_) {
        if (ability && token == ability->GetToken()->AsObject()) {
            return ability;
        }
    }

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordByToken(token)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordByToken(token)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordByToken(token);
}

std::shared_ptr<Mission> MissionListManager::GetMissionById(int missionId) const
{
    std::shared_ptr<Mission> mission = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (mission = missionList->GetMissionById(missionId)) != nullptr) {
            return mission;
        }
    }

    if ((mission = defaultSingleList_->GetMissionById(missionId)) != nullptr) {
        return mission;
    }

    if ((mission = launcherList_->GetMissionById(missionId)) != nullptr) {
        return mission;
    }

    return defaultStandardList_->GetMissionById(missionId);
}

int MissionListManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    HILOG_INFO("AbilityTransactionDone, state: %{public}s.", abilityState.c_str());

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto abilityRecord = GetAbilityFromTerminateList(token);
    if (abilityRecord == nullptr) {
        abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    }

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND_NEW) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int MissionListManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND_NEW: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND_NEW: {
            return DispatchForegroundNew(abilityRecord);
        }
        default: {
            HILOG_WARN("Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int MissionListManager::DispatchForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        HILOG_ERROR("DispatchForegroundNew Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING_NEW,
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, abilityRecord->GetEventId());
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteForegroundNew(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void MissionListManager::CompleteForegroundNew(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND_NEW);

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

    auto mission = abilityRecord->GetMission();
    if (mission && mission->IsMovingState()) {
        mission->SetMovingState(false);
        if (listenerController_) {
            listenerController_->NotifyMissionMovedToFront(mission->GetMissionId());
        }
    }

    auto self(shared_from_this());
    auto startWaittingAbilityTask = [self]() { self->StartWaittingAbility(); };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");

    /* PostTask to trigger start Ability from waiting queue */
    handler->PostTask(startWaittingAbilityTask, "startWaittingAbility", NEXTABILITY_TIMEOUT);
}

int MissionListManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING_NEW)) {
        HILOG_ERROR("Ability transition life state error. actual %{public}d", abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    // remove background timeout task.
    handler->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void MissionListManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING_NEW) {
        HILOG_ERROR("Ability state is %{public}d, it can't complete background.", abilityRecord->GetAbilityState());
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND_NEW);
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
    // Abilities ahead of the one started with SingleTask mode were put in terminate list, we need to terminate
    // them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND_NEW) {
            auto timeoutTask = [terminateAbility, self]() {
                HILOG_WARN("Disconnect ability terminate timeout.");
                self->CompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }
    auto mission = abilityRecord->GetMission();
    if (!mission) {
        HILOG_ERROR("snapshot: GetMission failed");
        return;
    }
    MissionSnapshot snapshot;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionSnapshot(mission->GetMissionId(),
        abilityRecord->GetToken(), snapshot);
    if (listenerController_) {
        listenerController_->NotifyMissionSnapshotChanged(mission->GetMissionId());
    }
}

int MissionListManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Terminate ability, ability is %{public}s", element.c_str());
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (abilityRecord->IsTerminating()) {
        HILOG_ERROR("Ability is on terminating.");
        return ERR_OK;
    }

    // double check to avoid the ability has been removed
    if (!GetAbilityRecordByToken(abilityRecord->GetToken())) {
        HILOG_ERROR("Ability has already been removed");
        return ERR_OK;
    }

    abilityRecord->SetTerminatingState();
    // save result to caller AbilityRecord
    if (resultWant != nullptr) {
        abilityRecord->SaveResultToCallers(resultCode, resultWant);
    }

    return TerminateAbilityLocked(abilityRecord);
}

int MissionListManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    HILOG_DEBUG("Terminate ability with result called.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    std::shared_ptr<AbilityRecord> targetAbility = GetAbilityRecordByCaller(caller, requestCode);
    if (!targetAbility) {
        HILOG_ERROR("%{public}s, Can't find target ability", __func__);
        return NO_FOUND_ABILITY_BY_CALLER;
    }

    int result = AbilityUtil::JudgeAbilityVisibleControl(targetAbility->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    return TerminateAbility(targetAbility, DEFAULT_INVAL_VALUE, nullptr);
}

int MissionListManager::TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Terminate ability locked, ability is %{public}s", element.c_str());
    // remove AbilityRecord out of stack
    RemoveTerminatingAbility(abilityRecord);
    abilityRecord->SendResultToCallers();

    // 1. if the ability was foregorund, first should find wether there is other ability foregorund
    if (abilityRecord->IsAbilityState(FOREGROUND_NEW) || abilityRecord->IsAbilityState(FOREGROUNDING_NEW)) {
        HILOG_DEBUG("current ability is active");
        if (abilityRecord->GetNextAbilityRecord()) {
            abilityRecord->GetNextAbilityRecord()->ProcessForegroundAbility();
        } else {
            MoveToBackgroundTask(abilityRecord);
        }
        return ERR_OK;
    }

    // 2. if the ability was BACKGROUNDING_NEW, waiting for completeBackgroundNew

    // 3. ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND_NEW) {
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            HILOG_WARN("Disconnect ability terminate timeout.");
            self->CompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    }
    return ERR_OK;
}

/**
 * @brief This method aims to do things as below
 * 1. remove the mission from the current missionList
 * 2. if the current missionList is empty after, then remove from the manager
 * 3. if the current ability is foreground, then should schedule the next ability to foreground before terminate
 *
 * @param abilityRecord the ability that was terminating
 */
void MissionListManager::RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("RemoveTerminatingAbility, ability is %{public}s", element.c_str());
    auto missionList = abilityRecord->GetOwnedMissionList();
    CHECK_POINTER(missionList);

    missionList->RemoveMissionByAbilityRecord(abilityRecord);
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken());
    terminateAbilityList_.push_back(abilityRecord);

    if (missionList->IsEmpty()) {
        HILOG_DEBUG("RemoveTerminatingAbility, missionList is empty, remove");
        RemoveMissionList(missionList);
    }

    // 1. clear old
    abilityRecord->SetNextAbilityRecord(nullptr);
    // 2. if the ability to terminate is background, just background
    if (!(abilityRecord->IsAbilityState(FOREGROUND_NEW) || abilityRecord->IsAbilityState(FOREGROUNDING_NEW))) {
        HILOG_DEBUG("ability state is %{public}d, just return", abilityRecord->GetAbilityState());
        return;
    }
    // 3. if run on a laptop, noting to do
    if (IsPC()) {
        HILOG_DEBUG("Run on a laptop, no need to schedule next ability.");
        return;
    }

    // 4. the ability should find the next ability to foreground
    if (missionList->IsEmpty()) {
        HILOG_DEBUG("missionList is empty, next is launcher");
        abilityRecord->SetNextAbilityRecord(GetCurrentTopAbilityLocked());
    } else {
        std::shared_ptr<AbilityRecord> needTopAbility = missionList->GetTopAbility();
        abilityRecord->SetNextAbilityRecord(needTopAbility);
        std::string element = needTopAbility->GetWant().GetElement().GetURI();
        HILOG_DEBUG("next top ability is %{public}s", element.c_str());
    }
}

void MissionListManager::RemoveMissionList(const std::shared_ptr<MissionList> &missionList)
{
    if (missionList == nullptr) {
        return;
    }

    for (auto iter = currentMissionLists_.begin(); iter != currentMissionLists_.end(); iter++) {
        if ((*iter) == missionList) {
            currentMissionLists_.erase(iter);
            return;
        }
    }
}

int MissionListManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        HILOG_ERROR("DispatchTerminate error, ability state is %{public}d", abilityRecord->GetAbilityState());
        return INNER_ERR;
    }

    // remove terminate timeout task.
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->PostTask(task);

    return ERR_OK;
}

void MissionListManager::CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        HILOG_ERROR("%{public}s, ability is not terminating.", __func__);
        return;
    }

    // notify AppMS terminate
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        // Don't return here
        HILOG_ERROR("AppMS fail to terminate ability.");
    }

    for (auto it : terminateAbilityList_) {
        if (it == abilityRecord) {
            terminateAbilityList_.remove(it);
            // update inner mission info time
            InnerMissionInfo innerMissionInfo;
            int result = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
                abilityRecord->GetMissionId(), innerMissionInfo);
            if (result != 0) {
                HILOG_INFO("Get missionInfo error, result is %{public}d, missionId is %{public}d",
                    result, abilityRecord->GetMissionId());
                break;
            }
            innerMissionInfo.missionInfo.time = Time2str(time(0));
            innerMissionInfo.missionInfo.runningState = -1;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);
            HILOG_DEBUG("Destroy ability record count %{public}ld", abilityRecord.use_count());
            break;
        }
        HILOG_WARN("Can't find ability in terminate list.");
    }
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityFromTerminateList(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    for (auto abilityRecord : terminateAbilityList_) {
        // token is type of IRemoteObject, abilityRecord->GetToken() is type of Token extending from IRemoteObject.
        if (abilityRecord && token == abilityRecord->GetToken()->AsObject()) {
            return abilityRecord;
        }
    }
    return nullptr;
}

int MissionListManager::ClearMission(int missionId)
{
    if (missionId < 0) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto mission = GetMissionById(missionId);
    if (mission == nullptr) {
        HILOG_ERROR("mission is nullptr for missionId:%{public}d", missionId);
        return ERR_INVALID_VALUE;
    }
    if (mission->GetMissionList() && mission->GetMissionList()->GetType() == MissionListType::LAUNCHER) {
        HILOG_ERROR("Mission id is launcher, can not clear.");
        return ERR_INVALID_VALUE;
    }
    return ClearMissionLocked(missionId, mission);
}

int MissionListManager::ClearMissionLocked(int missionId, std::shared_ptr<Mission> mission)
{
    if (missionId != -1) {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->DeleteMissionInfo(missionId);
        if (listenerController_) {
            listenerController_->NotifyMissionDestroyed(missionId);
        }
    }
    if (mission == nullptr) {
        HILOG_DEBUG("ability has already terminate, just remove mission.");
        return ERR_OK;
    }

    auto abilityRecord = mission->GetAbilityRecord();
    if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
        HILOG_WARN("Ability record is not exist or is on terminating.");
        return ERR_OK;
    }

    abilityRecord->SetTerminatingState();
    auto ret = TerminateAbilityLocked(abilityRecord);
    if (ret != ERR_OK) {
        HILOG_ERROR("clear mission error: %{public}d.", ret);
        return REMOVE_MISSION_FAILED;
    }

    return ERR_OK;
}

int MissionListManager::ClearAllMissions()
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->DeleteAllMissionInfos(listenerController_);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    ClearAllMissionsLocked(defaultStandardList_->GetAllMissions(), foregroundAbilities, false);
    ClearAllMissionsLocked(defaultSingleList_->GetAllMissions(), foregroundAbilities, false);

    for (auto listIter = currentMissionLists_.begin(); listIter != currentMissionLists_.end();) {
        auto missionList = (*listIter);
        listIter++;
        if (!missionList || missionList->GetType() == MissionListType::LAUNCHER) {
            continue;
        }
        ClearAllMissionsLocked(missionList->GetAllMissions(), foregroundAbilities, true);
    }

    ClearAllMissionsLocked(foregroundAbilities, foregroundAbilities, false);
    return ERR_OK;
}

void MissionListManager::ClearAllMissionsLocked(std::list<std::shared_ptr<Mission>> &missionList,
    std::list<std::shared_ptr<Mission>> &foregroundAbilities, bool searchActive)
{
    for (auto listIter = missionList.begin(); listIter != missionList.end();) {
        auto mission = (*listIter);
        listIter++;
        if (!mission || mission->IsLockedState()) {
            continue;
        }

        if (searchActive && mission->GetAbilityRecord() && mission->GetAbilityRecord()->IsActiveState()) {
            foregroundAbilities.push_front(mission);
            continue;
        }
        ClearMissionLocked(-1, mission);
    }
}

int MissionListManager::SetMissionLockedState(int missionId, bool lockedState)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (missionId < 0) {
        HILOG_ERROR("param is invalid");
        return MISSION_NOT_FOUND;
    }
    std::shared_ptr<Mission> mission = GetMissionById(missionId);
    if (mission) {
        mission->SetLockedState(lockedState);
    }

    // update inner mission info time
    InnerMissionInfo innerMissionInfo;
    auto ret = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, innerMissionInfo);
    if (ret != 0) {
        HILOG_ERROR("mission is not exist, missionId %{public}d", missionId);
        return MISSION_NOT_FOUND;
    }
    innerMissionInfo.missionInfo.time = Time2str(time(0));
    innerMissionInfo.missionInfo.lockedState = lockedState;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);
    return ERR_OK;
}

void MissionListManager::MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetIsNewWant(false);
    std::string backElement = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("Ability record: %{public}s", backElement.c_str());
    auto self(shared_from_this());
    auto task = [abilityRecord, self]() {
        HILOG_WARN("mission list manager move to background timeout.");
        self->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

void MissionListManager::OnTimeOut(uint32_t msgId, int64_t eventId)
{
    HILOG_DEBUG("On timeout, msgId is %{public}d", msgId);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto abilityRecord = GetAbilityRecordByEventId(eventId);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("MissionListManager on time out event: ability record is nullptr.");
        return;
    }
    HILOG_DEBUG("Ability timeout ,msg:%{public}d,name:%{public}s", msgId, abilityRecord->GetAbilityInfo().name.c_str());

    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            HandleLoadTimeout(abilityRecord);
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
        case AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG:
            HandleForgroundNewTimeout(abilityRecord);
            break;
        default:
            break;
    }
}

void MissionListManager::HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("MissionListManager on time out event: ability record is nullptr.");
        return;
    }
    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        ability->SetRestarting(true);
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        HILOG_INFO("Launcher root load timeout, restart.");
        DelayedStartLauncher();
        return;
    }

    // other
}

void MissionListManager::HandleForgroundNewTimeout(const std::shared_ptr<AbilityRecord> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("MissionListManager on time out event: ability record is nullptr.");
        return;
    }

    if (ability->GetMission()) {
        ability->GetMission()->SetMovingState(false);
    }

    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        HILOG_INFO("Launcher root load timeout, restart.");
        DelayedStartLauncher();
        return;
    }

    // other
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByCaller(
    const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    if (!caller) {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordByCaller(caller, requestCode)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordByCaller(caller, requestCode)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordByCaller(caller, requestCode);
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByEventId(int64_t eventId) const
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordById(eventId)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordById(eventId)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordById(eventId);
}

void MissionListManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId)
{
    HILOG_INFO("On ability died.");
    if (!abilityRecord) {
        HILOG_ERROR("OnAbilityDied come, abilityRecord is nullptr.");
        return;
    }
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("OnAbilityDied come, ability is %{public}s", element.c_str());
    if (abilityRecord->GetAbilityInfo().type != AbilityType::PAGE) {
        HILOG_ERROR("Ability type is not page.");
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    if (abilityRecord->IsLauncherRoot() && currentUserId != userId_) {
        HILOG_WARN("delay restart root launcher when start user.");
        HILOG_INFO("launcher root Ability died, state: INITIAL, %{public}d", __LINE__);
        abilityRecord->SetAbilityState(AbilityState::INITIAL);
        abilityRecord->SetRestarting(true);
        return;
    }

    HandleAbilityDied(abilityRecord);
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionList(int missionId, std::shared_ptr<Mission> &mission)
{
    mission = GetMissionById(missionId);
    if (mission) {
        HILOG_DEBUG("get mission by id successfully, missionId: %{public}d", missionId);
        auto missionList = mission->GetMissionList();
        if (!missionList) {
            // mission is not null ptr, so its missionList ptr should be not null ptr too.
            HILOG_ERROR("mission list ptr is null ptr");
            return nullptr;
        }

        auto missionType = missionList->GetType();
        std::shared_ptr<MissionList> targetMissionList = nullptr;
        switch (missionType) {
            case LAUNCHER:
                // lanucher list is also in current list, so get lanucher mission list is abnormal.
                HILOG_ERROR("get lanucher mission list, missionId: %{public}d", missionId);
                break;
            case CURRENT:
                targetMissionList = mission->GetMissionList();
                break;
            case DEFAULT_STANDARD:
            case DEFAULT_SINGLE:
                // generate a new missionList
                targetMissionList = std::make_shared<MissionList>();
                break;
            default:
                HILOG_ERROR("invalid missionType: %{public}d", missionType);
        }
        return targetMissionList;
    }

    // cannot find mission, may reasons: system restart or mission removed by system.
    HILOG_INFO("cannot find mission from MissionList by missionId: %{public}d", missionId);

    InnerMissionInfo innerMissionInfo;
    int getMission = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        missionId, innerMissionInfo);
    if (getMission != ERR_OK) {
        HILOG_ERROR("cannot find mission info from MissionInfoList by missionId: %{public}d", missionId);
        return nullptr;
    }

    // generate a new mission and missionList
    AbilityRequest abilityRequest;
    int generateAbility = DelayedSingleton<AbilityManagerService>::GetInstance()->GenerateAbilityRequest(
        innerMissionInfo.missionInfo.want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr);
    if (generateAbility != ERR_OK) {
        HILOG_ERROR("cannot find generate ability request, missionId: %{public}d", missionId);
        return nullptr;
    }

    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetUseNewMission();
    mission = std::make_shared<Mission>(innerMissionInfo.missionInfo.id, abilityRecord, innerMissionInfo.missionName);
    abilityRecord->SetMission(mission);
    std::shared_ptr<MissionList> newMissionList = std::make_shared<MissionList>();
    listenerController_->NotifyMissionCreated(innerMissionInfo.missionInfo.id);
    return newMissionList;
}

int32_t MissionListManager::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        return -1;
    }
    auto mission = abilityRecord->GetMission();
    if (!mission) {
        return -1;
    }
    return mission->GetMissionId();
}

sptr<IRemoteObject> MissionListManager::GetAbilityTokenByMissionId(int32_t missionId)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    sptr<IRemoteObject> result = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (result = missionList->GetAbilityTokenByMissionId(missionId)) != nullptr) {
            return result;
        }
    }

    if ((result = defaultSingleList_->GetAbilityTokenByMissionId(missionId)) != nullptr) {
        return result;
    }

    return defaultStandardList_->GetAbilityTokenByMissionId((missionId));
}

void MissionListManager::UpdateMissionTimeStamp(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto mission = abilityRecord->GetMission();
    if (!mission) {
        return;
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionTimeStamp(mission->GetMissionId(), Time2str(time(0)));
}

void MissionListManager::PostStartWaittingAbility()
{
    auto self(shared_from_this());
    auto startWaittingAbilityTask = [self]() { self->StartWaittingAbility(); };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");

    /* PostTask to trigger start Ability from waiting queue */
    handler->PostTask(startWaittingAbilityTask, "startWaittingAbility");
}

void MissionListManager::HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_INFO("Handle Ability Died.");
    CHECK_POINTER(abilityRecord);

    if (abilityRecord->GetAbilityInfo().type != AbilityType::PAGE) {
        HILOG_ERROR("Ability type is not page.");
        return;
    }

    if (abilityRecord->IsLauncherAbility()) {
        HandleLauncherDied(abilityRecord);
        return;
    }

    HandleAbilityDiedByDefault(abilityRecord);
}

void MissionListManager::HandleLauncherDied(std::shared_ptr<AbilityRecord> ability)
{
    HILOG_INFO("Handle launcher Ability Died.");
    auto mission = ability->GetMission();
    CHECK_POINTER_LOG(mission, "Fail to get launcher mission.");
    auto missionList = mission->GetMissionList();
    if (launcherList_ != missionList) {
        HILOG_ERROR("not launcher missionList.");
        return;
    }

    bool isForeground = ability->IsAbilityState(FOREGROUND_NEW) || ability->IsAbilityState(FOREGROUNDING_NEW);
    if (ability->IsLauncherRoot()) {
        HILOG_INFO("launcher root Ability died, state: INITIAL, %{public}d", __LINE__);
        ability->SetAbilityState(AbilityState::INITIAL);
        ability->SetRestarting(true);
    } else {
        HILOG_INFO("launcher Ability died, remove, %{public}d", __LINE__);
        missionList->RemoveMission(mission);
    }
    if (isForeground) {
        HILOG_INFO("active launchrer ability died, start launcher, %{public}d", __LINE__);
        DelayedStartLauncher();
    }
}

void MissionListManager::HandleAbilityDiedByDefault(std::shared_ptr<AbilityRecord> ability)
{
    HILOG_INFO("Handle Ability DiedByDefault.");
    auto mission = ability->GetMission();
    CHECK_POINTER_LOG(mission, "Fail to get mission.");
    auto missionList = mission->GetMissionList();
    CHECK_POINTER_LOG(missionList, "Fail to get mission list.");

    std::shared_ptr<AbilityRecord> launcherRoot = launcherList_->GetLauncherRoot();
    bool isLauncherActive = (launcherRoot &&
        (launcherRoot->IsAbilityState(FOREGROUND_NEW) || launcherRoot->IsAbilityState(FOREGROUNDING_NEW)));
    bool isForeground = ability->IsAbilityState(FOREGROUND_NEW) || ability->IsAbilityState(FOREGROUNDING_NEW);

    // remove from mission list.
    missionList->RemoveMission(mission);
    if (missionList->GetType() == MissionListType::CURRENT && missionList->IsEmpty()) {
        RemoveMissionList(missionList);
    }

    // update running state.
    InnerMissionInfo info;
    if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(mission->GetMissionId(), info) == 0) {
        info.missionInfo.runningState = -1;
        DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
    }

    // start launcher
    if (isForeground && !isLauncherActive) {
        HILOG_INFO("active ability died, start launcher later, %{public}d", __LINE__);
        DelayedStartLauncher();
    }
}

void MissionListManager::DelayedStartLauncher()
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpListMgr = shared_from_this();
    auto timeoutTask = [wpListMgr]() {
        HILOG_DEBUG("The launcher needs to be restarted.");
        auto listMgr = wpListMgr.lock();
        if (listMgr) {
            listMgr->BackToLauncher();
        }
    };
    handler->PostTask(timeoutTask, "Launcher_Restart");
}

void MissionListManager::BackToLauncher()
{
    HILOG_INFO("Back to launcher.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    CHECK_POINTER(launcherList_);

    auto currentTop = GetCurrentTopAbilityLocked();
    if (currentTop && (currentTop->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
        currentTop->IsAbilityState(AbilityState::FOREGROUNDING_NEW))) {
        HILOG_WARN("Current top ability is already foreground, no need to start launcher.");
        return;
    }

    auto launcherRootAbility = launcherList_->GetLauncherRoot();
    CHECK_POINTER_LOG(launcherRootAbility, "There is no root launcher ability, back to launcher failed.");
    auto launcherRootMission = launcherRootAbility->GetMission();
    CHECK_POINTER_LOG(launcherRootMission, "There is no root launcher mission, back to launcher failed.");
    if (launcherRootAbility->IsAbilityState(AbilityState::FOREGROUND_NEW) ||
        launcherRootAbility->IsAbilityState(AbilityState::FOREGROUNDING_NEW)) {
        HILOG_WARN("launcher is already foreground, no need to start launcher.");
        return;
    }
    std::queue<AbilityRequest> emptyQueue;
    std::swap(waittingAbilityQueue_, emptyQueue);

    launcherList_->AddMissionToTop(launcherRootMission);
    MoveMissionListToTop(launcherList_);
    launcherRootAbility->ProcessForegroundAbility();
}

int MissionListManager::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    if (!token) {
        HILOG_INFO("SetMissionLabel token is nullptr.");
        return -1;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId <= 0) {
        HILOG_INFO("SetMissionLabel find mission failed.");
        return -1;
    }

    return DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionLabel(missionId, label);
}

void MissionListManager::Dump(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = " current mission lists:{";
    info.push_back(dumpInfo);
    for (const auto& missionList : currentMissionLists_) {
        if (missionList) {
            missionList->Dump(info);
        }
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default stand mission list:{";
    info.push_back(dumpInfo);
    if (defaultStandardList_) {
        defaultStandardList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default single mission list:{";
    info.push_back(dumpInfo);
    if (defaultSingleList_) {
        defaultSingleList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " launcher mission list:{";
    info.push_back(dumpInfo);
    if (launcherList_) {
        launcherList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

void MissionListManager::DumpMissionList(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = " current mission lists:{";
    info.push_back(dumpInfo);
    for (const auto& missionList : currentMissionLists_) {
        if (missionList) {
            missionList->DumpList(info);
        }
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default stand mission list:{";
    info.push_back(dumpInfo);
    if (defaultStandardList_) {
        defaultStandardList_->DumpList(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default single mission list:{";
    info.push_back(dumpInfo);
    if (defaultSingleList_) {
        defaultSingleList_->DumpList(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " launcher mission list:{";
    info.push_back(dumpInfo);
    if (launcherList_) {
        launcherList_->DumpList(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

void MissionListManager::DumpMissionInfos(std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Dump(info);
}

void MissionListManager::DumpMission(int missionId, std::vector<std::string> &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    InnerMissionInfo innerMissionInfo;
    if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, innerMissionInfo) != 0) {
        info.push_back("error: invalid mission number, please see 'aa dump --mission-list'.");
        return;
    }
    innerMissionInfo.Dump(info);
}

void MissionListManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (waittingAbilityQueue_.empty()) {
        return;
    }

    AbilityRequest abilityRequest = waittingAbilityQueue_.front();
    waittingAbilityQueue_.pop();

    auto currentTopAbility = GetCurrentTopAbilityLocked();
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);

    auto mission = GetMissionBySpecifiedFlag(flag);
    if (mission) {
        auto ability = mission->GetAbilityRecord();
        if (!ability) {
            return;
        }
        ability->SetWant(abilityRequest.want);
        ability->SetIsNewWant(true);

        auto isCallerFromLauncher = (callerAbility && callerAbility->IsLauncherAbility());
        MoveMissionToFront(mission->GetMissionId(), isCallerFromLauncher);
        return;
    }

    abilityRequest.specifiedFlag = flag;
    StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

std::shared_ptr<Mission> MissionListManager::GetMissionBySpecifiedFlag(const std::string &flag) const
{
    std::shared_ptr<Mission> mission = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (mission = missionList->GetMissionBySpecifiedFlag(flag)) != nullptr) {
            return mission;
        }
    }

    if ((mission = defaultSingleList_->GetMissionBySpecifiedFlag(flag)) != nullptr) {
        return mission;
    }

    if ((mission = launcherList_->GetMissionBySpecifiedFlag(flag)) != nullptr) {
        return mission;
    }

    return defaultStandardList_->GetMissionBySpecifiedFlag(flag);
}

bool MissionListManager::IsPC()
{
    if (MissionDmInitCallback::isInit_) {
        return isPC_;
    }
    std::string pkgName = "ohos.aafwk.aafwk_standard";
    auto callback = std::make_shared<MissionDmInitCallback>();
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
    if (ret != ERR_OK) {
        HILOG_WARN("DeviceManager initialization failed.");
        return false;
    }
    DistributedHardware::DmDeviceInfo deviceInfo;
    ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceInfo(pkgName, deviceInfo);
    if (ret != ERR_OK) {
        HILOG_WARN("Failed to get local device info.");
        return false;
    }
    MissionDmInitCallback::isInit_ = true;
    if (deviceInfo.deviceTypeId != DistributedHardware::DmDeviceType::DEVICE_TYPE_PC) {
        HILOG_WARN("The device is not a laptop.");
        return false;
    }
    isPC_ = true;
    return isPC_;
}

bool MissionListManager::MissionDmInitCallback::isInit_ = false;
void MissionListManager::MissionDmInitCallback::OnRemoteDied()
{
    isInit_ = false;
    HILOG_WARN("DeviceManager died.");
}

void MissionListManager::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    DelayedSingleton<MissionInfoMgr>::GetInstance()->RegisterSnapshotHandler(handler);
}

void MissionListManager::GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot)
{
    DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionSnapshot(missionId, abilityToken, missionSnapshot);
}

void MissionListManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    auto func = [&info](const std::shared_ptr<Mission> &mission) {
        if (!mission) {
            return;
        }

        auto ability = mission->GetAbilityRecord();
        if (!ability) {
            return;
        }

        AbilityRunningInfo runningInfo;
        AppExecFwk::RunningProcessInfo processInfo;

        runningInfo.ability = ability->GetWant().GetElement();
        runningInfo.startTime = ability->GetStartTime();
        runningInfo.abilityState = static_cast<int>(ability->GetAbilityState());

        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
        runningInfo.pid = processInfo.pid_;
        runningInfo.uid = processInfo.uid_;
        runningInfo.processName = processInfo.processName_;
        info.emplace_back(runningInfo);
    };
    if (!(defaultStandardList_->GetAllMissions().empty())) {
        auto list = defaultStandardList_->GetAllMissions();
        std::for_each(list.begin(), list.end(), func);
    }
    if (!(defaultSingleList_->GetAllMissions().empty())) {
        auto list = defaultSingleList_->GetAllMissions();
        std::for_each(list.begin(), list.end(), func);
    }
    for (auto missionList : currentMissionLists_) {
        if (!(missionList->GetAllMissions().empty())) {
            auto list = missionList->GetAllMissions();
            std::for_each(list.begin(), list.end(), func);
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
