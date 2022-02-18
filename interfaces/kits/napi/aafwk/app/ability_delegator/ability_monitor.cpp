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

#include "ability_monitor.h"

#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
AbilityMonitor::AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor)
    : IAbilityMonitor(name), jsMonitor_(jsAbilityMonitor)
{}

void AbilityMonitor::OnAbilityStart()
{
    HILOG_INFO("onAbilityCreate is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onAbilityCreate();

    HILOG_INFO("onAbilityCreate is called end");
}

void AbilityMonitor::OnAbilityForeground()
{
    HILOG_INFO("onAbilityForeground is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onAbilityForeground();

    HILOG_INFO("onAbilityForeground is called end");
}

void AbilityMonitor::OnAbilityBackground()
{
    HILOG_INFO("onAbilityBackground is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onAbilityBackground();

    HILOG_INFO("onAbilityBackground is called end");
}

void AbilityMonitor::OnAbilityStop()
{
    HILOG_INFO("onAbilityDestroy is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onAbilityDestroy();

    HILOG_INFO("onAbilityDestroy is called end");
}

void AbilityMonitor::OnWindowStageCreate()
{
    HILOG_INFO("onWindowStageCreate is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onWindowStageCreate();

    HILOG_INFO("onWindowStageCreate is called end");
}

void AbilityMonitor::OnWindowStageRestore()
{
    HILOG_INFO("onWindowStageRestore is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onWindowStageRestore();

    HILOG_INFO("onWindowStageRestore is called end");
}

void AbilityMonitor::OnWindowStageDestroy()
{
    HILOG_INFO("onWindowStageDestroy is called");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->onWindowStageDestroy();

    HILOG_INFO("onWindowStageDestroy is called end");
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
