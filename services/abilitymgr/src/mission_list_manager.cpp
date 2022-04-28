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

#include "mission_list_manager.h"

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "bytrace.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "mission_info_mgr.h"
#include "hisysevent.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr uint32_t NEXTABILITY_TIMEOUT = 1000;         // ms
constexpr uint64_t NANO_SECOND_PER_SEC = 1000000000; // ns
std::string GetCurrentTime()
{
    struct timespec tn;
    clock_gettime(CLOCK_REALTIME, &tn);
    uint64_t uTime = static_cast<uint64_t>(tn.tv_sec) * NANO_SECOND_PER_SEC + tn.tv_nsec;
    return std::to_string(uTime);
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
        if (state == FOREGROUNDING) {
            HILOG_INFO("Top ability is foregrounding, so enqueue ability for waiting.");
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
    }

    return StartAbility(currentTopAbility, callerAbility, abilityRequest);
}

int MissionListManager::StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueWaittingAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return 0;
    }

    return StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

int MissionListManager::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    HILOG_INFO("Minimize ability, fromUser:%{public}d.", fromUser);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    // check if ability is in list to avoid user create fake token.
    CHECK_POINTER_AND_RETURN_LOG(
        GetAbilityRecordByToken(token), INNER_ERR, "Minimize ability fail, ability is not in mission list.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    return MinimizeAbilityLocked(abilityRecord, fromUser);
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

int MissionListManager::MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    return MoveMissionToFront(missionId, true, startOptions);
}

int MissionListManager::MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher,
    std::shared_ptr<StartOptions> startOptions)
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

    auto targetAbilityRecord = mission->GetAbilityRecord();
    if (!targetAbilityRecord) {
        HILOG_ERROR("get target ability record failed, missionId: %{public}d", missionId);
        return MOVE_MISSION_FAILED;
    }
    targetAbilityRecord->RemoveWindowMode();
    if (startOptions != nullptr) {
        targetAbilityRecord->SetWindowMode(startOptions->GetWindowMode());
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

void MissionListManager::EnqueueWaittingAbilityToFront(const AbilityRequest &abilityRequest)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::queue<AbilityRequest> abilityQueue;
    abilityQueue.push(abilityRequest);
    waittingAbilityQueue_.swap(abilityQueue);
    while (!abilityQueue.empty()) {
        AbilityRequest tempAbilityRequest = abilityQueue.front();
        abilityQueue.pop();
        waittingAbilityQueue_.push(tempAbilityRequest);
    }
}

void MissionListManager::StartWaittingAbility()
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto topAbility = GetCurrentTopAbilityLocked();
    CHECK_POINTER(topAbility);

    if (!topAbility->IsAbilityState(FOREGROUND)) {
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

    // 5. schedule target ability
    if (!currentTopAbility) {
        // top ability is null, then launch the first Ability.
        if (targetAbilityRecord->GetAbilityInfo().applicationInfo.isLauncherApp) {
            targetAbilityRecord->SetLauncherRoot();
        }
        return targetAbilityRecord->LoadAbility();
    } else {
        // schedule target ability to foreground.
        targetAbilityRecord->ProcessForegroundAbility();
        return 0;
    }
}

static int32_t CallType2StartMethod(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::INVALID_TYPE:
            return static_cast<int32_t>(StartMethod::START_NORMAL);
        case AbilityCallType::CALL_REQUEST_TYPE:
            return static_cast<int32_t>(StartMethod::START_CALL);
        default:
            break;
    }
    return -1;
}

static bool CallTypeFilter(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::CALL_REQUEST_TYPE:
            return true;
        default:
            break;
    }
    return false;
}

void MissionListManager::GetTargetMissionAndAbility(const AbilityRequest &abilityRequest,
    std::shared_ptr<Mission> &targetMission,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    auto startMethod = CallType2StartMethod(abilityRequest.callType);
    HILOG_DEBUG("GetTargetMissionAndAbility called startMethod is %{public}d.", startMethod);
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (reUsedMission) {
        HILOG_DEBUG("find reused mission in running list.");
        targetMission = reUsedMission;
        targetRecord = targetMission->GetAbilityRecord();
        if (targetRecord) {
            targetRecord->SetWant(abilityRequest.want);
            targetRecord->SetIsNewWant(true);
        }

        if (!(targetMission->IsStartByCall()
            && !CallTypeFilter(startMethod))) {
            HILOG_DEBUG("mission exists. No update required");
            return;
        }
        HILOG_DEBUG("mission exists. need to be updated");
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
    info.startMethod = startMethod;
    info.bundleName = abilityRequest.abilityInfo.bundleName;
    info.uid = abilityRequest.uid;
    info.missionInfo.runningState = 0;
    info.missionInfo.continuable = abilityRequest.abilityInfo.continuable;
    info.missionInfo.time = GetCurrentTime();
    info.missionInfo.iconPath = abilityRequest.appInfo.iconPath;
    info.missionInfo.want = abilityRequest.want;

    if (!findReusedMissionInfo) {
        info.missionInfo.label = abilityRequest.appInfo.label;
        if (!DelayedSingleton<MissionInfoMgr>::GetInstance()->GenerateMissionId(info.missionInfo.id)) {
            HILOG_DEBUG("failed to generate mission id.");
            return;
        }
    }

    if (targetMission == nullptr) {
        HILOG_DEBUG("Make new mission data.");
        targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        targetMission = std::make_shared<Mission>(info.missionInfo.id, targetRecord, missionName, startMethod);
        targetRecord->SetMission(targetMission);
    } else {
        HILOG_DEBUG("Update old mission data.");
        auto state = targetMission->UpdateMissionId(info.missionInfo.id, startMethod);
        if (!state) {
            HILOG_INFO("targetMission UpdateMissionId(%{public}d, %{public}d) failed", info.missionInfo.id,
                startMethod);
        }
        HILOG_DEBUG("Update MissionId UpdateMissionId(%{public}d, %{public}d) end", info.missionInfo.id, startMethod);
    }

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
        if (item == nullptr || item == mission) {
            break;
        }

        missionList->RemoveMission(item);
        if (item->IsSingletonAbility()) {
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

int MissionListManager::MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser)
{
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Minimize ability fail, ability record is null.");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("%{public}s called, ability:%{public}s.", __func__, abilityRecord->GetAbilityInfo().name.c_str());

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("Minimize ability fail, ability state is invalid, not foregroundnew or foregerounding_new.");
        return ERR_OK;
    }

    auto topAbility = GetCurrentTopAbilityLocked();
    if (!fromUser && abilityRecord == topAbility) {
        HILOG_ERROR("Minimize ability fail, top ability refuse to background when not from user.");
        return ERR_INVALID_OPERATION;
    }

    abilityRecord->SetMinimizeReason(fromUser);
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

    HILOG_DEBUG("AbilityMS attch abilityThread, name is %{public}s.", abilityRecord->GetAbilityInfo().name.c_str());

    std::shared_ptr<AbilityEventHandler> handler =
        DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());

    abilityRecord->SetScheduler(scheduler);

    if (abilityRecord->IsStartedByCall()) {
        // started by callability, directly move to background.
        abilityRecord->SetStartToBackground(true);
        MoveToBackgroundTask(abilityRecord);
        return ERR_OK;
    }

    if (abilityRecord->IsNeedToCallRequest()) {
        abilityRecord->CallRequest();
    }

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token);

    return ERR_OK;
}

void MissionListManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HILOG_DEBUG("Ability request state %{public}d done.", state);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    AppAbilityState abilitState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilitState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundAbility(abilityRecord->lifeCycleStateInfo_.sceneFlagBak);
    }
}

void MissionListManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    if (info.state == AppState::TERMINATED || info.state == AppState::END) {
        for (const auto& abilityRecord : terminateAbilityList_) {
            if (!abilityRecord) {
                HILOG_ERROR("abilityRecord is nullptr.");
                continue;
            }
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
    } else {
        for (const auto& missionList : currentMissionLists_) {
            auto missions = missionList->GetAllMissions();
            for (const auto& missionInfo : missions) {
                if (!missionInfo) {
                    HILOG_ERROR("missionInfo is nullptr.");
                    continue;
                }
                auto abilityRecord = missionInfo->GetAbilityRecord();
                if (info.processName == abilityRecord->GetAbilityInfo().process ||
                    info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                    abilityRecord->SetAppState(info.state);
                }
            }
        }
        auto defaultStandardListmissions = defaultStandardList_->GetAllMissions();
        for (const auto& missionInfo : defaultStandardListmissions) {
            if (!missionInfo) {
                HILOG_ERROR("defaultStandardListmissions is nullptr.");
                continue;
            }
            auto abilityRecord = missionInfo->GetAbilityRecord();
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
        auto defaultSingleListmissions = defaultSingleList_->GetAllMissions();
        for (const auto& missionInfo : defaultSingleListmissions) {
            if (!missionInfo) {
                HILOG_ERROR("defaultSingleListmissions is nullptr.");
                continue;
            }
            auto abilityRecord = missionInfo->GetAbilityRecord();
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
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

    if (targetState == AbilityState::BACKGROUND) {
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
        case AbilityState::BACKGROUND: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND: {
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

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("DispatchForegroundNew Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING,
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
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("ability: %{public}s", element.c_str());

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);

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

    auto mission = abilityRecord->GetMission();
    if (mission) {
        InnerMissionInfo info;
        if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
            mission->GetMissionId(), info) == 0) {
            info.missionInfo.time = GetCurrentTime();
            info.missionInfo.runningState = 0;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        }
    }

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

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
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
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        HILOG_ERROR("Ability state is %{public}d, it can't complete background.", abilityRecord->GetAbilityState());
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());

    if (abilityRecord->IsSwitchingPause()) {
        abilityRecord->SetSwitchingPause(false);
        return;
    }

    // Abilities ahead of the one started with SingleTask mode were put in terminate list, we need to terminate
    // them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND) {
            auto timeoutTask = [terminateAbility, self]() {
                HILOG_WARN("Disconnect ability terminate timeout.");
                self->PrintTimeOutLog(terminateAbility, AbilityManagerService::TERMINATE_TIMEOUT_MSG);
                self->CompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }

    // new version. started by caller, sdheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToBackground() && abilityRecord->IsReady()) {
        HILOG_DEBUG("call request after completing background state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToBackground(false);
    }
}

int MissionListManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant, bool flag)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Terminate ability, ability is %{public}s.", element.c_str());
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (abilityRecord->IsTerminating() && !abilityRecord->IsForeground()) {
        HILOG_ERROR("Ability is on terminating.");
        return ERR_OK;
    }

    if (abilityRecord->IsTerminating() && abilityRecord->IsForeground()) {
        HILOG_WARN("Ability is on terminating and ability state is foreground, force close");
        flag = false;
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

    return TerminateAbilityLocked(abilityRecord, flag);
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

    return TerminateAbility(targetAbility, DEFAULT_INVAL_VALUE, nullptr, true);
}

int MissionListManager::TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Terminate ability locked, ability is %{public}s.", element.c_str());
    // remove AbilityRecord out of stack
    RemoveTerminatingAbility(abilityRecord, flag);
    abilityRecord->SendResultToCallers();

    // 1. if the ability was foregorund, first should find wether there is other ability foregorund
    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        HILOG_DEBUG("current ability is active");
        if (abilityRecord->GetNextAbilityRecord()) {
            abilityRecord->GetNextAbilityRecord()->ProcessForegroundAbility();
        } else {
            MoveToBackgroundTask(abilityRecord);
        }
        return ERR_OK;
    }

    // 2. if the ability was BACKGROUNDING, waiting for completeBackgroundNew

    // 3. ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
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
void MissionListManager::RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag)
{
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Remove terminating ability, ability is %{public}s.", element.c_str());
    if (GetAbilityFromTerminateList(abilityRecord->GetToken())) {
        abilityRecord->SetNextAbilityRecord(nullptr);
        HILOG_DEBUG("Find ability in terminating list, return.");
        return;
    }

    auto missionList = abilityRecord->GetOwnedMissionList();
    CHECK_POINTER(missionList);

    missionList->RemoveMissionByAbilityRecord(abilityRecord);
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken());
    terminateAbilityList_.push_back(abilityRecord);

    if (missionList->IsEmpty()) {
        HILOG_DEBUG("Remove terminating ability, missionList is empty, remove.");
        RemoveMissionList(missionList);
    }

    // 1. clear old
    abilityRecord->SetNextAbilityRecord(nullptr);
    // 2. if the ability to terminate is background, just background
    if (!(abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING))) {
        HILOG_DEBUG("Ability state is %{public}d, just return.", abilityRecord->GetAbilityState());
        return;
    }
    // 3. if close ability, noting to do
    if (!flag) {
        HILOG_DEBUG("Close ability schedule.");
        return;
    }

    // 4. the ability should find the next ability to foreground
    std::shared_ptr<AbilityRecord> needTopAbility;
    if (missionList->IsEmpty()) {
        HILOG_DEBUG("MissionList is empty, next is launcher.");
        needTopAbility = GetCurrentTopAbilityLocked();
    } else {
        needTopAbility = missionList->GetTopAbility();
    }

    if (!needTopAbility) {
        HILOG_DEBUG("The ability need to top is null.");
        return;
    }
    AppExecFwk::ElementName elementName = needTopAbility->GetWant().GetElement();
    HILOG_DEBUG("Next top ability is %{public}s, state is %{public}d, minimizeReason is %{public}d.",
        elementName.GetURI().c_str(), needTopAbility->GetAbilityState(), needTopAbility->IsMinimizeFromUser());

    // 5. if caller is recent, close
    if (elementName.GetBundleName() == AbilityConfig::LAUNCHER_BUNDLE_NAME
        && elementName.GetAbilityName() == AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME) {
        HILOG_DEBUG("Next to need is recent, just to launcher.");
        needTopAbility = launcherList_->GetLauncherRoot();
    }

    if (!needTopAbility) {
        HILOG_DEBUG("NeedTopAbility of launcherRoot is null.");
        return;
    }

    if (!needTopAbility->IsForeground() && !needTopAbility->IsMinimizeFromUser()) {
        HILOG_DEBUG("%{public}s is need to foreground.", elementName.GetURI().c_str());
        abilityRecord->SetNextAbilityRecord(needTopAbility);
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

    CompleteTerminateAndUpdateMission(abilityRecord);
}

void MissionListManager::CompleteTerminateAndUpdateMission(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
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
            innerMissionInfo.missionInfo.time = GetCurrentTime();
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
    if (!token) {
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    for (auto abilityRecord : terminateAbilityList_) {
        // token is type of IRemoteObject, abilityRecord->GetToken() is type of Token extending from IRemoteObject.
        if (abilityRecord && abilityRecord->GetToken() && token == abilityRecord->GetToken()->AsObject()) {
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
    if (mission && mission->GetMissionList() && mission->GetMissionList()->GetType() == MissionListType::LAUNCHER) {
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
    auto ret = TerminateAbilityLocked(abilityRecord, false);
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
    innerMissionInfo.missionInfo.lockedState = lockedState;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);
    return ERR_OK;
}

void MissionListManager::MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        HILOG_ERROR("Move the ability to background fail, ability record is null.");
        return;
    }
    HILOG_INFO("Move the ability to background, ability:%{public}s.", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetIsNewWant(false);
    auto self(shared_from_this());
    UpdateMissionSnapshot(abilityRecord);
    auto task = [abilityRecord, self]() {
        HILOG_ERROR("Mission list manager move to background timeout.");
        self->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUNDNEW_TIMEOUT_MSG);
        self->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

void MissionListManager::PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    std::string msgContent;
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent = "ability load timeout";
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            msgContent = "ability active timeout";
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            msgContent = "ability inactive timeout";
            break;
        case AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG:
            msgContent = "ability foregroundnew timeout";
            break;
        case AbilityManagerService::BACKGROUNDNEW_TIMEOUT_MSG:
            msgContent = "ability backgroundnew timeout";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent = "ability terminate timeout";
            break;
        default:
            return;
    }
    std::string eventType = "LIFECYCLE_TIMEOUT";
    OHOS::HiviewDFX::HiSysEvent::Write(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, eventType,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_UID, std::to_string(processInfo.uid_),
        EVENT_KEY_PID, std::to_string(processInfo.pid_),
        EVENT_KEY_PACKAGE_NAME, processInfo.bundleNames,
        EVENT_KEY_PROCESS_NAME, processInfo.processName_,
        EVENT_KEY_MESSAGE, msgContent);

    HILOG_WARN("LIFECYCLE_TIMEOUT: uid：%{public}d, pid：%{public}d, abilityName: %{public}s, msg: %{public}s",
        processInfo.uid_,
        processInfo.pid_,
        ability->GetAbilityInfo().name.c_str(),
        msgContent.c_str());
}

void MissionListManager::UpdateMissionSnapshot(const std::shared_ptr<AbilityRecord>& abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    int32_t missionId = abilityRecord->GetMissionId();
    MissionSnapshot snapshot;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionSnapshot(missionId, abilityRecord->GetToken(),
        snapshot);
    if (listenerController_) {
        listenerController_->NotifyMissionSnapshotChanged(missionId);
    }
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
    PrintTimeOutLog(abilityRecord, msgId);
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
    HandleTimeoutAndResumeAbility(ability);
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

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("this ability is not forgrounding state.");
        return;
    }

    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        HILOG_INFO("Launcher root load timeout, restart.");
        DelayedStartLauncher();
        return;
    }

    // other
    HandleTimeoutAndResumeAbility(ability);
}

void MissionListManager::HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &timeOutAbilityRecord)
{
    HILOG_DEBUG("HandleTimeoutAndResumeTopAbility start");
    if (timeOutAbilityRecord == nullptr) {
        HILOG_ERROR("LoadAndForeGroundCommon: timeOutAbilityRecord is nullptr.");
        return;
    }

    // complete mission list mvoing
    MoveToTerminateList(timeOutAbilityRecord);

    // load and foreground timeout, notify appMs force terminate the ability.
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(timeOutAbilityRecord->GetToken());

    // caller not exist or caller is service or timeout ability is launcher, back to launcher
    auto callerAbility = timeOutAbilityRecord->GetCallerRecord();
    if ((callerAbility == nullptr) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::SERVICE) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::EXTENSION) ||
        timeOutAbilityRecord->IsLauncherAbility() ||
        callerAbility->IsLauncherAbility()) {
        HILOG_DEBUG("ability timeout, back to launcher.");
        DelayedStartLauncher();
        return;
    }

    DelayedResumeTimeout(callerAbility);

    HILOG_INFO("HandleTimeoutAndResumeTopAbility end");
}

void MissionListManager::DelayedResumeTimeout(const std::shared_ptr<AbilityRecord> &callerAbility)
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpListMgr = shared_from_this();
    auto timeoutTask = [wpListMgr, callerAbility]() {
        HILOG_DEBUG("The caller ability need to resume.");
        auto listMgr = wpListMgr.lock();
        if (listMgr) {
            listMgr->BackToCaller(callerAbility);
        }
    };
    handler->PostTask(timeoutTask, "Caller_Restart");
}

void MissionListManager::BackToCaller(const std::shared_ptr<AbilityRecord> &callerAbility)
{
    HILOG_INFO("Back to Caller.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    // caller is already the top ability and foregroundnew.
    auto topAbility = GetCurrentTopAbilityLocked();
    if (callerAbility == topAbility && topAbility->IsAbilityState(AbilityState::FOREGROUND)) {
        HILOG_DEBUG("caller is already the top ability and foregroundnew.");
        return;
    }

    // other , resume caller ability to top and foreground.
    MoveMissionToFront(callerAbility->GetMissionId(), false);
}

void MissionListManager::MoveToTerminateList(const std::shared_ptr<AbilityRecord>& abilityRecord)
{
    HILOG_INFO("MoveToDefaultList start.");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("timeout ability record is nullptr.");
        return;
    }
    auto missionList = abilityRecord->GetOwnedMissionList();
    if (missionList == nullptr) {
        HILOG_ERROR("timeout missionList is nullptr.");
        return;
    }
    auto selMisson = abilityRecord->GetMission();
    if (selMisson == nullptr) {
        HILOG_ERROR("timeout misson is nullptr.");
        return;
    }

    missionList->RemoveMission(selMisson);
    if (missionList->GetType() == MissionListType::CURRENT && missionList->IsEmpty()) {
        RemoveMissionList(missionList);
    }

    // load timeout will not wait for died event, directly remove.
    if (abilityRecord->IsAbilityState(AbilityState::INITIAL)) {
        HILOG_WARN("load timeout will not wait for died event, directly remove.");
        // update running state.
        InnerMissionInfo info;
        if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
            selMisson->GetMissionId(), info) == 0) {
            info.missionInfo.runningState = -1;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        }
        return;
    }
    HILOG_DEBUG("success move timeout ability to terminate mission list.");
    // other remove to terminate list.
    abilityRecord->SetTerminatingState();
    terminateAbilityList_.push_back(abilityRecord);

    HILOG_INFO("MoveToDefaultList end");
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
                // not support move launcher to front.
                HILOG_ERROR("get launcher mission list, missionId: %{public}d", missionId);
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
        innerMissionInfo.missionInfo.want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, userId_);
    if (generateAbility != ERR_OK) {
        HILOG_ERROR("cannot find generate ability request, missionId: %{public}d", missionId);
        return nullptr;
    }

    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::string curTime = GetCurrentTime();
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionTimeStamp(mission->GetMissionId(), curTime);
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

    bool isForeground = ability->IsAbilityState(FOREGROUND) || ability->IsAbilityState(FOREGROUNDING);
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
    CHECK_POINTER_LOG(ability, "ability is null.");
    if (ability->IsTerminating()) {
        HILOG_INFO("Handle Ability DiedByTerminating.");
        CompleteTerminateAndUpdateMission(ability);
        return;
    }

    auto mission = ability->GetMission();
    CHECK_POINTER_LOG(mission, "Fail to get mission.");
    auto missionList = mission->GetMissionList();
    CHECK_POINTER_LOG(missionList, "Fail to get mission list.");

    std::shared_ptr<AbilityRecord> launcherRoot = launcherList_->GetLauncherRoot();
    bool isLauncherActive = (launcherRoot &&
        (launcherRoot->IsAbilityState(FOREGROUND) || launcherRoot->IsAbilityState(FOREGROUNDING)));
    bool isForeground = ability->IsAbilityState(FOREGROUND) || ability->IsAbilityState(FOREGROUNDING);

    // remove from mission list.
    missionList->RemoveMission(mission);
    if (missionList->GetType() == MissionListType::CURRENT && missionList->IsEmpty()) {
        RemoveMissionList(missionList);
    }

    // update running state.
    if (!ability->IsUninstallAbility()) {
        InnerMissionInfo info;
        if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
            mission->GetMissionId(), info) == 0) {
            info.missionInfo.runningState = -1;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        }
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

    auto launcherRootAbility = launcherList_->GetLauncherRoot();
    if (!launcherRootAbility) {
        HILOG_WARN("no root launcher ability, no need back to launcher.");
        return;
    }
    auto launcherRootMission = launcherRootAbility->GetMission();
    if (!launcherRootMission) {
        HILOG_WARN("no root launcher mission, no need back to launcher.");
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

#ifdef SUPPORT_GRAPHICS
int MissionListManager::SetMissionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (!token) {
        HILOG_INFO("SetMissionIcon token is nullptr.");
        return -1;
    }

    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId <= 0) {
        HILOG_INFO("SetMissionIcon find mission failed.");
        return -1;
    }

    if (listenerController_) {
        listenerController_->NotifyMissionIconChanged(missionId, icon);
    }

    return 0;
}
#endif

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

void MissionListManager::DumpMissionListByRecordId(
    std::vector<std::string> &info, bool isClient, int32_t abilityRecordId, const std::vector<std::string> &params)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (const auto& missionList : currentMissionLists_) {
        if (missionList && missionList != launcherList_) {
            HILOG_INFO("missionList begain to call DumpMissionListByRecordId %{public}s", __func__);
            missionList->DumpStateByRecordId(info, isClient, abilityRecordId, params);
        }
    }

    if (defaultStandardList_) {
        HILOG_INFO("defaultStandardList begain to call DumpMissionListByRecordId %{public}s", __func__);
        defaultStandardList_->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }

    if (defaultSingleList_) {
        HILOG_INFO("defaultSingleList begain to call DumpMissionListByRecordId %{public}s", __func__);
        defaultSingleList_->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }

    if (launcherList_) {
        HILOG_INFO("launcherList begain to call DumpMissionListByRecordId %{public}s", __func__);
        launcherList_->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }
}
void MissionListManager::DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    if (args.size() != 0 &&
        args != "NORMAL" &&
        args != "DEFAULT_STANDARD" &&
        args != "DEFAULT_SINGLE" &&
        args != "LAUNCHER") {
        info.emplace_back("MissionList Type NORMAL|DEFAULT_STANDARD|DEFAULT_SINGLE|LAUNCHER");
        return;
    }

    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    if (args.size() == 0 || args == "NORMAL") {
        dumpInfo = "  Current mission lists:";
        info.push_back(dumpInfo);
        for (const auto& missionList : currentMissionLists_) {
            if (missionList) {
                missionList->DumpList(info, isClient);
            }
        }
    }

    if (args.size() == 0 || args == "DEFAULT_STANDARD") {
        dumpInfo = "  default stand mission list:";
        info.push_back(dumpInfo);
        if (defaultStandardList_) {
            defaultStandardList_->DumpList(info, isClient);
        }
    }

    if (args.size() == 0 || args == "DEFAULT_SINGLE") {
        dumpInfo = "  default single mission list:";
        info.push_back(dumpInfo);
        if (defaultSingleList_) {
            defaultSingleList_->DumpList(info, isClient);
        }
    }
    if (args.size() == 0 || args == "LAUNCHER") {
        dumpInfo = "  launcher mission list:";
        info.push_back(dumpInfo);
        if (launcherList_) {
            launcherList_->DumpList(info, isClient);
        }
    }
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

int MissionListManager::ResolveLocked(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("%{public}s, resolve ability_name:%{public}s",
        __func__,
        abilityRequest.want.GetElement().GetURI().c_str());

    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        HILOG_ERROR("%{public}s, resolve ability_name:", __func__);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    return CallAbilityLocked(abilityRequest);
}

int MissionListManager::CallAbilityLocked(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("call ability.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    // allow to start ability by called type without loading ui.
    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        HILOG_ERROR("start ability not by call.");
        return ERR_INVALID_VALUE;
    }

    // Get target mission and ability record.
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<Mission> targetMission;
    GetTargetMissionAndAbility(abilityRequest, targetMission, targetAbilityRecord);
    if (!targetMission || !targetAbilityRecord) {
        HILOG_ERROR("Failed to get mission or record.");
        return ERR_INVALID_VALUE;
    }

    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);

    // mission is first created, add mission to default call mission list.
    // other keep in current mission list.
    if (!targetMission->GetMissionList()) {
        defaultSingleList_->AddMissionToTop(targetMission);
    }

    // new version started by call type
    auto ret = ResolveAbility(targetAbilityRecord, abilityRequest);
    if (ret == ResolveResultType::OK_HAS_REMOTE_OBJ) {
        HILOG_DEBUG("target ability has been resolved.");
        return ERR_OK;
    } else if (ret == ResolveResultType::NG_INNER_ERROR) {
        HILOG_ERROR("resolve failed, error: %{public}d.", RESOLVE_CALL_ABILITY_INNER_ERR);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    // schedule target ability
    std::string element = targetAbilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("load ability record: %{public}s", element.c_str());

    // flag the first ability.
    auto currentTopAbility = GetCurrentTopAbilityLocked();
    if (!currentTopAbility) {
        if (targetAbilityRecord->GetAbilityInfo().applicationInfo.isLauncherApp) {
            targetAbilityRecord->SetLauncherRoot();
        }
    }

    return targetAbilityRecord->LoadAbility();
}

int MissionListManager::ReleaseLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_DEBUG("release ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecordByName(element);
    CHECK_POINTER_AND_RETURN(abilityRecord, RELEASE_CALL_ABILITY_INNER_ERR);

    if (!abilityRecord->Release(connect)) {
        HILOG_ERROR("ability release call record failed.");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    return ERR_OK;
}

int MissionListManager::ResolveAbility(
    const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("targetAbilityRecord resolve call record.");
    CHECK_POINTER_AND_RETURN(targetAbility, ResolveResultType::NG_INNER_ERROR);

    ResolveResultType result = targetAbility->Resolve(abilityRequest);
    switch (result) {
        case ResolveResultType::NG_INNER_ERROR:
        case ResolveResultType::OK_HAS_REMOTE_OBJ:
            return result;
        default:
            break;
    }

    if (targetAbility->IsReady()) {
        HILOG_DEBUG("targetAbility is ready, directly scheduler call request.");
        if (targetAbility->CallRequest()) {
            return ResolveResultType::OK_HAS_REMOTE_OBJ;
        }
    }

    HILOG_DEBUG("targetAbility need to call request after lifecycle.");
    return result;
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByName(const AppExecFwk::ElementName &element)
{
    // find in currentMissionLists_
    for (auto missionList : currentMissionLists_) {
        if (missionList != nullptr) {
            auto ability = missionList->GetAbilityRecordByName(element);
            if (ability != nullptr) {
                return ability;
            }
        }
    }

    // find in lanucheMissionList_
    auto ability = launcherList_->GetAbilityRecordByName(element);
    if (ability != nullptr) {
        return ability;
    }

    // find in default singlelist_
    return defaultSingleList_->GetAbilityRecordByName(element);
}

void MissionListManager::OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord)
{
    HILOG_INFO("On callConnect died.");
    CHECK_POINTER(callRecord);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    AppExecFwk::ElementName element = callRecord->GetTargetServiceName();
    auto abilityRecord = GetAbilityRecordByName(element);
    CHECK_POINTER(abilityRecord);
    abilityRecord->Release(callRecord->GetConCallBack());
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

    if (!flag.empty()) {
        auto mission = GetMissionBySpecifiedFlag(want, flag);
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
    }

    abilityRequest.specifiedFlag = flag;
    StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

void MissionListManager::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    if (waittingAbilityQueue_.empty()) {
        return;
    }
    waittingAbilityQueue_.pop();

    if (waittingAbilityQueue_.empty()) {
        return;
    }
    AbilityRequest abilityRequest = waittingAbilityQueue_.front();
    waittingAbilityQueue_.pop();

    auto currentTopAbility = GetCurrentTopAbilityLocked();
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    StartAbility(currentTopAbility, callerAbility, abilityRequest);
}

std::shared_ptr<Mission> MissionListManager::GetMissionBySpecifiedFlag(
    const AAFwk::Want &want, const std::string &flag) const
{
    std::shared_ptr<Mission> mission = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (mission = missionList->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
            return mission;
        }
    }

    if ((mission = defaultSingleList_->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
        return mission;
    }

    if ((mission = launcherList_->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
        return mission;
    }

    return defaultStandardList_->GetMissionBySpecifiedFlag(want, flag);
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

bool MissionListManager::GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot)
{
    HILOG_INFO("snapshot: Start get mission snapshot.");
    bool forceSnapshot = false;
    {
        std::lock_guard<std::recursive_mutex> guard(managerLock_);
        auto abilityRecord = GetAbilityRecordByToken(abilityToken);
        if (abilityRecord && abilityRecord->IsAbilityState(FOREGROUND)) {
            forceSnapshot = true;
        }
    }
    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionSnapshot(
        missionId, abilityToken, missionSnapshot, forceSnapshot);
}

void MissionListManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    auto func = [&info, isPerm](const std::shared_ptr<Mission> &mission) {
        if (!mission) {
            return;
        }

        auto ability = mission->GetAbilityRecord();
        if (!ability) {
            return;
        }

        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, ability);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = ability->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, ability);
            }
        }
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

std::shared_ptr<AbilityRecord> MissionListManager::GetCurrentTopAbility(const std::string &bundleName)
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);

    for (auto &missionList : currentMissionLists_) {
        if (!missionList) {
            HILOG_WARN("Invalid missionList.");
            continue;
        }

        auto missions = missionList->GetAllMissions();
        for (auto &mission : missions) {
            if (!mission) {
                HILOG_WARN("Invalid mission.");
                continue;
            }

            auto abilityRecord = mission->GetAbilityRecord();
            if (!abilityRecord) {
                HILOG_WARN("Invalid ability record.");
                continue;
            }

            auto appInfo = abilityRecord->GetApplicationInfo();
            if (bundleName.compare(appInfo.bundleName) == 0) {
                return abilityRecord;
            }
        }
    }

    return {};
}

void MissionListManager::UninstallApp(const std::string &bundleName, int32_t uid)
{
    HILOG_INFO("Uninstall app, bundleName: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpMgr = shared_from_this();
    auto task = [wpMgr, bundleName, uid]() {
        HILOG_INFO("Handle Uninstall app, bundleName: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
        auto mgr = wpMgr.lock();
        if (mgr) {
            mgr->AddUninstallTags(bundleName, uid);
        }
    };
    handler->PostTask(task);
}

void MissionListManager::AddUninstallTags(const std::string &bundleName, int32_t uid)
{
    HILOG_INFO("AddUninstallTags, bundleName: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    for (auto it = currentMissionLists_.begin(); it != currentMissionLists_.end();) {
        auto missionList = *it;
        if (missionList) {
            missionList->HandleUnInstallApp(bundleName, uid); // add tag here.
            if (missionList->IsEmpty()) {
                currentMissionLists_.erase(it++);
                continue;
            }
        }
        it++;
    }
    defaultSingleList_->HandleUnInstallApp(bundleName, uid);
    defaultStandardList_->HandleUnInstallApp(bundleName, uid);
    std::list<int32_t> matchedMissions;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->HandleUnInstallApp(bundleName, uid, matchedMissions);
    if (listenerController_) {
        listenerController_->HandleUnInstallApp(matchedMissions);
    }
}

bool MissionListManager::IsStarted()
{
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    auto launcherRoot = launcherList_->GetLauncherRoot();
    return launcherRoot != nullptr;
}

void MissionListManager::PauseManager()
{
    HILOG_INFO("MissionListManager PauseManager. move foreground to background.");
    std::lock_guard<std::recursive_mutex> guard(managerLock_);
    std::list<std::shared_ptr<AbilityRecord>> foregroundAbilities;
    GetAllForegroundAbilities(foregroundAbilities);

    for (auto& abilityRecord : foregroundAbilities) {
        if (!abilityRecord) {
            continue;
        }
        abilityRecord->SetSwitchingPause(true);
        MoveToBackgroundTask(abilityRecord);
    }
}

void MissionListManager::ResumeManager()
{
    HILOG_INFO("ResumeManager, back to launcher.");
    BackToLauncher();
}

void MissionListManager::GetAllForegroundAbilities(std::list<std::shared_ptr<AbilityRecord>>& foregroundList)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto& missionList : currentMissionLists_) {
        GetForegroundAbilities(missionList, foregroundList);
    }
    GetForegroundAbilities(defaultSingleList_, foregroundList);
    GetForegroundAbilities(defaultStandardList_, foregroundList);
}

void MissionListManager::GetForegroundAbilities(const std::shared_ptr<MissionList>& missionList,
    std::list<std::shared_ptr<AbilityRecord>>& foregroundList)
{
    if (!missionList || missionList->IsEmpty()) {
        return;
    }

    for (auto& mission : missionList->GetAllMissions()) {
        if (!mission) {
            continue;
        }

        auto abilityRecord = mission->GetAbilityRecord();
        if (!abilityRecord) {
            continue;
        }

        if (abilityRecord->IsActiveState()) {
            foregroundList.emplace_back(abilityRecord);
        }
    }
}

#ifdef ABILITY_COMMAND_FOR_TEST
int MissionListManager::BlockAbility(int32_t abilityRecordId)
{
    int ret = -1;
    for (const auto &missionList : currentMissionLists_) {
        if (missionList && missionList != launcherList_) {
            HILOG_INFO("missionList begain to call BlockAbilityByRecordId %{public}s", __func__);
            if (missionList->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
                HILOG_INFO("missionList call BlockAbilityByRecordId success");
                ret = ERR_OK;
            }
        }
    }

    if (defaultStandardList_) {
        HILOG_INFO("defaultStandardList begain to call BlockAbilityByRecordId %{public}s", __func__);
        if (defaultStandardList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            HILOG_INFO("defaultStandardList call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }

    if (defaultSingleList_) {
        HILOG_INFO("defaultSingleList begain to call BlockAbilityByRecordId %{public}s", __func__);
        if (defaultSingleList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            HILOG_INFO("defaultSingleList_ call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }

    if (launcherList_) {
        HILOG_INFO("launcherList begain to call BlockAbilityByRecordId %{public}s", __func__);
        if (launcherList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            HILOG_INFO("launcherList_ call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }
    return ret;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
