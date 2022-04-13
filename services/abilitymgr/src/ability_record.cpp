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

#include "ability_record.h"

#include <singleton.h>
#include <vector>

#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "ability_scheduler_stub.h"
#include "ability_util.h"
#include "bundle_mgr_client.h"
#include "bytrace.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "os_account_manager.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AAFwk {
const std::string DEBUG_APP = "debugApp";
int64_t AbilityRecord::abilityRecordId = 0;
int64_t AbilityRecord::g_abilityRecordEventId_ = 0;
const int32_t DEFAULT_USER_ID = 0;
const std::map<AbilityState, std::string> AbilityRecord::stateToStrMap = {
    std::map<AbilityState, std::string>::value_type(INITIAL, "INITIAL"),
    std::map<AbilityState, std::string>::value_type(INACTIVE, "INACTIVE"),
    std::map<AbilityState, std::string>::value_type(ACTIVE, "ACTIVE"),
    std::map<AbilityState, std::string>::value_type(BACKGROUND, "BACKGROUND"),
    std::map<AbilityState, std::string>::value_type(SUSPENDED, "SUSPENDED"),
    std::map<AbilityState, std::string>::value_type(INACTIVATING, "INACTIVATING"),
    std::map<AbilityState, std::string>::value_type(ACTIVATING, "ACTIVATING"),
    std::map<AbilityState, std::string>::value_type(MOVING_BACKGROUND, "MOVING_BACKGROUND"),
    std::map<AbilityState, std::string>::value_type(TERMINATING, "TERMINATING"),
    std::map<AbilityState, std::string>::value_type(FOREGROUND_NEW, "FOREGROUND_NEW"),
    std::map<AbilityState, std::string>::value_type(BACKGROUND_NEW, "BACKGROUND_NEW"),
    std::map<AbilityState, std::string>::value_type(FOREGROUNDING_NEW, "FOREGROUNDING_NEW"),
    std::map<AbilityState, std::string>::value_type(BACKGROUNDING_NEW, "BACKGROUNDING_NEW"),
};
const std::map<AppState, std::string> AbilityRecord::appStateToStrMap_ = {
    std::map<AppState, std::string>::value_type(AppState::BEGIN, "BEGIN"),
    std::map<AppState, std::string>::value_type(AppState::READY, "READY"),
    std::map<AppState, std::string>::value_type(AppState::FOREGROUND, "FOREGROUND"),
    std::map<AppState, std::string>::value_type(AppState::BACKGROUND, "BACKGROUND"),
    std::map<AppState, std::string>::value_type(AppState::SUSPENDED, "SUSPENDED"),
    std::map<AppState, std::string>::value_type(AppState::TERMINATED, "TERMINATED"),
    std::map<AppState, std::string>::value_type(AppState::END, "END"),
};
const std::map<AbilityLifeCycleState, AbilityState> AbilityRecord::convertStateMap = {
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INITIAL, INITIAL),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INACTIVE, INACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_ACTIVE, ACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_BACKGROUND, BACKGROUND),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_SUSPENDED, SUSPENDED),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_FOREGROUND_NEW, FOREGROUND_NEW),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_BACKGROUND_NEW, BACKGROUND_NEW),
};

Token::Token(std::weak_ptr<AbilityRecord> abilityRecord) : abilityRecord_(abilityRecord)
{}

Token::~Token()
{}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(token, nullptr);
    return (static_cast<Token *>(token.GetRefPtr()))->GetAbilityRecord();
}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecord() const
{
    return abilityRecord_.lock();
}

AbilityRecord::AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode, int32_t apiVersion)
    : want_(want), abilityInfo_(abilityInfo), applicationInfo_(applicationInfo),
    requestCode_(requestCode), compatibleVersion_(apiVersion)
{
    recordId_ = abilityRecordId++;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr) {
        abilityMgr->GetMaxRestartNum(restratMax_);
    }
    restartCount_ = restratMax_;
}

AbilityRecord::~AbilityRecord()
{
    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto object = scheduler_->AsObject();
        if (object != nullptr) {
            object->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (abilityRequest.IsNewVersion() && abilityRequest.abilityInfo.type == AbilityType::PAGE) {
        abilityRecord = std::make_shared<AbilityRecordNew>(abilityRequest.want, abilityRequest.abilityInfo,
            abilityRequest.appInfo, abilityRequest.requestCode, abilityRequest.compatibleVersion);
    } else {
        abilityRecord = std::make_shared<AbilityRecord>(abilityRequest.want, abilityRequest.abilityInfo,
            abilityRequest.appInfo, abilityRequest.requestCode, abilityRequest.compatibleVersion);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, nullptr);
    abilityRecord->SetUid(abilityRequest.uid);
    if (!abilityRecord->Init()) {
        HILOG_ERROR("failed to init new ability record");
        return nullptr;
    }
    if (abilityRequest.startSetting != nullptr) {
        HILOG_INFO("abilityRequest.startSetting...");
        abilityRecord->SetStartSetting(abilityRequest.startSetting);
    }
    if (abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        HILOG_INFO("abilityRequest.callType is CALL_REQUEST_TYPE.");
        abilityRecord->SetStartedByCall(true);
    }
    return abilityRecord;
}

bool AbilityRecord::Init()
{
    lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    CHECK_POINTER_RETURN_BOOL(lifecycleDeal_);

    token_ = new (std::nothrow) Token(weak_from_this());
    CHECK_POINTER_RETURN_BOOL(token_);

    if (applicationInfo_.isLauncherApp) {
        isLauncherAbility_ = true;
    }
    return true;
}

void AbilityRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t AbilityRecord::GetUid()
{
    return uid_;
}

int AbilityRecord::LoadAbility()
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("start load ability, name is %{public}s", abilityInfo_.name.c_str());
    startTime_ = AbilityUtil::SystemTimeMillis();
    CHECK_POINTER_AND_RETURN(token_, ERR_INVALID_VALUE);
    std::string appName = applicationInfo_.name;
    if (appName.empty()) {
        HILOG_ERROR("app name is empty");
        return ERR_INVALID_VALUE;
    }

    if (!CanRestartRootLauncher()) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        return ERR_INVALID_VALUE;
    }

    if (abilityInfo_.type != AppExecFwk::AbilityType::DATA) {
        SendEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, AbilityManagerService::LOAD_TIMEOUT);
    }
    sptr<Token> callerToken_ = nullptr;
    if (!callerList_.empty() && callerList_.back()) {
        auto caller = callerList_.back()->GetCaller();
        if (caller) {
            callerToken_ = caller->GetToken();
        }
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        token_, callerToken_, abilityInfo_, applicationInfo_, want_);
}

bool AbilityRecord::CanRestartRootLauncher()
{
    if (isLauncherRoot_ && isRestarting_ && IsLauncherAbility() && (restartCount_ <= 0)) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        return false;
    }
    return true;
}

void AbilityRecord::ForegroundAbility(uint32_t sceneFlag)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("ForegroundAbility.");
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, AbilityManagerService::FOREGROUNDNEW_TIMEOUT);

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::FOREGROUNDING_NEW;
    lifeCycleStateInfo_.sceneFlag = sceneFlag;
    lifecycleDeal_->ForegroundNew(want_, lifeCycleStateInfo_);
    lifeCycleStateInfo_.sceneFlag = 0;
    lifeCycleStateInfo_.sceneFlagBak = 0;

    // update ability state to appMgr service when restart
    if (IsNewWant()) {
        sptr<Token> preToken = nullptr;
        if (GetPreAbilityRecord()) {
            preToken = GetPreAbilityRecord()->GetToken();
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token_, preToken, 1, 1, 1);
    }
}

void AbilityRecord::ProcessForegroundAbility(uint32_t sceneFlag)
{
    std::string element = GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability record: %{public}s", element.c_str());

    if (isReady_) {
        if (IsAbilityState(AbilityState::BACKGROUND_NEW)) {
            // background to activte state
            HILOG_DEBUG("MoveToForground, %{public}s", element.c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token_);
        } else {
            HILOG_DEBUG("Activate %{public}s", element.c_str());
            ForegroundAbility(sceneFlag);
        }
    } else {
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        LoadAbility();
    }
}

void AbilityRecord::BackgroundAbility(const Closure &task)
{
    HILOG_INFO("Move to backgroundNew.");
    CHECK_POINTER(lifecycleDeal_);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::BACKGROUNDNEW_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }

    if (!IsTerminating() || IsRestarting()) {
        // schedule save ability state before moving to background.
        SaveAbilityState();
    }

    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::BACKGROUNDING_NEW;
    lifecycleDeal_->BackgroundNew(want_, lifeCycleStateInfo_);
}

int AbilityRecord::TerminateAbility()
{
    HILOG_INFO("%{public}s", __func__);
    return DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token_);
}

const AppExecFwk::AbilityInfo &AbilityRecord::GetAbilityInfo() const
{
    return abilityInfo_;
}

const AppExecFwk::ApplicationInfo &AbilityRecord::GetApplicationInfo() const
{
    return applicationInfo_;
}

AbilityState AbilityRecord::GetAbilityState() const
{
    return currentState_;
}

bool AbilityRecord::IsForeground() const
{
    return currentState_ == AbilityState::FOREGROUND_NEW || currentState_ == AbilityState::FOREGROUNDING_NEW;
}

void AbilityRecord::SetAbilityState(AbilityState state)
{
    currentState_ = state;
    if (state == AbilityState::FOREGROUND_NEW || state == AbilityState::ACTIVE) {
        SetRestarting(false);
    }
}

void AbilityRecord::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    if (scheduler != nullptr) {
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        if (schedulerDeathRecipient_ == nullptr) {
            schedulerDeathRecipient_ =
                new AbilitySchedulerRecipient(std::bind(&AbilityRecord::OnSchedulerDied, this, std::placeholders::_1));
        }
        isReady_ = true;
        scheduler_ = scheduler;
        lifecycleDeal_->SetScheduler(scheduler);
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->AddDeathRecipient(schedulerDeathRecipient_);
        }
    } else {
        HILOG_ERROR("scheduler is nullptr");
        isReady_ = false;
        isWindowAttached_ = false;
        SetIsNewWant(false);
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        scheduler_ = scheduler;
    }
}

sptr<Token> AbilityRecord::GetToken() const
{
    return token_;
}

void AbilityRecord::SetPreAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    preAbilityRecord_ = abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetPreAbilityRecord() const
{
    return preAbilityRecord_.lock();
}

void AbilityRecord::SetNextAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    nextAbilityRecord_ = abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetNextAbilityRecord() const
{
    return nextAbilityRecord_.lock();
}

void AbilityRecord::SetEventId(int64_t eventId)
{
    eventId_ = eventId;
}

int64_t AbilityRecord::GetEventId() const
{
    return eventId_;
}

bool AbilityRecord::IsReady() const
{
    return isReady_;
}

#ifdef SUPPORT_GRAPHICS
bool AbilityRecord::IsWindowAttached() const
{
    return isWindowAttached_;
}
#endif

bool AbilityRecord::IsLauncherAbility() const
{
    return isLauncherAbility_;
}

bool AbilityRecord::IsTerminating() const
{
    return isTerminating_;
}

void AbilityRecord::SetTerminatingState()
{
    isTerminating_ = true;
}

bool AbilityRecord::IsNewWant() const
{
    return lifeCycleStateInfo_.isNewWant;
}

void AbilityRecord::SetIsNewWant(bool isNewWant)
{
    lifeCycleStateInfo_.isNewWant = isNewWant;
}

bool AbilityRecord::IsCreateByConnect() const
{
    return isCreateByConnect_;
}

void AbilityRecord::SetCreateByConnectMode()
{
    isCreateByConnect_ = true;
}

void AbilityRecord::Activate()
{
    HILOG_INFO("Activate.");
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, AbilityManagerService::ACTIVE_TIMEOUT);

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::ACTIVATING;
    lifecycleDeal_->Activate(want_, lifeCycleStateInfo_);

    // update ability state to appMgr service when restart
    if (IsNewWant()) {
        sptr<Token> preToken = nullptr;
        if (GetPreAbilityRecord()) {
            preToken = GetPreAbilityRecord()->GetToken();
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token_, preToken, 1, 1, 1);
    }
}

void AbilityRecord::Inactivate()
{
    HILOG_INFO("Inactivate.");
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, AbilityManagerService::INACTIVE_TIMEOUT);

    // schedule inactive after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::INACTIVATING;
    lifecycleDeal_->Inactivate(want_, lifeCycleStateInfo_);
}

void AbilityRecord::MoveToBackground(const Closure &task)
{
    HILOG_INFO("Move to background.");
    CHECK_POINTER(lifecycleDeal_);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::BACKGROUND_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }

    if (!IsTerminating() || IsRestarting()) {
        // schedule save ability state before moving to background.
        SaveAbilityState();
    }

    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::MOVING_BACKGROUND;
    lifecycleDeal_->MoveToBackground(want_, lifeCycleStateInfo_);
}

void AbilityRecord::Terminate(const Closure &task)
{
    HILOG_INFO("Terminate ability.");
    CHECK_POINTER(lifecycleDeal_);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::TERMINATE_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }
    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::TERMINATING;
    lifecycleDeal_->Terminate(want_, lifeCycleStateInfo_);
}

void AbilityRecord::ConnectAbility()
{
    HILOG_INFO("Connect ability.");
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->ConnectAbility(want_);
}

void AbilityRecord::DisconnectAbility()
{
    HILOG_INFO("Disconnect ability.");
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->DisconnectAbility(want_);
}

void AbilityRecord::CommandAbility()
{
    HILOG_INFO("Command ability, startId_:%{public}d.", startId_);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->CommandAbility(want_, false, startId_);
}

void AbilityRecord::SaveAbilityState()
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SaveAbilityState();
}

void AbilityRecord::SaveAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s : pacmap save", __func__);
    stateDatas_ = inState;
}

void AbilityRecord::RestoreAbilityState()
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->RestoreAbilityState(stateDatas_);
    stateDatas_.Clear();
    isRestarting_ = false;
}

void AbilityRecord::SetWant(const Want &want)
{
    want_ = want;
}

const Want &AbilityRecord::GetWant() const
{
    return want_;
}

int AbilityRecord::GetRequestCode() const
{
    return requestCode_;
}

void AbilityRecord::SetResult(const std::shared_ptr<AbilityResult> &result)
{
    result_ = result;
}

std::shared_ptr<AbilityResult> AbilityRecord::GetResult() const
{
    return result_;
}

void AbilityRecord::SendResult()
{
    HILOG_INFO("Send result.");
    std::lock_guard<std::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);
    CHECK_POINTER(result_);
    scheduler_->SendResult(result_->requestCode_, result_->resultCode_, result_->resultWant_);
    GrantUriPermission(result_->resultWant_);
    // reset result to avoid send result next time
    result_.reset();
}

void AbilityRecord::SendResultToCallers()
{
    for (auto caller : GetCallerRecordList()) {
        if (caller == nullptr) {
            HILOG_WARN("Caller record is nullptr.");
            continue;
        }
        std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
        if (callerAbilityRecord != nullptr && callerAbilityRecord->GetResult() != nullptr) {
            callerAbilityRecord->SendResult();
        }
    }
}

void AbilityRecord::SaveResultToCallers(const int resultCode, const Want *resultWant)
{
    for (auto caller : GetCallerRecordList()) {
        if (caller == nullptr) {
            HILOG_WARN("Caller record is nullptr.");
            continue;
        }
        std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
        if (callerAbilityRecord != nullptr) {
            callerAbilityRecord->SetResult(
                std::make_shared<AbilityResult>(caller->GetRequestCode(), resultCode, *resultWant));
        }
    }
}

void AbilityRecord::AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    auto it = std::find(connRecordList_.begin(), connRecordList_.end(), connRecord);
    // found it
    if (it != connRecordList_.end()) {
        HILOG_DEBUG("Found it in list, so no need to add same connection");
        return;
    }
    // no found then add new connection to list
    HILOG_DEBUG("No found in list, so add new connection to list");
    connRecordList_.push_back(connRecord);
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityRecord::GetConnectRecordList() const
{
    return connRecordList_;
}

void AbilityRecord::RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    connRecordList_.remove(connRecord);
}

void AbilityRecord::AddCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Add caller record.");
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER(abilityRecord);

    auto isExist = [&abilityRecord](const std::shared_ptr<CallerRecord> &callerRecord) {
        return (callerRecord->GetCaller() == abilityRecord);
    };

    auto record = std::find_if(callerList_.begin(), callerList_.end(), isExist);
    if (record != callerList_.end()) {
        callerList_.erase(record);
    }

    callerList_.emplace_back(std::make_shared<CallerRecord>(requestCode, abilityRecord));

    lifeCycleStateInfo_.caller.requestCode = requestCode;
    lifeCycleStateInfo_.caller.deviceId = abilityRecord->GetAbilityInfo().deviceId;
    lifeCycleStateInfo_.caller.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    lifeCycleStateInfo_.caller.abilityName = abilityRecord->GetAbilityInfo().name;
    HILOG_INFO("caller %{public}s, %{public}s, %{public}s",
        abilityRecord->GetAbilityInfo().deviceId.c_str(),
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str());
}

std::list<std::shared_ptr<CallerRecord>> AbilityRecord::GetCallerRecordList() const
{
    return callerList_;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetCallerRecord() const
{
    if (callerList_.empty()) {
        return nullptr;
    }
    return callerList_.back()->GetCaller();
}

bool AbilityRecord::IsConnectListEmpty()
{
    return connRecordList_.empty();
}

std::shared_ptr<ConnectionRecord> AbilityRecord::GetConnectingRecord() const
{
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::CONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityRecord::GetConnectingRecordList()
{
    std::list<std::shared_ptr<ConnectionRecord>> connectingList;
    for (auto record : connRecordList_) {
        if (record && record->GetConnectState() == ConnectionState::CONNECTING) {
            connectingList.push_back(record);
        }
    }
    return connectingList;
}

std::shared_ptr<ConnectionRecord> AbilityRecord::GetDisconnectingRecord() const
{
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::DISCONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

void AbilityRecord::GetAbilityTypeString(std::string &typeStr)
{
    AppExecFwk::AbilityType type = GetAbilityInfo().type;
    switch (type) {
#ifdef SUPPORT_GRAPHICS
        case AppExecFwk::AbilityType::PAGE: {
            typeStr = "PAGE";
            break;
        }
#endif
        case AppExecFwk::AbilityType::SERVICE: {
            typeStr = "SERVICE";
            break;
        }
        // for config.json type
        case AppExecFwk::AbilityType::DATA: {
            typeStr = "DATA";
            break;
        }
        default: {
            typeStr = "UNKNOWN";
            break;
        }
    }
}

std::string AbilityRecord::ConvertAbilityState(const AbilityState &state)
{
    auto it = stateToStrMap.find(state);
    if (it != stateToStrMap.end()) {
        return it->second;
    }
    return "INVALIDSTATE";
}

std::string AbilityRecord::ConvertAppState(const AppState &state)
{
    auto it = appStateToStrMap_.find(state);
    if (it != appStateToStrMap_.end()) {
        return it->second;
    }
    return "INVALIDSTATE";
}

int AbilityRecord::ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state)
{
    auto it = convertStateMap.find(state);
    if (it != convertStateMap.end()) {
        return it->second;
    }
    return DEFAULT_INVAL_VALUE;
}

void AbilityRecord::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    // get ability type(unknown/page/service/provider)
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        next ability main name [" + nextAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
}

void AbilityRecord::DumpAbilityState(
    std::vector<std::string> &info, bool isClient, const std::vector<std::string> &params)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);

    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);
    dumpInfo = "        callee connections: ";
    info.push_back(dumpInfo);
    if (callContainer_) {
        callContainer_->Dump(info);
    }

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }

    // add dump client info
    if (isClient && scheduler_ && isReady_) {
        scheduler_->DumpAbilityInfo(params, info);
        if (!params.empty()) {
            return;
        }
        AppExecFwk::Configuration config;
        if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
            info.emplace_back("        configuration: " + config.GetName());
        }
    }
}

void AbilityRecord::SetStartTime()
{
    if (startTime_ == 0) {
        startTime_ = AbilityUtil::SystemTimeMillis();
    }
}

int64_t AbilityRecord::GetStartTime() const
{
    return startTime_;
}

void AbilityRecord::DumpService(std::vector<std::string> &info, bool isClient) const
{
    info.emplace_back("      AbilityRecord ID #" + std::to_string(GetRecordId()) + "   state #" +
                      AbilityRecord::ConvertAbilityState(GetAbilityState()) + "   start time [" +
                      std::to_string(GetStartTime()) + "]");
    info.emplace_back("      main name [" + GetAbilityInfo().name + "]");
    info.emplace_back("      bundle name [" + GetAbilityInfo().bundleName + "]");
    info.emplace_back("      ability type [SERVICE]");
    info.emplace_back("      app state #" + AbilityRecord::ConvertAppState(appState_));

    if (isLauncherRoot_) {
        info.emplace_back("      can restart num #" + std::to_string(restartCount_));
    }

    info.emplace_back("      Connections: " + std::to_string(connRecordList_.size()));
    for (auto &&conn : connRecordList_) {
        if (conn) {
            conn->Dump(info);
        }
    }
    // add dump client info
    if (isClient && scheduler_ && isReady_) {
        std::vector<std::string> params;
        scheduler_->DumpAbilityInfo(params, info);
        AppExecFwk::Configuration config;
        if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
            info.emplace_back("      configuration: " + config.GetName());
        }
    }
}

void AbilityRecord::GetAbilityRecordInfo(AbilityRecordInfo &recordInfo)
{
    recordInfo.elementName = want_.GetElement().GetURI();
    recordInfo.id = recordId_;
    recordInfo.appName = abilityInfo_.applicationName;
    recordInfo.mainName = abilityInfo_.name;
    recordInfo.abilityType = static_cast<int32_t>(abilityInfo_.type);

    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility) {
        recordInfo.previousAppName = preAbility->GetAbilityInfo().applicationName;
        recordInfo.previousMainName = preAbility->GetAbilityInfo().name;
    }

    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility) {
        recordInfo.nextAppName = nextAbility->GetAbilityInfo().applicationName;
        recordInfo.nextMainName = nextAbility->GetAbilityInfo().name;
    }

    recordInfo.state = static_cast<AbilityState>(currentState_);
    recordInfo.startTime = std::to_string(startTime_);
    recordInfo.ready = isReady_;
    recordInfo.windowAttached = isWindowAttached_;
    recordInfo.lanucher = isLauncherAbility_;
}

void AbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    HILOG_WARN("On scheduler died.");
    auto mission = GetMission();
    if (mission) {
        HILOG_WARN("On scheduler died. Is app not response Reason:%{public}d", mission->IsANRState());
    }
    std::lock_guard<std::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);

    auto object = remote.promote();
    CHECK_POINTER(object);

    if (object != scheduler_->AsObject()) {
        HILOG_ERROR("Ability on scheduler died: scheduler is not matches with remote.");
        return;
    }

    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    scheduler_.clear();
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SetScheduler(nullptr);
    isWindowAttached_ = false;

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);

    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);

    HILOG_INFO("Ability on scheduler died: '%{public}s'", abilityInfo_.name.c_str());
    auto task = [abilityManagerService, ability = shared_from_this()]() {
        abilityManagerService->OnAbilityDied(ability);
    };
    handler->PostTask(task);
    auto uriTask = [want = want_, ability = shared_from_this()]() {
        ability->SaveResultToCallers(-1, &want);
        ability->SendResultToCallers();
    };
    handler->PostTask(uriTask);
}

void AbilityRecord::SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject)
{
    connRemoteObject_ = remoteObject;
}

sptr<IRemoteObject> AbilityRecord::GetConnRemoteObject() const
{
    return connRemoteObject_;
}

void AbilityRecord::AddStartId()
{
    startId_++;
}
int AbilityRecord::GetStartId() const
{
    return startId_;
}

void AbilityRecord::SetIsUninstallAbility()
{
    isUninstall_ = true;
}

bool AbilityRecord::IsUninstallAbility() const
{
    return isUninstall_;
}

void AbilityRecord::SetLauncherRoot()
{
    isLauncherRoot_ = true;
}

bool AbilityRecord::IsLauncherRoot() const
{
    return isLauncherRoot_;
}

bool AbilityRecord::IsAbilityState(const AbilityState &state) const
{
    return (currentState_ == state);
}

bool AbilityRecord::IsActiveState() const
{
    return (IsAbilityState(AbilityState::ACTIVE) || IsAbilityState(AbilityState::ACTIVATING) ||
            IsAbilityState(AbilityState::INITIAL) || IsAbilityState(AbilityState::FOREGROUND_NEW) ||
            IsAbilityState(AbilityState::FOREGROUNDING_NEW));
}

void AbilityRecord::SendEvent(uint32_t msg, uint32_t timeOut)
{
    if (want_.GetBoolParam(DEBUG_APP, false)) {
        HILOG_INFO("Is debug mode, no need to handle time out.");
        return;
    }
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);

    g_abilityRecordEventId_++;
    eventId_ = g_abilityRecordEventId_;
    handler->SendEvent(msg, eventId_, timeOut);
}

void AbilityRecord::SetStartSetting(const std::shared_ptr<AbilityStartSetting> &setting)
{
    lifeCycleStateInfo_.setting = setting;
}

std::shared_ptr<AbilityStartSetting> AbilityRecord::GetStartSetting() const
{
    return lifeCycleStateInfo_.setting;
}

void AbilityRecord::SetRestarting(const bool isRestart)
{
    isRestarting_ = isRestart;
    HILOG_DEBUG("SetRestarting: %{public}d", isRestarting_);

    if (isLauncherRoot_ && IsLauncherAbility()) {
        restartCount_ = isRestart ? (--restartCount_) : restratMax_;
        HILOG_INFO("root launcher restart count: %{public}d", restartCount_);
    }
}

void AbilityRecord::SetRestarting(const bool isRestart, int32_t canReStartCount)
{
    isRestarting_ = isRestart;
    HILOG_DEBUG("SetRestarting: %{public}d, restart count: %{public}d", isRestarting_, canReStartCount);

    if (isLauncherRoot_ && IsLauncherAbility()) {
        restartCount_ = isRestart ? canReStartCount : restratMax_;
        HILOG_INFO("root launcher restart count: %{public}d", restartCount_);
    }
}

int32_t AbilityRecord::GetRestartCount() const
{
    return restartCount_;
}

bool AbilityRecord::IsRestarting() const
{
    return isRestarting_;
}

void AbilityRecord::SetAppState(const AppState &state)
{
    appState_ = state;
}

AppState AbilityRecord::GetAppState() const
{
    return appState_;
}

void AbilityRecord::ClearFlag()
{
    isRestarting_ = false;
    isUninstall_ = false;
    isTerminating_ = false;
    preAbilityRecord_.reset();
    nextAbilityRecord_.reset();
    startTime_ = 0;
    appState_ = AppState::END;
}

bool AbilityRecord::IsNewVersion()
{
    return compatibleVersion_ > API_VERSION_7;
}

void AbilityRecord::SetLaunchReason(const LaunchReason &reason)
{
    lifeCycleStateInfo_.launchParam.launchReason = reason;
}

void AbilityRecord::SetLastExitReason(const LastExitReason &reason)
{
    lifeCycleStateInfo_.launchParam.lastExitReason = reason;
}

void AbilityRecord::NotifyContinuationResult(int32_t result)
{
    HILOG_INFO("NotifyContinuationResult.");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->NotifyContinuationResult(result);
}

std::shared_ptr<MissionList> AbilityRecord::GetOwnedMissionList() const
{
    return missionList_.lock();
}

void AbilityRecord::SetMissionList(const std::shared_ptr<MissionList> &missionList)
{
    missionList_ = missionList;
}

void AbilityRecord::SetUseNewMission()
{
    lifeCycleStateInfo_.useNewMission = true;
}

void AbilityRecord::SetMission(const std::shared_ptr<Mission> &mission)
{
    if (mission) {
        missionId_ = mission->GetMissionId();
        HILOG_INFO("SetMission come, missionId is %{public}d.", missionId_);
    }
    mission_ = mission;
}

void AbilityRecord::SetMinimizeReason(bool fromUser)
{
    minimizeReason_ = fromUser;
}

bool AbilityRecord::IsMinimizeFromUser() const
{
    return minimizeReason_;
}

std::shared_ptr<Mission> AbilityRecord::GetMission() const
{
    return mission_.lock();
}

int32_t AbilityRecord::GetMissionId() const
{
    return missionId_;
}

void AbilityRecord::SetSpecifiedFlag(const std::string &flag)
{
    specifiedFlag_ = flag;
}

std::string AbilityRecord::GetSpecifiedFlag() const
{
    return specifiedFlag_;
}

AbilityRecordNew::AbilityRecordNew(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode, int32_t apiVersion)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode, apiVersion)
{
}

AbilityRecordNew::~AbilityRecordNew()
{
}

void AbilityRecordNew::Activate()
{
    ForegroundNew();
}

void AbilityRecordNew::Inactivate()
{
    HILOG_INFO("AbilityRecordNew Move to Inactivate.");
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        HILOG_ERROR("handler is nullptr or task is nullptr.");
        return;
    }
    SendEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, AbilityManagerService::INACTIVE_TIMEOUT);

    auto task = [token = token_]() {
        HILOG_DEBUG("AbilityRecordNew inactive done.");
        PacMap restoreData;
        DelayedSingleton<AbilityManagerService>::GetInstance()->AbilityTransitionDone(token,
            ABILITY_STATE_INACTIVE, restoreData);
    };
    handler->PostTask(task);
}

void AbilityRecordNew::ForegroundNew()
{
    HILOG_INFO("ForegroundingNew.");
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, AbilityManagerService::FOREGROUNDNEW_TIMEOUT);

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::FOREGROUNDING_NEW;
    lifecycleDeal_->ForegroundNew(want_, lifeCycleStateInfo_);

    // update ability state to appMgr service when restart
    if (IsNewWant()) {
        sptr<Token> preToken = nullptr;
        if (GetPreAbilityRecord()) {
            preToken = GetPreAbilityRecord()->GetToken();
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token_, preToken, 1, 1, 1);
    }
}

void AbilityRecordNew::MoveToBackground(const Closure &task)
{
    BackgroundNew(task);
}

void AbilityRecordNew::BackgroundNew(const Closure &task)
{
    HILOG_INFO("Move to backgroundNew.");
    CHECK_POINTER(lifecycleDeal_);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::BACKGROUNDNEW_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }

    if (!IsTerminating() || IsRestarting()) {
        // schedule save ability state before moving to background.
        SaveAbilityState();
    }

    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::BACKGROUNDING_NEW;
    lifecycleDeal_->BackgroundNew(want_, lifeCycleStateInfo_);
}

// new version  --start
bool AbilityRecord::IsStartedByCall() const
{
    return isStartedByCall_;
}

void AbilityRecord::SetStartedByCall(const bool isFlag)
{
    isStartedByCall_ = isFlag;
}

bool AbilityRecord::IsStartToBackground() const
{
    return isStartToBackground_;
}

void AbilityRecord::SetStartToBackground(const bool flag)
{
    isStartToBackground_ = flag;
}

bool AbilityRecord::CallRequest()
{
    HILOG_INFO("Call Request.");
    CHECK_POINTER_RETURN_BOOL(scheduler_);
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    // sync call request
    sptr<IRemoteObject> callStub = scheduler_->CallRequest();
    if (!callStub) {
        HILOG_ERROR("call request failed, callstub is nullptr.");
        return false;
    }

    // complete call request
    return callContainer_->CallRequestDone(callStub);
}

ResolveResultType AbilityRecord::Resolve(const AbilityRequest &abilityRequest)
{
    auto callback = abilityRequest.connect;
    if (abilityRequest.callType != AbilityCallType::CALL_REQUEST_TYPE || !callback) {
        HILOG_ERROR("only startd by call type can create a call record.");
        return ResolveResultType::NG_INNER_ERROR;
    }
    if (!callContainer_) {
        callContainer_ = std::make_shared<CallContainer>();
        if (!callContainer_) {
            HILOG_ERROR("mark_shared error.");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    HILOG_DEBUG("create call record for this resolve. callerUid:%{public}d ,targetname:%{public}s",
        abilityRequest.callerUid,
        abilityRequest.abilityInfo.name.c_str());

    std::shared_ptr<CallRecord> callRecord = callContainer_->GetCallRecord(callback);
    if (!callRecord) {
        callRecord = CallRecord::CreateCallRecord(
            abilityRequest.callerUid, shared_from_this(), callback, abilityRequest.callerToken);
        if (!callRecord) {
            HILOG_ERROR("mark_shared error.");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    callContainer_->AddCallRecord(callback, callRecord);

    if (callRecord->IsCallState(CallState::REQUESTED) && callRecord->GetCallStub()) {
        HILOG_DEBUG("this record has requested.");
        if (!callRecord->SchedulerConnectDone()) {
            HILOG_DEBUG("this callrecord has requested, but callback failed.");
            return ResolveResultType::NG_INNER_ERROR;
        }
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    callRecord->SetCallState(CallState::REQUESTING);
    return ResolveResultType::OK_NO_REMOTE_OBJ;
}

bool AbilityRecord::Release(const sptr<IAbilityConnection> & connect)
{
    HILOG_DEBUG("ability release call record by callback.");
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    return callContainer_->RemoveCallRecord(connect);
}

bool AbilityRecord::IsNeedToCallRequest() const
{
    HILOG_DEBUG("ability release call record by callback.");
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    return callContainer_->IsNeedToCallRequest();
}

void AbilityRecord::ContinueAbility(const std::string& deviceId)
{
    HILOG_INFO("ContinueAbility.");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->ContinueAbility(deviceId);
}

void AbilityRecord::SetSwitchingPause(bool state)
{
    isSwitchingPause_ = state;
}

bool AbilityRecord::IsSwitchingPause()
{
    return isSwitchingPause_;
}

void AbilityRecord::DumpSys(std::vector<std::string> &info, bool isClient)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        next ability main name [" + nextAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_ && IsNewVersion()) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
    DumpClientInfo(info, isClient);
}

void AbilityRecord::DumpClientInfo(std::vector<std::string> &info, bool isClient)
{
    if (isClient && scheduler_ && isReady_) {
        std::vector<std::string> params;
        scheduler_->DumpAbilityInfo(params, info);
        AppExecFwk::Configuration config;
        if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
            info.emplace_back("          configuration: " + config.GetName());
        }
    }
}

void AbilityRecord::GrantUriPermission(const Want &want)
{
    HILOG_DEBUG("AbilityRecord::GrantUriPermission is called.");
    auto flags = want.GetFlags();
    if (flags & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) {
        HILOG_INFO("Want to grant r/w permission of the uri");
        auto targetTokenId = abilityInfo_.applicationInfo.accessTokenId;
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr) {
            abilityMgr->GrantUriPermission(want, GetCurrentAccountId(), targetTokenId);
        }
    }
}

int AbilityRecord::GetCurrentAccountId()
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("QueryActiveOsAccountIds failed.");
        return DEFAULT_USER_ID;
    }

    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("QueryActiveOsAccountIds is empty, no accounts.");
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

void AbilityRecord::SetWindowMode(int32_t windowMode)
{
    want_.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
}

void AbilityRecord::RemoveWindowMode()
{
    want_.RemoveParam(Want::PARAM_RESV_WINDOW_MODE);
}

int AbilityRecord::BlockAbility()
{
    HILOG_INFO("BlockAbility.");
    if (scheduler_) {
        HILOG_INFO("scheduler_ begain to call BlockAbility %{public}s", __func__);
        return scheduler_->BlockAbility();
    }
    return ERR_NO_INIT;
}
}  // namespace AAFwk
}  // namespace OHOS
