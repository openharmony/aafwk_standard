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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_TOUCHEVENT_HANDLE_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_TOUCHEVENT_HANDLE_H

#include <memory>

#ifdef MMI_COMPILE
#include "multimodal_events_handler.h"
#include "touch_events_handler.h"
#endif

namespace OHOS {
namespace AppExecFwk {
class AbilityImpl;

#ifdef MMI_COMPILE
class AbilityTouchEventHandle : public MMI::TouchEventHandler {
#else
class AbilityTouchEventHandle {
#endif
public:
    AbilityTouchEventHandle(std::shared_ptr<AbilityImpl> ability);
    virtual ~AbilityTouchEventHandle();

#ifdef MMI_COMPILE
    /**
     * @brief Called back when on touch.
     */
    virtual bool OnTouch(const TouchEvent& touchEvent) override;
#endif

private:
    std::shared_ptr<AbilityImpl> abilityImpl_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_TOUCHEVENT_HANDLE_H