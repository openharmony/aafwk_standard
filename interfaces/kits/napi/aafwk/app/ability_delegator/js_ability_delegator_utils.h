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

#ifndef OHOS_ABILITY_DELEGATOR_JS_ABILITY_DELEGATOR_UTILS_H
#define OHOS_ABILITY_DELEGATOR_JS_ABILITY_DELEGATOR_UTILS_H

#include "js_runtime_utils.h"
#include "ability_delegator.h"
#include "ability_delegator_args.h"
#include "js_ability_delegator.h"
#include "native_engine/native_engine.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorJs {
NativeValue *CreateJsAbilityDelegator(NativeEngine &engine);
NativeValue *CreateJsAbilityDelegatorArguments(
    NativeEngine &engine, const std::shared_ptr<AbilityDelegatorArgs> &abilityDelegatorArgs);
NativeValue *CreateJsShellCmdResult(NativeEngine &engine, std::unique_ptr<ShellCmdResult> &shellResult);
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif // OHOS_ABILITY_DELEGATOR_JS_ABILITY_DELEGATOR_UTILS_H