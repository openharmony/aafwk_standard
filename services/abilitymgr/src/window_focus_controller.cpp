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

#include "window_focus_controller.h"

#include "ability_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<WindowFocusController> WindowFocusController::instance_ = nullptr;
std::mutex WindowFocusController::mutex_;

std::shared_ptr<WindowFocusController> WindowFocusController::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<WindowFocusController>();
        }
    }
    return instance_;
}

void WindowFocusController::SubscribeWindowFocus()
{
    if (!focusChangedListener_) {
        focusChangedListener_ = new WindowFocusController::FocusChangedListener();
        if (focusChangedListener_) {
            WindowManager::GetInstance().RegisterFocusChangedListener(focusChangedListener_);
        }
    }
}

void WindowFocusController::UnsubscribeWindowFocus()
{
    if (focusChangedListener_) {
        // unregister windowFocusObserver_
        WindowManager::GetInstance().UnregisterFocusChangedListener(focusChangedListener_);
        focusChangedListener_ = nullptr;
    }
}

void WindowFocusController::GetTopAbility(AppExecFwk::ElementName &elementName)
{
    HILOG_DEBUG("%{public}s start.", __func__);
    if (instance_ == nullptr) {
        HILOG_ERROR("%{public}s instance is null.", __func__);
        return;
    }
    if (instance_->focusChangeInfo_ == nullptr) {
        HILOG_ERROR("%{public}s focusChangeInfo is null.", __func__);
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(instance_->focusChangeInfo_->abilityToken_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("%{public}s abilityRecord is null.", __func__);
        return;
    }
    elementName = abilityRecord->GetWant().GetElement();
    HILOG_DEBUG("%{public}s end.", __func__);
}

void WindowFocusController::FocusChangedListener::OnFocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    HILOG_DEBUG("%{public}s OnFocused called.", __func__);
    if (instance_ != nullptr) {
        instance_->focusChangeInfo_ = focusChangeInfo;
    } else {
        HILOG_ERROR("%{public}s instance_ is null.", __func__);
    }
}

void WindowFocusController::FocusChangedListener::OnUnfocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    HILOG_DEBUG("%{public}s OnUnfocused called.", __func__);
}
} // namespace AAFwk
} // namespace OHOS
