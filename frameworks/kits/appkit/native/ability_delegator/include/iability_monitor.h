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

#ifndef FOUNDATION_APPEXECFWK_OHOS_IABILITY_MONITORE_H
#define FOUNDATION_APPEXECFWK_OHOS_IABILITY_MONITORE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "ability_delegator_infos.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityMonitor {
public:
    static constexpr int64_t MAX_TIME_OUT {5000};

public:
    explicit IAbilityMonitor(const std::string &abilityName);
    virtual ~IAbilityMonitor() = default;

    virtual bool Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify = false);

    virtual std::shared_ptr<ADelegatorAbilityProperty> WaitForAbility();
    virtual std::shared_ptr<ADelegatorAbilityProperty> WaitForAbility(const int64_t timeoutMs);

    virtual void OnAbilityStart();
    virtual void OnAbilityForeground();
    virtual void OnAbilityBackground();
    virtual void OnAbilityStop();

    virtual void OnWindowStageCreate();
    virtual void OnWindowStageRestore();
    virtual void OnWindowStageDestroy();

private:
    std::string abilityName_;
    std::shared_ptr<ADelegatorAbilityProperty> matchedAbility_;

    std::condition_variable cvMatch_;
    std::mutex mMatch_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_IABILITY_MONITORE_H
