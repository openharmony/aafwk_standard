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
#include <chrono>
#include "app_log_wrapper.h"
#include "iability_monitor.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AppExecFwk {
IAbilityMonitor::IAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

bool IAbilityMonitor::Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify)
{
    if (!ability) {
        APP_LOGW("Invalid ability property");
        return false;
    }

    const auto &aName = ability->name_;

    if (abilityName_.empty() || aName.empty()) {
        APP_LOGW("Invalid name");
        return false;
    }

    if (abilityName_.compare(aName)) {
        APP_LOGW("Different name");
        return false;
    }

    APP_LOGI("Matched : ability name : %{public}s, isNotify : %{public}s",
        abilityName_.data(), (isNotify ? "true" : "false"));

    if (isNotify) {
        APP_LOGI("Matched : notify ability matched");
        {
            std::lock_guard<std::mutex> matchLock(mMatch_);
            matchedAbility_ = ability;
        }
        cvMatch_.notify_one();
    }

    return true;
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::waitForAbility()
{
    return waitForAbility(MAX_TIME_OUT);
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::waitForAbility(const int64_t timeoutMs)
{
    auto realTime = timeoutMs;
    if (timeoutMs <= 0) {
        APP_LOGW("Timeout should be a positive number");
        realTime = MAX_TIME_OUT;
    }

    std::unique_lock<std::mutex> matchLock(mMatch_);

    auto condition = [this] { return this->matchedAbility_ != nullptr; };
    if (!cvMatch_.wait_for(matchLock, realTime * 1ms, condition)) {
        APP_LOGW("Wait ability timeout");
    }

    return matchedAbility_;
}

void IAbilityMonitor::OnAbilityStart()
{}

void IAbilityMonitor::OnAbilityForeground()
{}

void IAbilityMonitor::OnAbilityBackground()
{}

void IAbilityMonitor::OnAbilityStop()
{}

void IAbilityMonitor::OnWindowStageCreate()
{}

void IAbilityMonitor::OnWindowStageRestore()
{}

void IAbilityMonitor::OnWindowStageDestroy()
{}
}  // namespace AppExecFwk
}  // namespace OHOS
