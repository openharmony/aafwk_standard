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

#include "ability_connect_manager.h"

#include <algorithm>

#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "bytrace.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

AbilityConnectManager::AbilityConnectManager()
{}

AbilityConnectManager::~AbilityConnectManager()
{}

int AbilityConnectManager::StartAbility(const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    return StartAbilityLocked(abilityRequest);
}

int AbilityConnectManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    return TerminateAbilityLocked(token);
}

int AbilityConnectManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    HILOG_INFO("Terminate ability.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);

    std::shared_ptr<AbilityRecord> targetAbility = nullptr;
    int result = static_cast<int>(ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    std::for_each(serviceMap_.begin(),
        serviceMap_.end(),
        [&targetAbility, &caller, requestCode, &result](ServiceMapType::reference service) {
            auto callerList = service.second->GetCallerRecordList();
            for (auto &it : callerList) {
                if (it->GetCaller() == caller && it->GetRequestCode() == requestCode) {
                    targetAbility = service.second;
                    if (targetAbility) {
                        result = AbilityUtil::JudgeAbilityVisibleControl(targetAbility->GetAbilityInfo());
                    }
                    break;
                }
            }
        });

    if (!targetAbility) {
        return NO_FOUND_ABILITY_BY_CALLER;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    return TerminateAbilityLocked(targetAbility->GetToken());
}

int AbilityConnectManager::StopServiceAbility(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("Stop Service ability.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    return StopServiceAbilityLocked(abilityRequest);
}

int AbilityConnectManager::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    HILOG_INFO("Terminate ability result.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    return TerminateAbilityResultLocked(token, startId);
}

int AbilityConnectManager::StartAbilityLocked(const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Start ability locked, ability_name: %{public}s", abilityRequest.want.GetElement().GetURI().c_str());

    std::shared_ptr<AbilityRecord> targetService;
    bool isLoadedAbility = false;
    GetOrCreateServiceRecord(abilityRequest, false, targetService, isLoadedAbility);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);

    targetService->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);

    if (!isLoadedAbility) {
        LoadAbility(targetService);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        // It may have been started through connect
        CommandAbility(targetService);
    } else {
        HILOG_ERROR("Target service is already activing.");
        return START_SERVICE_ABILITY_ACTIVING;
    }

    sptr<Token> token = targetService->GetToken();
    sptr<Token> preToken = nullptr;
    if (targetService->GetPreAbilityRecord()) {
        preToken = targetService->GetPreAbilityRecord()->GetToken();
    }
    DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token, preToken, 0, 1, 1);
    return ERR_OK;
}

int AbilityConnectManager::TerminateAbilityLocked(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Terminate ability locked.");
    auto abilityRecord = GetServiceRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (abilityRecord->IsTerminating()) {
        HILOG_INFO("Ability is on terminating.");
        return ERR_OK;
    }

    if (!abilityRecord->GetConnectRecordList().empty()) {
        HILOG_INFO("Target service has been connected. Post disconnect task.");
        auto connectRecordList = abilityRecord->GetConnectRecordList();
        auto task = [connectRecordList, connectManager = shared_from_this()]() {
            connectManager->HandleDisconnectTask(connectRecordList);
        };
        eventHandler_->PostTask(task);
    }

    auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
        HILOG_WARN("Disconnect ability terminate timeout.");
        connectManager->HandleStopTimeoutTask(abilityRecord);
    };
    abilityRecord->Terminate(timeoutTask);

    return ERR_OK;
}

int AbilityConnectManager::TerminateAbilityResultLocked(const sptr<IRemoteObject> &token, int startId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Terminate ability result locked, startId: %{public}d", startId);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (abilityRecord->GetStartId() != startId) {
        HILOG_ERROR("Start id not equal.");
        return TERMINATE_ABILITY_RESULT_FAILED;
    }

    return TerminateAbilityLocked(token);
}

int AbilityConnectManager::StopServiceAbilityLocked(const AbilityRequest &abilityRequest)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Stop service ability locked.");
    AppExecFwk::ElementName element(
        abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    auto abilityRecord = GetServiceRecordByElementName(element.GetURI());
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (abilityRecord->IsTerminating()) {
        HILOG_INFO("Ability is on terminating.");
        return ERR_OK;
    }

    if (!abilityRecord->GetConnectRecordList().empty()) {
        HILOG_INFO("Target service has been connected. Post disconnect task.");
        auto connectRecordList = abilityRecord->GetConnectRecordList();
        auto task = [connectRecordList, connectManager = shared_from_this()]() {
            connectManager->HandleDisconnectTask(connectRecordList);
        };
        eventHandler_->PostTask(task);
    }

    auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
        HILOG_WARN("Disconnect ability terminate timeout.");
        connectManager->HandleStopTimeoutTask(abilityRecord);
    };
    abilityRecord->Terminate(timeoutTask);

    return ERR_OK;
}

void AbilityConnectManager::GetOrCreateServiceRecord(const AbilityRequest &abilityRequest,
    const bool isCreatedByConnect, std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ElementName element(
        abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    auto serviceMapIter = serviceMap_.find(element.GetURI());
    if (serviceMapIter == serviceMap_.end()) {
        targetService = AbilityRecord::CreateAbilityRecord(abilityRequest);
        if (isCreatedByConnect && targetService != nullptr) {
            targetService->SetCreateByConnectMode();
        }
        serviceMap_.emplace(element.GetURI(), targetService);
        isLoadedAbility = false;
    } else {
        targetService = serviceMapIter->second;
        if (targetService != nullptr) {
            // want may be changed for the same ability.
            targetService->SetWant(abilityRequest.want);
        }
        isLoadedAbility = true;
    }
}

void AbilityConnectManager::GetConnectRecordListFromMap(
    const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
    auto connectMapIter = connectMap_.find(connect->AsObject());
    if (connectMapIter != connectMap_.end()) {
        connectRecordList = connectMapIter->second;
    }
}

int AbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s, ability_name:%{public}s", __func__, abilityRequest.want.GetElement().GetURI().c_str());
    std::lock_guard<std::recursive_mutex> guard(Lock_);

    // 1. get target service ability record, and check whether it has been loaded.
    std::shared_ptr<AbilityRecord> targetService;
    bool isLoadedAbility = false;
    GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    // 2. get target connectRecordList, and check whether this callback has been connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    bool isCallbackConnected = !connectRecordList.empty();
    // 3. If this service ability and callback has been connected, There is no need to connect repeatedly
    if (isLoadedAbility && (isCallbackConnected) && IsAbilityConnected(targetService, connectRecordList)) {
        HILOG_ERROR("Service and callback was connected.");
        return ERR_OK;
    }

    // 4. Other cases , need to connect the service ability
    auto connectRecord = ConnectionRecord::CreateConnectionRecord(callerToken, targetService, connect);
    CHECK_POINTER_AND_RETURN(connectRecord, ERR_INVALID_VALUE);
    connectRecord->SetConnectState(ConnectionState::CONNECTING);
    targetService->AddConnectRecordToList(connectRecord);
    connectRecordList.push_back(connectRecord);
    if (isCallbackConnected) {
        RemoveConnectDeathRecipient(connect);
        connectMap_.erase(connectMap_.find(connect->AsObject()));
    }
    AddConnectDeathRecipient(connect);
    connectMap_.emplace(connect->AsObject(), connectRecordList);

    // 5. load or connect ability
    if (!isLoadedAbility) {
        LoadAbility(targetService);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        // this service ability has not first connect
        if (targetService->GetConnectRecordList().size() > 1) {
            if (eventHandler_ != nullptr) {
                auto task = [connectRecord]() { connectRecord->CompleteConnect(ERR_OK); };
                eventHandler_->PostTask(task);
            }
        } else {
            ConnectAbility(targetService);
        }
    } else {
        HILOG_ERROR("Target service is already activing.");
    }

    auto token = targetService->GetToken();
    auto preToken = iface_cast<Token>(connectRecord->GetToken());
    DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token, preToken, 0, 1, 1);
    return ERR_OK;
}

int AbilityConnectManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Disconnect ability locked.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);

    // 1. check whether callback was connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    if (connectRecordList.empty()) {
        HILOG_ERROR("Can't find the connect list from connect map by callback.");
        return CONNECTION_NOT_EXIST;
    }

    // 2. schedule disconnect to target service
    for (auto &connectRecord : connectRecordList) {
        if (connectRecord) {
            auto abilityRecord = connectRecord->GetAbilityRecord();
            CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
            int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
            if (result != ERR_OK) {
                HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
                return result;
            }
            int ret = connectRecord->DisconnectAbility();
            if (ret != ERR_OK) {
                HILOG_ERROR("Disconnect ability fail , ret = %{public}d.", ret);
                return ret;
            }
            HILOG_INFO("Disconnect ability ,connect record id %{public}d", connectRecord->GetRecordId());
        }
    }

    // 3. target servie has another connection, this record callback disconnected directly.
    if (eventHandler_ != nullptr) {
        auto task = [connectRecordList, connectManager = shared_from_this()]() {
            connectManager->HandleDisconnectTask(connectRecordList);
        };
        eventHandler_->PostTask(task);
    }

    return ERR_OK;
}

int AbilityConnectManager::AttachAbilityThreadLocked(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto abilityRecord = GetServiceRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (eventHandler_ != nullptr) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("LoadTimeout_") + std::to_string(recordId);
        eventHandler_->RemoveTask(taskName);
        eventHandler_->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());
    }
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("Ability: %{public}s", element.c_str());
    abilityRecord->SetScheduler(scheduler);

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token);
    return ERR_OK;
}

void AbilityConnectManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto abilitState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    auto abilityRecord = GetServiceRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("Ability: %{public}s", element.c_str());

    if (abilitState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        abilityRecord->Inactivate();
    } else if (abilitState == AppAbilityState::ABILITY_STATE_BACKGROUND) {
        DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token);
        RemoveServiceAbility(abilityRecord);
    }
}

void AbilityConnectManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    std::for_each(serviceMap_.begin(), serviceMap_.end(), [&info](ServiceMapType::reference service) {
        if (service.second && (info.processName == service.second->GetAbilityInfo().process ||
                                  info.processName == service.second->GetApplicationInfo().bundleName)) {
            auto appName = service.second->GetApplicationInfo().name;
            auto uid = service.second->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                service.second->SetAppState(info.state);
            }
        }
    });
}

int AbilityConnectManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto abilityRecord = GetServiceRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    HILOG_INFO("Ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    switch (state) {
        case AbilityState::INACTIVE: {
            DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
                token, AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE);
            return DispatchInactive(abilityRecord, state);
        }
        case AbilityState::INITIAL: {
            DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
                token, AppExecFwk::ExtensionState::EXTENSION_STATE_TERMINATED);
            return DispatchTerminate(abilityRecord);
        }
        default: {
            HILOG_WARN("Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int AbilityConnectManager::ScheduleConnectAbilityDoneLocked(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());

    if ((!abilityRecord->IsAbilityState(AbilityState::INACTIVE)) &&
        (!abilityRecord->IsAbilityState(AbilityState::ACTIVE))) {
        HILOG_ERROR("Ability record state is not inactive ,state：%{public}d", abilityRecord->GetAbilityState());
        return INVALID_CONNECTION_STATE;
    }

    DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
        token, AppExecFwk::ExtensionState::EXTENSION_STATE_CONNECTED);

    abilityRecord->SetConnRemoteObject(remoteObject);
    // There may be multiple callers waiting for the connection result
    auto connectRecordList = abilityRecord->GetConnectRecordList();
    for (auto &connectRecord : connectRecordList) {
        connectRecord->ScheduleConnectAbilityDone();
    }

    return ERR_OK;
}

int AbilityConnectManager::ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto abilityRecord = GetServiceRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, CONNECTION_NOT_EXIST);

    auto connect = abilityRecord->GetDisconnectingRecord();
    CHECK_POINTER_AND_RETURN(connect, CONNECTION_NOT_EXIST);

    if (!abilityRecord->IsAbilityState(AbilityState::ACTIVE)) {
        HILOG_ERROR("The service ability state is not active ,state：%{public}d", abilityRecord->GetAbilityState());
        return INVALID_CONNECTION_STATE;
    }

    DelayedSingleton<AppScheduler>::GetInstance()->UpdateExtensionState(
        token, AppExecFwk::ExtensionState::EXTENSION_STATE_DISCONNECTED);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("disconnect ability done with service %{public}s", element.c_str());

    // complete disconnect and remove record from conn map
    connect->ScheduleDisconnectAbilityDone();
    abilityRecord->RemoveConnectRecordFromList(connect);
    if (abilityRecord->IsConnectListEmpty() && abilityRecord->GetStartId() == 0) {
        HILOG_INFO("Service ability has no any connection, and not started , need terminate.");
        auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
            HILOG_WARN("Disconnect ability terminate timeout.");
            connectManager->HandleStopTimeoutTask(abilityRecord);
        };
        abilityRecord->Terminate(timeoutTask);
    }
    RemoveConnectionRecordFromMap(connect);

    return ERR_OK;
}

int AbilityConnectManager::ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("Ability: %{public}s", element.c_str());

    if ((!abilityRecord->IsAbilityState(AbilityState::INACTIVE)) &&
        (!abilityRecord->IsAbilityState(AbilityState::ACTIVE))) {
        HILOG_ERROR("Ability record state is not inactive ,state：%{public}d", abilityRecord->GetAbilityState());
        return INVALID_CONNECTION_STATE;
    }
    // complete command and pop waiting start ability from queue.
    CompleteCommandAbility(abilityRecord);

    return ERR_OK;
}

void AbilityConnectManager::CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);

    if (eventHandler_) {
        int recordId = abilityRecord->GetRecordId();
        std::string taskName = std::string("CommandTimeout_") + std::to_string(recordId) + std::string("_") +
                               std::to_string(abilityRecord->GetStartId());
        eventHandler_->RemoveTask(taskName);
    }

    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetServiceRecordByElementName(const std::string &element)
{
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto mapIter = serviceMap_.find(element);
    if (mapIter != serviceMap_.end()) {
        return mapIter->second;
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> AbilityConnectManager::GetServiceRecordByToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto IsMatch = [token](auto service) {
        sptr<IRemoteObject> srcToken = service.second->GetToken();
        return srcToken == token;
    };
    auto serviceRecord = std::find_if(serviceMap_.begin(), serviceMap_.end(), IsMatch);
    if (serviceRecord != serviceMap_.end()) {
        return serviceRecord->second;
    }
    return nullptr;
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityConnectManager::GetConnectRecordListByCallback(
    sptr<IAbilityConnection> callback)
{
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    std::list<std::shared_ptr<ConnectionRecord>> connectList;
    auto connectMapIter = connectMap_.find(callback->AsObject());
    if (connectMapIter != connectMap_.end()) {
        connectList = connectMapIter->second;
    }
    return connectList;
}

void AbilityConnectManager::RemoveAll()
{
    serviceMap_.clear();
    connectMap_.clear();
}

void AbilityConnectManager::LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);

    PostTimeOutTask(abilityRecord, AbilityManagerService::LOAD_TIMEOUT_MSG);

    sptr<Token> token = abilityRecord->GetToken();
    sptr<Token> perToken = nullptr;
    if (abilityRecord->IsCreateByConnect()) {
        perToken = iface_cast<Token>(abilityRecord->GetConnectingRecord()->GetToken());
    } else {
        auto callerList = abilityRecord->GetCallerRecordList();
        if (!callerList.empty()) {
            perToken = callerList.back()->GetCaller()->GetToken();
        }
    }
    DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        token, perToken, abilityRecord->GetAbilityInfo(), abilityRecord->GetApplicationInfo());

    abilityRecord->SetStartTime();
}

void AbilityConnectManager::PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(eventHandler_);
    if (messageId != AbilityConnectManager::LOAD_TIMEOUT_MSG &&
        messageId != AbilityConnectManager::CONNECT_TIMEOUT_MSG) {
        HILOG_ERROR("Timeout task messageId is error.");
        return;
    }

    int recordId;
    std::string taskName;
    int resultCode;
    uint32_t delayTime;
    if (messageId == AbilityManagerService::LOAD_TIMEOUT_MSG) {
        // first load ability, There is at most one connect record.
        recordId = abilityRecord->GetRecordId();
        taskName = std::string("LoadTimeout_") + std::to_string(recordId);
        resultCode = LOAD_ABILITY_TIMEOUT;
        delayTime = AbilityManagerService::LOAD_TIMEOUT;
    } else {
        auto connectRecord = abilityRecord->GetConnectingRecord();
        CHECK_POINTER(connectRecord);
        recordId = connectRecord->GetRecordId();
        taskName = std::string("ConnectTimeout_") + std::to_string(recordId);
        resultCode = CONNECTION_TIMEOUT;
        delayTime = AbilityManagerService::CONNECT_TIMEOUT;
    }

    auto timeoutTask = [abilityRecord, connectManager = shared_from_this(), resultCode]() {
        HILOG_WARN("Connect or load ability timeout.");
        connectManager->HandleStartTimeoutTask(abilityRecord, resultCode);
    };

    eventHandler_->PostTask(timeoutTask, taskName, delayTime);
}

void AbilityConnectManager::HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int resultCode)
{
    HILOG_DEBUG("Complete connect or load ability timeout.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER(abilityRecord);
    auto connectingList = abilityRecord->GetConnectingRecordList();
    for (auto &connectRecord : connectingList) {
        if (connectRecord == nullptr) {
            HILOG_WARN("ConnectRecord is nullptr.");
            continue;
        }
        connectRecord->CompleteConnect(resultCode);
        abilityRecord->RemoveConnectRecordFromList(connectRecord);
        RemoveConnectionRecordFromMap(connectRecord);
    }
    if (resultCode == LOAD_ABILITY_TIMEOUT) {
        HILOG_DEBUG("Load time out , remove target service record from services map.");
        RemoveServiceAbility(abilityRecord);
    }
}

void AbilityConnectManager::HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("Complete stop ability timeout start.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER(abilityRecord);
    TerminateDone(abilityRecord);
}

void AbilityConnectManager::HandleDisconnectTask(const ConnectListType &connectlist)
{
    HILOG_DEBUG("Complete disconnect ability.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    for (auto &connectRecord : connectlist) {
        if (!connectRecord) {
            continue;
        }
        auto targetService = connectRecord->GetAbilityRecord();
        if (targetService && connectRecord->GetConnectState() == ConnectionState::DISCONNECTED &&
            targetService->GetConnectRecordList().size() > 1) {
            HILOG_WARN("This record complete disconnect directly. recordId:%{public}d", connectRecord->GetRecordId());
            connectRecord->CompleteDisconnect(ERR_OK, false);
            targetService->RemoveConnectRecordFromList(connectRecord);
            RemoveConnectionRecordFromMap(connectRecord);
        };
    }
}

int AbilityConnectManager::DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    CHECK_POINTER_AND_RETURN(eventHandler_, ERR_INVALID_VALUE);
    if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
        HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
            AbilityState::INACTIVATING,
            abilityRecord->GetAbilityState(),
            state);
        return ERR_INVALID_VALUE;
    }
    eventHandler_->RemoveEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecord->GetEventId());

    // complete inactive
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    if (abilityRecord->IsCreateByConnect()) {
        ConnectAbility(abilityRecord);
    } else {
        CommandAbility(abilityRecord);
    }

    return ERR_OK;
}

int AbilityConnectManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // remove terminate timeout task
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(std::to_string(abilityRecord->GetEventId()));
    }
    // complete terminate
    TerminateDone(abilityRecord);
    return ERR_OK;
}

void AbilityConnectManager::ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    PostTimeOutTask(abilityRecord, AbilityConnectManager::CONNECT_TIMEOUT_MSG);
    abilityRecord->ConnectAbility();
}

void AbilityConnectManager::CommandAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (eventHandler_ != nullptr) {
        // first connect ability, There is at most one connect record.
        int recordId = abilityRecord->GetRecordId();
        abilityRecord->AddStartId();
        std::string taskName = std::string("CommandTimeout_") + std::to_string(recordId) + std::string("_") +
                               std::to_string(abilityRecord->GetStartId());
        auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
            HILOG_ERROR("Command ability timeout. %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
        };
        eventHandler_->PostTask(timeoutTask, taskName, AbilityManagerService::COMMAND_TIMEOUT);
        // scheduling command ability
        abilityRecord->CommandAbility();
    }
}

void AbilityConnectManager::TerminateDone(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!abilityRecord->IsAbilityState(AbilityState::TERMINATING)) {
        std::string expect = AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING);
        std::string actual = AbilityRecord::ConvertAbilityState(abilityRecord->GetAbilityState());
        HILOG_ERROR(
            "Transition life state error. expect %{public}s, actual %{public}s", expect.c_str(), actual.c_str());
        return;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
}

bool AbilityConnectManager::IsAbilityConnected(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
    auto isMatch = [abilityRecord](auto connectRecord) -> bool {
        if (abilityRecord == nullptr || connectRecord == nullptr) {
            return false;
        }
        if (abilityRecord != connectRecord->GetAbilityRecord()) {
            return false;
        }
        return true;
    };
    return std::any_of(connectRecordList.begin(), connectRecordList.end(), isMatch);
}

void AbilityConnectManager::RemoveConnectionRecordFromMap(const std::shared_ptr<ConnectionRecord> &connection)
{
    for (auto &connectCallback : connectMap_) {
        auto &connectList = connectCallback.second;
        auto connectRecord = std::find(connectList.begin(), connectList.end(), connection);
        if (connectRecord != connectList.end()) {
            HILOG_INFO("Remove connrecord(%{public}d) from maplist.", (*connectRecord)->GetRecordId());
            connectList.remove(connection);
            if (connectList.empty()) {
                HILOG_INFO("Remove connlist from map.");
                sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(connectCallback.first);
                RemoveConnectDeathRecipient(connect);
                connectMap_.erase(connectCallback.first);
            }
            return;
        }
    }
}

void AbilityConnectManager::RemoveServiceAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    const AppExecFwk::AbilityInfo &abilityInfo = abilityRecord->GetAbilityInfo();
    std::string element = abilityInfo.deviceId + "/" + abilityInfo.bundleName + "/" + abilityInfo.name;
    HILOG_INFO("Remove service(%{public}s) from map.", element.c_str());
    auto it = serviceMap_.find(element);
    if (it != serviceMap_.end()) {
        HILOG_INFO("Remove service(%{public}s) from map.", element.c_str());
        serviceMap_.erase(it);
    }
}

void AbilityConnectManager::AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = recipientMap_.find(connect->AsObject());
    if (it != recipientMap_.end()) {
        HILOG_ERROR("This death recipient has been added.");
        return;
    } else {
        sptr<IRemoteObject::DeathRecipient> deathRecipient = new AbilityConnectCallbackRecipient(
            std::bind(&AbilityConnectManager::OnCallBackDied, this, std::placeholders::_1));
        connect->AsObject()->AddDeathRecipient(deathRecipient);
        recipientMap_.emplace(connect->AsObject(), deathRecipient);
    }
}

void AbilityConnectManager::RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = recipientMap_.find(connect->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
        return;
    }
}

void AbilityConnectManager::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    CHECK_POINTER(object);
    if (eventHandler_) {
        auto task = [object, connectManager = shared_from_this()]() { connectManager->HandleCallBackDiedTask(object); };
        eventHandler_->PostTask(task, TASK_ON_CALLBACK_DIED);
    }
}

void AbilityConnectManager::HandleCallBackDiedTask(const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("Handle call back died task.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER(connect);
    auto it = connectMap_.find(connect);
    if (it != connectMap_.end()) {
        ConnectListType connectRecordList = it->second;
        for (auto &connRecord : connectRecordList) {
            connRecord->ClearConnCallBack();
        }
    } else {
        HILOG_INFO("Died object can't find from conn map.");
        return;
    }
    sptr<IAbilityConnection> object = iface_cast<IAbilityConnection>(connect);
    DisconnectAbilityLocked(object);
}

void AbilityConnectManager::OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_INFO("On ability died.");
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityInfo().type != AbilityType::SERVICE &&
        abilityRecord->GetAbilityInfo().type != AbilityType::EXTENSION) {
        HILOG_DEBUG("Ability type is not service.");
        return;
    }
    if (eventHandler_) {
        auto task = [abilityRecord, connectManager = shared_from_this()]() {
            connectManager->HandleAbilityDiedTask(abilityRecord);
        };
        eventHandler_->PostTask(task, TASK_ON_ABILITY_DIED);
    }
}

void AbilityConnectManager::HandleAbilityDiedTask(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_INFO("Handle ability died task.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    CHECK_POINTER(abilityRecord);
    if (!GetServiceRecordByToken(abilityRecord->GetToken())) {
        HILOG_ERROR("Died ability record is not exist in service map.");
        return;
    }

    if (abilityRecord->GetAbilityInfo().name == AbilityConfig::PHONE_SERVICE_ABILITY_NAME) {
        AbilityRequest requestInfo;
        requestInfo.want = abilityRecord->GetWant();
        requestInfo.abilityInfo = abilityRecord->GetAbilityInfo();
        requestInfo.appInfo = abilityRecord->GetApplicationInfo();

        RemoveServiceAbility(abilityRecord);
        StartAbilityLocked(requestInfo);
        return;
    }

    if (abilityRecord->GetAbilityInfo().name == AbilityConfig::CONTACTS_ABILITY_NAME) {
        AbilityRequest requestInfo;
        requestInfo.want = abilityRecord->GetWant();
        requestInfo.abilityInfo = abilityRecord->GetAbilityInfo();
        requestInfo.appInfo = abilityRecord->GetApplicationInfo();

        RemoveServiceAbility(abilityRecord);
        StartAbilityLocked(requestInfo);
        return;
    }

    if (abilityRecord->GetAbilityInfo().name == AbilityConfig::MMS_ABILITY_NAME) {
        AbilityRequest requestInfo;
        requestInfo.want = abilityRecord->GetWant();
        requestInfo.abilityInfo = abilityRecord->GetAbilityInfo();
        requestInfo.appInfo = abilityRecord->GetApplicationInfo();

        RemoveServiceAbility(abilityRecord);
        StartAbilityLocked(requestInfo);
        return;
    }

    ConnectListType connlist = abilityRecord->GetConnectRecordList();
    for (auto &connectRecord : connlist) {
        HILOG_WARN("This record complete disconnect directly. recordId:%{public}d", connectRecord->GetRecordId());
        connectRecord->CompleteDisconnect(ERR_OK, true);
        abilityRecord->RemoveConnectRecordFromList(connectRecord);
        RemoveConnectionRecordFromMap(connectRecord);
    }

    RemoveServiceAbility(abilityRecord);
}

void AbilityConnectManager::DumpState(std::vector<std::string> &info, bool isClient, const std::string &args) const
{
    if (!args.empty()) {
        auto it = std::find_if(serviceMap_.begin(), serviceMap_.end(), [&args](const auto &service) {
            return service.first.compare(args) == 0;
        });
        if (it != serviceMap_.end()) {
            info.emplace_back("uri [ " + it->first + " ]");
            it->second->DumpService(info, isClient);
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    } else {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr && abilityMgr->IsUseNewMission()) {
            info.emplace_back("  ExtensionRecords:");
        } else {
            info.emplace_back("  serviceAbilityRecords:");
        }
        for (auto &&service : serviceMap_) {
            info.emplace_back("    uri [" + service.first + "]");
            service.second->DumpService(info, isClient);
        }
    }
}

void AbilityConnectManager::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
    const int32_t userId)
{
    HILOG_INFO("Get extension running info.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);
    auto queryInfo = [&info, upperLimit, userId](ServiceMapType::reference service) {
        if (static_cast<int>(info.size()) >= upperLimit) {
            return;
        }
        auto abilityRecord = service.second;
        CHECK_POINTER(abilityRecord);
        ExtensionRunningInfo extensionInfo;
        AppExecFwk::RunningProcessInfo processInfo;
        extensionInfo.extension = abilityRecord->GetWant().GetElement();
        auto bms = AbilityUtil::GetBundleManager();
        CHECK_POINTER(bms);
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        bool queryResult = bms->QueryExtensionAbilityInfos(abilityRecord->GetWant(),
            AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos);
        if (queryResult) {
            HILOG_INFO("Query Extension Ability Infos Success.");
            auto abilityInfo = abilityRecord->GetAbilityInfo();
            auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo &extensionInfo) {
                HILOG_INFO("%{public}s, %{public}s", extensionInfo.bundleName.c_str(), extensionInfo.name.c_str());
                return extensionInfo.bundleName == abilityInfo.bundleName && extensionInfo.name == abilityInfo.name
                    && extensionInfo.applicationInfo.uid == abilityInfo.applicationInfo.uid;
            };
            auto infoIter = std::find_if(extensionInfos.begin(), extensionInfos.end(), isExist);
            if (infoIter != extensionInfos.end()) {
                HILOG_INFO("Get target success.");
                extensionInfo.type = (*infoIter).type;
            }
        }
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
        extensionInfo.pid = processInfo.pid_;
        extensionInfo.uid = processInfo.uid_;
        extensionInfo.processName = processInfo.processName_;
        extensionInfo.startTime = abilityRecord->GetStartTime();
        ConnectListType connectRecordList = abilityRecord->GetConnectRecordList();
        for (auto &connectRecord : connectRecordList) {
            CHECK_POINTER(connectRecord);
            auto callerAbilityRecord = Token::GetAbilityRecordByToken(connectRecord->GetToken());
            CHECK_POINTER(callerAbilityRecord);
            std::string package = callerAbilityRecord->GetAbilityInfo().bundleName;
            extensionInfo.clientPackage.emplace_back(package);
        }
        info.emplace_back(extensionInfo);
        // extension type
    };
    std::for_each(serviceMap_.begin(), serviceMap_.end(), queryInfo);
}

void AbilityConnectManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    HILOG_INFO("Query running ability infos.");
    std::lock_guard<std::recursive_mutex> guard(Lock_);

    auto queryInfo = [&info](ServiceMapType::reference service) {
        auto abilityRecord = service.second;
        CHECK_POINTER(abilityRecord);
        AbilityRunningInfo runningInfo;
        AppExecFwk::RunningProcessInfo processInfo;
        runningInfo.ability = abilityRecord->GetWant().GetElement();
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
        runningInfo.pid = processInfo.pid_;
        runningInfo.uid = processInfo.uid_;
        runningInfo.processName = processInfo.processName_;
        runningInfo.startTime = abilityRecord->GetStartTime();
        runningInfo.abilityState = static_cast<int>(abilityRecord->GetAbilityState());
        info.emplace_back(runningInfo);
    };

    std::for_each(serviceMap_.begin(), serviceMap_.end(), queryInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
