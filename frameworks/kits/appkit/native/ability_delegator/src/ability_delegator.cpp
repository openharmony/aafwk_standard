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

#include "ability_delegator.h"

#include "app_log_wrapper.h"
#include "ohos_application.h"
#include "ability_manager_client.h"
#include "ability_delegator_registry.h"
#include "itest_observer.h"

namespace OHOS {
namespace AppExecFwk {
AbilityDelegator::AbilityDelegator(const std::shared_ptr<AbilityRuntime::Context> &context,
    std::unique_ptr<TestRunner> runner, const sptr<IRemoteObject> &observer)
    : appContext_(context), testRunner_(std::move(runner)), observer_(observer)
{}

AbilityDelegator::~AbilityDelegator()
{}

void AbilityDelegator::AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        APP_LOGW("Monitor has been added");
        return;
    }

    abilityMonitors_.emplace_back(monitor);
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        abilityMonitors_.erase(pos);
    }
}

void AbilityDelegator::ClearAllMonitors()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    abilityMonitors_.clear();
}

size_t AbilityDelegator::GetMonitorsNum()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    return abilityMonitors_.size();
}

sptr<IRemoteObject> AbilityDelegator::WaitAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        APP_LOGW("Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->waitForAbility();
    if (!obtainedAbility) {
        APP_LOGW("Invalid obtained ability");
        return {};
    }

    return obtainedAbility->token_;
}

sptr<IRemoteObject> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
        APP_LOGW("Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->waitForAbility(timeoutMs);
    if (!obtainedAbility) {
        APP_LOGW("Invalid obtained ability");
        return {};
    }

    return obtainedAbility->token_;
}

std::shared_ptr<AbilityRuntime::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = DoesPropertyExist(token);
    if (!existedProperty) {
        APP_LOGW("Unknown ability token");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    return ConvertAbilityState(existedProperty->lifecycleState_);
}

sptr<IRemoteObject> AbilityDelegator::GetCurrentTopAbility()
{
    sptr<IRemoteObject> topAbilityToken;
    if (AAFwk::AbilityManagerClient::GetInstance()->GetCurrentTopAbility(topAbilityToken)) {
        APP_LOGE("Failed to get top ability");
        return {};
    }

    return topAbilityToken;
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
    APP_LOGI("Enter");
    if (!testRunner_) {
        APP_LOGW("Invalid TestRunner");
        return;
    }

    APP_LOGI("Call TestRunner::Prepare()");
    testRunner_->Prepare();

    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            APP_LOGE("Create delegatorThread failed");
            return;
        }
    }

    auto runTask = [this]() { this->OnRun(); };
    if (!delegatorThread_->Run(runTask)) {
        APP_LOGE("Run task on delegatorThread failed");
    }
}

void AbilityDelegator::OnRun()
{
    APP_LOGI("Enter");
    if (!testRunner_) {
        APP_LOGW("Invalid TestRunner");
        return;
    }

    APP_LOGI("Call TestRunner::Run(), Start run");
    testRunner_->Run();
    APP_LOGI("Run finished");
}

bool AbilityDelegator::DoAbilityForeground(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityForeground(token);
    if (ret) {
        APP_LOGE("Failed to call DelegatorDoAbilityForeground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

bool AbilityDelegator::DoAbilityBackground(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityBackground(token);
    if (ret) {
        APP_LOGE("Failed to call DelegatorDoAbilityBackground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

std::unique_ptr<ShellCmdResult> AbilityDelegator::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutMs)
{
    APP_LOGI("command : %{public}s, timeout : %{public}" PRId64, cmd.data(), timeoutMs);

    if (cmd.empty()) {
        APP_LOGE("Invalid cmd");
        return {};
    }

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        APP_LOGW("Invalid testObserver");
        return {};
    }

    auto result = testObserver->ExecuteShellCommand(cmd, timeoutMs);
    return std::make_unique<ShellCmdResult>(result);
}

void AbilityDelegator::Print(const std::string &msg)
{
    APP_LOGI("message to print : %{public}s", msg.data());
    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        APP_LOGW("Invalid testObserver");
        return;
    }

    testObserver->TestStatus(msg, 0);
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStart();
        }
    }
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageCreate();
        }
    }
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageRestore();
        }
    }
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageDestroy();
        }
    }
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityForeground();
        }
    }
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityBackground();
        }
    }
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityStop();
        }
    }
}

AbilityDelegator::AbilityState AbilityDelegator::ConvertAbilityState(
    const AbilityLifecycleExecutor::LifecycleState lifecycleState)
{
    AbilityDelegator::AbilityState abilityState {AbilityDelegator::AbilityState::UNINITIALIZED};
    switch (lifecycleState) {
        case AbilityLifecycleExecutor::LifecycleState::STARTED_NEW:
            abilityState = AbilityDelegator::AbilityState::STARTED;
            break;
        case AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::FOREGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::BACKGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::STOPED_NEW:
            abilityState = AbilityDelegator::AbilityState::STOPED;
            break;
        default:
            APP_LOGE("Unknown lifecycleState");
            break;
    }

    return abilityState;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid ability property");
        return;
    }

    APP_LOGW("ability property : name : %{public}s, state : %{public}d",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = DoesPropertyExist(ability->token_);
    if (existedProperty) {
        // update
        existedProperty->lifecycleState_ = ability->lifecycleState_;
        return;
    }

    abilityProperties_.emplace_back(ability);
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::DoesPropertyExist(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return {};
    }

    for (auto &it : abilityProperties_) {
        if (!it) {
            APP_LOGW("Invalid ability property");
            continue;
        }

        if (token == it->token_) {
            APP_LOGI("Porperty exists");
            return it;
        }
    }

    return {};
}

void AbilityDelegator::FinishUserTest(const std::string &msg, const int32_t resultCode)
{
    APP_LOGI("Enter");

    if (!observer_) {
        APP_LOGE("Invalid observer");
        return;
    }

    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (!delegatorArgs) {
        APP_LOGE("Invalid delegator args");
        return;
    }

    const auto &bundleName = delegatorArgs->GetTestBundleName();
    auto err = AAFwk::AbilityManagerClient::GetInstance()->FinishUserTest(msg, resultCode, bundleName, observer_);
    if (err) {
        APP_LOGE("MainThread::FinishUserTest is failed %{public}d", err);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
