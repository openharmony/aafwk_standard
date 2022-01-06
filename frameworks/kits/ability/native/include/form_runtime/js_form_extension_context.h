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

#ifndef FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_CONTEXT_H
#define FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_CONTEXT_H

#include <memory>

#include "form_extension_context.h"
#include "ability_connect_callback_stub.h"

class NativeEngine;
class NativeValue;
class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsFormExtensionContext(NativeEngine& engine, std::shared_ptr<FormExtensionContext> context);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_CONTEXT_H
