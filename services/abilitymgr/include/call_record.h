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

#ifndef OHOS_AAFWK_CALL_RECORD_H
#define OHOS_AAFWK_CALL_RECORD_H

#include "ability_connect_callback_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;
/**
 * @class AbilityCallRecipient
 * AbilityCallRecipient notices IRemoteBroker died.
 */
class AbilityCallRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;

    AbilityCallRecipient(RemoteDiedHandler handler) : handler_(handler) {};
    virtual ~AbilityCallRecipient() = default;

    void OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote) override
    {
        if (handler_) {
            handler_(remote);
        }
    };

private:
    RemoteDiedHandler handler_;
};

/**
 * @enum CallState
 * CallState defines the state of calling ability.
 */
enum class CallState { INIT, REQUESTING, REQUESTED };
/**
 * @class CallRecord
 * CallRecord,This class is used to record information about a connection of calling ability.
 */
class CallRecord : public std::enable_shared_from_this<CallRecord> {
public:
    CallRecord(const int32_t callerUid, const std::shared_ptr<AbilityRecord> &targetService,
        const sptr<IAbilityConnection> &connCallback, const sptr<IRemoteObject> &callToken);
    virtual ~CallRecord();

    static std::shared_ptr<CallRecord> CreateCallRecord(const int32_t callerUid,
        const std::shared_ptr<AbilityRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
        const sptr<IRemoteObject> &callToken);

    void SetCallStub(const sptr<IRemoteObject> & call);
    sptr<IRemoteObject> GetCallStub();
    void SetConCallBack(const sptr<IAbilityConnection> &connCallback);
    sptr<IAbilityConnection> GetConCallBack() const;

    void Dump(std::vector<std::string> &info) const;
    bool SchedulerConnectDone();
    bool SchedulerDisConnectDone();
    void OnCallStubDied(const wptr<IRemoteObject> & remote);

    int32_t GetCallerUid() const;
    bool IsCallState(const CallState& state) const;
    void SetCallState(const CallState& state);
    int32_t GetCallRecordId() const;
    AppExecFwk::ElementName GetTargetServiceName() const;
    sptr<IRemoteObject> GetCallerToken() const;
    
private:
    static int64_t callRecordId;
    int recordId_;                                  // record id
    int32_t callerUid_;                             // caller uid
    CallState state_;                               // call state
    sptr<IRemoteObject> callRemoteObject_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> callDeathRecipient_ = nullptr;
    std::weak_ptr<AbilityRecord> service_;             // target:service need to be called
    sptr<IAbilityConnection> connCallback_ = nullptr;  // service connect callback
    sptr<IRemoteObject> callerToken_ = nullptr;     // caller token
    int64_t startTime_ = 0;                         // records first time of ability start

    DISALLOW_COPY_AND_MOVE(CallRecord);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CALL_RECORD_H