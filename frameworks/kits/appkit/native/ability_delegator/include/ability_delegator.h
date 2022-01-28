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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H

#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "iability_monitor.h"
#include "delegator_thread.h"
#include "shell_cmd_result.h"
#include "test_runner.h"

#include "ability_lifecycle_callbacks.h"
#include "ability_lifecycle_executor.h"
#include "ability_runtime/context/context.h"
#include "main_thread.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegator : public AbilityLifecycleCallbacks, public std::enable_shared_from_this<AbilityDelegator> {
public:
    enum class AbilityState : uint8_t {
        UNINITIALIZED = 0,
        STARTED,
        FOREGROUND,
        BACKGROUND,
        STOPED
    };

    using ability_property = std::tuple<sptr<IRemoteObject>, std::shared_ptr<Ability>, AbilityDelegator::AbilityState>;
    using list_ability_property = std::list<ability_property>;

public:
    AbilityDelegator(const sptr<MainThread> &mainThread, std::unique_ptr<TestRunner> runner,
        const sptr<IRemoteObject> &observer);
    ~AbilityDelegator();

    void Init();

    void OnAbilityStart(const std::shared_ptr<Ability> &ability) override;
    void OnAbilityInactive(const std::shared_ptr<Ability> &ability) override;
    void OnAbilityBackground(const std::shared_ptr<Ability> &ability) override;
    void OnAbilityForeground(const std::shared_ptr<Ability> &ability) override;
    void OnAbilityActive(const std::shared_ptr<Ability> &ability) override;
    void OnAbilityStop(const std::shared_ptr<Ability> &ability) override;
    void OnAbilitySaveState(const PacMap &outState) override;

    void AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);
    void RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);
    void ClearAllMonitors();
    size_t GetMonitorsNum();

    sptr<IRemoteObject> WaitAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);
    sptr<IRemoteObject> WaitAbilityMonitor(
        const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs);

    std::shared_ptr<AbilityRuntime::Context> GetAppContext() const;
    AbilityDelegator::AbilityState GetAbilityState(const sptr<IRemoteObject> &token);
    sptr<IRemoteObject> GetCurrentTopAbility();
    std::string GetThreadName() const;

    void Prepare();
    void OnRun();

    bool DoAbilityForeground(const sptr<IRemoteObject> &token);
    bool DoAbilityBackground(const sptr<IRemoteObject> &token);

    std::unique_ptr<ShellCmdResult> ExecuteShellCommand(const std::string &cmd, const int64_t timeoutMs);

    void Print(const std::string &msg);

    void PrePerformStart(const std::shared_ptr<Ability> &ability);
    void PostPerformStart(const std::shared_ptr<Ability> &ability);
    void PrePerformScenceCreated(const std::shared_ptr<Ability> &ability);
    void PrePerformScenceRestored(const std::shared_ptr<Ability> &ability);
    void PrePerformScenceDestroyed(const std::shared_ptr<Ability> &ability);
    void PrePerformForeground(const std::shared_ptr<Ability> &ability);
    void PrePerformBackground(const std::shared_ptr<Ability> &ability);
    void PrePerformStop(const std::shared_ptr<Ability> &ability);

private:
    AbilityDelegator::AbilityState ConvertAbilityState(const AbilityLifecycleExecutor::LifecycleState lifecycleState);
    void ProcessAbilityProperties(const std::shared_ptr<Ability> &ability);
    sptr<IRemoteObject> GetAbilityToken(const std::shared_ptr<Ability> &ability);
    std::optional<ability_property> DoesPropertyExist(const sptr<IRemoteObject> &token);
    void FinishUserTest(const int32_t resultCode);

private:
    static constexpr size_t FIRST_PROPERTY  {0};
    static constexpr size_t SECOND_PROPERTY {1};
    static constexpr size_t THIRD_PROPERTY  {2};

private:
    sptr<MainThread> mainThread_;
    std::unique_ptr<TestRunner> testRunner_;
    std::unique_ptr<DelegatorThread> delegatorThread_;
    list_ability_property abilityProperties_;
    std::vector<std::shared_ptr<IAbilityMonitor>> abilityMonitors_;
    std::shared_ptr<AbilityRuntime::Context> appContext_;
    sptr<IRemoteObject> observer_;

    std::mutex mutexMonitor_;
    std::mutex mutexAbilityProperties_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H
