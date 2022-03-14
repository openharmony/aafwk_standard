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
const int32_t MOCK_MAIN_USER_ID = 100;
const int32_t MOCK_MISSION_ID = 10000;
const int32_t MOCK_U0_USER_ID = 0;
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

class AbilityTimeoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockOnStart();
    static void MockOnStop();
    static constexpr int TEST_WAIT_TIME = 100000;

public:
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

void AbilityTimeoutTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase.";
}

void AbilityTimeoutTest::TearDownTestCase()
{
    MockOnStop();
    GTEST_LOG_(INFO) << "TearDownTestCase.";
}

void AbilityTimeoutTest::SetUp()
{
    MockOnStart();
}

void AbilityTimeoutTest::TearDown()
{
    WaitUntilTaskFinishedByTimer();
    abilityMs_->handler_->RemoveAllEvents();
    abilityMs_->currentMissionListManager_->terminateAbilityList_.clear();
    abilityMs_->currentMissionListManager_->launcherList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultStandardList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultSingleList_->missions_.clear();
    abilityMs_->currentMissionListManager_->currentMissionLists_.clear();
    abilityMs_->currentMissionListManager_->currentMissionLists_.push_front(
        abilityMs_->currentMissionListManager_->launcherList_);
}

void AbilityTimeoutTest::MockOnStart()
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
    abilityMs_->useNewMission_ = abilityMs_->amsConfigResolver_->IsUseNewMission();

    abilityMs_->SetStackManager(userId, true);

    abilityMs_->InitMissionListManager(userId, true);
    abilityMs_->SwitchManagers(MOCK_U0_USER_ID, false);

    abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;
    abilityMs_->iBundleManager_ = new BundleMgrService();
    abilityMs_->eventLoop_->Run();

    WaitUntilTaskFinished();
}

void AbilityTimeoutTest::MockOnStop()
{
    WaitUntilTaskFinishedByTimer();
    auto abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMs_) {
        GTEST_LOG_(ERROR) << "Mock OnStart failed.";
        return;
    }

    abilityMs_->handler_->RemoveAllEvents();
    abilityMs_->stackManagers_.clear();
    abilityMs_->currentStackManager_.reset();
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
/*
 * Feature: AbilityManagerService
 * Function: GetMaxRestartNum
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService GetMaxRestartNum success
 */
HWTEST_F(AbilityTimeoutTest, GetMaxRestartNum_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->amsConfigResolver_ != nullptr);

    int maxRestart = -1;
    abilityMs_->GetMaxRestartNum(maxRestart);

    EXPECT_TRUE(maxRestart > -1);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService OnAbilityDied success
 */
HWTEST_F(AbilityTimeoutTest, OnAbilityDied_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto defList = abilityMs_->currentMissionListManager_->defaultStandardList_;
    EXPECT_TRUE(defList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.DiedAbility001";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.name = "com.test";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(defList);
    defList->AddMissionToTop(mission);
    EXPECT_TRUE(defList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    EXPECT_TRUE(abilityMs_->useNewMission_);
    abilityMs_->OnAbilityDied(abilityRecord);

    EXPECT_TRUE(defList->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService OnAbilityDied success
 */
HWTEST_F(AbilityTimeoutTest, OnAbilityDied_002, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);
    EXPECT_EQ((int)(abilityMs_->currentMissionListManager_->currentMissionLists_.size()), 1);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.DiedAbility002";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(lauList);
    abilityRecord->SetLauncherRoot();
    lauList->AddMissionToTop(mission);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    abilityMs_->useNewMission_ = true;
    abilityMs_->OnAbilityDied(abilityRecord);

    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_TRUE(abilityRecord->IsRestarting());
    EXPECT_TRUE(abilityRecord->restartCount_ < abilityRecord->restratMax_);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    EXPECT_TRUE(abilityMs_->currentMissionListManager_ != nullptr);
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(lauList != nullptr);
    EXPECT_EQ((int)(abilityMs_->currentMissionListManager_->currentMissionLists_.size()), 1);

    // root launcher ability load timeout
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.Timeout001";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(abilityRecord != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(lauList);
    abilityRecord->SetLauncherRoot();
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    lauList->AddMissionToTop(mission);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);

    EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_TRUE(abilityRecord->IsRestarting());
    EXPECT_TRUE(abilityRecord->restartCount_ < abilityRecord->restratMax_);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_002, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);
    EXPECT_EQ((int)(abilityMs_->currentMissionListManager_->currentMissionLists_.size()), 1);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(launcher != nullptr);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability load timeout
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.Timeout002";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(abilityRecord != nullptr);

    abilityRecord->SetMission(mission);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);

    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_003, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);
    EXPECT_EQ((int)(abilityMs_->currentMissionListManager_->currentMissionLists_.size()), 1);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
    // common ability by caller
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerMission = std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(caller != nullptr);
    EXPECT_TRUE(callerMission != nullptr);
    caller->SetMission(callerMission);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    caller->SetMissionList(missionList);
    caller->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    missionList->AddMissionToTop(callerMission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(caller->GetToken()) != nullptr);
    // common ability load timeout
    abilityRequest.abilityInfo.name = "com.test.Timeout003";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto mission =
        std::make_shared<Mission>(MOCK_MISSION_ID + 2, abilityRecord, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(abilityRecord != nullptr);
    EXPECT_TRUE(mission != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    abilityRecord->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(caller, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_004, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by caller with service ability type
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.caller";
    abilityRequest.abilityInfo.type = AbilityType::SERVICE;
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability load timeout
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.Timeout004";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(missionList != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    abilityRecord->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_005, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by caller with extension ability type
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability load timeout
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.Timeout005";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(missionList != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    abilityRecord->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_006, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by caller as launcher type
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability load timeout
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.appInfo.name = "com.test.Timeout006";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(missionList != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    abilityRecord->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_007, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by caller
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common launcher ability load timeout
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.abilityInfo.name = "com.test.Timeout007";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto mission = std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(mission != nullptr);
    EXPECT_TRUE(missionList != nullptr);
    abilityRecord->SetMission(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    abilityRecord->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(mission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
    EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleLoadTimeOut(abilityRecord->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // test root launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    launcher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    abilityMs_->HandleForegroundNewTimeOut(launcher->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_002, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common launcher ability timeout
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground002";
    auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonLauncher != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    commonLauncher->SetMission(commonMissionLauncher);
    commonLauncher->SetMissionList(lauList);
    commonLauncher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    lauList->AddMissionToTop(commonMissionLauncher);
    commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

    // test common launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonLauncher->eventId_);

    EXPECT_TRUE(lauList->GetAbilityRecordByToken(commonLauncher->GetToken()) == nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_003, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;
    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
    // common ability by caller
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerMission = std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(caller != nullptr);
    EXPECT_TRUE(callerMission != nullptr);
    caller->SetMission(callerMission);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    caller->SetMissionList(missionList);
    caller->eventId_ = (AbilityRecord::g_abilityRecordEventId_++);
    missionList->AddMissionToTop(callerMission);
    curListManager->MoveMissionListToTop(missionList);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(caller->GetToken()) != nullptr);
    // common ability timeout
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground003";
    auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonLauncher != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID + 2, commonLauncher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    commonLauncher->SetMission(commonMissionLauncher);
    commonLauncher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    commonLauncher->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(commonMissionLauncher);
    curListManager->MoveMissionListToTop(missionList);
    commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    // test common launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonLauncher->eventId_);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(caller, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_004, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by caller (launcher type)
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability timeout
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground004";
    auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonLauncher != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    commonLauncher->SetMissionList(missionList);
    commonLauncher->SetMission(commonMissionLauncher);
    commonLauncher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    commonLauncher->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(commonMissionLauncher);
    curListManager->MoveMissionListToTop(missionList);
    commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

    // test common launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonLauncher->eventId_);

    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_005, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by service ability
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.type = AbilityType::SERVICE;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability timeout
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground005";
    auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonLauncher != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    commonLauncher->SetMissionList(missionList);
    commonLauncher->SetMission(commonMissionLauncher);
    commonLauncher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    commonLauncher->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(commonMissionLauncher);
    curListManager->MoveMissionListToTop(missionList);
    commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

    // test common launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonLauncher->eventId_);

    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_006, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability by service ability
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.name = "com.test.caller";
    auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(caller != nullptr);

    // common ability timeout
    abilityRequest.appInfo.isLauncherApp = false;
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground006";
    auto commonAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonAbility != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID, commonAbility, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    commonAbility->SetMissionList(missionList);
    commonAbility->SetMission(commonMissionLauncher);
    commonAbility->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    commonAbility->AddCallerRecord(caller->GetToken(), -1);
    missionList->AddMissionToTop(commonMissionLauncher);
    curListManager->MoveMissionListToTop(missionList);
    commonAbility->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);

    // test common ability foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonAbility->eventId_);

    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_007, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    auto curListManager = abilityMs_->currentMissionListManager_;
    auto lauList = abilityMs_->currentMissionListManager_->launcherList_;

    EXPECT_TRUE(curListManager != nullptr);
    EXPECT_TRUE(lauList != nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(launcher != nullptr);
    auto missionLauncher = std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(missionLauncher != nullptr);
    launcher->SetMission(missionLauncher);
    launcher->SetMissionList(lauList);
    launcher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    launcher->SetLauncherRoot();
    lauList->AddMissionToTop(missionLauncher);
    EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

    // common ability timeout without caller
    abilityRequest.abilityInfo.name = "com.test.TimeoutForeground007";
    auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(commonLauncher != nullptr);
    auto commonMissionLauncher =
        std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
    EXPECT_TRUE(commonMissionLauncher != nullptr);
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    commonLauncher->SetMissionList(missionList);
    commonLauncher->SetMission(commonMissionLauncher);
    commonLauncher->eventId_ = AbilityRecord::g_abilityRecordEventId_++;
    missionList->AddMissionToTop(commonMissionLauncher);
    curListManager->MoveMissionListToTop(missionList);
    commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING_NEW);
    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

    // test common launcher foreground timeout.
    abilityMs_->useNewMission_ = true;
    abilityMs_->HandleForegroundNewTimeOut(commonLauncher->eventId_);

    EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    auto topAbility = curListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(launcher, topAbility);
}
}  // namespace AAFwk
}  // namespace OHOS