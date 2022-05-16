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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_JS_STATIC_SUBSCRIBER_EXTENSION_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_JS_STATIC_SUBSCRIBER_EXTENSION_H

#include "common_event_data.h"
#include "static_subscriber_extension.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
/**
 * @brief js-level static subscriber extension.
 */
class JsStaticSubscriberExtension : public StaticSubscriberExtension {
public:
    JsStaticSubscriberExtension(JsRuntime& jsRuntime);
    virtual ~JsStaticSubscriberExtension() override;

    /**
     * @brief Create JsStaticSubscriberExtension.
     *
     * @param runtime The runtime.
     * @return The JsStaticSubscriberExtension instance.
     */
    static JsStaticSubscriberExtension* Create(const std::unique_ptr<Runtime>& runtime);

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
              const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
              std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
              const sptr<IRemoteObject>& token) override;

    void OnStart(const AAFwk::Want& want) override;

    sptr<IRemoteObject> OnConnect(const AAFwk::Want& want) override;

    void OnDisconnect(const AAFwk::Want& want) override;

    void OnStop() override;

    void OnReceiveEvent(std::shared_ptr<EventFwk::CommonEventData> data) override;
private:
    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_ABILITYRUNTIME_OHOS_JS_STATIC_SUBSCRIBER_EXTENSION_H