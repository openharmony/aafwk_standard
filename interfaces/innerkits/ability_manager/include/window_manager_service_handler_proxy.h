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

#ifndef OHOS_AAFWK_WMS_HANDLER_PROXY_H
#define OHOS_AAFWK_WMS_HANDLER_PROXY_H

#ifdef SUPPORT_GRAPHICS
#include "iremote_proxy.h"
#include "window_manager_service_handler.h"

namespace OHOS {
namespace AAFwk {
class WindowManagerServiceHandlerProxy : public IRemoteProxy<IWindowManagerServiceHandler> {
public:
    explicit WindowManagerServiceHandlerProxy(const sptr<IRemoteObject> &impl);
    virtual ~WindowManagerServiceHandlerProxy() = default;

    virtual void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo,
        sptr<AbilityTransitionInfo> toInfo) override;

    virtual int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken) override;

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info, sptr<Media::PixelMap> pixelMap,
        uint32_t bgColor) override;

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info, sptr<Media::PixelMap> pixelMap) override;

    virtual void CancelStartingWindow(sptr<IRemoteObject> abilityToken) override;

private:
    static inline BrokerDelegator<WindowManagerServiceHandlerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
#endif  // OHOS_AAFWK_WMS_HANDLER_PROXY_H
