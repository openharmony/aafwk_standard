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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#include "ams_configuration_parameter.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#include "os_account_info.h"
#endif // OS_ACCOUNT_PART_ENABLED
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
#ifdef OS_ACCOUNT_PART_ENABLED
using namespace OHOS::AccountSA;
#endif // OS_ACCOUNT_PART_ENABLED
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
constexpr int32_t DISPLAY_ID = 256;

static void WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}
}  // namespace

class StartOptionDisplayIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartAms();
    void OnStopAms();

public:
    std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
};

void StartOptionDisplayIdTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void StartOptionDisplayIdTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void StartOptionDisplayIdTest::SetUp()
{
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartAms();

    auto missionListMgr = abilityMs_->GetListManagerByUserId(USER_ID_U100);
    if (!missionListMgr) {
        return;
    }

    auto topAbility = missionListMgr->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND);
    }
}

void StartOptionDisplayIdTest::TearDown()
{
    OnStopAms();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void StartOptionDisplayIdTest::OnStartAms()
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;

        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);

        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
        EXPECT_TRUE(abilityMs_->handler_);

        // init user controller.
        abilityMs_->userController_ = std::make_shared<UserController>();
        EXPECT_TRUE(abilityMs_->userController_);
        abilityMs_->userController_->Init();
        int userId = USER_ID_U100;
        abilityMs_->userController_->SetCurrentUserId(userId);
        abilityMs_->InitConnectManager(userId, true);
        abilityMs_->InitDataAbilityManager(userId, true);
        abilityMs_->InitPendWantManager(userId, true);
        abilityMs_->systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(abilityMs_->systemDataAbilityManager_);

        abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs_->amsConfigResolver_);
        abilityMs_->amsConfigResolver_->Parse();

        abilityMs_->InitMissionListManager(userId, true);
        abilityMs_->connectManager_->SetEventHandler(abilityMs_->handler_);
        abilityMs_->eventLoop_->Run();

        WaitUntilTaskFinished();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void StartOptionDisplayIdTest::OnStopAms()
{
    abilityMs_->eventLoop_.reset();
    abilityMs_->handler_.reset();
    abilityMs_->state_ = ServiceRunningState::STATE_NOT_START;
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the diplayId is correctly parsed test.
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_display_id_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMs_->StartAbility(want, option, nullptr);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        auto defualtDisplayId = 0;
        auto displayId = topAbility->GetWant().GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        EXPECT_EQ(displayId, DISPLAY_ID);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that displayId does not support service startup.
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_display_id_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMs_->StartAbility(want, option, nullptr);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}
}  // namespace AAFwk
}  // namespace OHOS
