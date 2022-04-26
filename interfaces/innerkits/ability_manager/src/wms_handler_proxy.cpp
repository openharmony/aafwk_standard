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

#ifdef SUPPORT_GRAPHICS
#include "window_manager_service_handler_proxy.h"

#include "hilog_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
WindowManagerServiceHandlerProxy::WindowManagerServiceHandlerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IWindowManagerServiceHandler>(impl) {}

void WindowManagerServiceHandlerProxy::NotifyWindowTransition(sptr<WindowTransitionInfo> fromInfo,
    sptr<WindowTransitionInfo> toInfo)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(fromInfo.GetRefPtr())) {
        HILOG_ERROR("Write fromInfo failed.");
        return;
    }
    if (!data.WriteParcelable(toInfo.GetRefPtr())) {
        HILOG_ERROR("Write toInfo failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(WMSCmd::ON_NOTIFY_WINDOW_TRANSITION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fial, error: %{public}d", error);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
