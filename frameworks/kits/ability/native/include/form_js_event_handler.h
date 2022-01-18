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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_JS_EVENT_HANDLER_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_JS_EVENT_HANDLER_H

#include <singleton.h>
#include "ability.h"
#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
class FormJsEventHandler : public EventHandler {
public:
    FormJsEventHandler(const std::shared_ptr<EventRunner> &runner,
        const std::shared_ptr<Ability>& ability, const FormJsInfo& formJsInfo);
    ~FormJsEventHandler() = default;

    /**
     * Process the event. Developers should override this method.
     *
     * @param event The event should be processed.
     */
    void ProcessEvent(const InnerEvent::Pointer &event) override;

    enum {
        FORM_ROUTE_EVENT = 100,
        FORM_MESSAGE_EVENT,
    };
private:
    /**
     * @brief Process js router event.
     * @param want Indicates the event to be processed.
     */
    void  ProcessRouterEvent(Want &want);

    /**
     * @brief Process js message event.
     * @param want Indicates the event to be processed.
     */
    void  ProcessMessageEvent(const Want &want);
    bool IsSystemApp() const;

private:
    FormJsInfo formJsInfo_;
    std::shared_ptr<Ability> ability_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_JS_EVENT_HANDLER_H
