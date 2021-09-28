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

#ifndef OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_STUB_H
#define OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_distributed_connection_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityDistributedConnectionStub
 * AbilityDistributeConnect Stub.
 */
class AbilityDistributedConnectionStub : public IRemoteStub<IAbilityDistributedConnection> {
public:
    AbilityDistributedConnectionStub();
    virtual ~AbilityDistributedConnectionStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(AbilityDistributedConnectionStub);
};

/**
 * @class AbilityDistriubuteConnectCallbackRecipient
 *  AbilityDistriubuteConnectCallbackRecipient notices IRemoteBroker died.
 */
class AbilityDistriubuteConnectCallbackRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    AbilityDistriubuteConnectCallbackRecipient(RemoteDiedHandler handler);
    virtual ~AbilityDistriubuteConnectCallbackRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_STUB_H
