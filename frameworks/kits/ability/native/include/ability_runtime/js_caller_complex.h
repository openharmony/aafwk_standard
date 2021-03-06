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
#ifndef ABILITY_RUNTIME_JS_CALLERCOMPLEX_H
#define ABILITY_RUNTIME_JS_CALLERCOMPLEX_H

#include <memory>
#include <native_engine/native_value.h>

#include "iremote_object.h"
#include "foundation/aafwk/standard/frameworks/kits/ability/ability_runtime/include/ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsCallerComplex(
    NativeEngine& engine, std::shared_ptr<AbilityContext> context, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack);

NativeValue* CreateJsCalleeRemoteObject(NativeEngine& engine, sptr<IRemoteObject> callee);
} // AbilityRuntime
} // OHOS
#endif  // ABILITY_RUNTIME_JS_CALLERCOMPLEX_H