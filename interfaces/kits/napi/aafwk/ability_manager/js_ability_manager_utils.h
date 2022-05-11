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

#include "ability_running_info.h"
#include "extension_running_info.h"
#include "element_name.h"
#include "native_engine/native_engine.h"

#ifndef OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_UTILS_H
#define OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_UTILS_H

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsAbilityRunningInfoArray(
    NativeEngine &engine, const std::vector<AAFwk::AbilityRunningInfo> &infos);
NativeValue* CreateJsExtensionRunningInfoArray(
    NativeEngine &engine, const std::vector<AAFwk::ExtensionRunningInfo> &infos);
NativeValue* CreateJsAbilityRunningInfo(NativeEngine &engine, const AAFwk::AbilityRunningInfo &info);
NativeValue* CreateJsExtensionRunningInfo(NativeEngine &engine, const AAFwk::ExtensionRunningInfo &info);
NativeValue *AbilityStateInit(NativeEngine *engine);
NativeValue* CreateJsElementName(NativeEngine &engine, const AppExecFwk::ElementName &elementName);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_APPEXECFWK_RUNTIME_APP_MANAGER_UTILS_H