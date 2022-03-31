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
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "mock_bundle_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
    const std::string DEVICE_ID = "15010038475446345206a332922cb765";
    const std::string BUNDLE_NAME = "testBundle";
    const std::string NAME = ".testMainAbility";
}

namespace OHOS {
namespace AAFwk {
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

#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

namespace {
const std::string WaitUntilTaskFinishedByTimer = "BundleMgrService";
}  // namespace

class AbilityManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartAms();
    void OnStopAms();
    int StartAbility(const Want &want);
    static constexpr int TEST_WAIT_TIME = 100000;

public:
    AbilityRequest abilityRequest_;
    std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

int AbilityManagerServiceTest::StartAbility(const Want &want)
{
    int ref = -1;
    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();
    if (topAbility) {
        topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    ref = abilityMs_->StartAbility(want);
    WaitUntilTaskFinished();
    return ref;
}

void AbilityManagerServiceTest::OnStartAms()
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;

        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);

        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
        abilityMs_->connectManager_ = std::make_shared<AbilityConnectManager>();
        abilityMs_->connectManagers_.emplace(0, abilityMs_->connectManager_);
        EXPECT_TRUE(abilityMs_->handler_);
        EXPECT_TRUE(abilityMs_->connectManager_);

        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        abilityMs_->dataAbilityManagers_.emplace(0, abilityMs_->dataAbilityManager_);
        EXPECT_TRUE(abilityMs_->dataAbilityManager_);

        abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs_->amsConfigResolver_);
        abilityMs_->amsConfigResolver_->Parse();

        abilityMs_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs_->pendingWantManager_);

        abilityMs_->currentMissionListManager_ = std::make_shared<MissionListManager>(0);
        abilityMs_->currentMissionListManager_->Init();
        abilityMs_->eventLoop_->Run();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityManagerServiceTest::OnStopAms()
{
    abilityMs_->OnStop();
}

void AbilityManagerServiceTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityManagerServiceTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityManagerServiceTest::SetUp()
{
    OnStartAms();
    WaitUntilTaskFinished();
    if (abilityRecord_ == nullptr) {
        abilityRequest_.appInfo.bundleName = "data.client.bundle";
        abilityRequest_.abilityInfo.name = "ClientAbility";
        abilityRequest_.abilityInfo.type = AbilityType::DATA;
        abilityRecord_ = AbilityRecord::CreateAbilityRecord(abilityRequest_);
    }
}

void AbilityManagerServiceTest::TearDown()
{
    OnStopAms();
}

/**
 * @tc.name: CheckCallPermissions_001
 * @tc.desc: Verify function CheckCallPermissions return RESOLVE_CALL_NO_PERMISSIONS
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallPermissions_001, TestSize.Level1)
{
    abilityRequest_.callerUid = 0;
    EXPECT_EQ(RESOLVE_CALL_NO_PERMISSIONS, abilityMs_->CheckCallPermissions(abilityRequest_));
}

/**
 * @tc.name: CheckCallPermissions_002
 * @tc.desc: Verify function CheckCallPermissions return ERR_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallPermissions_002, TestSize.Level1)
{
    abilityRequest_.callerUid = 1000;
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(ERR_OK, abilityMs_->CheckCallPermissions(abilityRequest_));
}
}
}