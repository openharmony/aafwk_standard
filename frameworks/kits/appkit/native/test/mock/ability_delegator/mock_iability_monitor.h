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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_ABILITY_DELEGATOR_MOCK_IABILITY_MONITOR_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_ABILITY_DELEGATOR_MOCK_IABILITY_MONITOR_H

#include "gmock/gmock.h"

#include "iability_monitor.h"

namespace OHOS {
namespace AppExecFwk {
class MockIabilityMonitor : public IAbilityMonitor {
public:

    explicit MockIabilityMonitor(const std::string &abilityName);
    virtual ~MockIabilityMonitor() = default;

    virtual bool Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify = false);

    std::shared_ptr<ADelegatorAbilityProperty> waitForAbility()
    {
        return std::make_shared<ADelegatorAbilityProperty>();
    };
    std::shared_ptr<ADelegatorAbilityProperty> waitForAbility(const int64_t timeoutMs)
    {
        return std::make_shared<ADelegatorAbilityProperty>();
    };

    void OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj) override;
    void OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj) override;
    void OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj) override;
    void OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj) override;

    void OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj) override;
    void OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj) override;
    void OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj) override;

    bool start_;
    bool foreground_;
    bool background_;
    bool stop_;
    bool windowStageCreate_;
    bool windowStageRestore_;
    bool windowStageDestroy_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_MOCK_IABILITY_MONITOR__H