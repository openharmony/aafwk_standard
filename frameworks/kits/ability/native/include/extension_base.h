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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_BASE_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_BASE_H

#include <string>

#include "ability_local_record.h"
#include "extension.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class OHOSApplication;
class AbilityHandler;
}
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
class ExtensionContext;
/**
 * @brief The ExtensionBase class for all extensions.
 */
template<class C = ExtensionContext>
class ExtensionBase : public Extension {
public:
    ExtensionBase() = default;
    virtual ~ExtensionBase() = default;

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create and init context.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<C> CreateAndInitContext(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token);

    /**
     * @brief Get context.
     *
     * @return The context object.
     */
    virtual std::shared_ptr<C> GetContext();
private:
    std::shared_ptr<C> context_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#include "extension_base.inl"
#endif  // FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_BASE_H