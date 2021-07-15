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

#include "want_sender_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AAFwk {
WantSenderStub::WantSenderStub()
{
    requestFuncMap_[WANT_SENDER_SEND] = &WantSenderStub::SendInner;
}

WantSenderStub::~WantSenderStub()
{
    requestFuncMap_.clear();
}

int WantSenderStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("WantSendStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = WantSenderStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("WantSenderStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WantSenderStub::SendInner(MessageParcel &data, MessageParcel &reply)
{
    SenderInfo *senderInfo = data.ReadParcelable<SenderInfo>();
    if (senderInfo == nullptr) {
        HILOG_ERROR("WantSenderStub: senderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    Send(*senderInfo);
    delete senderInfo;
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS