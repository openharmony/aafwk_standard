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

#include "lock_screen_event_subscriber.h"

#include "ability_config.h"
#include "ability_manager_service.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
void LockScreenEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    HILOG_INFO("OnReceiveEvent: event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("OnReceiveEvent: data=%{public}s", data.GetData().c_str());
    HILOG_INFO("OnReceiveEvent: code=%{public}d", data.GetCode());
    std::string eventName = data.GetWant().GetAction();
    if (eventName == AbilityConfig::LOCK_SCREEN_EVENT_NAME) {
        bool isLockScreen = static_cast<bool>(data.GetCode());
        auto task = [this, isLockScreen]() {
            auto service = DelayedSingleton<AbilityManagerService>::GetInstance();
            service->UpdateLockScreenState(isLockScreen);
        };
        handler_->PostTask(task);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
