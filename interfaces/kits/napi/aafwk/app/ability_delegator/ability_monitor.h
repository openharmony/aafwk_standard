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
    AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor);
    ~AbilityMonitor() = default;

    void OnAbilityStart() override;
    void OnAbilityForeground() override;
    void OnAbilityBackground() override;
    void OnAbilityStop() override;
    void OnWindowStageCreate() override;
    void OnWindowStageRestore() override;
    void OnWindowStageDestroy() override;

private:
    std::shared_ptr<JSAbilityMonitor> jsMonitor_;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_DELEGATOR_JS_ABILITY_MONITOR_H