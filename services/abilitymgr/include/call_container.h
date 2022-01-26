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

#ifndef OHOS_AAFWK_CALL_CONTAINER_H
#define OHOS_AAFWK_CALL_CONTAINER_H

#include <map>
#include <string>
#include <mutex>

#include "ability_connect_callback_interface.h"
#include "call_record.h"
#include "element_name.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class CallRecord;
/**
 * @class CallContainer
 * CallContainer provides a facility for managing the call records of ability.
 */
class CallContainer : public std::enable_shared_from_this<CallContainer> {
public:
    using CallMapType = std::map<sptr<IRemoteObject>, std::shared_ptr<CallRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;

    CallContainer();
    virtual ~CallContainer();

    void AddCallRecord(const sptr<IAbilityConnection> & connect, const std::shared_ptr<CallRecord>& callRecord);
    std::shared_ptr<CallRecord> GetCallRecord(const sptr<IAbilityConnection> & connect) const;
    bool RemoveCallRecord(const sptr<IAbilityConnection> & connect);
    bool CallRequestDone(const sptr<IRemoteObject> & callStub);
    void Dump(std::vector<std::string> &info, const std::string &args = "") const;
    bool IsNeedToCallRequest() const;

private:
    void RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect);
    void AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect);
    void OnConnectionDied(const wptr<IRemoteObject> & remote);

private:
    CallMapType callRecordMap_;
    RecipientMapType deathRecipientMap_;

    DISALLOW_COPY_AND_MOVE(CallContainer);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CALL_CONTAINER_H

