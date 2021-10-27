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

#include <thread>
#include <functional>
#include <fstream>
#include <nlohmann/json.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#define protected public
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback_stub.h"
#include "mock_ability_scheduler.h"
#include "mock_app_mgr_client.h"
#include "mock_bundle_mgr.h"
#include "ability_record_info.h"
#include "ability_manager_errors.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ability_manager_service.h"
#include "ability_connect_callback_proxy.h"
#include "ability_config.h"
#undef private
#undef protected

using namespace std::placeholders;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::Return;

namespace OHOS {
namespace AAFwk {
class AbilityMgrModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    Want CreateWant(const std::string &entity);
    AbilityInfo CreateAbilityInfo(const std::string &name, const std::string &appName, const std::string &bundleName);
    ApplicationInfo CreateAppInfo(const std::string &appName, const std::string &name);
    Want CreateWant(const std::string &abilityName, const std::string &bundleName);
    void WaitAMS();
    std::shared_ptr<AbilityRecord> GreatePageAbility(const std::string &abilityName, const std::string &bundleName);
    void MockAbilityTransitionDone(bool &testFailed, sptr<IRemoteObject> &dataAbilityToken,
        sptr<MockAbilityScheduler> &mockDataAbilityScheduler, std::shared_ptr<AbilityManagerService> &abilityMgrServ);
    void MockDataAbilityLoadHandlerInner(bool &testFailed, sptr<IRemoteObject> &dataAbilityToken,
        sptr<MockAbilityScheduler> &mockDataAbilityScheduler, std::shared_ptr<AbilityManagerService> &abilityMgrServ);
    void CreateAbilityRequest(const std::string &abilityName, const std::string bundleName, Want &want,
        std::shared_ptr<MissionStack> &curMissionStack, sptr<IRemoteObject> &recordToken);
    void MockServiceAbilityLoadHandlerInner(bool &testResult, const std::string &bundleName,
        const std::string &abilityName, sptr<IRemoteObject> &testToken);
    void CreateServiceRecord(std::shared_ptr<AbilityRecord> &record, Want &want,
        const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2);
    void CheckTestRecord(std::shared_ptr<AbilityRecord> &record1, std::shared_ptr<AbilityRecord> &record2,
        const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2);
    void MockLoadHandlerInner(int &testId, sptr<MockAbilityScheduler> &scheduler);
    bool MockAppClent();
    void SetActive();
    std::shared_ptr<AbilityRecord> GetTopAbility();
    void ClearStack();

    inline static std::shared_ptr<MockAppMgrClient> mockAppMgrClient_{nullptr};
    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_{nullptr};
    sptr<MockAbilityScheduler> scheduler_{nullptr};
    inline static bool doOnce_ = false;  // In order for mock to execute once

    static constexpr int TEST_WAIT_TIME = 100000;
};

void AbilityMgrModuleTest::SetActive()
{
    if (!abilityMgrServ_) {
        return;
    }

    auto stackMgr = abilityMgrServ_->GetStackManager();
    if (stackMgr) {
        auto topAbility = stackMgr->GetCurrentTopAbility();
        if (topAbility) {
            topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityMgrModuleTest::GetTopAbility()
{
    if (!abilityMgrServ_) {
        return nullptr;
    }
    auto stackMgr = abilityMgrServ_->GetStackManager();
    if (stackMgr) {
        auto topAbility = stackMgr->GetCurrentTopAbility();
        if (topAbility) {
            return topAbility;
        }
    }
    return nullptr;
}

void AbilityMgrModuleTest::ClearStack()
{
    if (!abilityMgrServ_) {
        return;
    }
    auto stackMgr = abilityMgrServ_->GetStackManager();
    if (stackMgr) {
        auto topAbility = stackMgr->GetCurrentTopAbility();
        stackMgr->missionStackList_.front()->RemoveAll();
    }
}

bool AbilityMgrModuleTest::MockAppClent()
{
    if (!mockAppMgrClient_) {
        GTEST_LOG_(INFO) << "MockAppClent::1";
        return false;
    }

    if (!abilityMgrServ_->appScheduler_) {
        GTEST_LOG_(INFO) << "MockAppClent::2";
        return false;
    }

    abilityMgrServ_->appScheduler_->appMgrClient_.reset(mockAppMgrClient_.get());
    return true;
}

static void OnStartAms()
{
    if (AbilityMgrModuleTest::abilityMgrServ_) {
        if (AbilityMgrModuleTest::abilityMgrServ_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }
   
        AbilityMgrModuleTest::abilityMgrServ_->state_ = ServiceRunningState::STATE_RUNNING;
        
        AbilityMgrModuleTest::abilityMgrServ_->eventLoop_ = 
            AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->eventLoop_);

        AbilityMgrModuleTest::abilityMgrServ_->handler_ = 
            std::make_shared<AbilityEventHandler>(AbilityMgrModuleTest::abilityMgrServ_->eventLoop_, 
                                                    AbilityMgrModuleTest::abilityMgrServ_);
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->handler_);
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->connectManager_);

        AbilityMgrModuleTest::abilityMgrServ_->connectManager_-> \
            SetEventHandler(AbilityMgrModuleTest::abilityMgrServ_->handler_);

        AbilityMgrModuleTest::abilityMgrServ_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->dataAbilityManager_);

        AbilityMgrModuleTest::abilityMgrServ_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->amsConfigResolver_);
        AbilityMgrModuleTest::abilityMgrServ_->amsConfigResolver_->Parse();

        AbilityMgrModuleTest::abilityMgrServ_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->pendingWantManager_);
      
        int userId = AbilityMgrModuleTest::abilityMgrServ_->GetUserId();
        AbilityMgrModuleTest::abilityMgrServ_->SetStackManager(userId);
        AbilityMgrModuleTest::abilityMgrServ_->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
        EXPECT_TRUE(AbilityMgrModuleTest::abilityMgrServ_->systemAppManager_);

        AbilityMgrModuleTest::abilityMgrServ_->eventLoop_->Run();

        GTEST_LOG_(INFO)<< "OnStart success";
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityMgrModuleTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());

    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    mockAppMgrClient_ = std::make_shared<MockAppMgrClient>();

    OnStartAms();
}

void AbilityMgrModuleTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
    abilityMgrServ_.reset();
    mockAppMgrClient_.reset();
}
void AbilityMgrModuleTest::SetUp(void)
{
    scheduler_ = new MockAbilityScheduler();
    if (!doOnce_) {
        doOnce_ = true;

        MockAppClent();
    }
    WaitAMS();
}

void AbilityMgrModuleTest::TearDown(void)
{}

Want AbilityMgrModuleTest::CreateWant(const std::string &entity)
{
    Want want;
    if (!entity.empty()) {
        want.AddEntity(entity);
    }
    return want;
}

AbilityInfo AbilityMgrModuleTest::CreateAbilityInfo(
    const std::string &name, const std::string &appName, const std::string &bundleName)
{
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationName = "hiMusic";
    abilityInfo.applicationInfo.name = "hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    return abilityInfo;
}

ApplicationInfo AbilityMgrModuleTest::CreateAppInfo(const std::string &appName, const std::string &bundleName)
{
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

Want AbilityMgrModuleTest::CreateWant(const std::string &abilityName, const std::string &bundleName)
{
    ElementName element;
    element.SetDeviceID("devices");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return want;
}

std::shared_ptr<AbilityRecord> AbilityMgrModuleTest::GreatePageAbility(
    const std::string &abilityName, const std::string &bundleName)
{
    Want want = CreateWant(abilityName, bundleName);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    int testRequestCode = 1;
    SetActive();
    abilityMgrServ_->StartAbility(want, testRequestCode);
    WaitAMS();

    auto stack = abilityMgrServ_->GetStackManager();
    if (!stack) {
        return nullptr;
    }
    auto topAbility = stack->GetCurrentTopAbility();
    if (!topAbility) {
        return nullptr;
    }
    topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    return topAbility;
}

void AbilityMgrModuleTest::MockAbilityTransitionDone(bool &testFailed, sptr<IRemoteObject> &dataAbilityToken,
    sptr<MockAbilityScheduler> &mockDataAbilityScheduler, std::shared_ptr<AbilityManagerService> &abilityMgrServ)
{
    auto mockAbilityTransation = [&testFailed, &dataAbilityToken, &abilityMgrServ](
                                     const Want &want, const LifeCycleStateInfo &targetState) {
        testFailed = testFailed || (targetState.state != ABILITY_STATE_ACTIVE);
        std::thread(&AbilityManagerService::AbilityTransitionDone, abilityMgrServ.get(), dataAbilityToken, ACTIVE)
            .detach();
    };

    EXPECT_CALL(*mockDataAbilityScheduler, ScheduleAbilityTransaction(_, _))
        .Times(1)
        .WillOnce(Invoke(mockAbilityTransation));
}

void AbilityMgrModuleTest::MockDataAbilityLoadHandlerInner(bool &testFailed, sptr<IRemoteObject> &dataAbilityToken,
    sptr<MockAbilityScheduler> &mockDataAbilityScheduler, std::shared_ptr<AbilityManagerService> &abilityMgrServ)
{
    // MOCK: data ability load handler

    auto mockLoadAbility = [&testFailed, &dataAbilityToken, &mockDataAbilityScheduler, &abilityMgrServ](
                               const sptr<IRemoteObject> &token,
                               const sptr<IRemoteObject> &preToken,
                               const AbilityInfo &abilityInfo,
                               const ApplicationInfo &appInfo) {
        dataAbilityToken = token;
        testFailed = testFailed || (abilityInfo.type != AbilityType::DATA);
        std::thread(&AbilityManagerService::AttachAbilityThread, abilityMgrServ.get(), mockDataAbilityScheduler, token)
            .detach();
        return AppMgrResultCode::RESULT_OK;
    };

    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Invoke(mockLoadAbility));
    int counts = 2;
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _))
        .Times(counts)
        .WillOnce(Invoke([&testFailed](const sptr<IRemoteObject> &token, const OHOS::AppExecFwk::AbilityState state) {
            testFailed = testFailed || (state != OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
            return AppMgrResultCode::RESULT_OK;
        }))
        .WillOnce(Invoke([&testFailed](const sptr<IRemoteObject> &token, const OHOS::AppExecFwk::AbilityState state) {
            testFailed = testFailed || (state != OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
            return AppMgrResultCode::RESULT_OK;
        }));
}

void AbilityMgrModuleTest::CreateAbilityRequest(const std::string &abilityName, const std::string bundleName,
    Want &want, std::shared_ptr<MissionStack> &curMissionStack, sptr<IRemoteObject> &recordToken)
{
    Want want2 = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest2;
    abilityRequest2.want = want2;
    abilityRequest2.abilityInfo.type = OHOS::AppExecFwk::AbilityType::PAGE;
    abilityRequest2.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest2);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord2);
    auto stackManager_ = abilityMgrServ_->GetStackManager();
    curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);
    recordToken = abilityRecord2->GetToken();

    want = CreateWant(abilityName + "_service", bundleName + "_service");
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo =
        CreateAbilityInfo(abilityName + "_service", bundleName + "_service", bundleName + "_service");
    abilityRequest.abilityInfo.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    abilityRequest.appInfo = CreateAppInfo(bundleName + "_service", bundleName + "_service");
    abilityMgrServ_->RemoveAllServiceRecord();
}

void AbilityMgrModuleTest::MockServiceAbilityLoadHandlerInner(
    bool &testResult, const std::string &bundleName, const std::string &abilityName, sptr<IRemoteObject> &testToken)
{
    auto mockHandler = [&testResult, &bundleName, &abilityName, &testToken](const sptr<IRemoteObject> &token,
                           const sptr<IRemoteObject> &preToken,
                           const AbilityInfo &abilityInfo,
                           const ApplicationInfo &appInfo) {
        testToken = token;
        testResult = !!testToken && abilityInfo.bundleName == bundleName && abilityInfo.name == abilityName &&
                     appInfo.bundleName == bundleName;
        return AppMgrResultCode::RESULT_OK;
    };

    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));
}

void AbilityMgrModuleTest::CreateServiceRecord(std::shared_ptr<AbilityRecord> &record, Want &want,
    const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2)
{
    record = abilityMgrServ_->connectManager_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_TRUE(abilityMgrServ_->connectManager_->IsAbilityConnected(
        record, abilityMgrServ_->GetConnectRecordListByCallback(callback1)));
    EXPECT_TRUE(abilityMgrServ_->connectManager_->IsAbilityConnected(
        record, abilityMgrServ_->GetConnectRecordListByCallback(callback2)));
    int size = 2;
    EXPECT_EQ((std::size_t)size, record->GetConnectRecordList().size());
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
}

void AbilityMgrModuleTest::CheckTestRecord(std::shared_ptr<AbilityRecord> &record1,
    std::shared_ptr<AbilityRecord> &record2, const sptr<AbilityConnectionProxy> &callback1,
    const sptr<AbilityConnectionProxy> &callback2)
{
    int size = 2;
    int counts = 2;
    EXPECT_EQ((std::size_t)1, record1->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)1, record2->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->GetConnectRecordListByCallback(callback1).size());
    EXPECT_EQ((std::size_t)size, abilityMgrServ_->connectManager_->GetServiceMap().size());

    abilityMgrServ_->DisconnectAbility(callback2);

    abilityMgrServ_->ScheduleDisconnectAbilityDone(record1->GetToken());
    abilityMgrServ_->ScheduleDisconnectAbilityDone(record2->GetToken());
    EXPECT_EQ((std::size_t)0, record1->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, record2->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->GetConnectRecordListByCallback(callback2).size());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(counts);
    abilityMgrServ_->AbilityTransitionDone(record1->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);
    abilityMgrServ_->AbilityTransitionDone(record2->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record1->GetAbilityState());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record2->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(counts);
    abilityMgrServ_->OnAbilityRequestDone(
        record1->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    abilityMgrServ_->OnAbilityRequestDone(
        record2->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetConnectMap().size());
}

void AbilityMgrModuleTest::MockLoadHandlerInner(int &testId, sptr<MockAbilityScheduler> &scheduler)
{
    auto handler = [&testId](const Want &want, bool restart, int startid) { testId = startid; };
    int counts = 3;
    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _))
        .Times(counts)
        .WillOnce(Invoke(handler))
        .WillOnce(Invoke(handler))
        .WillOnce(Invoke(handler));
}

void AbilityMgrModuleTest::WaitAMS()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (!abilityMgrServ_) {
        return;
    }
    auto handler = abilityMgrServ_->GetEventHandler();
    if (!handler) {
        return;
    }
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

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);

    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";
    Want want = CreateWant(abilityName, bundleName);
    bool testResult = false;
    sptr<IRemoteObject> testToken;

    auto mockHandler = [&](const sptr<IRemoteObject> &token,
                           const sptr<IRemoteObject> &preToken,
                           const AbilityInfo &abilityInfo,
                           const ApplicationInfo &appInfo) {
        testToken = token;
        testResult = !!testToken && abilityInfo.bundleName == bundleName && abilityInfo.name == abilityName &&
                     appInfo.bundleName == bundleName;
        return AppMgrResultCode::RESULT_OK;
    };

    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);

    EXPECT_TRUE(testResult);
    EXPECT_TRUE(testToken);

    auto testAbilityRecord = stackManager->GetAbilityRecordByToken(testToken);
    EXPECT_TRUE(testAbilityRecord);
    EXPECT_EQ(testAbilityRecord->GetRequestCode(), testRequestCode);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: terminate ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_002, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "1", appName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);

    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "2", appName, bundleName);
    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    abilityRecord2->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);
    mission->AddAbilityRecordToTop(abilityRecord2);
    abilityRecord2->SetMissionRecord(mission);

    auto stackManager_ = abilityMgrServ_->GetStackManager();
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    int result = abilityMgrServ_->TerminateAbility(abilityRecord2->GetToken(), -1, nullptr);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, abilityRecord2->GetAbilityState());
    EXPECT_EQ(2, mission->GetAbilityRecordCount());
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: connect ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_003, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    Want want2 = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest2;
    abilityRequest2.want = want2;
    abilityRequest2.abilityInfo.type = OHOS::AppExecFwk::AbilityType::PAGE;
    abilityRequest2.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest2);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord2);
    auto stackManager_ = abilityMgrServ_->GetStackManager();
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    Want want = CreateWant(abilityName + "_service", bundleName + "_service");
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "_service", bundleName, bundleName + "_service");
    abilityRequest.abilityInfo.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName + "_service");
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMgrServ_->RemoveAllServiceRecord();
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    std::shared_ptr<ConnectionRecord> connectRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord->GetToken(), abilityRecord, callback);
    EXPECT_TRUE(connectRecord != nullptr);
    connectRecord->SetConnectState(ConnectionState::CONNECTED);
    abilityRecord->AddConnectRecordToList(connectRecord);
    std::list<std::shared_ptr<ConnectionRecord> > connectList;
    connectList.push_back(connectRecord);
    abilityMgrServ_->connectManager_->connectMap_.emplace(callback->AsObject(), connectList);
    abilityMgrServ_->connectManager_->serviceMap_.emplace(abilityRequest.want.GetElement().GetURI(), abilityRecord);

    int result = abilityMgrServ_->ConnectAbility(want, callback, abilityRecord2->GetToken());
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());

    abilityMgrServ_->RemoveAllServiceRecord();
    curMissionStack->RemoveAll();

    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: OnStart/OnStop
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: OnStart/OnStop
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_004, TestSize.Level1)
{
    // It's turned on during initialization, so it's turned off here
    abilityMgrServ_->state_ = ServiceRunningState::STATE_NOT_START;

    auto state = abilityMgrServ_->QueryServiceState();
    EXPECT_EQ(state, ServiceRunningState::STATE_NOT_START);

    abilityMgrServ_->state_ = ServiceRunningState::STATE_RUNNING;
    WaitAMS();

    EXPECT_TRUE(abilityMgrServ_->dataAbilityManager_);
    state = abilityMgrServ_->QueryServiceState();
    EXPECT_EQ(state, ServiceRunningState::STATE_RUNNING);

    auto handler = abilityMgrServ_->GetEventHandler();
    EXPECT_TRUE(handler);
    auto eventRunner = handler->GetEventRunner();
    EXPECT_TRUE(eventRunner);
    EXPECT_TRUE(eventRunner->IsRunning());

    abilityMgrServ_->OnStop();

    state = abilityMgrServ_->QueryServiceState();
    EXPECT_EQ(state, ServiceRunningState::STATE_NOT_START);
    EXPECT_FALSE(abilityMgrServ_->GetEventHandler());
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: AddWindowInfo.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_005, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    Want want = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);
    auto stackManager_ = abilityMgrServ_->GetStackManager();
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    abilityMgrServ_->AddWindowInfo(abilityRecord->GetToken(), 1);
    EXPECT_TRUE(abilityRecord->GetWindowInfo() != nullptr);
    EXPECT_EQ((std::size_t)1, stackManager_->windowTokenToAbilityMap_.size());
    abilityRecord->RemoveWindowInfo();
    stackManager_->windowTokenToAbilityMap_.clear();
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: AttachAbilityThread.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_006, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    Want want = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(abilityRecord);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    EXPECT_TRUE(mission);
    mission->AddAbilityRecordToTop(abilityRecord);

    auto stackManager_ = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager_);
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);
    sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());

    EXPECT_CALL(*scheduler, AsObject()).Times(4);
    abilityRecord->SetScheduler(scheduler);
    auto eventLoop = OHOS::AppExecFwk::EventRunner::Create("NAME_ABILITY_MGR_SERVICE");
    std::shared_ptr<AbilityEventHandler> handler = std::make_shared<AbilityEventHandler>(eventLoop, abilityMgrServ_);
    abilityMgrServ_->handler_ = handler;
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    abilityMgrServ_->AttachAbilityThread(scheduler, abilityRecord->GetToken());

    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: AbilityTransitionDone.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_007, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    Want want = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_TRUE(abilityRecord);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    EXPECT_TRUE(mission);
    mission->AddAbilityRecordToTop(abilityRecord);
    auto stackManager_ = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager_);
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(curMissionStack);
    curMissionStack->AddMissionRecordToTop(mission);

    int result = abilityMgrServ_->AbilityTransitionDone(abilityRecord->GetToken(), OHOS::AAFwk::AbilityState::ACTIVE);
    usleep(50 * 1000);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::ACTIVE, abilityRecord->GetAbilityState());
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: connect ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_008, TestSize.Level1)
{

    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    Want want;
    std::shared_ptr<MissionStack> curMissionStack;
    sptr<IRemoteObject> recordToken;
    CreateAbilityRequest(abilityName, bundleName, want, curMissionStack, recordToken);

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

    bool testResult = false;
    sptr<IRemoteObject> testToken;
    MockServiceAbilityLoadHandlerInner(testResult, bundleName, abilityName, testToken);

    int result = abilityMgrServ_->ConnectAbility(want, callback, recordToken);
    EXPECT_EQ(testResult, result);

    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->connectManager_->GetServiceRecordByToken(testToken);
    EXPECT_TRUE(record);
    ElementName element;

    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectRecordList().front();
    EXPECT_TRUE(connectRecord);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);

    EXPECT_CALL(*scheduler, AsObject()).Times(1);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    EXPECT_TRUE(record->GetConnectingRecord());
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::CONNECTING, connectRecord->GetConnectState());

    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);

    EXPECT_EQ(OHOS::AAFwk::ConnectionState::CONNECTED, connectRecord->GetConnectState());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::ACTIVE, record->GetAbilityState());

    abilityMgrServ_->RemoveAllServiceRecord();
    curMissionStack->RemoveAll();

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager services
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: disconnect ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_009, TestSize.Level1)
{

    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    std::shared_ptr<AbilityRecord> record =
        abilityMgrServ_->connectManager_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    connectRecord->SetConnectState(ConnectionState::CONNECTED);
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, ScheduleDisconnectAbility(_)).Times(1);
    abilityMgrServ_->DisconnectAbility(callback);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTING, connectRecord->GetConnectState());

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);

    abilityMgrServ_->ScheduleDisconnectAbilityDone(record->GetToken());
    EXPECT_EQ((std::size_t)0, record->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetConnectMap().size());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: disconnect ability.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_010, TestSize.Level1)
{
    std::string abilityName1 = "MusicAbility_service_1";
    std::string bundleName1 = "com.ix.hiMusic_service_1";
    Want want1 = CreateWant(abilityName1, bundleName1);
    std::string abilityName2 = "MusicAbility_service_2";
    std::string bundleName2 = "com.ix.hiMusic_service_2";
    Want want2 = CreateWant(abilityName2, bundleName2);

    sptr<MockAbilityConnectCallbackStub> stub1(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback1(new AbilityConnectionProxy(stub1));
    sptr<MockAbilityConnectCallbackStub> stub2(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback2(new AbilityConnectionProxy(stub2));

    abilityMgrServ_->RemoveAllServiceRecord();

    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(2);
    abilityMgrServ_->ConnectAbility(want1, callback1, nullptr);
    abilityMgrServ_->ConnectAbility(want2, callback1, nullptr);
    abilityMgrServ_->ConnectAbility(want1, callback2, nullptr);
    abilityMgrServ_->ConnectAbility(want2, callback2, nullptr);

    EXPECT_EQ((std::size_t)2, abilityMgrServ_->connectManager_->GetServiceMap().size());
    EXPECT_EQ((std::size_t)2, abilityMgrServ_->connectManager_->GetConnectMap().size());

    std::shared_ptr<AbilityRecord> record1;
    std::shared_ptr<AbilityRecord> record2;
    CreateServiceRecord(record1, want1, callback1, callback2);
    CreateServiceRecord(record2, want2, callback1, callback2);

    for (auto &connectRecord : record1->GetConnectRecordList()) {
        connectRecord->SetConnectState(ConnectionState::CONNECTED);
    }
    for (auto &connectRecord : record2->GetConnectRecordList()) {
        connectRecord->SetConnectState(ConnectionState::CONNECTED);
    }

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(4);
    record1->SetScheduler(scheduler);
    record2->SetScheduler(scheduler);
    EXPECT_CALL(*stub1, OnAbilityDisconnectDone(_, _)).Times(2);
    abilityMgrServ_->DisconnectAbility(callback1);
    usleep(100);

    EXPECT_CALL(*scheduler, ScheduleDisconnectAbility(_)).Times(2);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(2);
    EXPECT_CALL(*stub2, OnAbilityDisconnectDone(_, _)).Times(2);

    CheckTestRecord(record1, record2, callback1, callback2);
    testing::Mock::AllowLeak(stub1);
    testing::Mock::AllowLeak(callback1);
    testing::Mock::AllowLeak(stub2);
    testing::Mock::AllowLeak(callback2);
    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_011, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);
    bool testResult = false;
    sptr<IRemoteObject> testToken;
    auto mockHandler = [&](const sptr<IRemoteObject> &token,
                           const sptr<IRemoteObject> &preToken,
                           const AbilityInfo &abilityInfo,
                           const ApplicationInfo &appInfo) {
        testToken = token;
        testResult = !!testToken && abilityInfo.bundleName == bundleName && abilityInfo.name == abilityName &&
                     appInfo.bundleName == bundleName;
        return AppMgrResultCode::RESULT_OK;
    };
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));

    int testRequestCode = 123;
    SetActive();
    abilityMgrServ_->StartAbility(want, testRequestCode);
    EXPECT_TRUE(testResult);
    EXPECT_TRUE(testToken);

    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    int testId = 0;
    auto handler = [&](const Want &want, bool restart, int startid) { testId = startid; };
    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _)).Times(1).WillOnce(Invoke(handler));
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    EXPECT_EQ(1, testId);

    abilityMgrServ_->ScheduleCommandAbilityDone(record->GetToken());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::ACTIVE, record->GetAbilityState());

    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _))
        .Times(3)
        .WillOnce(Invoke(handler))
        .WillOnce(Invoke(handler))
        .WillOnce(Invoke(handler));

    for (int i = 0; i < 3; ++i) {
        abilityMgrServ_->StartAbility(want, testRequestCode);
    }
    EXPECT_EQ(4, testId);
    abilityMgrServ_->RemoveAllServiceRecord();
    testing::Mock::AllowLeak(scheduler);
}
/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_012, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);
    bool testResult = false;
    sptr<IRemoteObject> testToken;
    auto mockHandler = [&](const sptr<IRemoteObject> &token,
                           const sptr<IRemoteObject> &preToken,
                           const AbilityInfo &abilityInfo,
                           const ApplicationInfo &appInfo) {
        testToken = token;
        testResult = !!testToken && abilityInfo.bundleName == bundleName && abilityInfo.name == abilityName &&
                     appInfo.bundleName == bundleName;
        return AppMgrResultCode::RESULT_OK;
    };
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    WaitAMS();
    EXPECT_TRUE(testResult);
    EXPECT_TRUE(testToken);

    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    int testId = 0;
    auto handler = [&](const Want &want, bool restart, int startid) { testId = startid; };
    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _)).Times(1).WillOnce(Invoke(handler));
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE);
    EXPECT_EQ(1, testId);

    abilityMgrServ_->ScheduleCommandAbilityDone(record->GetToken());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::ACTIVE, record->GetAbilityState());

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ((size_t)1, abilityMgrServ_->GetConnectRecordListByCallback(callback).size());

    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);
    EXPECT_EQ(ConnectionState::CONNECTED, connectRecord->GetConnectState());

    abilityMgrServ_->RemoveAllServiceRecord();

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_013, TestSize.Level1)
{
    std::string abilityName1 = "MusicAbility_service_1";
    std::string bundleName1 = "com.ix.hiMusic_service_1";
    Want want1 = CreateWant(abilityName1, bundleName1);
    std::string abilityName2 = "MusicAbility_service_2";
    std::string bundleName2 = "com.ix.hiMusic_service_2";
    Want want2 = CreateWant(abilityName2, bundleName2);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(2);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want1, testRequestCode);
    abilityMgrServ_->StartAbility(want2, testRequestCode);

    std::shared_ptr<AbilityRecord> record1 =
        abilityMgrServ_->GetServiceRecordByElementName(want1.GetElement().GetURI());
    EXPECT_TRUE(record1);
    EXPECT_FALSE(record1->IsCreateByConnect());

    std::shared_ptr<AbilityRecord> record2 =
        abilityMgrServ_->GetServiceRecordByElementName(want2.GetElement().GetURI());
    EXPECT_TRUE(record2);
    EXPECT_FALSE(record2->IsCreateByConnect());

    abilityMgrServ_->RemoveAllServiceRecord();
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: TerminateAbility  (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_014, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->TerminateAbility(record->GetToken(), -1);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);

    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());

    abilityMgrServ_->RemoveAllServiceRecord();

    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: TerminateAbility  (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_015, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->StopServiceAbility(want);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);

    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());

    abilityMgrServ_->RemoveAllServiceRecord();
    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: StopServiceAbility  (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_016, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ((size_t)1, abilityMgrServ_->GetConnectRecordListByCallback(callback).size());
    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);
    EXPECT_EQ(ConnectionState::CONNECTED, connectRecord->GetConnectState());

    int result = abilityMgrServ_->TerminateAbility(record->GetToken(), -1);
    EXPECT_EQ(TERMINATE_SERVICE_IS_CONNECTED, result);

    abilityMgrServ_->RemoveAllServiceRecord();

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: StopServiceAbility  (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_017, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ((size_t)1, abilityMgrServ_->GetConnectRecordListByCallback(callback).size());
    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);
    EXPECT_EQ(ConnectionState::CONNECTED, connectRecord->GetConnectState());

    int result = abilityMgrServ_->StopServiceAbility(want);
    EXPECT_EQ(TERMINATE_SERVICE_IS_CONNECTED, result);

    abilityMgrServ_->RemoveAllServiceRecord();

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: disconnectAbility and stop service ability (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_018, TestSize.Level1)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    SetActive();
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->AddStartId();
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);

    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ((size_t)1, abilityMgrServ_->GetConnectRecordListByCallback(callback).size());
    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);
    EXPECT_EQ(ConnectionState::CONNECTED, connectRecord->GetConnectState());

    EXPECT_CALL(*scheduler, ScheduleDisconnectAbility(_)).Times(1);
    abilityMgrServ_->DisconnectAbility(callback);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTING, connectRecord->GetConnectState());

    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);
    abilityMgrServ_->ScheduleDisconnectAbilityDone(record->GetToken());
    EXPECT_EQ((std::size_t)0, record->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetConnectMap().size());
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTED, connectRecord->GetConnectState());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->GetServiceMap().size());

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->StopServiceAbility(want);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);
    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());

    abilityMgrServ_->RemoveAllServiceRecord();
    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: disconnectAbility and terminate (serive ability).
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_019, TestSize.Level1)
{
    std::string abilityName = "MusicAbilityService";
    std::string bundleName = "com.ix.hiservice";
    Want want = CreateWant(abilityName, bundleName);
    EXPECT_TRUE(abilityMgrServ_);
    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());
    record->AddStartId();
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    record->SetScheduler(scheduler);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ((size_t)1, abilityMgrServ_->GetConnectRecordListByCallback(callback).size());
    EXPECT_CALL(*stub, OnAbilityConnectDone(_, _, _)).Times(1);
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), nullptr);
    EXPECT_EQ(ConnectionState::CONNECTED, connectRecord->GetConnectState());

    EXPECT_CALL(*scheduler, ScheduleDisconnectAbility(_)).Times(1);
    abilityMgrServ_->DisconnectAbility(callback);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTING, connectRecord->GetConnectState());

    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);
    abilityMgrServ_->ScheduleDisconnectAbilityDone(record->GetToken());
    EXPECT_EQ((std::size_t)0, record->GetConnectRecordList().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetConnectMap().size());
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTED, connectRecord->GetConnectState());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->GetServiceMap().size());

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->TerminateAbility(record->GetToken(), -1);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL);
    EXPECT_CALL(*mockAppMgrClient_, TerminateAbility(_)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: OnAbilityDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test service abilities death notification.
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_020, TestSize.Level3)
{
    std::string abilityName = "MusicAbility_service";
    std::string bundleName = "com.ix.hiMusic_service";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());
    std::shared_ptr<AbilityRecord> record =
        abilityMgrServ_->connectManager_->GetServiceRecordByElementName(want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_TRUE(record->IsCreateByConnect());
    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    connectRecord->SetConnectState(ConnectionState::CONNECTED);
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);
    abilityMgrServ_->OnAbilityDied(record);
    usleep(100 * 1000);

    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->connectMap_.size());

    abilityMgrServ_->RemoveAllServiceRecord();
    testing::Mock::AllowLeak(stub);
    testing::Mock::AllowLeak(callback);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_021, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);
    ClearStack();

    std::string abilityName = "MusicSAbility";
    std::string bundleName = "com.ix.hiMusic";
    std::string abilityName2 = "RadioAbility";
    std::string bundleName2 = "com.ix.hiRadio";
    Want want = CreateWant(abilityName, bundleName);
    Want want2 = CreateWant(abilityName2, bundleName2);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(2);
    EXPECT_CALL(*mockAppMgrClient_, AbilityAttachTimeOut(_)).Times(1);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(2);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    abilityMgrServ_->StartAbility(want);
    auto testAbilityRecord = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord);
    abilityMgrServ_->AttachAbilityThread(scheduler, testAbilityRecord->GetToken());
    SetActive();
    abilityMgrServ_->StartAbility(want2);
    auto testAbilityRecord2 = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord2);
    auto stackMgr = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackMgr);
    stackMgr->CompleteInactive(testAbilityRecord);
    testAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::BACKGROUND);
    abilityMgrServ_->HandleLoadTimeOut(testAbilityRecord2->GetEventId());
    testAbilityRecord = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord);
    EXPECT_TRUE(testAbilityRecord->GetAbilityInfo().bundleName == bundleName);
    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleLoadTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify function HandleLoadTimeOut
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_022, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);
    ClearStack();

    std::string abilityName = "TVAbility";
    std::string bundleName = COM_IX_TV;
    std::string abilityName2 = "FilmAbility";
    std::string bundleName2 = COM_IX_Film;
    Want want = CreateWant(abilityName, bundleName);
    Want want2 = CreateWant(abilityName2, bundleName2);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(2);
    EXPECT_CALL(*mockAppMgrClient_, AbilityAttachTimeOut(_)).Times(1);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(2);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    abilityMgrServ_->StartAbility(want);
    auto testAbilityRecord = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord);
    abilityMgrServ_->AttachAbilityThread(scheduler, testAbilityRecord->GetToken());
    SetActive();
    abilityMgrServ_->StartAbility(want2);
    auto testAbilityRecord2 = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord2);
    auto stackMgr = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackMgr);
    stackMgr->CompleteInactive(testAbilityRecord);
    abilityMgrServ_->HandleLoadTimeOut(testAbilityRecord2->GetEventId());
    testAbilityRecord = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord);
    EXPECT_TRUE(testAbilityRecord->GetAbilityInfo().bundleName == bundleName);
    testing::Mock::AllowLeak(scheduler);
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
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_023, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);
    ClearStack();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    Want want = CreateWant("RadioTopAbility", COM_IX_HIRADIO);
    abilityMgrServ_->StartAbility(want);
    auto testAbilityRecord = GetTopAbility();
    EXPECT_TRUE(testAbilityRecord);
    SetActive();
    auto resultTvFunction = abilityMgrServ_->IsFirstInMission(testAbilityRecord->GetToken());
    EXPECT_EQ(resultTvFunction, true);
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
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_024, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);
    ClearStack();
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _)).Times(1);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    Want want = CreateWant("PhoneAbility1", COM_IX_PHONE);
    abilityMgrServ_->StartAbility(want);
    auto ability = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(ability);
    SetActive();
    Want want2 = CreateWant("PhoneAbility2", COM_IX_PHONE);
    abilityMgrServ_->StartAbility(want2);
    EXPECT_TRUE(scheduler);
    auto abilityTv = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(abilityTv);
    abilityTv->SetScheduler(scheduler);
    SetActive();
    auto resultFunction = abilityMgrServ_->MoveMissionToEnd(abilityTv->GetToken(), true);
    EXPECT_EQ(resultFunction, ERR_OK);
    testing::Mock::AllowLeak(scheduler);
}

/*
 * Feature: AbilityManagerService
 * Function: CompelVerifyPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Verify function CompelVerifyPermission
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_025, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    EXPECT_TRUE(mockAppMgrClient_);
    const std::string permission = "permission";
    int pid = 100;
    int uid = 1000;
    std::string message;
    int runTimes = 1;
    EXPECT_CALL(*mockAppMgrClient_, CompelVerifyPermission(_, _, _, _)).Times(runTimes);
    auto resultFunction = abilityMgrServ_->CompelVerifyPermission(permission, pid, uid, message);
    EXPECT_EQ(resultFunction, 0);
}
/*
 * Feature: AbilityManagerService
 * Function: AmsConfigurationParameter
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Judge the acquired properties
 */
HWTEST_F(AbilityMgrModuleTest, AmsConfigurationParameter_026, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_->amsConfigResolver_);
    EXPECT_FALSE(abilityMgrServ_->amsConfigResolver_->NonConfigFile());

    // Open a path that does not exist
    auto ref = abilityMgrServ_->amsConfigResolver_->LoadAmsConfiguration("/system/etc/ams.txt");
    // faild return 1
    EXPECT_EQ(ref, 1);
}

/*
 * Feature: AbilityManagerService
 * Function: AmsConfigurationParameter
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompelVerifyPermission
 * EnvConditions: NA
 * CaseDescription: Judge the acquired properties
 */
HWTEST_F(AbilityMgrModuleTest, AmsConfigurationParameter_027, TestSize.Level1)
{
    bool startLauncher = false;
    bool startstatusbar = false;
    bool startnavigationbar = false;
    nlohmann::json info;
    std::ifstream file(AmsConfig::AMS_CONFIG_FILE_PATH, std::ios::in);
    if (file.is_open()) {
        file >> info;
        info.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_LAUNCHER).get_to(startLauncher);
        info.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_STATUS_BAR).get_to(startstatusbar);
        info.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::STARTUP_NAVIGATION_BAR).get_to(startnavigationbar);
    }

    EXPECT_EQ(startLauncher, abilityMgrServ_->amsConfigResolver_->GetStartLauncherState());
    EXPECT_EQ(startstatusbar, abilityMgrServ_->amsConfigResolver_->GetStatusBarState());
    EXPECT_EQ(startnavigationbar, abilityMgrServ_->amsConfigResolver_->GetNavigationBarState());
}

}  // namespace AAFwk
}  // namespace OHOS
