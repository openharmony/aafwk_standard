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
#include "window_manager_service_handler_stub.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
int WindowManagerServiceHandlerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IWindowManagerServiceHandler::GetDescriptor()) {
        HILOG_ERROR("InterfaceToken not equal IWindowManagerServiceHandler's descriptor.");
        return ERR_INVALID_VALUE;
    }
    ErrCode errCode = ERR_OK;
    switch (code) {
        case WMSCmd::ON_NOTIFY_WINDOW_TRANSITION : {
            sptr<AbilityTransitionInfo> fromInfo(data.ReadParcelable<AbilityTransitionInfo>());
            if (!fromInfo) {
                errCode = ERR_DEAD_OBJECT;
                HILOG_ERROR("To read fromInfo failed.");
                break;
            }
            sptr<AbilityTransitionInfo> toInfo(data.ReadParcelable<AbilityTransitionInfo>());
            if (!toInfo) {
                errCode = ERR_DEAD_OBJECT;
                HILOG_ERROR("To read toInfo failed.");
                break;
            }
            NotifyWindowTransition(fromInfo, toInfo);
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return errCode;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
