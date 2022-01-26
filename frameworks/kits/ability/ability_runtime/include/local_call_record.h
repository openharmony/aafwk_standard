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

#ifndef ABILITY_RUNTIME_LOCAL_CALL_RECORD_H
#define ABILITY_RUNTIME_LOCAL_CALL_RECORD_H

#include "caller_callback.h"
#include "element_name.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class LocalCallRecord
 * LocalCallRecord record local call info.
 */
class LocalCallRecord : public std::enable_shared_from_this<LocalCallRecord> {
public:
    LocalCallRecord(const AppExecFwk::ElementName &elementName);
    virtual ~LocalCallRecord();

    void SetRemoteObject(const sptr<IRemoteObject> &call);
    void AddCaller(const std::shared_ptr<CallerCallBack> &callback);
    bool RemoveCaller(const std::shared_ptr<CallerCallBack> &callback);
    void OnCallStubDied(const wptr<IRemoteObject> &remote);
    sptr<IRemoteObject> GetRemoteObject() const;
    void InvokeCallBack() const;
    AppExecFwk::ElementName GetElementName() const;
    bool IsExistCallBack() const;

private:
    static int64_t callRecordId;
    int recordId_ = 0;                                     // record id
    sptr<IRemoteObject> remoteObject_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> callRecipient_ = nullptr;
    std::vector<std::shared_ptr<CallerCallBack>> callers_;
    AppExecFwk::ElementName elementName_ = {};
};

/**
 * @class CallRecipient
 * CallRecipient notices IRemoteBroker died.
 */
class CallRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;

    CallRecipient(RemoteDiedHandler handler) : handler_(handler) {};
    virtual ~CallRecipient() = default;

    void OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote) override
    {
        if (handler_) {
            handler_(remote);
        }
    };

private:
    RemoteDiedHandler handler_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_LOCAL_CALL_RECORD_H
