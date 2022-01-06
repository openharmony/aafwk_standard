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

#include "js_extension_context.h"

#include <cstdint>

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsExtensionContext(NativeEngine& engine, std::shared_ptr<ExtensionContext> context)
{
    HILOG_INFO("CreateJsExtensionContext begin");
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
