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
AbilityDelegator::AbilityDelegator(const sptr<MainThread> &mainThread, std::unique_ptr<TestRunner> runner,
    const sptr<IRemoteObject> &observer)
    : mainThread_(mainThread), testRunner_(std::move(runner)), observer_(observer)
{}

AbilityDelegator::~AbilityDelegator()
{
    if (mainThread_) {
        auto app = mainThread_->GetApplication();
        if (app) {
            app->UnregisterAbilityLifecycleCallbacks(shared_from_this());
        }
    }
}

void AbilityDelegator::Init()
{
    APP_LOGI("Enter");

    if (!mainThread_) {
        APP_LOGE("Invalid mainThread");
        return;
    }

    auto app = mainThread_->GetApplication();
    if (!app) {
        APP_LOGE("Invalid application");
        return;
    }

    appContext_ = app->GetAppContext();
    app->RegisterAbilityLifecycleCallbacks(shared_from_this());
}

void AbilityDelegator::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    ProcessAbilityProperties(ability);
}

void AbilityDelegator::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{}

void AbilityDelegator::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    ProcessAbilityProperties(ability);
}

void AbilityDelegator::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    ProcessAbilityProperties(ability);
}

void AbilityDelegator::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{}

void AbilityDelegator::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    ProcessAbilityProperties(ability);
}

void AbilityDelegator::OnAbilitySaveState(const PacMap &outState)
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

    return GetAbilityToken(obtainedAbility);
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

    return GetAbilityToken(obtainedAbility);
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

    return std::get<THIRD_PROPERTY>(existedProperty.value());
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

    FinishUserTest(0);
}

bool AbilityDelegator::DoAbilityForeground(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return false;
    }

    if (AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityForeground(token)) {
        APP_LOGE("Failed to call DelegatorDoAbilityForeground");
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

    if (AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityBackground(token)) {
        APP_LOGE("Failed to call DelegatorDoAbilityBackground");
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
    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        APP_LOGW("Invalid testObserver");
        return;
    }

    testObserver->TestStatus(msg, 0);
}

void AbilityDelegator::PrePerformStart(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnAbilityStart();
    }
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
    }
}

void AbilityDelegator::PrePerformScenceCreated(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnWindowStageCreate();
    }
}

void AbilityDelegator::PrePerformScenceRestored(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnWindowStageRestore();
    }
}

void AbilityDelegator::PrePerformScenceDestroyed(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnWindowStageDestroy();
    }
}

void AbilityDelegator::PrePerformForeground(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnAbilityForeground();
    }
}

void AbilityDelegator::PrePerformBackground(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnAbilityBackground();
    }
}

void AbilityDelegator::PrePerformStop(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        APP_LOGW("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        monitor->Match(ability, {});
        monitor->OnAbilityStop();
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

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid ability");
        return;
    }

    auto abilityToken = GetAbilityToken(ability);
    if (!abilityToken) {
        APP_LOGE("Invalid ability token");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = DoesPropertyExist(abilityToken);
    if (existedProperty) {
        abilityProperties_.remove(existedProperty.value());
    }

    auto abilityState = ConvertAbilityState(ability->GetState());
    if (abilityState == AbilityDelegator::AbilityState::FOREGROUND) {
        abilityProperties_.emplace_front(abilityToken, ability, abilityState);
    } else {
        abilityProperties_.emplace_back(abilityToken, ability, abilityState);
    }
}

sptr<IRemoteObject> AbilityDelegator::GetAbilityToken(const std::shared_ptr<Ability> &ability)
{
    if (!ability) {
        APP_LOGW("Invalid ability");
        return {};
    }

    auto abilityContext = ability->GetAbilityContext();
    if (!abilityContext) {
        APP_LOGE("Invalid ability context");
        return {};
    }

    return abilityContext->GetToken();
}

std::optional<AbilityDelegator::ability_property> AbilityDelegator::DoesPropertyExist(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGW("Invalid input parameter");
        return std::nullopt;
    }

    for (auto &it : abilityProperties_) {
        auto tmpToken = std::get<FIRST_PROPERTY>(it);
        if (token == tmpToken) {
            return it;
        }
    }

    return std::nullopt;
}

void AbilityDelegator::FinishUserTest(const int32_t resultCode)
{
    APP_LOGI("Enter");

    if (!mainThread_) {
        APP_LOGE("Invalid mainThread");
        return;
    }

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

    mainThread_->FinishUserTest("UserTest finished", resultCode, bundleName, observer_);
}
}  // namespace AppExecFwk
}  // namespace OHOS
