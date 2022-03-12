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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ability_delegator_infos.h"
#include "iability_monitor.h"
#include "delegator_thread.h"
#include "shell_cmd_result.h"
#include "test_runner.h"

#include "ability_lifecycle_executor.h"
#include "ability_runtime/context/context.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegator : public std::enable_shared_from_this<AbilityDelegator> {
public:
    enum class AbilityState : uint8_t {
        UNINITIALIZED = 0,
        STARTED,
        FOREGROUND,
        BACKGROUND,
        STOPPED
    };

public:
    AbilityDelegator(const std::shared_ptr<AbilityRuntime::Context> &context, std::unique_ptr<TestRunner> runner,
        const sptr<IRemoteObject> &observer);
    ~AbilityDelegator();

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

    ErrCode StartAbility(const AAFwk::Want &want);

    bool DoAbilityForeground(const sptr<IRemoteObject> &token);
    bool DoAbilityBackground(const sptr<IRemoteObject> &token);

    std::unique_ptr<ShellCmdResult> ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec);

    void Print(const std::string &msg);

    void PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    void PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability);

    void FinishUserTest(const std::string &msg, const int32_t resultCode);

private:
    AbilityDelegator::AbilityState ConvertAbilityState(const AbilityLifecycleExecutor::LifecycleState lifecycleState);
    void ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability);
    std::shared_ptr<ADelegatorAbilityProperty> DoesPropertyExist(const sptr<IRemoteObject> &token);

private:
    std::shared_ptr<AbilityRuntime::Context> appContext_;
    std::unique_ptr<TestRunner> testRunner_;
    sptr<IRemoteObject> observer_;

    std::unique_ptr<DelegatorThread> delegatorThread_;
    std::list<std::shared_ptr<ADelegatorAbilityProperty>> abilityProperties_;
    std::vector<std::shared_ptr<IAbilityMonitor>> abilityMonitors_;

    std::mutex mutexMonitor_;
    std::mutex mutexAbilityProperties_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_H
