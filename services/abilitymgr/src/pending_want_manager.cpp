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

#include "pending_want_manager.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::EventFwk;
using namespace std::chrono;
using namespace std::placeholders;

PendingWantManager::PendingWantManager()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
}

PendingWantManager::~PendingWantManager()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
}

sptr<IWantSender> PendingWantManager::GetWantSender(const int32_t callingUid, const int32_t uid,
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("PendingWantManager::GetWantSender begin.");

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (callingUid != 0 && callingUid != SYSTEM_UID) {
        if (callingUid != uid) {
            HILOG_INFO("is not allowed to send");
            return nullptr;
        }
    }
    WantSenderInfo info;
    info = wantSenderInfo;
    return GetWantSenderLocked(callingUid, uid, wantSenderInfo.userId, info, callerToken);
}

sptr<IWantSender> PendingWantManager::GetWantSenderLocked(const int32_t callingUid, const int32_t uid,
    const int32_t userId, WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    bool needCreate = ((int)wantSenderInfo.flags & (int)Flags::NO_BUILD_FLAG) != 0;
    bool needCancel = ((int)wantSenderInfo.flags & (int)Flags::CANCEL_PRESENT_FLAG) != 0;
    bool needUpdate = ((int)wantSenderInfo.flags & (int)Flags::UPDATE_PRESENT_FLAG) != 0;

    wantSenderInfo.flags &=
        ~((uint32_t)Flags::NO_BUILD_FLAG | (uint32_t)Flags::CANCEL_PRESENT_FLAG | (uint32_t)Flags::UPDATE_PRESENT_FLAG);

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    auto ref = GetPendingWantRecordByKey(pendingKey);
    if (ref != nullptr) {
        if (!needCancel) {
            if (needUpdate && wantSenderInfo.allWants.size() > 0) {
                ref->GetKey()->SetRequestWant(wantSenderInfo.allWants.back().want);
                ref->GetKey()->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
                wantSenderInfo.allWants.back().want = ref->GetKey()->GetRequestWant();
                wantSenderInfo.allWants.back().resolvedTypes = ref->GetKey()->GetRequestResolvedType();
                ref->GetKey()->SetAllWantsInfos(wantSenderInfo.allWants);
                ref->SetCallerUid(callingUid);
            }
            return ref;
        }
        MakeWantSenderCanceledLocked(*ref);
        wantRecords_.erase(ref->GetKey());
        return nullptr;
    }

    if (needCreate) {
        return (ref != nullptr) ? ref : nullptr;
    }

    sptr<PendingWantRecord> rec =
        new (std::nothrow) PendingWantRecord(shared_from_this(), uid, callerToken, pendingKey);
    if (rec != nullptr) {
        rec->SetCallerUid(callingUid);
        pendingKey->SetCode(PendingRecordIdCreate());
        wantRecords_.insert(std::make_pair(pendingKey, rec));
        return rec;
    }
    return nullptr;
}

void PendingWantManager::MakeWantSenderCanceledLocked(PendingWantRecord &record)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    record.SetCanceled();
    for (auto &callback : record.GetCancelCallbacks()) {
        callback->Send(record.GetKey()->GetRequestCode());
    }
}

sptr<PendingWantRecord> PendingWantManager::GetPendingWantRecordByKey(const std::shared_ptr<PendingWantKey> &key)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    for (const auto &item : wantRecords_) {
        const auto &pendingKey = item.first;
        const auto &pendingRecord = item.second;
        if ((pendingRecord != nullptr) && CheckPendingWantRecordByKey(pendingKey, key)) {
            return pendingRecord;
        }
    }
    return nullptr;
}

bool PendingWantManager::CheckPendingWantRecordByKey(
    const std::shared_ptr<PendingWantKey> &inputKey, const std::shared_ptr<PendingWantKey> &key)
{
    if (inputKey->GetBundleName().compare(key->GetBundleName()) != 0) {
        return false;
    }
    if (inputKey->GetType() != key->GetType()) {
        return false;
    }
    if (inputKey->GetRequestWho().compare(key->GetRequestWho()) != 0) {
        return false;
    }
    if (inputKey->GetRequestCode() != key->GetRequestCode()) {
        return false;
    }
    if (!inputKey->GetRequestWant().OperationEquals(key->GetRequestWant())) {
        return false;
    }
    if (inputKey->GetRequestResolvedType().compare(key->GetRequestResolvedType()) != 0) {
        return false;
    }
    if (inputKey->GetFlags() != key->GetFlags()) {
        return false;
    }
    if (inputKey->GetUserId() != key->GetUserId()) {
        return false;
    }
    return true;
}

int32_t PendingWantManager::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:sender is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    SenderInfo info = senderInfo;
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(target->AsObject());
    return record->SenderInner(info);
}

void PendingWantManager::CancelWantSender(const int32_t callingUid, const int32_t uid, const sptr<IWantSender> &sender)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (sender == nullptr) {
        HILOG_ERROR("%{public}s:sender is nullptr.", __func__);
        return;
    }

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (callingUid != uid) {
        HILOG_DEBUG("is not allowed to send");
        return;
    }
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(sender->AsObject());
    CancelWantSenderLocked(*record, true);
}

void PendingWantManager::CancelWantSenderLocked(PendingWantRecord &record, bool cleanAbility)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    MakeWantSenderCanceledLocked(record);
    if (cleanAbility) {
        wantRecords_.erase(record.GetKey());
    }
}

int32_t PendingWantManager::PendingWantStartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t requestCode)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    return DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, callerToken, requestCode);
}

int32_t PendingWantManager::PendingWantStartAbilitys(
    const std::vector<WantsInfo> wantsInfo, const sptr<IRemoteObject> &callerToken, int32_t requestCode)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    int32_t result = ERR_OK;
    for (const auto &item : wantsInfo) {
        const auto &want = item.want;
        result = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, callerToken, requestCode);
        if (result != ERR_OK && result != START_ABILITY_WAITING) {
            HILOG_ERROR("%{public}s:result != ERR_OK && result != START_ABILITY_WAITING.", __func__);
            return result;
        }
    }
    return result;
}

int32_t PendingWantManager::PendingWantPublishCommonEvent(
    const Want &want, const SenderInfo &senderInfo, int32_t callerUid)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    CommonEventData eventData;
    eventData.SetWant(want);
    eventData.SetCode(senderInfo.code);

    CommonEventPublishInfo eventPublishData;
    if (!senderInfo.requiredPermission.empty()) {
        std::vector<std::string> permissions;
        permissions.emplace_back(senderInfo.requiredPermission);
        eventPublishData.SetSubscriberPermissions(permissions);
    }

    std::shared_ptr<PendingWantCommonEvent> pendingWantCommonEvent = nullptr;
    if (senderInfo.finishedReceiver != nullptr) {
        eventPublishData.SetOrdered(true);
        pendingWantCommonEvent = std::make_shared<PendingWantCommonEvent>();
        pendingWantCommonEvent->SetFinishedReceiver(senderInfo.finishedReceiver);
        WantParams wantParams = {};
        pendingWantCommonEvent->SetWantParams(wantParams);
    }
    bool result = DelayedSingleton<EventFwk::CommonEvent>::GetInstance()->PublishCommonEvent(
        eventData, eventPublishData, pendingWantCommonEvent, callerUid);
    return ((result == true) ? ERR_OK : (-1));
}

int32_t PendingWantManager::PendingRecordIdCreate()
{
    HILOG_INFO("%{public}s:begin.", __func__);

    static std::atomic_int id(0);
    return ++id;
}

sptr<PendingWantRecord> PendingWantManager::GetPendingWantRecordByCode(int32_t code)
{
    HILOG_INFO("%{public}s:begin. wantRecords_ size = %{public}zu", __func__, wantRecords_.size());

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    auto iter = std::find_if(wantRecords_.begin(), wantRecords_.end(), [&code](const auto &pair) {
        return pair.second->GetKey()->GetCode() == code;
    });
    return ((iter == wantRecords_.end()) ? nullptr : iter->second);
}

int32_t PendingWantManager::GetPendingWantUid(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetUid()) : (-1));
}

int32_t PendingWantManager::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetUserId()) : (-1));
}

std::string PendingWantManager::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return "";
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record != nullptr) {
        return record->GetKey()->GetBundleName();
    }
    return "";
}

int32_t PendingWantManager::GetPendingWantCode(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetCode()) : (-1));
}

int32_t PendingWantManager::GetPendingWantType(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetType()) : (-1));
}

void PendingWantManager::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if ((sender == nullptr) || (recevier == nullptr)) {
        HILOG_ERROR("%{public}s:sender is nullptr or recevier is nullptr.", __func__);
        return;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(sender->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        HILOG_ERROR("%{public}s:record is nullptr. code = %{public}d", __func__, targetRecord->GetKey()->GetCode());
        return;
    }
    bool cancel = record->GetCanceled();
    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (!cancel) {
        record->RegisterCancelListener(recevier);
    }
}

void PendingWantManager::UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (sender == nullptr || recevier == nullptr) {
        HILOG_ERROR("%{public}s:sender is nullptr or recevier is nullptr.", __func__);
        return;
    }

    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(sender->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        HILOG_ERROR("%{public}s:record is nullptr.", __func__);
        return;
    }
    std::lock_guard<std::recursive_mutex> locker(mutex_);
    record->UnregisterCancelListener(recevier);
}

int32_t PendingWantManager::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    if (want == nullptr) {
        HILOG_ERROR("%{public}s:want is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        HILOG_ERROR("%{public}s:record is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    want.reset(new (std::nothrow) Want(record->GetKey()->GetRequestWant()));
    HILOG_ERROR("%{public}s:want is ok.", __func__);
    return NO_ERROR;
}

}  // namespace AAFwk
}  // namespace OHOS
