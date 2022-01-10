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

#include "stop_user_callback_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
StopUserCallbackStub::StopUserCallbackStub()
{
    vecMemberFunc_.resize(StopUserCallbackCmd::CMD_MAX);
    vecMemberFunc_[StopUserCallbackCmd::ON_STOP_USER_DONE] = &StopUserCallbackStub::OnStopUserDoneInner;
}

int StopUserCallbackStub::OnStopUserDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto accountId = data.ReadInt32();
    auto errCode = data.ReadInt32();
    OnStopUserDone(accountId, errCode);
    return NO_ERROR;
}

int StopUserCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = StopUserCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code < StopUserCallbackCmd::ON_STOP_USER_DONE && code >= 0) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS