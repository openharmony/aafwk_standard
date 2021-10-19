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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_KEYEVENT_HANDLE_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_KEYEVENT_HANDLE_H

#include <memory>
#include "multimodal_event_handler.h"
#include "key_event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityImpl;
class AbilityKeyEventHandle : public MMI::KeyEventHandler
{
public:
    AbilityKeyEventHandle(const std::shared_ptr<AbilityImpl> &ability);
    ~AbilityKeyEventHandle();

    /**
     * @brief Called back when on key.
     */
    virtual bool OnKey(const KeyEvent& keyEvent) override;

private:
    std::shared_ptr<AbilityImpl> abilityImpl_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_KEYEVENT_HANDLE_H