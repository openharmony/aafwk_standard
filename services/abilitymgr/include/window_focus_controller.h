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

#ifndef OHOS_OS_AAFWK_WINDOW_FOCUS_CONTROLLER_H
#define OHOS_OS_AAFWK_WINDOW_FOCUS_CONTROLLER_H

#include <shared_mutex>

#include "element_name.h"
#include "foundation/windowmanager/interfaces/innerkits/wm/window_manager.h"

namespace OHOS {
namespace AAFwk {
using OHOS::Rosen::FocusChangeInfo;
using OHOS::Rosen::IFocusChangedListener;
using OHOS::Rosen::WindowManager;

class WindowFocusController {
public:
    WindowFocusController() = default;
    virtual ~WindowFocusController() = default;
    static std::shared_ptr<WindowFocusController> GetInstance();

    void SubscribeWindowFocus();
    void UnsubscribeWindowFocus();
    void GetTopAbility(AppExecFwk::ElementName &elementName);

    class FocusChangedListener : public Rosen::IFocusChangedListener {
    public:
        void OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
        void OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
    };

private:
    static std::mutex mutex_;
    static std::shared_ptr<WindowFocusController> instance_;
    sptr<FocusChangeInfo> focusChangeInfo_ = nullptr;
    sptr<IFocusChangedListener> focusChangedListener_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_OS_AAFWK_WINDOW_FOCUS_CONTROLLER_H
