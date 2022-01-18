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

#include "service_extension_module_loader.h"
#include "service_extension.h"

namespace OHOS::AbilityRuntime {
ServiceExtensionModuleLoader::ServiceExtensionModuleLoader() = default;
ServiceExtensionModuleLoader::~ServiceExtensionModuleLoader() = default;

Extension *ServiceExtensionModuleLoader::Create(const std::unique_ptr<Runtime>& runtime) const
{
    return ServiceExtension::Create(runtime);
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &ServiceExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::AbilityRuntime