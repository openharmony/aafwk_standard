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
#include "mock_ability_scheduler.h"
#include "mock_app_mgr_client.h"
#include "mock_bundle_mgr.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t MOCK_MAIN_USER_ID = 100;
const int32_t MOCK_U0_USER_ID = 0;
static int MOCK_MISSION_ID = 10000;
}
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

static void WaitUntilTaskFinishedByTimer()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    int sleepingTime = 5000;
    if (handler->PostTask(f, "AbilityManagerServiceTest", sleepingTime)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

class AbilityTimeoutModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockOnStart();
    static void MockOnStop();
    static constexpr int TEST_WAIT_TIME = 100000;
    std::shared_ptr<AbilityRecord> CreateRootLauncher();
    std::shared_ptr<AbilityRecord> CreateCommonAbility();
    std::shared_ptr<AbilityRecord> CreateLauncherAbility();
    std::shared_ptr<AbilityRecord> CreateServiceAbility();
    std::shared_ptr<AbilityRecord> CreateExtensionAbility();

public:
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

void AbilityTimeoutModuleTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase.";
}

void AbilityTimeoutModuleTest::TearDownTestCase()
{
    MockOnStop();
    GTEST_LOG_(INFO) << "TearDownTestCase.";
}

void AbilityTimeoutModuleTest::SetUp()
{
    MockOnStart();
}

void AbilityTimeoutModuleTest::TearDown()
{
    WaitUntilTaskFinishedByTimer();
    abilityMs_->handler_->RemoveAllEvents();
    abilityMs_->currentMissionListManager_->terminateAbilityList_.clear();
    abilityMs_->currentMissionListManager_->launcherList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultStandardList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultSingleList_->missions_.clear();
    abilityMs_->currentMissionListManager_->currentMissionLists_.clear();
    abilityMs_->currentMissionListManager_->currentMissionLists_
        .push_front(abilityMs_->currentMissionListManager_->launcherList_);
}

void AbilityTimeoutModuleTest::MockOnStart()
{
    if (!abilityMs_) {
        GTEST_LOG_(ERROR) << "Mock OnStart failed.";
        return;
    }
    if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
        return;
    }
    abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
    EXPECT_TRUE(abilityMs_->handler_);
    EXPECT_TRUE(abilityMs_->eventLoop_);

    // init user controller.
    abilityMs_->userController_ = std::make_shared<UserController>();
    EXPECT_TRUE(abilityMs_->userController_);
    abilityMs_->userController_->Init();
    int userId = MOCK_MAIN_USER_ID;

    abilityMs_->InitConnectManager(userId, true);
    abilityMs_->InitDataAbilityManager(userId, true);
    abilityMs_->InitPendWantManager(userId, true);
    abilityMs_->systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();
    EXPECT_TRUE(abilityMs_->systemDataAbilityManager_);

    abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    EXPECT_TRUE(abilityMs_->amsConfigResolver_);
    abilityMs_->amsConfigResolver_->Parse();

    abilityMs_->InitMissionListManager(userId, true);
    abilityMs_->SwitchManagers(MOCK_U0_USER_ID, false);

    abilityMs_->userController_->SetCurrentUserId(MOCK_MAIN_USER_ID);
    
    abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;
    abilityMs_->iBundleManager_ = new BundleMgrService();
    abilityMs_->eventLoop_->Run();

    WaitUntilTaskFinished();
}

void AbilityTimeoutModuleTest::MockOnStop()
{
    WaitUntilTaskFinishedByTimer();
    auto abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMs_) {
        GTEST_LOG_(ERROR) << "Mock OnStart failed.";
        return;
    }

    abilityMs_->handler_->RemoveAllEvents();
    abilityMs_->connectManagers_.clear();
    abilityMs_->connectManager_.reset();
    abilityMs_->iBundleManager_.clear();
    abilityMs_->appScheduler_.reset();
    abilityMs_->dataAbilityManagers_.clear();
    abilityMs_->dataAbilityManager_.reset();
    abilityMs_->systemDataAbilityManager_.reset();
    abilityMs_->pendingWantManagers_.clear();
    abilityMs_->pendingWantManager_.reset();
    abilityMs_->amsConfigResolver_.reset();
    abilityMs_->missionListManagers_.clear();
    abilityMs_->currentMissionListManager_.reset();
    abilityMs_->userController_.reset();
    abilityMs_->abilityController_.clear();
    abilityMs_->OnStop();
}

std::shared_ptr<AbilityRecord> AbilityTimeoutModuleTest::CreateRootLauncher()
{
    if (!abilityMs_->currentMissionListManager_ || !abilityMs_->currentMissionListManager_->launcherList_) {
        return nullptr;
    }
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.ix.hiworld.MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.ix.hiworld";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.ix.hiworld";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID++, abilityRecord, abilityRequest.abilityInfo.bundleName);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(lauList);
    abilityRecord->SetLauncherRoot();
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    lauList->AddMissionToTop(mission);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    return abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityTimeoutModuleTest::CreateLauncherAbility()
{
    if (!abilityMs_->currentMissionListManager_ || !abilityMs_->currentMissionListManager_->launcherList_) {
        return nullptr;
    }
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.ix.hiworld.SecAbility";
    abilityRequest.abilityInfo.bundleName = "com.ix.hiworld";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.ix.hiworld";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID++, abilityRecord, abilityRequest.abilityInfo.bundleName);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(lauList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    lauList->AddMissionToTop(mission);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    return abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityTimeoutModuleTest::CreateServiceAbility()
{
    auto curListManager = abilityMs_->currentMissionListManager_;
    if (!curListManager) {
        return nullptr;
    }

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::SERVICE;
    abilityRequest.abilityInfo.name = "om.ix.Common.ServiceAbility";
    abilityRequest.abilityInfo.bundleName = "com.ix.Common";
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.appInfo.name = "com.ix.Common";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    
    return abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityTimeoutModuleTest::CreateExtensionAbility()
{
    auto curListManager = abilityMs_->currentMissionListManager_;
    if (!curListManager) {
        return nullptr;
    }

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.name = "om.ix.Common.ExtensionAbility";
    abilityRequest.abilityInfo.bundleName = "com.ix.Common";
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.appInfo.name = "com.ix.Common";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    return abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityTimeoutModuleTest::CreateCommonAbility()
{
    auto curListManager = abilityMs_->currentMissionListManager_;
    if (!curListManager) {
        return nullptr;
    }

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "om.ix.Common.MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.ix.Common";
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.appInfo.name = "com.ix.Common";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID++, abilityRecord, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(abilityRecord != nullptr);
    EXPECT_TRUE(mission != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    abilityRecord->SetMissionList(missionList);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    return abilityRecord;
}


/*
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: OnAbilityDied
 */
HWTEST_F(AbilityTimeoutModuleTest, OnAbilityDied_001, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);

    int maxRestart = -1;
    abilityMs_->GetMaxRestartNum(maxRestart);
    EXPECT_TRUE(maxRestart > -1);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());

    GTEST_LOG_(INFO) << "userId:" << abilityMs_->GetUserId();
    GTEST_LOG_(INFO) << "currentmanager userId" << abilityMs_->currentMissionListManager_->userId_;

    // died rootlauncher ability
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);
    abilityMs_->OnAbilityDied(rootLauncher);

    EXPECT_TRUE(lauList->GetAbilityRecordByToken(rootLauncher->GetToken()) != nullptr);
    EXPECT_TRUE(rootLauncher->IsRestarting());
    EXPECT_TRUE(rootLauncher->restartCount_ < rootLauncher->restratMax_);
    GTEST_LOG_(INFO) << "restart count:" << rootLauncher->restartCount_;
}


/*
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: OnAbilityDied
 */
HWTEST_F(AbilityTimeoutModuleTest, OnAbilityDied_002, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);

    int maxRestart = -1;
    abilityMs_->GetMaxRestartNum(maxRestart);
    EXPECT_TRUE(maxRestart > -1);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    topAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);
    
    // died rootlauncher ability
    abilityMs_->OnAbilityDied(rootLauncher);
    WaitUntilTaskFinishedByTimer();
    topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_EQ(topAbility, rootLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(rootLauncher->GetToken()) != nullptr);
    EXPECT_TRUE(rootLauncher->IsRestarting());
    EXPECT_TRUE(rootLauncher->restartCount_ < rootLauncher->restratMax_);
    GTEST_LOG_(INFO) << "restart count:" << rootLauncher->restartCount_;
}

/*
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: OnAbilityDied
 */
HWTEST_F(AbilityTimeoutModuleTest, OnAbilityDied_003, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);

    int maxRestart = -1;
    abilityMs_->GetMaxRestartNum(maxRestart);
    EXPECT_TRUE(maxRestart > -1);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());

    // died rootlauncher ability
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);
    int i = 0;
    while (i < rootLauncher->restratMax_) {
        abilityMs_->OnAbilityDied(rootLauncher);
        usleep(100);
        i++;
    }

    EXPECT_TRUE(lauList->GetAbilityRecordByToken(rootLauncher->GetToken()) != nullptr);
    EXPECT_TRUE(rootLauncher->IsRestarting());
    EXPECT_TRUE(rootLauncher->restartCount_ == 0);
    GTEST_LOG_(INFO) << "restartCount." << rootLauncher->restartCount_;
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_001, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);
    

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(rootLauncher->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(rootLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_002, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_003, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs as caller
    auto callerAbility = CreateCommonAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, callerAbility);
    callerAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(callerAbility, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_004, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add launcher ability to abilityMs as caller
    auto callerAbility = CreateLauncherAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, callerAbility);
    callerAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_005, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add service ability to abilityMs as caller
    auto callerAbility = CreateServiceAbility();

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    auto currentList = abilityMs_->currentMissionListManager_->currentMissionLists_;
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_006, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add extension ability to abilityMs as caller
    auto callerAbility = CreateExtensionAbility();

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    auto currentList = abilityMs_->currentMissionListManager_->currentMissionLists_;
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleLoadTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleLoadTimeOut_007, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common laucher ability to abilityMs
    auto commonLauncherAbility = CreateLauncherAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonLauncherAbility);
    
    // rootlauncher load timeout
    abilityMs_->HandleLoadTimeOut(commonLauncherAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncherAbility->GetToken()) == nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_001, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);
    

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(rootLauncher->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(rootLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_002, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_003, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs as caller
    auto callerAbility = CreateCommonAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, callerAbility);
    callerAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(callerAbility, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_004, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add launcher ability to abilityMs as caller
    auto callerAbility = CreateLauncherAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, callerAbility);
    callerAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_005, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add service ability to abilityMs as caller
    auto callerAbility = CreateServiceAbility();

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    auto currentList = abilityMs_->currentMissionListManager_->currentMissionLists_;
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_006, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add extension ability to abilityMs as caller
    auto callerAbility = CreateExtensionAbility();

    // add common ability to abilityMs
    auto commonAbility = CreateCommonAbility();
    commonAbility->AddCallerRecord(callerAbility->GetToken(), -1);
    auto currentList = abilityMs_->currentMissionListManager_->currentMissionLists_;
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonAbility);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}

/*
 * Function: HandleForegroundNewTimeOut
 * SubFunction: NA
 * FunctionPoints: HandleForegroundNewTimeOut
 * EnvConditions: NA
 * CaseDescription: HandleForegroundNewTimeOut
 */
HWTEST_F(AbilityTimeoutModuleTest, HandleForegroundNewTimeOut_007, TestSize.Level1)
{
    // test config is success.
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    auto curListManager = abilityMs_->currentMissionListManager_;
    EXPECT_TRUE(lauList != nullptr);

    // add rootlauncher to abilityMs.
    auto ability = CreateRootLauncher();
    auto rootLauncher = lauList->GetTopAbility();
    EXPECT_EQ(rootLauncher, ability);
    EXPECT_TRUE(rootLauncher->IsLauncherRoot());
    rootLauncher->SetAbilityState(AbilityState::FOREGROUND_NEW);

    // add common laucher ability to abilityMs
    auto commonLauncherAbility = CreateLauncherAbility();
    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_EQ(topAbility, commonLauncherAbility);
    commonLauncherAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    
    // rootlauncher load timeout
    abilityMs_->HandleForegroundNewTimeOut(commonLauncherAbility->eventId_);
    WaitUntilTaskFinishedByTimer();
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncherAbility->GetToken()) != nullptr);
    topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(rootLauncher, topAbility);
}
}  // namespace AAFwk
}  // namespace OHOS