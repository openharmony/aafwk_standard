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

#include "stop_user_callback_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void StopUserCallbackProxy::OnStopUserDone(int accountId, int errcode)
{
    SendRequestCommon(accountId, errcode, IStopUserCallback::StopUserCallbackCmd::ON_STOP_USER_DONE);
}

void StopUserCallbackProxy::SendRequestCommon(int accountId, int errcode, IStopUserCallback::StopUserCallbackCmd cmd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    HILOG_INFO("StopUserCallbackProxy, sendrequest, cmd:%{public}d, accountId:%{public}d, errcode:%{public}d",
        cmd, accountId, errcode);
    if (!data.WriteInterfaceToken(IStopUserCallback::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(accountId)) {
        HILOG_ERROR("Write accountId error.");
        return;
    }

    if (!data.WriteInt32(errcode)) {
        HILOG_ERROR("Write errcode error.");
        return;
    }

    int error = Remote()->SendRequest(cmd, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnStopUserDone fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS