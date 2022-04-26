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
#ifndef OHOS_AAFWK_WMS_HANDLER_H
#define OHOS_AAFWK_WMS_HANDLER_H

#include "iremote_broker.h"
#include "window_info.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IWindowManagerServiceHandler
 * Window Manager Service Handler, use to call methods of WMS
 */
class IWindowManagerServiceHandler : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.WindowManagerServiceHandler");

    virtual void NotifyWindowTransition(sptr<WindowTransitionInfo> fromInfo, sptr<WindowTransitionInfo> toInfo) = 0;

    enum WMSCmd {
        // ipc id for NotifyWindowTransition
        ON_NOTIFY_WINDOW_TRANSITION,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_WMS_HANDLER_H
#endif