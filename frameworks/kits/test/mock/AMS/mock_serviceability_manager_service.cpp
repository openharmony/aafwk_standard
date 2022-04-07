/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "mock_serviceability_manager_service.h"
#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
MockServiceAbilityManagerService::MockServiceAbilityManagerService()
{}

MockServiceAbilityManagerService::~MockServiceAbilityManagerService()
{}

int MockServiceAbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::StartAbility";
    if (abilityScheduler_ != nullptr) {
        startAbility = true;
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        want_.SetElementName("BundleName", "abilityName");
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
        return ERR_OK;
    }
    return 0;
}

int MockServiceAbilityManagerService::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    return 0;
}

int MockServiceAbilityManagerService::TerminateAbility(
    const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::TerminateAbility";
    if (abilityScheduler_ != nullptr) {
        startAbility = false;
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
        want_.SetElementName("BundleName", "abilityName");
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
        return ERR_OK;
    }
    return 0;
}

int MockServiceAbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::connectAbility";
    if (abilityScheduler_ != nullptr) {
        startAbility = false;
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        want_.SetElementName("BundleName", "abilityName");
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
        return ERR_OK;
    }
    return -1;
}

int MockServiceAbilityManagerService::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::DisconnectAbility";
    if (abilityScheduler_ != nullptr) {
        Want want;
        want.SetElementName("BundleName", "abilityName");
        abilityScheduler_->ScheduleDisconnectAbility(want_);
        return ERR_OK;
    }
    return 0;
}

int MockServiceAbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    abilityScheduler_ = scheduler;
    EXPECT_NE(nullptr, token);
    return 0;
}

void MockServiceAbilityManagerService::DumpState(const std::string &args, std::vector<std::string> &info)
{}

void MockServiceAbilityManagerService::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{}

int MockServiceAbilityManagerService::AbilityTransitionDone(
    const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::AbilityTransitionDone startAbility is " << startAbility;
    want_.SetElementName("BundleName", "abilityName");
    if (abilityScheduler_ != nullptr && state == AAFwk::ABILITY_STATE_INACTIVE) {
        want_.SetElementName("BundleName", "abilityName");
        if (startAbility) {
            abilityScheduler_->ScheduleCommandAbility(want_, false, -1);
        } else {
            abilityScheduler_->ScheduleConnectAbility(want_);
        }
    }
    return 0;
}

int MockServiceAbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::ScheduleConnectAbilityDone";
    return 0;
}

int MockServiceAbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::ScheduleDisconnectAbilityDone";

    if (abilityScheduler_ != nullptr) {
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
        return ERR_OK;
    }
    return 0;
}

int MockServiceAbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    return 0;
}

int MockServiceAbilityManagerService::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    return 0;
}

int MockServiceAbilityManagerService::TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    return 0;
}

int MockServiceAbilityManagerService::StopServiceAbility(const Want &want, int32_t userId)
{
    GTEST_LOG_(INFO) << "MockServiceAbilityManagerService::StopServiceAbility";
    if (abilityScheduler_ != nullptr) {
        startAbility = false;
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
        want_.SetElementName("BundleName", "abilityName");
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
        return ERR_OK;
    }
    return -1;
}
}  // namespace AAFwk
}  // namespace OHOS
