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
#include "hilog_wrapper.h"
#include "ability_util.h"
#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
LifecycleDeal::LifecycleDeal()
{}

LifecycleDeal::~LifecycleDeal()
{}

void LifecycleDeal::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    abilityScheduler_ = scheduler;
}

void LifecycleDeal::Activate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Activate.");
    CHECK_POINTER(abilityScheduler_);
    HILOG_INFO("caller %{public}s, %{public}s, %{public}s",
        stateInfo.caller.deviceId.c_str(),
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
    abilityScheduler_->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::Inactivate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Inactivate.");
    CHECK_POINTER(abilityScheduler_);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
    abilityScheduler_->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::MoveToBackground(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Move to background.");
    CHECK_POINTER(abilityScheduler_);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND;
    abilityScheduler_->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::ConnectAbility(const Want &want)
{
    HILOG_INFO("Connect ability.");
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleConnectAbility(want);
}

void LifecycleDeal::DisconnectAbility(const Want &want)
{
    HILOG_INFO("Disconnect ability.");
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleDisconnectAbility(want);
}

void LifecycleDeal::Terminate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    HILOG_INFO("Terminate.");
    CHECK_POINTER(abilityScheduler_);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    abilityScheduler_->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::CommandAbility(const Want &want, bool reStart, int startId)
{
    HILOG_INFO("Command ability.");
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleCommandAbility(want, reStart, startId);
}

void LifecycleDeal::SaveAbilityState()
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleSaveAbilityState();
}

void LifecycleDeal::RestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleRestoreAbilityState(inState);
}

void LifecycleDeal::UpdateConfiguration(const DummyConfiguration &config)
{
    HILOG_INFO("%{public}s, %{public}d", __func__, __LINE__);
    CHECK_POINTER(abilityScheduler_);
    abilityScheduler_->ScheduleUpdateConfiguration(config);
}
}  // namespace AAFwk
}  // namespace OHOS
