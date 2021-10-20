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

#include "ability_keyevent.h"
#include "ability_impl.h"
#include "app_log_wrapper.h"
#include "ability_post_event_timeout.h"

namespace OHOS {
namespace AppExecFwk {
AbilityKeyEventHandle::AbilityKeyEventHandle(const std::shared_ptr<AbilityImpl> &abilityImpl)
    : abilityImpl_(abilityImpl)
{
    APP_LOGI("AbilityKeyEventHandle is created");
}

AbilityKeyEventHandle::~AbilityKeyEventHandle()
{
    APP_LOGI("AbilityKeyEventHandle is destroyed");
}

/**
 * @brief Called back when on key.
 */
bool AbilityKeyEventHandle::OnKey(const KeyEvent &keyEvent)
{
    APP_LOGI("AbilityKeyEventHandle::OnKey called.");
    bool ret = false;
    if (abilityImpl_ == nullptr) {
        APP_LOGE("AbilityImpl::OnKey abilityImpl_ is nullptr. KeyCode %{public}d.", keyEvent.GetKeyCode());
        return ret;
    }

    if (keyEvent.IsKeyDown()) {
        std::string taskHead("OnKey");
        std::string taskCode = std::to_string(keyEvent.GetKeyCode());
        std::string taskTail("Down");
        auto timeOut = abilityImpl_->CreatePostEventTimeouter(taskHead + taskCode + taskTail);
        if (timeOut == nullptr) {
            APP_LOGW("AbilityKeyEventHandle::OnKeyDown timeouter Create return nullptr");
            ret = abilityImpl_->DoKeyDown(keyEvent.GetKeyCode(), keyEvent);
        } else {
            timeOut->TimingBegin();
            ret = abilityImpl_->DoKeyDown(keyEvent.GetKeyCode(), keyEvent);
            timeOut->TimeEnd();
        }
        APP_LOGI("AbilityImpl::OnKeyDown keyCode: %{public}d.", keyEvent.GetKeyCode());
    } else {
        std::string taskHead("OnKey");
        std::string taskCode = std::to_string(keyEvent.GetKeyCode());
        std::string taskTail("Up");
        auto timeOut = abilityImpl_->CreatePostEventTimeouter(taskHead + taskCode + taskTail);
        if (timeOut == nullptr) {
            APP_LOGW("AbilityKeyEventHandle::OnKeyUp timeouter Create return nullptr");
            ret = abilityImpl_->DoKeyUp(keyEvent.GetKeyCode(), keyEvent);
        } else {
            timeOut->TimingBegin();
            ret = abilityImpl_->DoKeyUp(keyEvent.GetKeyCode(), keyEvent);
            timeOut->TimeEnd();
        }
        APP_LOGI("AbilityImpl::DoKeyUp keyCode: %{public}d.", keyEvent.GetKeyCode());
    }

    APP_LOGI("AbilityKeyEventHandle::OnKey called end. return %{public}s", ret ? "true" : "false");
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS