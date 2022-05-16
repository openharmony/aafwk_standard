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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_STATIC_SUBSCRIBER_EXTENSION_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_STATIC_SUBSCRIBER_EXTENSION_H

#include "common_event_data.h"
#include "extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
class StaticSubscriberExtensionContext;
class Runtime;

class StaticSubscriberExtension : public ExtensionBase<StaticSubscriberExtensionContext> {
public:
    StaticSubscriberExtension() = default;
    virtual ~StaticSubscriberExtension() = default;

    std::shared_ptr<StaticSubscriberExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord>& record,
        const std::shared_ptr<OHOSApplication>& application,
        std::shared_ptr<AbilityHandler>& handler,
        const sptr<IRemoteObject>& token) override;

    void Init(const std::shared_ptr<AbilityLocalRecord>& record,
        const std::shared_ptr<OHOSApplication>& application,
        std::shared_ptr<AbilityHandler>& handler,
        const sptr<IRemoteObject>& token) override;

    static StaticSubscriberExtension* Create(const std::unique_ptr<Runtime>& runtime);

    virtual void OnReceiveEvent(std::shared_ptr<EventFwk::CommonEventData> data);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_ABILITYRUNTIME_OHOS_STATIC_SUBSCRIBER_EXTENSION_H