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

#include "pending_want_record.h"

#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "pending_want_manager.h"

namespace OHOS {
namespace AAFwk {
PendingWantRecord::PendingWantRecord()
{}

PendingWantRecord::PendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
    const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key)
    : pendingWantManager_(pendingWantManager), uid_(uid), callerToken_(callerToken), key_(key)
{}

PendingWantRecord::~PendingWantRecord()
{}

void PendingWantRecord::Send(SenderInfo &senderInfo)
{
    SenderInner(senderInfo);
}

void PendingWantRecord::RegisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    if (receiver == nullptr) {
        return;
    }
    mCancelCallbacks_.emplace_back(receiver);
}

void PendingWantRecord::UnregisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    if (receiver == nullptr) {
        return;
    }
    if (mCancelCallbacks_.size()) {
        auto it = std::find(mCancelCallbacks_.cbegin(), mCancelCallbacks_.cend(), receiver);
        if (it != mCancelCallbacks_.cend()) {
            mCancelCallbacks_.erase(it);
        }
    }
}

int32_t PendingWantRecord::SenderInner(SenderInfo &senderInfo)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    std::lock_guard<std::recursive_mutex> locker(lock_);
    if (canceled_) {
        return START_CANCELED;
    }

    auto pendingWantManager = pendingWantManager_.lock();
    if (pendingWantManager == nullptr) {
        HILOG_ERROR("%{public}s:pendingWantManager is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    if (((uint32_t)key_->GetFlags() & (uint32_t)Flags::ONE_TIME_FLAG) != 0) {
        pendingWantManager->CancelWantSenderLocked(*this, true);
    }

    Want want;
    if (key_->GetAllWantsInfos().size() != 0) {
        want = key_->GetRequestWant();
    }
    bool immutable = ((uint32_t)key_->GetFlags() & (uint32_t)Flags::CONSTANT_FLAG) != 0;
    senderInfo.resolvedType = key_->GetRequestResolvedType();
    if (!immutable) {
        want.AddFlags(key_->GetFlags());
    }

    bool sendFinish = (senderInfo.finishedReceiver != nullptr);
    int res = NO_ERROR;
    switch (key_->GetType()) {
        case (int32_t)OperationType::START_ABILITY:
            res = pendingWantManager->PendingWantStartAbility(want, callerToken_, -1, callerUid_);
            break;
        case (int32_t)OperationType::START_ABILITIES: {
            std::vector<WantsInfo> allWantsInfos = key_->GetAllWantsInfos();
            allWantsInfos.back().want = want;
            res = pendingWantManager->PendingWantStartAbilitys(allWantsInfos, callerToken_, -1, callerUid_);
            break;
        }
        case (int32_t)OperationType::START_SERVICE:
        case (int32_t)OperationType::START_FOREGROUND_SERVICE:
            res = pendingWantManager->PendingWantStartAbility(want, callerToken_, -1, callerUid_);
            break;
        case (int32_t)OperationType::SEND_COMMON_EVENT:
            res = pendingWantManager->PendingWantPublishCommonEvent(want, senderInfo, callerUid_);
            (res == ERR_OK) ? (sendFinish = false) : (sendFinish = (senderInfo.finishedReceiver != nullptr));
            break;
        default:
            break;
    }

    if (sendFinish && res != START_CANCELED) {
        WantParams wantParams = {};
        senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "", wantParams, false, false, 0);
    }

    return res;
}

std::shared_ptr<PendingWantKey> PendingWantRecord::GetKey()
{
    return key_;
}

int32_t PendingWantRecord::GetUid() const
{
    return uid_;
}

void PendingWantRecord::SetCanceled()
{
    canceled_ = true;
}
bool PendingWantRecord::GetCanceled()
{
    return canceled_;
}

void PendingWantRecord::SetCallerUid(const int32_t callerUid)
{
    callerUid_ = callerUid;
}

std::list<sptr<IWantReceiver>> PendingWantRecord::GetCancelCallbacks()
{
    return mCancelCallbacks_;
}
}  // namespace AAFwk
}  // namespace OHOS