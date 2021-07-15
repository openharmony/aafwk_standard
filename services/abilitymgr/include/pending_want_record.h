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

#ifndef OHOS_AAFWK_PENDING_WANT_RECORD_H
#define OHOS_AAFWK_PENDING_WANT_RECORD_H

#include <list>
#include <mutex>

#include "iremote_object.h"
#include "refbase.h"

#include "ability_record.h"
#include "want_sender_stub.h"
#include "sender_info.h"
#include "pending_want_key.h"

namespace OHOS {
namespace AAFwk {
class PendingWantManager;
class PendingWantRecord : public WantSenderStub {
public:
    static const int START_CANCELED = -96;

    PendingWantRecord();
    PendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
        const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key);
    virtual ~PendingWantRecord();

    virtual void Send(SenderInfo &senderInfo);
    virtual void RegisterCancelListener(const sptr<IWantReceiver> &receiver);
    virtual void UnregisterCancelListener(const sptr<IWantReceiver> &receiver);

    virtual int32_t SenderInner(SenderInfo &senderInfo);

public:
    std::shared_ptr<PendingWantKey> GetKey();
    int32_t GetUid() const;
    void SetCanceled();
    bool GetCanceled();
    std::list<sptr<IWantReceiver>> GetCancelCallbacks();

private:
    std::weak_ptr<PendingWantManager> pendingWantManager_;
    int32_t uid_;
    sptr<IRemoteObject> callerToken_;
    bool canceled_ = false;
    std::shared_ptr<PendingWantKey> key_;
    std::list<sptr<IWantReceiver>> mCancelCallbacks_;
    std::recursive_mutex lock_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_PENDING_WANT_RECORD_H