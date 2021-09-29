
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

#include "ability_distributed_connect_callback_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
AbilityDistributedConnectionStub::AbilityDistributedConnectionStub()
{}

AbilityDistributedConnectionStub::~AbilityDistributedConnectionStub()
{}

int AbilityDistributedConnectionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AbilityDistributedConnectionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto element = data.ReadParcelable<AppExecFwk::ElementName>();
    switch (code) {
        case IAbilityDistributedConnection::ON_ABILITY_DISTRIBUTE_CONNECT_DONE: {
            if (element == nullptr) {
                HILOG_ERROR("callback stub receive element is nullptr");
                return ERR_INVALID_VALUE;
            }
            auto remoteObject = data.ReadRemoteObject();
            auto resultCode = data.ReadInt32();
            OnAbilityConnectDone(*element, remoteObject, resultCode);
            delete element;
            return NO_ERROR;
        }
        case IAbilityDistributedConnection::ON_ABILITY_DISTRIBUTE_DISCONNECT_DONE: {
            if (element == nullptr) {
                HILOG_ERROR("callback stub receive element is nullptr");
                return ERR_INVALID_VALUE;
            }
            auto resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            delete element;
            return NO_ERROR;
        }
        default: {
            if (element != nullptr) {
                delete element;
            }
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AbilityDistriubuteConnectCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    HILOG_ERROR("On remote died.");
    if (handler_) {
        handler_(remote);
    }
}

AbilityDistriubuteConnectCallbackRecipient::AbilityDistriubuteConnectCallbackRecipient(RemoteDiedHandler handler)
    : handler_(handler)
{}

AbilityDistriubuteConnectCallbackRecipient::~AbilityDistriubuteConnectCallbackRecipient()
{}
}  // namespace AAFwk
}  // namespace OHOS
