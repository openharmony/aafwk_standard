/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "call_container.h"

#include "hilog_wrapper.h"
#include "ability_manager_errors.h"
#include "ability_connect_callback_stub.h"
#include "ability_util.h"
#include "ability_event_handler.h"
#include "ability_manager_service.h"

namespace OHOS {
namespace AAFwk {
CallContainer::CallContainer()
{}

CallContainer::~CallContainer()
{
    std::for_each(deathRecipientMap_.begin(),
        deathRecipientMap_.end(),
        [&](RecipientMapType::reference recipient) {
            recipient.first->RemoveDeathRecipient(recipient.second);
        });

    deathRecipientMap_.clear();
    callRecordMap_.clear();
}

void CallContainer::AddCallRecord(const sptr<IAbilityConnection> & connect,
    const std::shared_ptr<CallRecord>& callRecord)
{
    CHECK_POINTER(callRecord);
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    
    auto iter = callRecordMap_.find(connect->AsObject());
    if (iter != callRecordMap_.end()) {
        RemoveConnectDeathRecipient(connect);
        callRecordMap_.erase(callRecordMap_.find(connect->AsObject()));
    }

    AddConnectDeathRecipient(connect);
    callRecord->SetConCallBack(connect);
    callRecordMap_.emplace(connect->AsObject(), callRecord);

    HILOG_DEBUG("Add call record to callcontainer, target: %{public}s",
        callRecord->GetTargetServiceName().GetURI().c_str());
}

std::shared_ptr<CallRecord> CallContainer::GetCallRecord(const sptr<IAbilityConnection> & connect) const
{
    CHECK_POINTER_AND_RETURN(connect, nullptr);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), nullptr);

    auto mapIter = callRecordMap_.find(connect->AsObject());
    if (mapIter != callRecordMap_.end()) {
        return mapIter->second;
    }

    return nullptr;
}

bool CallContainer::RemoveCallRecord(const sptr<IAbilityConnection> & connect)
{
    HILOG_DEBUG("call container release call record by callback.");
    CHECK_POINTER_AND_RETURN(connect, nullptr);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), nullptr);

    auto iter = callRecordMap_.find(connect->AsObject());
    if (iter != callRecordMap_.end()) {
        auto callrecord = iter->second;
        if (callrecord) {
            callrecord->SchedulerDisConnectDone();
        }
        RemoveConnectDeathRecipient(connect);
        callRecordMap_.erase(callRecordMap_.find(connect->AsObject()));
        HILOG_DEBUG("remove call record is success.");
        return true;
    }

    if (callRecordMap_.empty()) {
        // notify soft resouce service.
        HILOG_DEBUG("this ability has no callrecord.");
    }

    HILOG_WARN("remove call record is not exist.");
    return false;
}

void CallContainer::OnConnectionDied(const wptr<IRemoteObject> & remote)
{
    HILOG_WARN("Call back is died.");
    auto object = remote.promote();
    CHECK_POINTER(object);

    std::shared_ptr<CallRecord> callRecord = nullptr;
    auto mapIter = callRecordMap_.find(object);
    if (mapIter != callRecordMap_.end()) {
        callRecord = mapIter->second;
    }

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto task = [abilityManagerService, callRecord]() {
        abilityManagerService->OnCallConnectDied(callRecord);
    };
    handler->PostTask(task);
}

bool CallContainer::CallRequestDone(const sptr<IRemoteObject> & callStub)
{
    HILOG_INFO("Call Request Done start.");

    CHECK_POINTER_AND_RETURN(callStub, false);

    std::for_each(callRecordMap_.begin(),
        callRecordMap_.end(),
        [&callStub](CallMapType::reference service) {
            std::shared_ptr<CallRecord> callRecord = service.second;
            if (callRecord && callRecord->IsCallState(CallState::REQUESTING)) {
                callRecord->SetCallStub(callStub);
                callRecord->SchedulerConnectDone();
            }
        });

    HILOG_INFO("Call Request Done end.");
    return true;
}

void CallContainer::Dump(std::vector<std::string> &info) const
{
    HILOG_INFO("Dump call records.");
    for (auto &iter : callRecordMap_) {
        auto callRecord = iter.second;
        if (callRecord) {
            callRecord->Dump(info);
        }
    }
}

bool CallContainer::IsNeedToCallRequest() const
{
    for (auto &iter : callRecordMap_) {
        auto callRecord = iter.second;
        if (callRecord && !callRecord->IsCallState(CallState::REQUESTED)) {
            return true;
        }
    }
    return false;
}

void CallContainer::AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        HILOG_ERROR("This death recipient has been added.");
        return;
    } else {
        sptr<IRemoteObject::DeathRecipient> deathRecipient = new AbilityConnectCallbackRecipient(
            std::bind(&CallContainer::OnConnectionDied, this, std::placeholders::_1));
        connect->AsObject()->AddDeathRecipient(deathRecipient);
        deathRecipientMap_.emplace(connect->AsObject(), deathRecipient);
    }
}

void CallContainer::RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        deathRecipientMap_.erase(it);
        return;
    }
}
}  // namespace AAFwk
}  // namesspace OHOS
