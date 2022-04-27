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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_MODULE_LOADER_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_MODULE_LOADER_H

#include "extension.h"
#include "runtime.h"
#include "singleton.h"

namespace OHOS::AbilityRuntime {
/**
 * @brief Load extension lib.
 */
class ExtensionModuleLoader {
public:
    virtual ~ExtensionModuleLoader() = default;

    static ExtensionModuleLoader& GetLoader(const char* sharedLibrary = nullptr);

    /**
     * @brief Create Extension.
     *
     * @param runtime The runtime.
     * @return The Extension instance.
     */
    virtual Extension *Create(const std::unique_ptr<Runtime>& runtime) const = 0;

    /**
     * @brief Get Extension config params.
     *
     * @return The Extension config params.
     */
    virtual std::map<std::string, std::string> GetParams();
};
} // namespace OHOS::AbilityRuntime

#endif // FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_MODULE_LOADER_H
