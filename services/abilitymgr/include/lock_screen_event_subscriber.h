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

#ifndef OHOS_AAFWK_LOCK_SCREEN_EVENT_SUBSCRIBER_H
#define OHOS_AAFWK_LOCK_SCREEN_EVENT_SUBSCRIBER_H

#include <mutex>
#include <stdint.h>
#include "ability_event_handler.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace AAFwk {
class LockScreenEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit LockScreenEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp,
        const std::shared_ptr<AbilityEventHandler> &handler): EventFwk::CommonEventSubscriber(sp), handler_(handler)
    {}
    virtual ~LockScreenEventSubscriber()
    {}
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    std::shared_ptr<AbilityEventHandler> handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_LOCK_SCREEN_EVENT_SUBSCRIBER_H
