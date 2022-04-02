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

#include "want_receiver_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

#include "pac_map.h"

namespace OHOS {
namespace AAFwk {
WantReceiverStub::WantReceiverStub()
{
    requestFuncMap_[WANT_RECEIVER_SEND] = &WantReceiverStub::SendInner;
    requestFuncMap_[WANT_RECEIVER_PERFORM_RECEIVE] = &WantReceiverStub::PerformReceiveInner;
}

WantReceiverStub::~WantReceiverStub()
{
    requestFuncMap_.clear();
}

int WantReceiverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("WantReceiverStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = WantReceiverStub::GetDescriptor();
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
    HILOG_WARN("WantReceiverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WantReceiverStub::SendInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t resultCode = data.ReadInt32();
    Send(resultCode);
    return NO_ERROR;
}

int WantReceiverStub::PerformReceiveInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilityManagerStub: want is nullptr");
        return ERR_INVALID_VALUE;
    }

    int resultCode = data.ReadInt32();
    std::string bundleName = Str16ToStr8(data.ReadString16());

    WantParams *wantParams = data.ReadParcelable<WantParams>();
    if (wantParams == nullptr) {
        HILOG_ERROR("AbilityManagerStub: wantParams is nullptr");
        delete want;
        return ERR_INVALID_VALUE;
    }

    bool serialized = data.ReadBool();
    bool sticky = data.ReadBool();
    int sendingUser = data.ReadInt32();
    PerformReceive(*want, resultCode, bundleName, *wantParams, serialized, sticky, sendingUser);
    delete want;
    delete wantParams;
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS