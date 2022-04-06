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

#ifndef OHOS_ABILITY_DELEGATOR_ABILITY_MONITOR_H
#define OHOS_ABILITY_DELEGATOR_ABILITY_MONITOR_H

#include <memory>
#include "iability_monitor.h"
#include "js_ability_monitor.h"
#include "native_engine/native_reference.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AppExecFwk;
class AbilityMonitor : public IAbilityMonitor {
public:
    /**
     * A constructor used to create a AbilityMonitor instance with the input parameter passed.
     *
     * @param name Indicates the specified ability name.
     * @param jsAbilityMonitor Indicates the JSAbilityMonitor object.
     */
    AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~AbilityMonitor() = default;

    /**
     * Called when ability is started.
     */
    void OnAbilityStart() override;

    /**
     * Called when ability is in foreground.
     */
    void OnAbilityForeground() override;

    /**
     * Called when ability is in background.
     */
    void OnAbilityBackground() override;

    /**
     * Called when ability is stopped.
     */
    void OnAbilityStop() override;

    /**
     * Called when window stage is created.
     */
    void OnWindowStageCreate() override;

    /**
     * Called when window stage is restored.
     */
    void OnWindowStageRestore() override;

    /**
     * Called when window stage is destroyed.
     */
    void OnWindowStageDestroy() override;

private:
    std::shared_ptr<JSAbilityMonitor> jsMonitor_;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_DELEGATOR_JS_ABILITY_MONITOR_H