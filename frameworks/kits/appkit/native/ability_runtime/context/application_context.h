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

#ifndef ABILITY_RUNTIME_APPLICATION_CONTEXT_H
#define ABILITY_RUNTIME_APPLICATION_CONTEXT_H

#include "context.h"
#include "js_application_ability_lifecycle_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class ApplicationContext : public Context {
public:
    ApplicationContext() = default;
    virtual ~ApplicationContext() = default;
    int32_t registerAbilityLifecycleCallback(
        const std::shared_ptr<JsApplicationContextAbilityLifecycleCallback> &abilityLifecycleCallback);
    void unregisterAbilityLifecycleCallback(const int32_t callbackId);
};
}  // AbilityRuntime
}  // namespace OHOS
#endif // ABILITY_RUNTIME_APPLICATION_CONTEXT_H