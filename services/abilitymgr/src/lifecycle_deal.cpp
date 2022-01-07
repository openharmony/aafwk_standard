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

#include "lifecycle_deal.h"

#include "ability_record.h"
#include "ability_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
LifecycleDeal::LifecycleDeal()
{}

LifecycleDeal::~LifecycleDeal()
{}

void LifecycleDeal::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    std::unique_lock<std::shared_mutex> lock(schedulerMutex_);
    abilityScheduler_ = scheduler;
}

sptr<IAbilityScheduler> LifecycleDeal::GetScheduler()
{
    std::shared_lock<std::shared_mutex> lock(schedulerMutex_);
    return abilityScheduler_;
}

void LifecycleDeal::Activate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Activate.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    HILOG_INFO("caller %{public}s, %{public}s, %{public}s",
        stateInfo.caller.deviceId.c_str(),
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::Inactivate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Inactivate.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::MoveToBackground(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Move to background.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::ConnectAbility(const Want &want)
{
    HILOG_INFO("Connect ability.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleConnectAbility(want);
}

void LifecycleDeal::DisconnectAbility(const Want &want)
{
    HILOG_INFO("Disconnect ability.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleDisconnectAbility(want);
}

void LifecycleDeal::Terminate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Terminate.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::CommandAbility(const Want &want, bool reStart, int startId)
{
    HILOG_INFO("Command ability. startId:%{public}d", startId);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleCommandAbility(want, reStart, startId);
}

void LifecycleDeal::SaveAbilityState()
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleSaveAbilityState();
}

void LifecycleDeal::RestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleRestoreAbilityState(inState);
}

void LifecycleDeal::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleUpdateConfiguration(config);
}

void LifecycleDeal::ForegroundNew(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("ForegroundNew.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    HILOG_INFO("caller %{public}s, %{public}s, %{public}s",
        stateInfo.caller.deviceId.c_str(),
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::BackgroundNew(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Go background.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    HILOG_INFO("caller %{public}s, %{public}s, %{public}s",
        stateInfo.caller.deviceId.c_str(),
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::ContinueAbility(const std::string& deviceId)
{
    HILOG_INFO("ContinueAbility.");
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ContinueAbility(deviceId);
}

void LifecycleDeal::NotifyContinuationResult(const int32_t result)
{
    HILOG_INFO("NotifyContinuationResult.");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->NotifyContinuationResult(result);
}
}  // namespace AAFwk
}  // namespace OHOS
