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

#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

namespace {
const std::string NAME_BUNDLE_MGR_SERVICE = "BundleMgrService";
static int32_t g_windowToken = 0;
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
        int userId = abilityMs_->GetUserId();
        abilityMs_->SetStackManager(userId, true);
        EXPECT_TRUE(abilityMs_->GetStackManager());
        abilityMs_->stackManagers_.emplace(0, abilityMs_->GetStackManager());
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

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the result of StartAbility is failed if the param of StartAbility is illegal.
 */
HWTEST_F(AbilityManagerServiceTest, Interface_001, TestSize.Level1)
{
    Want want;
    want.AddEntity(Want::ENTITY_HOME);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the result of StartAbility is successful if the param of StartAbility is normal.
 */
HWTEST_F(AbilityManagerServiceTest, Interface_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, abilityMs_->QueryServiceState());
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the singleton startup mode, start multiple times, and do not recreate
 */
HWTEST_F(AbilityManagerServiceTest, Interface_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicSAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    auto abilityId = topAbility->GetRecordId();

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want2;
    ElementName element2("device", "com.ix.hiMusic", "MusicSAbility");
    want2.SetElement(element2);
    auto result2 = StartAbility(want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result2);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();

    EXPECT_EQ(topAbility->GetRecordId(), abilityId);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the standard startup mode, start several times, and create a new record
 */
HWTEST_F(AbilityManagerServiceTest, Interface_004, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    auto id = topAbility->GetRecordId();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    auto abilityId = topAbility->GetRecordId();

    EXPECT_NE(abilityId, id);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the singletop startup mode, start several times, and create a new record
 */
HWTEST_F(AbilityManagerServiceTest, Interface_005, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicTopAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    auto abilityId = topAbility->GetRecordId();

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want2;
    ElementName element2("device", "com.ix.hiMusic", "MusicTopAbility");
    want2.SetElement(element2);
    auto result2 = StartAbility(want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result2);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();

    EXPECT_EQ(topAbility->GetRecordId(), abilityId);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that service ability started successfully
 */
HWTEST_F(AbilityManagerServiceTest, Interface_006, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    EXPECT_EQ(1, static_cast<int>(serviceMap.size()));
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Top ability is not active, so enqueue ability for waiting. StartAbility failed
 */
HWTEST_F(AbilityManagerServiceTest, Interface_007, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(START_ABILITY_WAITING, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Failed to verify terminate ability
 */
HWTEST_F(AbilityManagerServiceTest, Interface_008, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiData", "DataAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }

    sptr<IRemoteObject> nullToekn = nullptr;
    auto result1 = abilityMs_->TerminateAbility(nullToekn, -1, &want);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_INVALID_VALUE, result1);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> toekn1 = new Token(ability);
    auto result2 = abilityMs_->TerminateAbility(toekn1, -1, &want);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_INVALID_VALUE, result2);

    auto result3 = abilityMs_->TerminateAbility(token, -1, &want);
    WaitUntilTaskFinished();
    EXPECT_NE(ERR_OK, result3);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verification service terminate ability failure
 */
HWTEST_F(AbilityManagerServiceTest, Interface_009, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    EXPECT_EQ(1, static_cast<int>(serviceMap.size()));
    for (auto &it : serviceMap) {
        EXPECT_EQ(it.first, element.GetURI());
    }
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    auto result1 = abilityMs_->TerminateAbility(service->GetToken(), -1, &want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verification service terminate ability failure
 */
HWTEST_F(AbilityManagerServiceTest, Interface_010, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicSAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want2;
    ElementName element2("device", "com.ix.hiRadio", "RadioAbility");
    want2.SetElement(element2);
    auto result2 = StartAbility(want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result2);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();

    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    auto result3 = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result3);
    topAbility = stackManager->GetCurrentTopAbility();

    auto result4 = abilityMs_->TerminateAbility(topAbility->GetToken(), -1, &want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result4);

    topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiRadio");

    auto result5 = abilityMs_->TerminateAbility(topAbility->GetToken(), -1, &want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result5);

    topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Attachabilitythread failed due to empty token or scheduler
 */
HWTEST_F(AbilityManagerServiceTest, Interface_011, TestSize.Level1)
{
    Want wantLuncher;
    ElementName elementLun("device", "com.ohos.launcher", "com.ohos.launcher.MainAbility");
    wantLuncher.SetElement(elementLun);
    abilityMs_->StartAbility(wantLuncher);
    WaitUntilTaskFinished();
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbility->GetAbilityInfo().name, "com.ohos.launcher.MainAbility");

    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    OHOS::sptr<IAbilityScheduler> nullScheduler = nullptr;
    EXPECT_EQ(abilityMs_->AttachAbilityThread(nullScheduler, topAbility->GetToken()), OHOS::ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> record = nullptr;
    OHOS::sptr<Token> nullToken = new Token(record);
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, nullToken), OHOS::ERR_INVALID_VALUE);

    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, topAbility->GetToken()), OHOS::ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: AbilityTransitionDone failed due to empty token or scheduler
 */
HWTEST_F(AbilityManagerServiceTest, Interface_012, TestSize.Level1)
{
    Want wantLuncher;
    ElementName elementLun("device", "com.ix.music", "MusicAbility");
    wantLuncher.SetElement(elementLun);
    abilityMs_->StartAbility(wantLuncher);
    WaitUntilTaskFinished();
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();

    EXPECT_EQ(topAbility->GetAbilityInfo().name, "MusicAbility");

    OHOS::sptr<Token> nullToken = nullptr;
    PacMap saveData;
    auto res = abilityMs_->AbilityTransitionDone(nullToken, OHOS::AAFwk::AbilityState::ACTIVE, saveData);
    EXPECT_EQ(res, OHOS::ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> record = nullptr;
    OHOS::sptr<Token> token = new Token(record);
    auto res1 = abilityMs_->AbilityTransitionDone(token, OHOS::AAFwk::AbilityState::ACTIVE, saveData);
    EXPECT_EQ(res1, OHOS::ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SetStackManager and GetStackManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetStackManager and GetStackManager
 * EnvConditions: NA
 * CaseDescription: Verify set and get
 */
HWTEST_F(AbilityManagerServiceTest, Interface_013, TestSize.Level1)
{
    abilityMs_->SetStackManager(0, true);
    EXPECT_NE(nullptr, abilityMs_->GetStackManager());
}

/*
 * Feature: AbilityManagerService
 * Function: DumpWaittingAbilityQueue
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpWaittingAbilityQueue
 * EnvConditions: NA
 * CaseDescription: Verify dumpwaittingabilityqueue result
 */
HWTEST_F(AbilityManagerServiceTest, Interface_014, TestSize.Level1)
{
    std::string dump;
    abilityMs_->DumpWaittingAbilityQueue(dump);
    EXPECT_EQ(false, (dump.find("User ID #0") != string::npos));
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: OnAbilityRequestDone failed due to empty token
 */
HWTEST_F(AbilityManagerServiceTest, Interface_015, TestSize.Level1)
{
    Want wantLuncher;
    ElementName elementLun("device", "com.ix.hiworld", "LauncherAbility");
    wantLuncher.SetElement(elementLun);
    abilityMs_->StartAbility(wantLuncher);
    WaitUntilTaskFinished();
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    OHOS::sptr<Token> nullToken = nullptr;

    abilityMs_->OnAbilityRequestDone(nullToken, 2);
    EXPECT_EQ(topAbility->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMs_->OnAbilityRequestDone(topAbility->GetToken(), 2);
    EXPECT_EQ(topAbility->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVATING);
}

/*
 * Feature: AbilityManagerService
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RemoveMission
 * EnvConditions: NA
 * CaseDescription: 1.MissionId < 0, RemoveMission failed
 *                  2.MissionId no exist, RemoveMission failed
 *                  3.Top mission is launcher, RemoveMission failed
 *                  4.The current mission cannot be removed
 *                  5.current mission is launcher, remove default mission is success
 */
HWTEST_F(AbilityManagerServiceTest, Interface_016, TestSize.Level1)
{
    Want wantLuncher;
    ElementName elementLun("device", "com.ix.hiworld", "LauncherAbility");
    wantLuncher.SetElement(elementLun);
    StartAbility(wantLuncher);
    WaitUntilTaskFinished();

    EXPECT_TRUE(abilityMs_->GetStackManager() != nullptr);
    EXPECT_TRUE(abilityMs_->GetStackManager()->GetTopMissionRecord() != nullptr);
    EXPECT_TRUE(abilityMs_->GetStackManager()->GetTopMissionRecord()->GetTopAbilityRecord() != nullptr);
    auto launcherWant = abilityMs_->GetStackManager()->GetTopMissionRecord()->GetTopAbilityRecord()->GetWant();

    EXPECT_EQ(abilityMs_->RemoveMission(-1), OHOS::ERR_INVALID_VALUE);

    EXPECT_EQ(abilityMs_->RemoveMission(100), REMOVE_MISSION_ID_NOT_EXIST);

    auto luncherMissionId = abilityMs_->GetStackManager()->GetTopMissionRecord()->GetMissionRecordId();
    EXPECT_EQ(abilityMs_->RemoveMission(luncherMissionId), REMOVE_MISSION_LAUNCHER_DENIED);
    GTEST_LOG_(INFO) << "luncherMissionId " << luncherMissionId;

    abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    Want want;
    ElementName element("device", "com.ix.music", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topMissionId = abilityMs_->GetStackManager()->GetTopMissionRecord()->GetMissionRecordId();
    GTEST_LOG_(INFO) << "topMissionId " << topMissionId;
    EXPECT_FALSE(abilityMs_->GetStackManager()->IsLauncherMission(topMissionId));

    // remove current music mission
    EXPECT_EQ(abilityMs_->RemoveMission(topMissionId), ERR_OK);

    auto result1 = StartAbility(launcherWant);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RemoveMission
 * EnvConditions: window visible is true, ability state is inactive
 * CaseDescription: Verify RemoveMission operation fail
 */
HWTEST_F(AbilityManagerServiceTest, Interface_017, TestSize.Level1)
{
    Want wantLuncher;
    ElementName elementLun("device", "com.ix.hiworld", "LauncherAbility");
    wantLuncher.SetElement(elementLun);
    abilityMs_->StartAbility(wantLuncher);
    WaitUntilTaskFinished();
    
    Want want;
    ElementName element("device", "com.ix.music", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();
    abilityMs_->AddWindowInfo(topAbility->GetToken(), ++g_windowToken);
    topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    auto topMissionId = abilityMs_->GetStackManager()->GetTopMissionRecord()->GetMissionRecordId();

    // return to luncher
    auto result1 = StartAbility(wantLuncher);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);

    EXPECT_EQ(abilityMs_->RemoveMission(topMissionId), OHOS::ERR_OK);
    WaitUntilTaskFinished();
}

/*
 * Feature: AbilityManagerService
 * Function: GetRecentMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRecentMissions
 * EnvConditions: NA
 * CaseDescription: Verify GetRecentMissions operation
 */
HWTEST_F(AbilityManagerServiceTest, Interface_018, TestSize.Level1)
{
    Want want;
    want.AddEntity(Want::ENTITY_HOME);
    ElementName element("device", "com.ix.music", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    Want want1;
    want1.AddEntity(Want::ENTITY_HOME);
    ElementName element1("device", "com.ix.radio", "RadioAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);

    std::vector<AbilityMissionInfo> info;
    auto res = abilityMs_->GetRecentMissions(-1, 1, info);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, res);

    auto res1 = abilityMs_->GetRecentMissions(INT_MAX, -1, info);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, res1);

    auto res2 = abilityMs_->GetRecentMissions(INT_MAX, 1, info);
    EXPECT_EQ(OHOS::ERR_OK, res2);
    EXPECT_EQ(static_cast<int>(info.size()), 1);
    EXPECT_EQ(info[0].runingState, -1);
    EXPECT_EQ(info[0].missionDescription.label, "app label");
    EXPECT_EQ(info[0].missionDescription.iconPath, "icon path");
    EXPECT_EQ(info[0].baseWant.GetElement().GetAbilityName(), want.GetElement().GetAbilityName());
    EXPECT_EQ(info[0].baseWant.GetElement().GetBundleName(), want.GetElement().GetBundleName());
    EXPECT_EQ(info[0].baseWant.GetElement().GetDeviceID(), want.GetElement().GetDeviceID());

    EXPECT_EQ(info[0].baseAbility.GetAbilityName(), want.GetElement().GetAbilityName());
    EXPECT_EQ(info[0].baseAbility.GetBundleName(), want.GetElement().GetBundleName());
    EXPECT_EQ(info[0].baseAbility.GetDeviceID(), want.GetElement().GetDeviceID());

    EXPECT_EQ(info[0].topAbility.GetAbilityName(), want1.GetElement().GetAbilityName());
    EXPECT_EQ(info[0].topAbility.GetBundleName(), want1.GetElement().GetBundleName());
    EXPECT_EQ(info[0].topAbility.GetDeviceID(), want1.GetElement().GetDeviceID());
}

/*
 * Feature: AbilityManagerService
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RemoveStack
 * EnvConditions: NA
 * CaseDescription: 1.stackid < 0, RemoveStack failed
 *                  2.stackid no exist, RemoveStack failed
 *                  3.don't allow remove luncher mission stack
 */
HWTEST_F(AbilityManagerServiceTest, Interface_019, TestSize.Level1)
{
    EXPECT_EQ(abilityMs_->RemoveStack(-1), OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->RemoveStack(INT_MAX), REMOVE_STACK_ID_NOT_EXIST);
    EXPECT_EQ(abilityMs_->RemoveStack(0), REMOVE_STACK_LAUNCHER_DENIED);
}

/*
 * Feature: AbilityManagerService
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RemoveStack
 * EnvConditions: NA
 * CaseDescription: remove default stack success
 */
HWTEST_F(AbilityManagerServiceTest, Interface_020, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.music", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManage = abilityMs_->GetStackManager();
    stackManage->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.music", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);

    EXPECT_EQ(stackManage->GetCurrentMissionStack()->GetMissionRecordCount(), 1);
    EXPECT_EQ(abilityMs_->RemoveStack(1), ERR_OK);

}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.callback is nullptr, connectAbility failed
 * 2.ability type is page, connectAbility failed
 * 3.ability type is service and callback is not nullptr, connectAbility success
 */
HWTEST_F(AbilityManagerServiceTest, Interface_021, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, nullptr, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Want want1;
    ElementName element1("", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->ConnectAbility(want1, callback, nullptr);
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);

    auto result2 = abilityMs_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ(result2, ERR_OK);
}

/**
 * @tc.name: ConnectRemoteAbility_001
 * @tc.desc: Verify the following:
 * 1.callback is nullptr, connectAbility failed
 * 2.ability type is page, connectAbility failed
 * 3.ability type is service and callback is not nullptr, connectAbility success
 * @tc.type: FUNC
 * @tc.require: AR000GI8IC
 */
HWTEST_F(AbilityManagerServiceTest, ConnectRemoteAbility_001, TestSize.Level1)
{
    Want want;
    ElementName element("remoteDeviceId", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, nullptr, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Want want1;
    ElementName element1("remoteDeviceId", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->ConnectAbility(want1, callback, nullptr);
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    auto result2 = abilityMs_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ(result2, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.callback is nullptr, disconnect ability failed
 * 2.connect ability is not connected, connectAbility failed
 */
HWTEST_F(AbilityManagerServiceTest, Interface_022, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ(result, ERR_OK);

    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    auto result1 = abilityMs_->DisconnectAbility(nullptr);
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    auto result2 = abilityMs_->DisconnectAbility(callback);
    EXPECT_EQ(result2, ERR_OK);
}

/**
 * @tc.name: DisconnectRemoteAbility_001
 * @tc.desc: Verify the following:
 * 1.callback is nullptr, disconnect ability failed
 * 2.connect ability is not connected, connectAbility failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IC
 */
HWTEST_F(AbilityManagerServiceTest, DisconnectRemoteAbility_001, TestSize.Level1)
{
    Want want;
    ElementName element("remoteDeviceId", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto result1 = abilityMs_->DisconnectAbility(nullptr);
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    auto result2 = abilityMs_->DisconnectAbility(callback);
    EXPECT_EQ(result2, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.token is nullptr, ScheduleConnectAbilityDone failed
 * 2.ability record is nullptr, ScheduleConnectAbilityDone failed
 * 2.ability type is not service, ScheduleConnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceTest, Interface_023, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    const sptr<IRemoteObject> nulltToken = nullptr;
    auto result1 = abilityMs_->ScheduleConnectAbilityDone(nulltToken, callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    const sptr<IRemoteObject> token = new Token(ability);
    auto result2 = abilityMs_->ScheduleConnectAbilityDone(token, callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result2, ERR_INVALID_VALUE);

    Want want1;
    want1.AddEntity(Want::ENTITY_HOME);
    ElementName element1("", "com.ix.radio", "RadioAbility");
    want1.SetElement(element1);
    auto result3 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result3);
    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();

    auto result4 = abilityMs_->ScheduleConnectAbilityDone(topAbility->GetToken(), callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result4, TARGET_ABILITY_NOT_SERVICE);

    auto result5 = abilityMs_->ScheduleConnectAbilityDone(service->GetToken(), callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result5, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.token is nullptr, ScheduleDisconnectAbilityDone failed
 * 2.ability record is nullptr, ScheduleDisconnectAbilityDone failed
 * 2.ability type is not service, ScheduleDisconnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceTest, Interface_024, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    const sptr<IRemoteObject> nulltToken = nullptr;
    auto result1 = abilityMs_->ScheduleDisconnectAbilityDone(nulltToken);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    const sptr<IRemoteObject> token = new Token(ability);
    auto result2 = abilityMs_->ScheduleDisconnectAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result2, ERR_INVALID_VALUE);

    Want want1;
    want1.AddEntity(Want::ENTITY_HOME);
    ElementName element1("", "com.ix.radio", "RadioAbility");
    want1.SetElement(element1);
    auto result3 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result3);
    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();

    auto result4 = abilityMs_->ScheduleDisconnectAbilityDone(topAbility->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result4, TARGET_ABILITY_NOT_SERVICE);

    auto result5 = abilityMs_->ScheduleDisconnectAbilityDone(service->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result5, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.token is nullptr, ScheduleCommandAbilityDone failed
 * 2.ability record is nullptr, ScheduleCommandAbilityDone failed
 * 2.ability type is not service, ScheduleCommandAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceTest, Interface_025, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    const sptr<IRemoteObject> nulltToken = nullptr;
    auto result1 = abilityMs_->ScheduleCommandAbilityDone(nulltToken);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    const sptr<IRemoteObject> token = new Token(ability);
    auto result2 = abilityMs_->ScheduleCommandAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result2, ERR_INVALID_VALUE);

    Want want1;
    want1.AddEntity(Want::ENTITY_HOME);
    ElementName element1("", "com.ix.radio", "RadioAbility");
    want1.SetElement(element1);
    auto result3 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result3);
    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();

    auto result4 = abilityMs_->ScheduleCommandAbilityDone(topAbility->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result4, TARGET_ABILITY_NOT_SERVICE);

    auto result5 = abilityMs_->ScheduleCommandAbilityDone(service->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result5, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllStackInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAllStackInfo
 * EnvConditions: NA
 * CaseDescription: Verify getallstackenfo results
 */
HWTEST_F(AbilityManagerServiceTest, Interface_026, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicTopAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want2;
    ElementName element2("device", "com.ix.hiMusic", "MusicTopAbility");
    want2.SetElement(element2);
    auto result2 = StartAbility(want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result2);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();

    StackInfo info;
    stackManager->GetAllStackInfo(info);
    EXPECT_EQ(static_cast<int>(info.missionStackInfos.size()), 2);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify StopServiceAbility results
 */
HWTEST_F(AbilityManagerServiceTest, Interface_027, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto serviceMap = abilityMs_->connectManager_->GetServiceMap();
    EXPECT_EQ(1, static_cast<int>(serviceMap.size()));
    for (auto &it : serviceMap) {
        EXPECT_EQ(it.first, element.GetURI());
    }
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StopServiceAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE, result1);

    auto result2 = abilityMs_->StopServiceAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result2);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToTop
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionToTop results
 */
HWTEST_F(AbilityManagerServiceTest, Interface_028, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    auto mission = topAbility->GetMissionRecord();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicSAbility");
    want1.SetElement(element1);
    auto result1 = StartAbility(want1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    abilityMs_->MoveMissionToTop(mission->GetMissionRecordId());
    WaitUntilTaskFinished();
    stackManager = abilityMs_->GetStackManager();
    topAbility = stackManager->GetCurrentTopAbility();
    auto topMission = topAbility->GetMissionRecord();
    EXPECT_EQ(topMission, mission);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 * EnvConditions: NA
 * CaseDescription: Verify KillProcess Failure situation
 */
HWTEST_F(AbilityManagerServiceTest, Interface_029, TestSize.Level1)
{
    abilityMs_->currentStackManager_ = nullptr;
    auto result = abilityMs_->KillProcess("bundle");
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AbilityManagerService
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallApp
 * EnvConditions: NA
 * CaseDescription: Verify UninstallApp Failure situation
 */
HWTEST_F(AbilityManagerServiceTest, Interface_030, TestSize.Level1)
{
    abilityMs_->currentStackManager_ = nullptr;
    auto result = abilityMs_->UninstallApp("bundle");
    EXPECT_EQ(ERR_NO_INIT, result);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when AbilityManagerService not
 * dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_001, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");

    // assert ability record
    EXPECT_TRUE(abilityRecord_);

    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when AbilityManagerService not iBundleManager_.
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_002, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    // assert ability record
    EXPECT_TRUE(abilityRecord_);
    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when uri not start with 'dataablity'
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_003, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("mydataability:///data.bundle.DataAbility");
    // assert ability record
    EXPECT_TRUE(abilityRecord_);
    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when uri start with empty
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_004, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability://");
    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when QueryAbilityInfoByUri false.
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_005, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function AcquireDataAbility return nullptr when appInfo name empty
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_AcquireDataAbility_006, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");

    if (abilityRecord_ == nullptr) {
        abilityRequest_.appInfo.bundleName = "data.client.bundle";
        abilityRequest_.abilityInfo.name = "ClientAbility";
        abilityRequest_.abilityInfo.type = AbilityType::DATA;
        abilityRecord_ = AbilityRecord::CreateAbilityRecord(abilityRequest_);
    }

    EXPECT_EQ(abilityMs_->AcquireDataAbility(dataAbilityUri, true, abilityRecord_->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function ReleaseDataAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, AbilityManagerService_ReleaseDataAbility_001, TestSize.Level1)
{
    // assert ability record
    EXPECT_TRUE(abilityRecord_);
    EXPECT_EQ(abilityMs_->ReleaseDataAbility(nullptr, nullptr), OHOS::ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify function ReleaseDataAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, TerminateAbilityResult_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result1 = abilityMs_->TerminateAbilityResult(token, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_INVALID_VALUE, result1);

    // not service aa
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    auto token1 = topAbility->GetToken();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    auto result2 = abilityMs_->TerminateAbilityResult(token1, topAbility->GetStartId());
    WaitUntilTaskFinished();
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE, result2);

    // an service aa
    Want want2;
    ElementName element2("device", "com.ix.musicService", "MusicService");
    want2.SetElement(element2);
    result = StartAbility(want2);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    stackManager = abilityMs_->GetStackManager();
    auto topAbility2 = stackManager->GetCurrentTopAbility();
    auto token2 = topAbility->GetToken();
    topAbility2->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    auto result3 = abilityMs_->TerminateAbilityResult(token2, -1);
    EXPECT_NE(OHOS::ERR_OK, result3);

    // current stack top is com.ix.hiMusic
    auto stackManagerAfter = abilityMs_->GetStackManager();
    auto topAbilityAfter = stackManagerAfter->GetCurrentTopAbility();
    Want want3 = topAbilityAfter->GetWant();
    EXPECT_EQ(want3.GetElement().GetURI(), want.GetElement().GetURI());
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_001, TestSize.Level1)
{
    // first run a service aa
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }

    // start other aa
    Want want1;
    ElementName element1("", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element1);
    auto result1 = abilityMs_->StartAbility(want, token);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_002, TestSize.Level1)
{
    // first run a service aa
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }

    // start self
    auto result1 = abilityMs_->StartAbility(want, token);
    EXPECT_NE(OHOS::ERR_OK, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_003, TestSize.Level1)
{
    // first run a service aa
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }

    // start a date aa
    Want want1;
    ElementName element1("", "com.ix.hiData", "DataAbility");
    want.SetElement(element1);
    auto result1 = abilityMs_->StartAbility(want1, token);
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result1);
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_004, TestSize.Level1)
{
    // run a page aa
    Want want;
    ElementName element("", "com.ix.hiworld", "WorldService");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }

    // start a page aa
    Want want1;
    ElementName element1("", "com.ix.hiMusic", "hiMusic");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StartAbility(want1, token);
    EXPECT_EQ(OHOS::ERR_OK, result1);

    // current stack top is com.ix.hiMusic
    auto stackManagerAfter = abilityMs_->GetStackManager();
    auto topAbilityAfter = stackManagerAfter->GetCurrentTopAbility();
    Want want2 = topAbilityAfter->GetWant();
    EXPECT_EQ(want1.GetElement().GetURI(), want2.GetElement().GetURI());
}

/**
 * @tc.name: startContinuation_001
 * @tc.desc: test StartContinuation when get dmsproxy failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, startContinuation_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    sptr<IRemoteObject> abilityToken = new (std::nothrow) MockAbilityToken();
    auto result = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/**
 * @tc.name: startContinuation_002
 * @tc.desc: test StartContinuation when deviceId is local
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, startContinuation_002, TestSize.Level1)
{
    Want want;
    std::string localDeviceId;
    bool getLocalDeviceId = abilityMs_->GetLocalDeviceId(localDeviceId);
    EXPECT_EQ(true, getLocalDeviceId);
    ElementName element(localDeviceId, "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    sptr<IRemoteObject> abilityToken = new (std::nothrow) MockAbilityToken();
    auto result = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_NE(OHOS::ERR_OK, result);
}

/**
 * @tc.name: startContinuation_003
 * @tc.desc: test StartContinuation when abilityToken is null
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, startContinuation_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);

    sptr<IRemoteObject> abilityToken = nullptr;
    auto result = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_NE(OHOS::ERR_OK, result);
}

/**
 * @tc.name: startContinuation_004
 * @tc.desc: test StartContinuation when deviceId/bundleName/abilityName is empty
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, startContinuation_004, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    sptr<IRemoteObject> abilityToken = new (std::nothrow) MockAbilityToken();
    auto result = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_NE(OHOS::ERR_OK, result);

    ElementName element1("device", "", "MusicAbility");
    want.SetElement(element1);
    auto result1 = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_NE(OHOS::ERR_OK, result1);

    ElementName element2("device", "com.ix.hiMusic", "");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartContinuation(want, abilityToken, 0);
    EXPECT_NE(OHOS::ERR_OK, result2);
}

/**
 * @tc.name: NotifyContinuationResult_001
 * @tc.desc: test NotifyContinuationResult when abilityToken not exist
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, NotifyContinuationResult_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);

    int32_t missionId = 0;
    int32_t isSuccess = 0;
    auto result = abilityMs_->NotifyContinuationResult(missionId, isSuccess);
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/**
 * @tc.name: NotifyContinuationResult_002
 * @tc.desc: test NotifyContinuationResult
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, NotifyContinuationResult_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto topMissionId = abilityMs_->GetStackManager()->GetTopMissionRecord()->GetMissionRecordId();

    int32_t isSuccess = 0;
    result = abilityMs_->NotifyContinuationResult(topMissionId, isSuccess);
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/**
 * @tc.name: ContinueMission_001
 * @tc.desc: test ContinueMission
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(AbilityManagerServiceTest, ContinueMission_001, TestSize.Level0)
{
    std::string srcId = "123";
    std::string dstId = "456";
    int32_t missionId = 0;
    WantParams wantParams;
    int res = abilityMs_->ContinueMission(srcId, dstId, missionId, nullptr, wantParams);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: ContinueAbility_001
 * @tc.desc: test ContinueAbility
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(AbilityManagerServiceTest, ContinueAbility_001, TestSize.Level0)
{
    std::string dstId = "123";
    int32_t missionId = 0;
    int res = abilityMs_->ContinueAbility(dstId, missionId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyCompleteContinuation_001
 * @tc.desc: test NotifyCompleteContinuation
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(AbilityManagerServiceTest, NotifyCompleteContinuation_001, TestSize.Level0)
{
    std::string deviceId = "123";
    int32_t sessionId = 0;
    bool isSuccess = true;
    abilityMs_->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

/*
 * @tc.name: startAbility_005
 * @tc.desc: Verify function startAbility with illegal deviceId
 * to start an indicated device's service pa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_005, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }
    auto result = abilityMs_->StartAbility(want, token);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
}

/**
 * @tc.name: startAbility_006
 * @tc.desc: Verify function startAbility with illegal deviceId
 * to start an indicated device's data pa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_006, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.hiData", "DataAbility");
    want.SetElement(element);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }
    auto result = abilityMs_->StartAbility(want, token);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
}

/**
 * @tc.name: startAbility_007
 * @tc.desc: Verify function startAbility with illegal deviceId
 * to start an indicated device's fa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_007, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.hiMusic", "hiMusic");
    want.SetElement(element);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }
    auto result1 = abilityMs_->StartAbility(want, token);
    EXPECT_EQ(ERR_INVALID_VALUE, result1);
}

/**
 * @tc.name: startAbility_008
 * @tc.desc: Verify function startAbility with continuation flag
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerServiceTest, startAbility_008, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.hiMusic", "hiMusic");
    want.SetElement(element);
    want.SetFlags(AAFwk::Want::FLAG_ABILITY_CONTINUATION);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);
        token = topAbility->GetToken();
    }
    auto result1 = abilityMs_->StartAbility(want, token);
    EXPECT_EQ(ERR_INVALID_VALUE, result1);
}

/**
 * @tc.name: StartRemoteAbility_001
 * @tc.desc: Verify function StartRemoteAbility with illegal deviceId
 * to start a remote device's service pa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, StartRemoteAbility_001, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = abilityMs_->StartRemoteAbility(want, 0);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
}

/**
 * @tc.name: StartRemoteAbility_002
 * @tc.desc: Verify function StartRemoteAbility with illegal deviceId
 * to start a remote device's data pa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, StartRemoteAbility_002, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.hiData", "DataAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartRemoteAbility(want, 0);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
}

/**
 * @tc.name: StartRemoteAbility_003
 * @tc.desc: Verify function StartRemoteAbility with illegal deviceId
 * to start a remote device's fa, return ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require: AR000GH27H
 */
HWTEST_F(AbilityManagerServiceTest, StartRemoteAbility_003, TestSize.Level1)
{
    Want want;
    ElementName element("illegalDeviceId", "com.ix.hiMusic", "hiMusic");
    want.SetElement(element);
    auto result = abilityMs_->StartRemoteAbility(want, 0);
    EXPECT_EQ(ERR_INVALID_VALUE, result);
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, systemDialog_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.hiworld", "WorldService");
    want.SetElement(element);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto topAbility = stackManager->GetCurrentTopAbility();
    auto token = topAbility->GetToken();
    topAbility->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    // statrt a dialog
    Want want1;
    ElementName elementdialog1("", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_DIALOG_NAME);
    want1.SetElement(elementdialog1);
    auto result1 = abilityMs_->StartAbility(want1, token);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result1);
    auto dialogAbility = stackManager->GetCurrentTopAbility();
    auto dialogtoken = topAbility->GetToken();

    // current stack top is dialog
    auto stackManagerAfter = abilityMs_->GetStackManager();
    auto topAbilityAfter = stackManagerAfter->GetCurrentTopAbility();
    Want want2 = topAbilityAfter->GetWant();
    EXPECT_EQ(want1.GetElement().GetURI(), want2.GetElement().GetURI());

    auto dialogRecord = stackManager->GetAbilityRecordByToken(dialogtoken);
    EXPECT_EQ(dialogRecord->GetAbilityState(), AAFwk::AbilityState::INACTIVATING);

    // start other dialog
    Want want3;
    ElementName elementdialog2("", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_DIALOG_NAME);
    want3.SetElement(elementdialog2);
    auto result2 = abilityMs_->StartAbility(want3);

    EXPECT_NE(OHOS::ERR_OK, result2);
    auto dialogAbility2 = stackManager->GetCurrentTopAbility();
    auto dialogtoken2 = dialogAbility2->GetToken();
    EXPECT_EQ(dialogAbility2->GetAbilityState(), AAFwk::AbilityState::INITIAL);

    // start a luncher aa,should be fail
    Want wantLuncher;
    ElementName elementLun("", "com.ix.hiworld", "WorldService");
    wantLuncher.SetElement(elementLun);
    auto result3 = abilityMs_->StartAbility(wantLuncher);
    // should be fail
    EXPECT_NE(OHOS::ERR_OK, result3);
    auto topAbilityLun = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(topAbilityLun->GetStartId(), dialogAbility2->GetStartId());
}

/*
 * Feature: AbilityManagerService
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService startAbility
 * EnvConditions: NA
 * CaseDescription: Verify function startAbility
 * return nullptr when AbilityManagerService not dataAbilityManager_.
 */
HWTEST_F(AbilityManagerServiceTest, systemDialog_002, TestSize.Level1)
{
    // start dialog when device get up
    Want want;
    ElementName elementdialog("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_DIALOG_NAME);
    want.SetElement(elementdialog);
    EXPECT_TRUE(abilityMs_->GetStackManager());
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto dialogAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(dialogAbility);
    auto dialogtoken = dialogAbility->GetToken();
    EXPECT_TRUE(dialogAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    // other same aa
    Want want1;
    ElementName elementdialog1("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_DIALOG_NAME);
    want1.SetElement(elementdialog1);
    auto result1 = StartAbility(want1);
    EXPECT_EQ(OHOS::ERR_OK, result1);

    // start a page aa, waiting
    Want want2;
    ElementName elementdialog2("device", "com.ix.hiMusic", "hiMusic");
    want2.SetElement(elementdialog2);
    auto result2 = StartAbility(want2);
    EXPECT_EQ(OHOS::ERR_OK, result2);
    auto MusicAbility = stackManager->GetCurrentTopAbility();
    EXPECT_EQ(MusicAbility->GetAbilityInfo().bundleName, "com.ix.hiMusic");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_001, TestSize.Level1)
{
    // start SYSTEM_UI_STATUS_BAR when device get up
    Want want;
    ElementName elementbar("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_STATUS_BAR);
    want.SetElement(elementbar);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto barAbility = stackManager->GetCurrentTopAbility();
    AbilityRecordInfo barAbilityInfo;
    barAbility->GetAbilityRecordInfo(barAbilityInfo);
    auto dialogtoken = barAbility->GetToken();
    
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, dialogtoken), OHOS::ERR_OK);
    EXPECT_TRUE(barAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    abilityMs_->HandleLoadTimeOut(barAbility->GetEventId());

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newBarAbility = newStackManager->GetCurrentTopAbility();

    // Remove Ability Record
    EXPECT_FALSE(newBarAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_002, TestSize.Level1)
{
    // start SYSTEM_UI_NAVIGATION_BAR when device get up
    Want want;
    ElementName elementdialog("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
    want.SetElement(elementdialog);
    auto result = StartAbility(want);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto navigationAbility = stackManager->GetCurrentTopAbility();
    auto dialogtoken = navigationAbility->GetToken();

    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, dialogtoken), OHOS::ERR_OK);
    EXPECT_TRUE(navigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    abilityMs_->HandleLoadTimeOut(navigationAbility->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newNavigationAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newNavigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);
    EXPECT_NE(navigationAbility, newNavigationAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when timeout ,the HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_003, TestSize.Level1)
{
    // start SYSTEM_UI_STATUS_BAR when device get up
    Want want;
    ElementName elementbar("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_STATUS_BAR);
    want.SetElement(elementbar);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto barAbility = stackManager->GetCurrentTopAbility();
    AbilityRecordInfo barAbilityInfo;
    barAbility->GetAbilityRecordInfo(barAbilityInfo);
    EXPECT_TRUE(barAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    abilityMs_->HandleLoadTimeOut(barAbility->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newBarAbility = newStackManager->GetCurrentTopAbility();
    AbilityRecordInfo newAbilityInfo;
    newBarAbility->GetAbilityRecordInfo(newAbilityInfo);
    EXPECT_TRUE(newBarAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);
    EXPECT_NE(barAbilityInfo.id, newAbilityInfo.id);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when timeout ,the HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_004, TestSize.Level1)
{
    // start SYSTEM_UI_NAVIGATION_BAR when device get up
    Want want;
    ElementName elementbar("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
    want.SetElement(elementbar);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto navigationAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(navigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    abilityMs_->HandleLoadTimeOut(navigationAbility->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newNavigationAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newNavigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);
    EXPECT_NE(navigationAbility, newNavigationAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_005, TestSize.Level1)
{
    // start SYSTEM_UI_STATUS_BAR when device get up
    Want want;
    ElementName elementbar("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_STATUS_BAR);
    want.SetElement(elementbar);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto barAbility = stackManager->GetCurrentTopAbility();
    auto bartoken = barAbility->GetToken();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, bartoken), OHOS::ERR_OK);
    EXPECT_TRUE(barAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newBarAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newBarAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);
    EXPECT_EQ(barAbility, newBarAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start systemui, when HandleLoadTimeOut called, restart systemui
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_006, TestSize.Level1)
{
    // start SYSTEM_UI_NAVIGATION_BAR when device get up
    Want want;
    ElementName elementbar("device", AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
    want.SetElement(elementbar);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->systemAppManager_;
    auto navigationAbility = stackManager->GetCurrentTopAbility();
    auto navigationtoken = navigationAbility->GetToken();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();

    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, navigationtoken), OHOS::ERR_OK);
    EXPECT_TRUE(navigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);

    auto newStackManager = abilityMs_->systemAppManager_;
    auto newNavigationAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newNavigationAbility->GetAbilityInfo().bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME);
    EXPECT_EQ(navigationAbility, newNavigationAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when HandleLoadTimeOut called, restart previous ability
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_007, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, abilityToken), OHOS::ERR_OK);
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == "com.ix.hiMusic");

    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();

    abilityMs_->HandleLoadTimeOut(abilityTv->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->GetStackManager();
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName == "com.ix.hiMusic");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when HandleLoadTimeOut called, restart previous ability
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_008, TestSize.Level1)
{
    Want want;
    ElementName element("device", COM_IX_HIWORLD, "helloAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();

    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == COM_IX_HIWORLD);
 
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);
    
    // helloAbility inactive
    stackManager->CompleteInactive(ability);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();
    auto abilityTokenTv = abilityTv->GetToken();
    OHOS::sptr<IAbilityScheduler> newScheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(newScheduler, abilityTokenTv), OHOS::ERR_OK);
    abilityTv->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMs_->HandleLoadTimeOut(abilityTv->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->GetStackManager();
    EXPECT_TRUE(newStackManager);
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName == COM_IX_HIWORLD);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when Activate timeout, the HandleLoadTimeOut called, restart launch
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_009, TestSize.Level1)
{
    Want want;
    ElementName element("device", COM_IX_HIWORLD, "helloAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();

    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == COM_IX_HIWORLD);

    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);
    
    // helloAbility inactive
    stackManager->CompleteInactive(ability);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();
    auto abilityTokenTv = abilityTv->GetToken();
    OHOS::sptr<IAbilityScheduler> newScheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(newScheduler, abilityTokenTv), OHOS::ERR_OK);
    abilityTv->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMs_->HandleLoadTimeOut(abilityTv->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->GetStackManager();
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName != "com.ix.hiTv");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when InActivate timeout, the HandleLoadTimeOut called, restart launch
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_010, TestSize.Level1)
{
    Want want;
    ElementName element("device", COM_IX_HIWORLD, "helloAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == COM_IX_HIWORLD);

    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);

    // helloAbility inactive
    stackManager->CompleteInactive(ability);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();
    auto abilityTokenTv = abilityTv->GetToken();
    OHOS::sptr<IAbilityScheduler> newScheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(newScheduler, abilityTokenTv), OHOS::ERR_OK);
    abilityTv->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMs_->HandleLoadTimeOut(abilityTv->GetEventId());
    WaitUntilTaskFinishedByTimer();

    auto newStackManager = abilityMs_->GetStackManager();
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName != "com.ix.hiTv");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when HandleLoadTimeOut called,the parameter is the maximum,nothing is done.
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_011, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == "com.ix.hiMusic");

    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);

    // helloAbility inactive
    stackManager->DispatchInactive(ability, INACTIVE);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();

    abilityMs_->HandleLoadTimeOut(INT32_MAX);
  
    auto newStackManager = abilityMs_->GetStackManager();
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName == "com.ix.hiTv");
    EXPECT_NE(ability, newAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 * When you start page ability, when HandleLoadTimeOut called,the parameter is the minimum,nothing is done.
 */
HWTEST_F(AbilityManagerServiceTest, handleloadtimeout_012, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == "com.ix.hiMusic");

    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);

    // helloAbility inactive
    stackManager->DispatchInactive(ability, INACTIVE);

    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();

    abilityMs_->HandleLoadTimeOut(INT32_MIN);

    auto newStackManager = abilityMs_->GetStackManager();
    auto newAbility = newStackManager->GetCurrentTopAbility();
    EXPECT_TRUE(newAbility->GetAbilityInfo().bundleName == "com.ix.hiTv");
    EXPECT_NE(ability, newAbility);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * IsFirstInMission parameter
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_001, TestSize.Level1)
{
    auto result = abilityMs_->IsFirstInMission(nullptr);
    EXPECT_EQ(result, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * IsFirstInMission parameter
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    AbilityRequest requestInfo;
    requestInfo.want = ability->GetWant();
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.appInfo = ability->GetApplicationInfo();
    requestInfo.requestCode = ability->GetRequestCode();
    auto abilitySelf = AbilityRecord::CreateAbilityRecord(requestInfo);
    auto resultFunction = abilityMs_->IsFirstInMission(abilitySelf->GetToken());
    EXPECT_EQ(resultFunction, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * IsFirstInMission parameter
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    abilityMs_->currentStackManager_ = nullptr;
    auto resultFunction = abilityMs_->IsFirstInMission(abilityToken);
    EXPECT_EQ(resultFunction, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * IsFirstInMission parameter
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_004, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    auto resultFunction = abilityMs_->IsFirstInMission(abilityToken);
    EXPECT_EQ(resultFunction, true);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * Checks whether this ability is the first ability in a mission.
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_005, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, abilityToken), OHOS::ERR_OK);
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == "com.ix.hiMusic");
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);
    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();
    EXPECT_TRUE(abilityTv->GetAbilityInfo().bundleName == "com.ix.hiTv");
    auto resultFunction = abilityMs_->IsFirstInMission(abilityToken);
    EXPECT_EQ(resultFunction, true);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * Checks whether this ability is the first ability in a mission.
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_006, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(scheduler, abilityToken), OHOS::ERR_OK);
    EXPECT_TRUE(ability->GetAbilityInfo().bundleName == "com.ix.hiMusic");
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    auto resultTv = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultTv);
    auto stackManagerTv = abilityMs_->GetStackManager();
    auto abilityTv = stackManagerTv->GetCurrentTopAbility();
    EXPECT_TRUE(abilityTv->GetAbilityInfo().bundleName == "com.ix.hiTv");
    auto resultFunction = abilityMs_->IsFirstInMission(abilityTv->GetToken());
    EXPECT_EQ(resultFunction, false);
}

/*
 * Feature: AbilityManagerService
 * Function: IsFirstInMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsFirstInMission
 * EnvConditions: NA
 * CaseDescription: Verify function IsFirstInMission
 * Checks whether this ability is the first ability in a mission.
 */
HWTEST_F(AbilityManagerServiceTest, isfirstinmission_007, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    auto resultTvFunction = abilityMs_->IsFirstInMission(abilityTv->GetToken());
    EXPECT_EQ(resultTvFunction, true);
    auto resultMusicFunction = abilityMs_->IsFirstInMission(ability->GetToken());
    EXPECT_EQ(resultMusicFunction, true);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * MoveMissionToEnd parameter
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_001, TestSize.Level1)
{
    auto result = abilityMs_->MoveMissionToEnd(nullptr, true);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * MoveMissionToEnd parameter
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_002, TestSize.Level1)
{
    auto result = abilityMs_->MoveMissionToEnd(nullptr, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * MoveMissionToEnd parameter, Only one MissionRecord in Stack,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    ability->SetAbilityState(AbilityState::BACKGROUND);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityToken, true);
    EXPECT_EQ(resultFunction, MOVE_MISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * MoveMissionToEnd parameter, Only one MissionRecord in Stack,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_004, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    auto abilityToken = ability->GetToken();
    ability->SetAbilityState(AbilityState::BACKGROUND);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityToken, false);
    EXPECT_EQ(resultFunction, MOVE_MISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When AbilityState not equal ACTIVE,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_005, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityTv->GetToken(), false);
    EXPECT_EQ(resultFunction, OHOS::ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When AbilityState not equal ACTIVE,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_006, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityTv->GetToken(), true);
    EXPECT_EQ(resultFunction, OHOS::ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When AbilityState not equal ACTIVE,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_007, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityTv->GetToken(), false);
    EXPECT_EQ(resultFunction, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When AbilityState not equal ACTIVE,MoveMissionToEnd failed
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_008, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityTv->GetToken(), true);
    EXPECT_EQ(resultFunction, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When this ability is not the first ability in a mission,
 * and MoveMissionToEnd second parameter is false.
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_009, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    ElementName elementPhone("device", "com.ix.hiMusic", "PhoneAbility");
    want.SetElement(elementPhone);
    auto resultPhone = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultPhone);
    auto abilityPhone = stackManager->GetCurrentTopAbility();
    abilityPhone->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityPhone->GetToken(), false);
    EXPECT_EQ(resultFunction, MOVE_MISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When this ability is not the first ability in a mission,
 * and MoveMissionToEnd second parameter is true.
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_010, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    ElementName elementPhone("device", "com.ix.hiMusic", "PhoneAbility");
    want.SetElement(elementPhone);
    auto resultPhone = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultPhone);
    auto abilityPhone = stackManager->GetCurrentTopAbility();
    abilityPhone->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(abilityPhone->GetToken(), true);
    EXPECT_EQ(resultFunction, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When this ability is the first ability in a mission,
 * and MoveMissionToEnd second parameter is false,but this ability AbilityState not equal ACTIVE.
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_011, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    ElementName elementPhone("device", "com.ix.hiPhone", "PhoneAbility");
    want.SetElement(elementPhone);
    auto resultPhone = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultPhone);
    auto abilityPhone = stackManager->GetCurrentTopAbility();
    abilityPhone->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(ability->GetToken(), false);
    EXPECT_EQ(resultFunction, MOVE_MISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToEnd
 * EnvConditions: NA
 * CaseDescription: Verify function MoveMissionToEnd
 * Multiple MissionRecord in Stack,When this ability is the first ability in a mission,
 * and MoveMissionToEnd second parameter is true,but this ability AbilityState not equal ACTIVE.
 */
HWTEST_F(AbilityManagerServiceTest, movemissiontoend_012, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto stackManager = abilityMs_->GetStackManager();
    auto ability = stackManager->GetCurrentTopAbility();
    ability->SetAbilityState(AbilityState::ACTIVE);
    ElementName elementPhone("device", "com.ix.hiPhone", "PhoneAbility");
    want.SetElement(elementPhone);
    auto resultPhone = StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, resultPhone);
    auto abilityPhone = stackManager->GetCurrentTopAbility();
    abilityPhone->SetAbilityState(AbilityState::ACTIVE);
    AbilityRequest requestInfo;
    ElementName elementTv("device", "com.ix.hiTv", "TvAbility");
    want.SetElement(elementTv);
    requestInfo.want = want;
    requestInfo.abilityInfo = ability->GetAbilityInfo();
    requestInfo.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    result = stackManager->StartAbility(requestInfo);
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    abilityTv->SetAbilityState(AbilityState::ACTIVE);
    auto resultFunction = abilityMs_->MoveMissionToEnd(ability->GetToken(), true);
    EXPECT_EQ(resultFunction, MOVE_MISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: CompelVerifyPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Verify function CompelVerifyPermission
 */
HWTEST_F(AbilityManagerServiceTest, compelverifypermission_001, TestSize.Level1)
{
    const std::string permission = "permission";
    int pid = 100;
    int uid = 1000;
    std::string message;
    auto resultFunction = abilityMs_->CompelVerifyPermission(permission, pid, uid, message);
    EXPECT_EQ(resultFunction, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: AmsConfigurationParameter
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Already configured
 */
HWTEST_F(AbilityManagerServiceTest, AmsConfigurationParameter_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_->amsConfigResolver_);
    EXPECT_FALSE(abilityMs_->amsConfigResolver_->NonConfigFile());
}

/*
 * Feature: AbilityManagerService
 * Function: AmsConfigurationParameter
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Already configured
 */
HWTEST_F(AbilityManagerServiceTest, AmsConfigurationParameter_002, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_->amsConfigResolver_);
    // At present, all three are started and may be changed later
    EXPECT_TRUE(abilityMs_->amsConfigResolver_->GetStartLauncherState());
    EXPECT_TRUE(abilityMs_->amsConfigResolver_->GetStatusBarState());
    EXPECT_TRUE(abilityMs_->amsConfigResolver_->GetNavigationBarState());
}

/*
 * Feature: AbilityManagerService
 * Function: AmsConfigurationParameter
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Already configured
 */
HWTEST_F(AbilityManagerServiceTest, AmsConfigurationParameter_003, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_->amsConfigResolver_);
    auto ref = abilityMs_->amsConfigResolver_->LoadAmsConfiguration(" ");
    EXPECT_EQ(ref, 1);
}

/*
 * Function: AmsGetSystemMemoryAttr
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Already configured
 */
HWTEST_F(AbilityManagerServiceTest, AmsGetSystemMemoryAttr_001, TestSize.Level1)
{
    OHOS::AppExecFwk::SystemMemoryAttr memInfo;

    memInfo.availSysMem_ = -1;
    memInfo.totalSysMem_ = -1;
    memInfo.threshold_ = -1;

    abilityMs_->GetSystemMemoryAttr(memInfo);

    EXPECT_NE(-1, memInfo.availSysMem_);
    EXPECT_NE(-1, memInfo.totalSysMem_);
    EXPECT_NE(-1, memInfo.threshold_);
}

/**
 * @tc.name: CheckIfOperateRemote_001
 * @tc.desc: Verify function CheckIfOperateRemote if operate local or remote
 * with empty deviceId
 * @tc.type: FUNC
 * @tc.require: AR000GH27N
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    EXPECT_EQ(false, abilityMs_->CheckIfOperateRemote(want));
}

/**
 * @tc.name: CheckIfOperateRemote_002
 * @tc.desc: Verify function CheckIfOperateRemote if operate local or remote
 * with empty bundle name
 * @tc.type: FUNC
 * @tc.require: AR000GH27N
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "", "MusicAbility");
    want.SetElement(element);
    EXPECT_EQ(false, abilityMs_->CheckIfOperateRemote(want));
}

/**
 * @tc.name: CheckIfOperateRemote_003
 * @tc.desc: Verify function CheckIfOperateRemote if operate local or remote
 * with empty ability name
 * @tc.type: FUNC
 * @tc.require: AR000GH27N
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "");
    want.SetElement(element);
    EXPECT_EQ(false, abilityMs_->CheckIfOperateRemote(want));
}

/**
 * @tc.name: CheckIfOperateRemote_004
 * @tc.desc: Verify function CheckIfOperateRemote if operate local or remote
 * with local deviceId
 * @tc.type: FUNC
 * @tc.require: AR000GH27N
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_004, TestSize.Level1)
{
    Want want;
    std::string localDeviceId;
    bool getLocalDeviceId = abilityMs_->GetLocalDeviceId(localDeviceId);
    EXPECT_EQ(true, getLocalDeviceId);
    ElementName element(localDeviceId, "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    EXPECT_EQ(false, abilityMs_->CheckIfOperateRemote(want));
}

/**
 * @tc.name: CheckIfOperateRemote_005
 * @tc.desc: Verify function CheckIfOperateRemote if operate local or remote
 * with deviceId, bundle name and ability name
 * @tc.type: FUNC
 * @tc.require: AR000GH27N
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_005, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    EXPECT_EQ(true, abilityMs_->CheckIfOperateRemote(want));
}

}  // namespace AAFwk
}  // namespace OHOS
