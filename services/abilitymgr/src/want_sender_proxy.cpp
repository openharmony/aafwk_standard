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

#include "want_sender_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AAFwk {
bool WantSenderProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(WantSenderProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void WantSenderProxy::Send(SenderInfo &senderInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        HILOG_ERROR("fail to WriteParcelable value");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IWantSender::WANT_SENDER_SEND), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AAFwk
}  // namespace OHOS