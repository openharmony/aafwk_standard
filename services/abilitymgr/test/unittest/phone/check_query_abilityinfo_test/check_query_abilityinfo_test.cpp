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
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string COM_OHOS_Test = "com.ohos.test";
const std::string MAIN_ABILITY = ".MainAbility";
const std::string MY_APPLICATION = ".MyApplication";
const std::string DESCRIPTION = "$string:mainability_description";
const std::string LABEL = "$string:label";
const std::string ICON = "$media:icon";
const std::string THEME = "mytheme";
const std::string KIND = "page";
const std::string PROCESS = "p8";
const std::string READ_PERMISSION = "readPermission";
const std::string WRITE_PERMISSION = "writePermission";
const std::string RESOURCE_PATH = "/data/app/com.ohos.test";
const std::string SRC_PATH = "/resources/base/profile";
const std::string SRC_LANGUAGE = "C++";
const uint32_t NUMBER_ONE = 10001;
const uint32_t NUMBER_TWO = 10002;
const uint32_t NUMBER_THREE = 10003;
}  // namespace

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
class CheckQueryAbilityinfoTest : public testing::Test {
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

int CheckQueryAbilityinfoTest::StartAbility(const Want &want)
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

void CheckQueryAbilityinfoTest::OnStartAms()
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

void CheckQueryAbilityinfoTest::OnStopAms()
{
    abilityMs_->OnStop();
}

void CheckQueryAbilityinfoTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void CheckQueryAbilityinfoTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void CheckQueryAbilityinfoTest::SetUp()
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

void CheckQueryAbilityinfoTest::TearDown()
{
    OnStopAms();
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryAbilityInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Test the abilityInfo query in StartAbility.
 */
HWTEST_F(CheckQueryAbilityinfoTest, CheckQueryAbilityinfo_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ohos.test", "com.ohos.test.MainAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, abilityMs_->QueryServiceState());
    auto stackManager = abilityMs_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    EXPECT_EQ(topAbility->GetAbilityInfo().name, COM_OHOS_Test + MAIN_ABILITY);
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().applicationInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().applicationName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().applicationInfo.name, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, COM_OHOS_Test);
    EXPECT_EQ(topAbility->GetAbilityInfo().moduleName, MY_APPLICATION);
    EXPECT_EQ(topAbility->GetAbilityInfo().kind, KIND);
    EXPECT_EQ(topAbility->GetAbilityInfo().process, PROCESS);
    EXPECT_EQ(topAbility->GetAbilityInfo().labelId, NUMBER_ONE);
    EXPECT_EQ(topAbility->GetAbilityInfo().label, LABEL);
    EXPECT_EQ(topAbility->GetAbilityInfo().descriptionId, NUMBER_TWO);
    EXPECT_EQ(topAbility->GetAbilityInfo().description, DESCRIPTION);
    EXPECT_EQ(topAbility->GetAbilityInfo().iconId, NUMBER_THREE);
    EXPECT_EQ(topAbility->GetAbilityInfo().iconPath, ICON);
    EXPECT_EQ(topAbility->GetAbilityInfo().theme, THEME);
    EXPECT_EQ(topAbility->GetAbilityInfo().readPermission, READ_PERMISSION);
    EXPECT_EQ(topAbility->GetAbilityInfo().writePermission, WRITE_PERMISSION);
    EXPECT_EQ(topAbility->GetAbilityInfo().resourcePath, RESOURCE_PATH);
    EXPECT_EQ(topAbility->GetAbilityInfo().srcPath, SRC_PATH);
    EXPECT_EQ(topAbility->GetAbilityInfo().srcLanguage, SRC_LANGUAGE);
    EXPECT_FALSE(topAbility->GetAbilityInfo().multiUserShared);
    EXPECT_FALSE(topAbility->GetAbilityInfo().supportPipMode);
    EXPECT_FALSE(topAbility->GetAbilityInfo().grantPermission);
    EXPECT_FALSE(topAbility->GetAbilityInfo().isLauncherAbility);
    EXPECT_FALSE(topAbility->GetAbilityInfo().isNativeAbility);
    EXPECT_FALSE(topAbility->GetAbilityInfo().enabled);
    EXPECT_FALSE(topAbility->GetAbilityInfo().formEnabled);
}
}  // namespace AAFwk
}  // namespace OHOS
