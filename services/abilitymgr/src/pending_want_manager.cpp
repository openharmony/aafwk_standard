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

#include "ability_manager_service.h"
#include "ability_util.h"
#include "distributed_client.h"
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

sptr<IWantSender> PendingWantManager::GetWantSender(const int32_t callingUid, const int32_t uid, const bool isSystemApp,
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("PendingWantManager::GetWantSender begin.");

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (callingUid != 0 && callingUid != SYSTEM_UID && !isSystemApp) {
        if (callingUid != uid) {
            HILOG_INFO("is not allowed to send");
            return nullptr;
        }
    }
    WantSenderInfo info = wantSenderInfo;
    return GetWantSenderLocked(callingUid, uid, wantSenderInfo.userId, info, callerToken);
}

sptr<IWantSender> PendingWantManager::GetWantSenderLocked(const int32_t callingUid, const int32_t uid,
    const int32_t userId, WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    bool needCreate = ((uint32_t)wantSenderInfo.flags & (uint32_t)Flags::NO_BUILD_FLAG) != 0;
    bool needCancel = ((uint32_t)wantSenderInfo.flags & (uint32_t)Flags::CANCEL_PRESENT_FLAG) != 0;
    bool needUpdate = ((uint32_t)wantSenderInfo.flags & (uint32_t)Flags::UPDATE_PRESENT_FLAG) != 0;

    wantSenderInfo.flags =
        ((uint32_t)wantSenderInfo.flags & (~((uint32_t)Flags::NO_BUILD_FLAG | (uint32_t)Flags::CANCEL_PRESENT_FLAG |
                                              (uint32_t)Flags::UPDATE_PRESENT_FLAG)));

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
        HILOG_INFO("wantRecords_ size %{public}zu", wantRecords_.size());
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

void PendingWantManager::CancelWantSender(
    const int32_t callingUid, const int32_t uid, const bool isSystemApp, const sptr<IWantSender> &sender)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (sender == nullptr) {
        HILOG_ERROR("%{public}s:sender is nullptr.", __func__);
        return;
    }

    std::lock_guard<std::recursive_mutex> locker(mutex_);
    if (callingUid != 0 && callingUid != SYSTEM_UID && !isSystemApp) {
        if (callingUid != uid) {
            HILOG_INFO("is not allowed to send");
            return;
        }
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
int32_t PendingWantManager::DeviceIdDetermine(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid)
{
    int32_t result = ERR_OK;
    if (want.GetElement().GetDeviceID() == "") {
        result = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(
            want, callerToken, requestCode, callerUid);
        if (result != ERR_OK && result != START_ABILITY_WAITING) {
            HILOG_ERROR("%{public}s:result != ERR_OK && result != START_ABILITY_WAITING.", __func__);
        }
        return result;
    }

    sptr<IRemoteObject> remoteObject =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("failed to get distributed schedule manager service");
        result = ERR_INVALID_VALUE;
        return result;
    }
    DistributedClient dmsClient;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    result = dmsClient.StartRemoteAbility(want, callerUid, requestCode, accessToken);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s: StartRemoteAbility Error! result = %{public}d", __func__, result);
    }

    return result;
}

int32_t PendingWantManager::PendingWantStartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    int32_t result = DeviceIdDetermine(want, callerToken, requestCode, callerUid);
    return result;
}

int32_t PendingWantManager::PendingWantStartAbilitys(const std::vector<WantsInfo> wantsInfo,
    const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    int32_t result = ERR_OK;
    for (const auto &item : wantsInfo) {
        auto res = DeviceIdDetermine(item.want, callerToken, requestCode, callerUid);
        if (res != ERR_OK && res != START_ABILITY_WAITING) {
            result = res;
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

int32_t PendingWantManager::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    if (target == nullptr) {
        HILOG_ERROR("%{public}s:target is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    if (info == nullptr) {
        HILOG_ERROR("%{public}s:info is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> targetRecord = iface_cast<PendingWantRecord>(target->AsObject());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        HILOG_ERROR("%{public}s:record is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.requestCode = record->GetKey()->GetRequestCode();
    wantSenderInfo.type = record->GetKey()->GetType();
    wantSenderInfo.flags = record->GetKey()->GetFlags();
    wantSenderInfo.allWants = record->GetKey()->GetAllWantsInfos();
    info.reset(new (std::nothrow) WantSenderInfo(wantSenderInfo));
    HILOG_ERROR("%{public}s:want is ok.", __func__);
    return NO_ERROR;
}

void PendingWantManager::ClearPendingWantRecord(const std::string &bundleName)
{
    HILOG_INFO("ClearPendingWantRecord, bundleName: %{public}s", bundleName.c_str());
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto task = [bundleName, self = shared_from_this()]() { self->ClearPendingWantRecordTask(bundleName); };
    handler->PostTask(task);
}

void PendingWantManager::ClearPendingWantRecordTask(const std::string &bundleName)
{
    HILOG_INFO("ClearPendingWantRecordTask, bundleName: %{public}s", bundleName.c_str());
    std::lock_guard<std::recursive_mutex> locker(mutex_);
    auto iter = wantRecords_.begin();
    while (iter != wantRecords_.end()) {
        bool hasBundle = false;
        const auto &pendingRecord = iter->second;
        if ((pendingRecord != nullptr)) {
            auto wantInfos = pendingRecord->GetKey()->GetAllWantsInfos();
            for (const auto &wantInfo: wantInfos) {
                if (wantInfo.want.GetBundle() == bundleName) {
                    hasBundle = true;
                    break;
                }
            }
            if (hasBundle) {
                iter = wantRecords_.erase(iter);
                HILOG_INFO("wantRecords_ size %{public}zu", wantRecords_.size());
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
}

void PendingWantManager::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "    PendingWantRecords:";
    info.push_back(dumpInfo);

    for (const auto &item : wantRecords_) {
        const auto &pendingKey = item.first;
        dumpInfo = "        PendWantRecord ID #" + std::to_string(pendingKey->GetCode()) +
            "  type #" + std::to_string(pendingKey->GetType());
        info.push_back(dumpInfo);
        dumpInfo = "        bundle name [" + pendingKey->GetBundleName() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        result who [" + pendingKey->GetRequestWho() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        request code #" + std::to_string(pendingKey->GetRequestCode()) +
            "  flags #" + std::to_string(pendingKey->GetFlags());
        info.push_back(dumpInfo);
        dumpInfo = "        resolved type [" + pendingKey->GetRequestResolvedType() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        Wants:";
        info.push_back(dumpInfo);
        auto Wants = pendingKey->GetAllWantsInfos();
        for (const auto &Want : Wants) {
            dumpInfo = "  	    uri [" + Want.want.GetElement().GetDeviceID() + "//" +
                Want.want.GetElement().GetBundleName() + "/" + Want.want.GetElement().GetAbilityName() + "]";
            info.push_back(dumpInfo);
            dumpInfo = "  	    resolved types [" + Want.resolvedTypes + "]";
            info.push_back(dumpInfo);
        }
    }
}
void PendingWantManager::DumpByRecordId(std::vector<std::string> &info, const std::string &args)
{
    std::string dumpInfo = "    PendingWantRecords:";
    info.push_back(dumpInfo);

    for (const auto &item : wantRecords_) {
        const auto &pendingKey = item.first;
        if (args == std::to_string(pendingKey->GetCode())) {
            dumpInfo = "        PendWantRecord ID #" + std::to_string(pendingKey->GetCode()) +
                "  type #" + std::to_string(pendingKey->GetType());
                info.push_back(dumpInfo);
                dumpInfo = "        bundle name [" + pendingKey->GetBundleName() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "        result who [" + pendingKey->GetRequestWho() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "        request code #" + std::to_string(pendingKey->GetRequestCode()) +
                "  flags #" + std::to_string(pendingKey->GetFlags());
                info.push_back(dumpInfo);
            dumpInfo = "        resolved type [" + pendingKey->GetRequestResolvedType() + "]";
            info.push_back(dumpInfo);
            dumpInfo = "        Wants:";
            info.push_back(dumpInfo);
            auto Wants = pendingKey->GetAllWantsInfos();
            for (const auto& Want : Wants) {
                dumpInfo = "  	    uri [" + Want.want.GetElement().GetDeviceID() + "//" +
                    Want.want.GetElement().GetBundleName() + "/" + Want.want.GetElement().GetAbilityName() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "  	    resolved types [" + Want.resolvedTypes + "]";
                info.push_back(dumpInfo);
            }
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
