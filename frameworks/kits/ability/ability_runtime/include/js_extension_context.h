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

#ifndef ABILITY_RUNTIME_JS_EXTENSION_CONTEXT_H
#define ABILITY_RUNTIME_JS_EXTENSION_CONTEXT_H

#include <memory>

#include "extension_context.h"

class NativeEngine;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class JsExtensionContext final {
public:
    JsExtensionContext(const std::shared_ptr<ExtensionContext>& context) : context_(context) {}
    ~JsExtensionContext() = default;

    static void ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

private:
    std::weak_ptr<ExtensionContext> context_;
};

NativeValue* CreateJsExtensionContext(NativeEngine& engine, std::shared_ptr<ExtensionContext> context);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_EXTENSION_CONTEXT_H
