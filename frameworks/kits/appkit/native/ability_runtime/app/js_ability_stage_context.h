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

#ifndef ABILITY_RUNTIME_JS_ABILITY_STAGE_CONTEXT_H
#define ABILITY_RUNTIME_JS_ABILITY_STAGE_CONTEXT_H

#include "foundation/aafwk/standard/frameworks/kits/appkit/native/ability_runtime/context/context.h"
#include "configuration.h"

class NativeEngine;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityStageContext final {
public:
    explicit JsAbilityStageContext(const std::shared_ptr<AbilityRuntime::Context>& context) : context_(context) {}
    ~JsAbilityStageContext() = default;

    static void ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<AbilityRuntime::Context> GetContext()
    {
        return context_.lock();
    }

private:
    std::weak_ptr<AbilityRuntime::Context> context_;
};

NativeValue* CreateJsAbilityStageContext(NativeEngine& engine, std::shared_ptr<AbilityRuntime::Context> context);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_ABILITY_STAGE_CONTEXT_H