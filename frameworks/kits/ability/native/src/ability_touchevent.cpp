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

#include "ability_touchevent.h"
#include "ability_impl.h"
#include "app_log_wrapper.h"
#include "ability_post_event_timeout.h"

namespace OHOS {
namespace AppExecFwk {
AbilityTouchEventHandle::AbilityTouchEventHandle(std::shared_ptr<AbilityImpl> abilityImpl) : abilityImpl_(abilityImpl)
{
    APP_LOGI("AbilityTouchEventHandle is created");
}

AbilityTouchEventHandle::~AbilityTouchEventHandle()
{
    APP_LOGI("AbilityTouchEventHandle is destroyed");
}

/**
 * @brief Called back when on touch.
 */
bool AbilityTouchEventHandle::OnTouch(const TouchEvent &touchEvent)
{
    APP_LOGI("AbilityTouchEventHandle::OnTouch called.");
    bool ret = false;
    if (abilityImpl_ == nullptr) {
        APP_LOGE("AbilityTouchEventHandle::OnTouch abilityImpl_ is nullptr");
        return ret;
    }

    std::string taskHead("OnTouch");
    std::string taskCodeAction = std::to_string(touchEvent.GetAction());
    std::string taskCodePhase = std::to_string(touchEvent.GetPhase());
    std::string taskSplit("-");
    auto timeOut =
        abilityImpl_->CreatePostEventTimeouter(taskHead + taskCodeAction + taskSplit + taskCodePhase + taskSplit);
    if (timeOut == nullptr) {
        APP_LOGW("AbilityTouchEventHandle::OnTouch timeouter Create return nullptr");
        ret = abilityImpl_->DoTouchEvent(touchEvent);
    } else {
        timeOut->TimingBegin();
        ret = abilityImpl_->DoTouchEvent(touchEvent);
        timeOut->TimeEnd();
    }

    APP_LOGI("AbilityImpl::DoTouchEvent action: %{public}d phase: %{public}d.",
        touchEvent.GetAction(),
        touchEvent.GetPhase());
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS