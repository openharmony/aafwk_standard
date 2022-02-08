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

#ifndef EXTENSION_CONTEXT_H
#define EXTENSION_CONTEXT_H

#include "ability_info.h"
#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief context supply for extension
 *
 */
class ExtensionContext : public ContextImpl {
public:
    ExtensionContext() = default;
    virtual ~ExtensionContext() = default;

    /**
     * @brief Obtains information about the current ability.
     * The returned information includes the class name, bundle name, and other information about the current ability.
     *
     * @return Returns the AbilityInfo object for the current ability.
     */
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> GetAbilityInfo() const;

    /**
     * @brief Set AbilityInfo when init.
     *
     */
    void SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo);

    using SelfType = ExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }

private:
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // EXTENSION_CONTEXT_H