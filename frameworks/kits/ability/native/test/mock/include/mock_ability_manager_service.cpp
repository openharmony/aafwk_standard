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

#include "mock_ability_manager_service.h"
#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
MockAbilityManagerService::MockAbilityManagerService() : abilityScheduler_(nullptr)
{}

MockAbilityManagerService::~MockAbilityManagerService()
{}

int MockAbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    AbilityLifeCycleState state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    RequestCode request = static_cast<RequestCode>(requestCode);
    switch (request) {
        // Test code, representing the life cycle: Ability_ STATE_ INITIAL
        case RequestCode::E_STATE_INITIAL:
            state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
            break;
        // Test code, representing the life cycle: ABILITY_STATE_INACTIVE
        case RequestCode::E_STATE_INACTIVE:
            state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
            break;
        // Test code, representing the life cycle: ABILITY_STATE_ACTIVE
        case RequestCode::E_STATE_ACTIVE:
            state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
            break;
        // Test code, representing the life cycle: ABILITY_STATE_BACKGROUND
        case RequestCode::E_STATE_BACKGROUND:
            state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND;
            break;
        default:
            break;
    }

    if (abilityScheduler_ != nullptr) {
        want_ = want;
        want_.SetElementName("BundleName", "abilityName");
        LifeCycleStateInfo stateInfo;
        stateInfo.state = state;
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);
    }

    return 0;
}

int MockAbilityManagerService::TerminateAbility(
    const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    GTEST_LOG_(INFO) << "MockAbilityManagerService::TerminateAbility";

    if (abilityScheduler_ != nullptr) {
        LifeCycleStateInfo stateInfo;
        stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
        abilityScheduler_->ScheduleAbilityTransaction(want_, stateInfo);

        int ceode = 250;
        abilityScheduler_->SendResult(ceode, resultCode, *resultWant);
    }
    return 0;
}

int MockAbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    GTEST_LOG_(INFO) << "MockAbilityManagerService::ConnectAbility";
    return ERR_OK;
}

int MockAbilityManagerService::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    GTEST_LOG_(INFO) << "MockAbilityManagerService::DisconnectAbility";
    return ERR_OK;
}

int MockAbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    abilityScheduler_ = scheduler;
    EXPECT_NE(nullptr, token);
    return 0;
}

void MockAbilityManagerService::DumpState(const std::string &args, std::vector<std::string> &info)
{}

void MockAbilityManagerService::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{}

int MockAbilityManagerService::AbilityTransitionDone(
    const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    return 0;
}

int MockAbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    return 0;
}

int MockAbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    return 0;
}

int MockAbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    return 0;
}

int MockAbilityManagerService::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    GTEST_LOG_(INFO) << "MockAbilityManagerService::TerminateAbilityResult";
    return ERR_OK;
}

int MockAbilityManagerService::StopServiceAbility(const Want &want, int32_t userId)
{
    GTEST_LOG_(INFO) << "MockAbilityManagerService::StopServiceAbility";
    return ERR_OK;
}

int MockAbilityManagerService::KillProcess(const std::string &bundleName)
{
    return 0;
}

int MockAbilityManagerService::UninstallApp(const std::string &bundleName, int32_t uid)
{
    return 0;
}
}  // namespace AAFwk
}  // namespace OHOS
