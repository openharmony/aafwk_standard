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

#include "mock_iability_monitor.h"
#include "hilog_wrapper.h"

namespace {
const std::string ABILITY_NAME = "com.example.myapplication.MainAbility";
}

namespace OHOS {
namespace AppExecFwk {
MockIabilityMonitor::MockIabilityMonitor(const std::string &abilityName) : IAbilityMonitor(abilityName)
{
    start_ = false;
    foreground_ = false;
    background_ = false;
    stop_ = false;
    windowStageCreate_ = false;
    windowStageRestore_ = false;
    windowStageDestroy_ = false;
}

void MockIabilityMonitor::OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnAbilityStart is called");
    start_ = true;
}

void MockIabilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnAbilityForeground is called");
    foreground_ = true;
}

void MockIabilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnAbilityBackground is called");
    background_ = true;
}

void MockIabilityMonitor::OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnAbilityStop is called");
    stop_ = true;
}
  
void MockIabilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnWindowStageCreate is called");
    windowStageCreate_ = true;
}

void MockIabilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnWindowStageRestore is called");
    windowStageRestore_ = true;
}

void MockIabilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("MockIabilityMonitor::OnWindowStageDestroy is called");
    windowStageDestroy_ = true;
}

bool MockIabilityMonitor::Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify)
{
    HILOG_INFO("MockIabilityMonitor::Match is called");

    bool ret = false;

    if (ability->name_ == ABILITY_NAME) {
        ret = true;
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
