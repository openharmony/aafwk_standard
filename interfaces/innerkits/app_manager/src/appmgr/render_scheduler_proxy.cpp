/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "render_scheduler_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"


namespace OHOS {
namespace AppExecFwk {
RenderSchedulerProxy::RenderSchedulerProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IRenderScheduler>(impl)
{}

bool RenderSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(RenderSchedulerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void RenderSchedulerProxy::NotifyBrowserFd(int32_t ipcFd, int32_t sharedFd)
{
    HILOG_DEBUG("NotifyBrowserFd start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd)) {
        HILOG_ERROR("want fd failed, ipcFd:%{public}d, sharedFd:%{public}d", ipcFd, sharedFd);
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IRenderScheduler::Message::NOTIFY_BROWSER_FD),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("NotifyBrowserFd end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
