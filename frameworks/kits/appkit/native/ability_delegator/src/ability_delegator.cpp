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

#include "ability_delegator.h"

#include "hilog_wrapper.h"
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
        HILOG_WARN("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        HILOG_WARN("Monitor has been added");
        return;
    }

    abilityMonitors_.emplace_back(monitor);
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        HILOG_WARN("Invalid input parameter");
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

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        HILOG_WARN("Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility();
    if (!obtainedAbility) {
        HILOG_WARN("Invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
        HILOG_WARN("Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility(timeoutMs);
    if (!obtainedAbility) {
        HILOG_WARN("Invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<AbilityRuntime::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const sptr<IRemoteObject> &token)
{
    if (!token) {
        HILOG_WARN("Invalid input parameter");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(token);
    if (!existedProperty) {
        HILOG_WARN("Unknown ability token");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    return ConvertAbilityState(existedProperty->lifecycleState_);
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::GetCurrentTopAbility()
{
    sptr<IRemoteObject> topAbilityToken;
    if (AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility(topAbilityToken)) {
        HILOG_ERROR("Failed to get top ability");
        return {};
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(topAbilityToken);
    if (!existedProperty) {
        HILOG_WARN("Unknown ability token");
        return {};
    }

    return existedProperty;
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
    HILOG_INFO("Enter");
    if (!testRunner_) {
        HILOG_WARN("Invalid TestRunner");
        return;
    }

    HILOG_INFO("Call TestRunner::Prepare()");
    testRunner_->Prepare();

    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            HILOG_ERROR("Create delegatorThread failed");
            return;
        }
    }

    auto runTask = [this]() { this->OnRun(); };
    if (!delegatorThread_->Run(runTask)) {
        HILOG_ERROR("Run task on delegatorThread failed");
    }
}

void AbilityDelegator::OnRun()
{
    HILOG_INFO("Enter");
    if (!testRunner_) {
        HILOG_WARN("Invalid TestRunner");
        return;
    }

    HILOG_INFO("Call TestRunner::Run(), Start run");
    testRunner_->Run();
    HILOG_INFO("Run finished");
}

ErrCode AbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    HILOG_INFO("Enter");

    auto realWant(want);
    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (delegatorArgs && delegatorArgs->FindDebugFlag()) {
        HILOG_INFO("Set Debug to the want which used for starting the ability");
        realWant.SetParam("debugApp", true);
    }

    return AbilityManagerClient::GetInstance()->StartAbility(realWant);
}

bool AbilityDelegator::DoAbilityForeground(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Enter");

    if (!token) {
        HILOG_WARN("Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityForeground(token);
    if (ret) {
        HILOG_ERROR("Failed to call DelegatorDoAbilityForeground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

bool AbilityDelegator::DoAbilityBackground(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Enter");

    if (!token) {
        HILOG_WARN("Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityBackground(token);
    if (ret) {
        HILOG_ERROR("Failed to call DelegatorDoAbilityBackground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

std::unique_ptr<ShellCmdResult> AbilityDelegator::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec)
{
    HILOG_INFO("command : %{public}s, timeout : %{public}" PRId64, cmd.data(), timeoutSec);

    if (cmd.empty()) {
        HILOG_ERROR("Invalid cmd");
        return {};
    }

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        HILOG_WARN("Invalid testObserver");
        return {};
    }

    auto result = testObserver->ExecuteShellCommand(cmd, timeoutSec);
    return std::make_unique<ShellCmdResult>(result);
}

void AbilityDelegator::Print(const std::string &msg)
{
    HILOG_INFO("Enter");

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        HILOG_WARN("Invalid testObserver");
        return;
    }

    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
        HILOG_WARN("Too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
    }
    HILOG_INFO("message to print : %{public}s", realMsg.data());

    testObserver->TestStatus(realMsg, 0);
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStart(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageCreate(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageRestore(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageDestroy(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityForeground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityBackground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityStop(ability->object_);
        }
    }

    RemoveAbilityProperty(ability);
    CallClearFunc(ability);
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
            abilityState = AbilityDelegator::AbilityState::STOPPED;
            break;
        default:
            HILOG_ERROR("Unknown lifecycleState");
            break;
    }

    return abilityState;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid ability property");
        return;
    }

    HILOG_WARN("ability property : name : %{public}s, state : %{public}d",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(ability->token_);
    if (existedProperty) {
        // update
        existedProperty->lifecycleState_ = ability->lifecycleState_;
        return;
    }

    abilityProperties_.emplace_back(ability);
}

void AbilityDelegator::RemoveAbilityProperty(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");

    if (!ability) {
        HILOG_WARN("Invalid ability property");
        return;
    }

    HILOG_INFO("ability property { name : %{public}s, state : %{public}d }",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    abilityProperties_.remove(ability);
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::FindPropertyByToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Enter");

    if (!token) {
        HILOG_WARN("Invalid input parameter");
        return {};
    }

    for (auto &it : abilityProperties_) {
        if (!it) {
            HILOG_WARN("Invalid ability property");
            continue;
        }

        if (token == it->token_) {
            HILOG_INFO("Porperty exists");
            return it;
        }
    }

    return {};
}

void AbilityDelegator::FinishUserTest(const std::string &msg, const int64_t resultCode)
{
    HILOG_INFO("Enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);

    if (!observer_) {
        HILOG_ERROR("Invalid observer");
        return;
    }

    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (!delegatorArgs) {
        HILOG_ERROR("Invalid delegator args");
        return;
    }

    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
        HILOG_WARN("Too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
    }

    const auto &bundleName = delegatorArgs->GetTestBundleName();
    auto err = AAFwk::AbilityManagerClient::GetInstance()->FinishUserTest(realMsg, resultCode, bundleName);
    if (err) {
        HILOG_ERROR("Failed to call FinishUserTest : %{public}d", err);
    }
}

void AbilityDelegator::RegisterClearFunc(ClearFunc func)
{
    HILOG_INFO("Enter");
    if (!func) {
        HILOG_ERROR("Invalid func");
        return;
    }

    clearFunc_ = func;
}

inline void AbilityDelegator::CallClearFunc(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter");
    if (clearFunc_) {
        clearFunc_(ability);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
