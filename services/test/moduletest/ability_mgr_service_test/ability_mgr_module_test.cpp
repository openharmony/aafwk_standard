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
#include "configuration_distributor.h"
#include "pending_want_manager.h"
#include "pending_want_record.h"
#undef private
#undef protected
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"

using namespace std::placeholders;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::AtLeast;

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
    void CreateAbilityRequest(const std::string &abilityName, const std::string bundleName, Want &want,
        std::shared_ptr<MissionStack> &curMissionStack, sptr<IRemoteObject> &recordToken);
    void CreateServiceRecord(std::shared_ptr<AbilityRecord> &record, Want &want, int uid,
        const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2);
    void CheckTestRecord(std::shared_ptr<AbilityRecord> &record1, std::shared_ptr<AbilityRecord> &record2,
        const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2);
    void SetActive();
    std::shared_ptr<AbilityRecord> GetTopAbility();
    void ClearStack();
    void OnStartAms();
    WantSenderInfo MakeWantSenderInfo(std::vector<Want> &wants, int32_t flags, int32_t userId, int32_t type = 1);
    WantSenderInfo MakeWantSenderInfo(Want &want, int32_t flags, int32_t userId, int32_t type = 1);

    std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
    sptr<MockAbilityScheduler> scheduler_ {nullptr};
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

void AbilityMgrModuleTest::OnStartAms()
{
    if (abilityMgrServ_) {
        if (abilityMgrServ_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }
        abilityMgrServ_->state_ = ServiceRunningState::STATE_RUNNING;
        
        abilityMgrServ_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMgrServ_->eventLoop_);

        abilityMgrServ_->handler_ = std::make_shared<AbilityEventHandler>(abilityMgrServ_->eventLoop_,
                                                                abilityMgrServ_);
        EXPECT_TRUE(abilityMgrServ_->handler_);
        EXPECT_TRUE(abilityMgrServ_->connectManager_);

        abilityMgrServ_->connectManager_->SetEventHandler(abilityMgrServ_->handler_);

        abilityMgrServ_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(abilityMgrServ_->dataAbilityManager_);

        abilityMgrServ_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMgrServ_->amsConfigResolver_);
        abilityMgrServ_->amsConfigResolver_->Parse();

        abilityMgrServ_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMgrServ_->pendingWantManager_);

        abilityMgrServ_->parameterContainer_ = std::make_shared<AbilityParameterContainer>();
        EXPECT_TRUE(abilityMgrServ_->parameterContainer_);
        
        abilityMgrServ_->waitmultiAppReturnStorage_ = std::make_shared<WaitMultiAppReturnStorage>();
        EXPECT_TRUE(abilityMgrServ_->waitmultiAppReturnStorage_);
        abilityMgrServ_->configuration_ = std::make_shared<AppExecFwk::Configuration>();
        EXPECT_TRUE(abilityMgrServ_->configuration_);
        abilityMgrServ_->GetGlobalConfiguration();

        int userId = abilityMgrServ_->GetUserId();
        abilityMgrServ_->SetStackManager(userId);
        abilityMgrServ_->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
        EXPECT_TRUE(abilityMgrServ_->systemAppManager_);

        abilityMgrServ_->eventLoop_->Run();

        GTEST_LOG_(INFO) << "OnStart success";
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityMgrModuleTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());
}

void AbilityMgrModuleTest::TearDownTestCase(void)
{
}
void AbilityMgrModuleTest::SetUp(void)
{
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartAms();

    scheduler_ = new MockAbilityScheduler();
}

void AbilityMgrModuleTest::TearDown(void)
{
    abilityMgrServ_->OnStop();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
    abilityMgrServ_.reset();
}

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
    element.SetDeviceID("");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return want;
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

    std::string serviceName = "hiService";
    std::string serviceBundleName = "com.ix.musicService";
    want = CreateWant(serviceName, serviceBundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo =
        CreateAbilityInfo(serviceName, serviceBundleName, serviceBundleName);
    abilityRequest.abilityInfo.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    abilityRequest.appInfo = CreateAppInfo(serviceName, serviceBundleName);
    abilityMgrServ_->RemoveAllServiceRecord();
}

void AbilityMgrModuleTest::CreateServiceRecord(std::shared_ptr<AbilityRecord> &record, Want &want, int uid,
    const sptr<AbilityConnectionProxy> &callback1, const sptr<AbilityConnectionProxy> &callback2)
{
    record = abilityMgrServ_->connectManager_->GetServiceRecordByElementName(
        std::to_string(uid) + "/" + want.GetElement().GetURI());
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

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record1->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);
    abilityMgrServ_->AbilityTransitionDone(record2->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record1->GetAbilityState());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record2->GetAbilityState());

    abilityMgrServ_->OnAbilityRequestDone(
        record1->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    abilityMgrServ_->OnAbilityRequestDone(
        record2->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetServiceMap().size());
    EXPECT_EQ((std::size_t)0, abilityMgrServ_->connectManager_->GetConnectMap().size());
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

WantSenderInfo AbilityMgrModuleTest::MakeWantSenderInfo(
    std::vector<Want> &wants, int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
    // wantSenderInfo.type is OperationType::START_ABILITY
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    int requestCode = 10;
    wantSenderInfo.requestCode = requestCode;
    std::vector<WantsInfo> allWant;
    for (auto want : wants) {
        WantsInfo wantsInfo;
        wantsInfo.want = want;
        wantsInfo.resolvedTypes = "";
        wantSenderInfo.allWants.push_back(wantsInfo);
    }
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    return wantSenderInfo;
}

WantSenderInfo AbilityMgrModuleTest::MakeWantSenderInfo(Want &want, int32_t flags, int32_t userId, int32_t type)
{
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = type;
    // wantSenderInfo.type is OperationType::START_ABILITY
    wantSenderInfo.bundleName = "com.ix.hiRadio";
    wantSenderInfo.resultWho = "RadioTopAbility";
    int requestCode = 10;
    wantSenderInfo.requestCode = requestCode;
    std::vector<WantsInfo> allWant;
    WantsInfo wantInfo;
    wantInfo.want = want;
    wantInfo.resolvedTypes = "nihao";
    allWant.emplace_back(wantInfo);
    wantSenderInfo.allWants = allWant;
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    return wantSenderInfo;
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
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);

    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";
    Want want = CreateWant(abilityName, bundleName);
    sptr<IRemoteObject> testToken;

    int testRequestCode = 123;
    auto ref = abilityMgrServ_->StartAbility(want, testRequestCode);
    EXPECT_EQ(ref, 0);
    auto stackMgr = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackMgr);
    auto topAbility = stackMgr->GetCurrentTopAbility();
    EXPECT_TRUE(stackMgr);
    testToken = topAbility->GetToken();

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

    std::string serviceName = "hiService";
    std::string serviceBundleName = "com.ix.musicService";
    Want want = CreateWant(serviceName, serviceBundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(serviceName, bundleName, serviceBundleName);
    abilityRequest.abilityInfo.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    abilityRequest.appInfo = CreateAppInfo(serviceBundleName, serviceBundleName);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    abilityMgrServ_->RemoveAllServiceRecord();
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

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

    PacMap saveData;
    int result = abilityMgrServ_->AbilityTransitionDone(
        abilityRecord->GetToken(), OHOS::AAFwk::AbilityState::ACTIVE, saveData);
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

    int result = abilityMgrServ_->ConnectAbility(want, callback, recordToken);
    EXPECT_EQ(testResult, result);

    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);

    std::string key = std::to_string(10005) + "/" + want.GetElement().GetURI();
    auto record = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(record);

    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectRecordList().front();
    EXPECT_TRUE(connectRecord);

    EXPECT_CALL(*scheduler, AsObject()).Times(1);
    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    EXPECT_CALL(*scheduler, ScheduleConnectAbility(_)).Times(1);
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE, saveData);
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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

    abilityMgrServ_->RemoveAllServiceRecord();

    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ(ERR_OK, result);
    auto record = abilityMgrServ_->connectManager_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::TERMINATING, record->GetAbilityState());

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
    std::string abilityName1 = "hiService";
    std::string bundleName1 = "com.ix.musicService";
    Want want1 = CreateWant(abilityName1, bundleName1);
    std::string abilityName2 = "hiService";
    std::string bundleName2 = "com.ix.hiService";
    Want want2 = CreateWant(abilityName2, bundleName2);

    sptr<MockAbilityConnectCallbackStub> stub1(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback1(new AbilityConnectionProxy(stub1));
    sptr<MockAbilityConnectCallbackStub> stub2(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback2(new AbilityConnectionProxy(stub2));

    abilityMgrServ_->RemoveAllServiceRecord();

    abilityMgrServ_->ConnectAbility(want1, callback1, nullptr);
    abilityMgrServ_->ConnectAbility(want2, callback1, nullptr);
    abilityMgrServ_->ConnectAbility(want1, callback2, nullptr);
    abilityMgrServ_->ConnectAbility(want2, callback2, nullptr);

    EXPECT_EQ((std::size_t)2, abilityMgrServ_->connectManager_->GetServiceMap().size());
    EXPECT_EQ((std::size_t)2, abilityMgrServ_->connectManager_->GetConnectMap().size());

    std::shared_ptr<AbilityRecord> record1;
    std::shared_ptr<AbilityRecord> record2;
    CreateServiceRecord(record1, want1, 10005, callback1, callback2);
    CreateServiceRecord(record2, want2, 10004, callback1, callback2);

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
    usleep(1000);

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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);
    int testRequestCode = 123;
    SetActive();
    abilityMgrServ_->StartAbility(want, testRequestCode);

    auto record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);

    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    int testId = 0;
    auto handler = [&](const Want &want, bool restart, int startid) { testId = startid; };
    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _)).Times(1).WillOnce(Invoke(handler));
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE, saveData);
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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);
   
    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    WaitAMS();

    auto record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
    EXPECT_TRUE(record);
    EXPECT_FALSE(record->IsCreateByConnect());

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);

    abilityMgrServ_->AttachAbilityThread(scheduler, record->GetToken());
    EXPECT_TRUE(record->isReady_);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        record->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::INACTIVATING, record->GetAbilityState());

    int testId = 0;
    auto handler = [&](const Want &want, bool restart, int startid) { testId = startid; };
    EXPECT_CALL(*scheduler, ScheduleCommandAbility(_, _, _)).Times(1).WillOnce(Invoke(handler));
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INACTIVE, saveData);
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
    std::string abilityName1 = "hiService";
    std::string bundleName1 = "com.ix.musicService";
    Want want1 = CreateWant(abilityName1, bundleName1);
    std::string abilityName2 = "hiService";
    std::string bundleName2 = "com.ix.hiService";
    Want want2 = CreateWant(abilityName2, bundleName2);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want1, testRequestCode);
    abilityMgrServ_->StartAbility(want2, testRequestCode);

    std::shared_ptr<AbilityRecord> record1 = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want1.GetElement().GetURI());
    EXPECT_TRUE(record1);
    EXPECT_FALSE(record1->IsCreateByConnect());

    std::shared_ptr<AbilityRecord> record2 =
        abilityMgrServ_->GetServiceRecordByElementName(std::to_string(10004) + "/" + want2.GetElement().GetURI());
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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);

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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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
    
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);

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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    SetActive();
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);

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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);
    EXPECT_TRUE(abilityMgrServ_);
    abilityMgrServ_->RemoveAllServiceRecord();

    int testRequestCode = 123;
    abilityMgrServ_->StartAbility(want, testRequestCode);
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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
    abilityMgrServ_->ScheduleConnectAbilityDone(record->GetToken(), stub);
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

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(record->GetToken(), AbilityLifeCycleState::ABILITY_STATE_INITIAL, saveData);

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
    std::string abilityName = "hiService";
    std::string bundleName = "com.ix.musicService";
    Want want = CreateWant(abilityName, bundleName);

    abilityMgrServ_->RemoveAllServiceRecord();

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

    abilityMgrServ_->ConnectAbility(want, callback, nullptr);
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());
    std::shared_ptr<AbilityRecord> record = abilityMgrServ_->connectManager_->GetServiceRecordByElementName(
        std::to_string(10005) + "/" + want.GetElement().GetURI());
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
    ClearStack();

    std::string abilityName = "MusicSAbility";
    std::string bundleName = "com.ix.hiMusic";
    std::string abilityName2 = "RadioAbility";
    std::string bundleName2 = "com.ix.hiRadio";
    Want want = CreateWant(abilityName, bundleName);
    Want want2 = CreateWant(abilityName2, bundleName2);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
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
    ClearStack();

    std::string abilityName = "TVAbility";
    std::string bundleName = COM_IX_TV;
    std::string abilityName2 = "FilmAbility";
    std::string bundleName2 = COM_IX_Film;
    Want want = CreateWant(abilityName, bundleName);
    Want want2 = CreateWant(abilityName2, bundleName2);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);

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
    ClearStack();

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
    ClearStack();
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();

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
    const std::string permission = "permission";
    int pid = 100;
    int uid = 1000;
    std::string message;
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
/*
 * Feature: AbilityManagerService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: normal ability, page
 */
HWTEST_F(AbilityMgrModuleTest, UpdateConfiguration_028, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    SetActive();
    Want want = CreateWant(abilityName, bundleName);
    auto startRef = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(startRef, 0);
    auto abilityRecord = abilityMgrServ_->GetStackManager()->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecord);

    int displeyId = 1001;
    std::string key = GlobalConfigurationKey::SYSTEM_LANGUAGE;
    std::string val = "Chinese";
    auto Compare = [displeyId, key, val](const AppExecFwk::Configuration &config) {
        auto item = config.GetItem(displeyId, key);
        EXPECT_EQ(item, val);
    };

    sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, ScheduleUpdateConfiguration(_))
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Invoke(Compare));

    auto ref = abilityMgrServ_->AttachAbilityThread(scheduler, abilityRecord->GetToken());
    EXPECT_EQ(ref, 0);

    int size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_TRUE(size > 0);

    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(displeyId, key, val);

    auto updateRef = abilityMgrServ_->UpdateConfiguration(newConfig);
    EXPECT_EQ(updateRef, 0);

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(abilityRecord->GetToken(), 0, saveData);

    int num = size - 1;
    size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, num);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: service ability
 */
HWTEST_F(AbilityMgrModuleTest, UpdateConfiguration_029, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";

    SetActive();
    Want want = CreateWant(abilityName, bundleName);
    auto startRef = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(startRef, 0);

    std::shared_ptr<AbilityRecord> abilityRecord = abilityMgrServ_->GetStackManager()->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecord);

    int displeyId = 1001;
    std::string key = GlobalConfigurationKey::SYSTEM_LANGUAGE;
    std::string val = "German";
    auto Compare = [displeyId, key, val](const AppExecFwk::Configuration &config) {
        auto item = config.GetItem(displeyId, key);
        EXPECT_EQ(item, val);
    };

    sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, ScheduleUpdateConfiguration(_))
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Invoke(Compare));

    abilityMgrServ_->AttachAbilityThread(scheduler, abilityRecord->GetToken());

    int size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_TRUE(size > 0);

    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(displeyId, key, val);

    auto updateRef = abilityMgrServ_->UpdateConfiguration(newConfig);
    EXPECT_EQ(updateRef, 0);

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(abilityRecord->GetToken(), 0, saveData);

    int num = size - 1;
    size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, num);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: luncher ability
 */
HWTEST_F(AbilityMgrModuleTest, UpdateConfiguration_030, TestSize.Level1)
{
    EXPECT_TRUE(abilityMgrServ_);
    std::string abilityName = "luncher";
    std::string bundleName = "com.ix.hiworld";

    SetActive();
    Want want = CreateWant(abilityName, bundleName);
    auto startRef = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(startRef, 0);

    std::shared_ptr<AbilityRecord> abilityRecord = abilityMgrServ_->GetStackManager()->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecord);

    int displeyId = 1001;
    std::string key = GlobalConfigurationKey::SYSTEM_LANGUAGE;
    std::string val = "Italian";
    auto Compare = [displeyId, key, val](const AppExecFwk::Configuration &config) {
        auto item = config.GetItem(displeyId, key);
        EXPECT_EQ(item, val);
    };

    sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, ScheduleUpdateConfiguration(_))
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Invoke(Compare));

    abilityMgrServ_->AttachAbilityThread(scheduler, abilityRecord->GetToken());

    int size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_TRUE(size > 0);

    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(displeyId, key, val);

    auto updateRef = abilityMgrServ_->UpdateConfiguration(newConfig);
    EXPECT_EQ(updateRef, 0);

    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(abilityRecord->GetToken(), 0, saveData);

    int num = size - 1;
    size = DelayedSingleton<ConfigurationDistributor>::GetInstance()->observerList_.size();
    EXPECT_EQ(size, num);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability type is AbilityCallType::START_ABILITY_TYPE
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_028, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();

    // Start the application more open
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int uid = 10012;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = uid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = uid+1;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    Want wantRadio;
    ElementName elementRadio("deviceId", "com.ix.clock", "ClockAbility");
    wantRadio.SetElement(elementRadio);


    request.want = wantRadio;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, topAbility);

    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);

    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    auto selectorToken = selectorAbility->GetToken();

    auto abilityName = selectorAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(abilityName, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);

    Want requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, uid);

    int ref = abilityMgrServ_->TerminateAbility(selectorToken, -1, &requstWant);
    EXPECT_EQ(ref, 0);
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    WaitAMS();

    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    auto name = radioAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(name, "ClockAbility");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: start ability type is AbilityCallType::START_ABILITY_SETTING_TYPE
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_029, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();

    // Start the application more open
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int uid = 10012;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = uid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = uid+1;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);


    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    AbilityRequest request;
    Want wantRadio;
    ElementName elementRadio("deviceId", "com.ix.clock", "ClockAbility");
    wantRadio.SetElement(elementRadio);

    request.want = wantRadio;
    request.callerToken = callerToken;
    request.callType = AbilityCallType::START_ABILITY_SETTING_TYPE;
    request.startSetting = abilityStartSetting;

    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, topAbility);

    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    auto selectorToken = selectorAbility->GetToken();

    auto abilityName = selectorAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(abilityName, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);

    
    Want requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, uid + 1);

    int ref = abilityMgrServ_->TerminateAbility(selectorToken, -1, &requstWant);
    EXPECT_EQ(ref, 0);
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    WaitAMS();

    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    auto name = radioAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(name, "ClockAbility");
    EXPECT_EQ(radioAbility->GetMissionStackId(), 2);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PageAbility Start multi ServiceAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_033, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;
    abilityMgrServ_->Init();

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::START_ABILITY_TYPE;
    abilityRequest1.callerUid = -1;
    abilityRequest1.want = serviceWant;
    abilityRequest1.requestCode = -1;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, rootUid);
    auto abilityRequest = abilityMgrServ_->StartSelectedApplication(&requstWant, selectorToken);
    EXPECT_FALSE(abilityRequest);
    WaitAMS();
    std::string key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, false);
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PageAbility Start multi ServiceAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_034, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;
    abilityMgrServ_->Init();

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::START_ABILITY_TYPE;
    abilityRequest1.callerUid = -1;
    abilityRequest1.want = serviceWant;
    abilityRequest1.requestCode = -1;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);
    auto abilityRequest = abilityMgrServ_->StartSelectedApplication(&requstWant, selectorToken);
    EXPECT_FALSE(abilityRequest);
    WaitAMS();
    std::string key = std::to_string(twinUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility/DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PageAbility Connect/Disconnect multi ServiceAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_035, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;
    abilityMgrServ_->Init();

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::CONNECT_ABILITY_TYPE;
    abilityRequest1.connect = callback;
    abilityRequest1.want = serviceWant;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);
    auto abilityRequest = abilityMgrServ_->StartSelectedApplication(&requstWant, selectorToken);
    EXPECT_FALSE(abilityRequest);
    WaitAMS();
    std::string key = std::to_string(twinUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord1);
    std::shared_ptr<ConnectionRecord> connectRecord = serviceRecord->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::CONNECTING, connectRecord->GetConnectState());
    connectRecord->SetConnectState(ConnectionState::CONNECTED);
    abilityMgrServ_->DisconnectAbility(callback);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTING, connectRecord->GetConnectState());
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility/DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PageAbility Connect/Disconnect multi ServiceAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_036, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;
    abilityMgrServ_->Init();

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    // com.ix.hiService is multi app
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::CONNECT_ABILITY_TYPE;
    abilityRequest1.connect = callback;
    abilityRequest1.want = serviceWant;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, rootUid);
    auto abilityRequest = abilityMgrServ_->StartSelectedApplication(&requstWant, selectorToken);
    EXPECT_FALSE(abilityRequest);
    WaitAMS();
    std::string key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, false);
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    auto serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord1);
    std::shared_ptr<ConnectionRecord> connectRecord = serviceRecord->GetConnectingRecord();
    EXPECT_TRUE(connectRecord);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::CONNECTING, connectRecord->GetConnectState());
    connectRecord->SetConnectState(ConnectionState::CONNECTED);
    abilityMgrServ_->DisconnectAbility(callback);
    EXPECT_EQ(OHOS::AAFwk::ConnectionState::DISCONNECTING, connectRecord->GetConnectState());
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: ServiceAbility Start multi ServiceAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_037, TestSize.Level1)
{
    int defValue = -1;
    int callerUid = 10004;
    int rootUid = 10011;
    int twinUid = 10010;
    abilityMgrServ_->Init();

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    Want callerWant;
    ElementName element("deviceId", "com.ix.hiService", "hiService");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    // uid Need add
    std::string callerKey = std::to_string(callerUid) + "/" + element.GetURI();
    auto callerServiceRecord = abilityMgrServ_->GetServiceRecordByElementName(callerKey);
    EXPECT_TRUE(callerServiceRecord);
    auto callerToken = callerServiceRecord->GetToken();
    EXPECT_TRUE(callerToken);

    Want serviceWant;
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    result = abilityMgrServ_->StartAbility(serviceWant, callerToken, defValue, defValue, defValue);
    EXPECT_EQ(result, ERR_OK);
    std::string key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, false);
    // uid is other
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: ServiceAbility Start multi DataAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_038, TestSize.Level1)
{
    int callerUid = 10004;
    abilityMgrServ_->Init();

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    Want callerWant;
    ElementName element("deviceId", "com.ix.hiService", "hiService");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    std::string callerKey = std::to_string(callerUid) + "/" + element.GetURI();
    auto callerServiceRecord = abilityMgrServ_->GetServiceRecordByElementName(callerKey);
    EXPECT_TRUE(callerServiceRecord);
    auto callerToken = callerServiceRecord->GetToken();
    EXPECT_TRUE(callerToken);

    Want serviceWant;
    std::string startBundleName = "com.ix.hiData";
    std::string startAbilityName = "hiData";
    ElementName elementService("deviceId", startBundleName, startAbilityName);
    serviceWant.SetElement(elementService);
    Uri dataAbilityUri("dataability:///" + startBundleName + "." + startAbilityName);
    abilityMgrServ_->AcquireDataAbility(dataAbilityUri, false, callerToken);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: PageAbility Start multi DataAbility
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_039, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiPhone", "PhoneAbility1");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();

    // Start the application more open
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    // this uid is for data ability
    int uid = 10006;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = uid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = 100061;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    Want wantRadio;
    ElementName elementRadio("deviceId", "com.ix.hiData", "hiData");
    wantRadio.SetElement(elementRadio);
    std::string dataAbilityUri;
    dataAbilityUri = "dataability:///com.ix.hiData.hiData";

    request.want = wantRadio;
    request.callType = AbilityCallType::ACQUIRE_DATA_ABILITY_TYPE;
    request.callerToken = callerToken;

    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, topAbility);

    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);

    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    auto selectorToken = selectorAbility->GetToken();

    auto abilityName = selectorAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(abilityName, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);

    Want requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, uid);

    AbilityRequest needToStartRequest;
    needToStartRequest.appInfo.name = "hiData";

    auto abilityMgr = abilityMgrServ_.get();
    auto task = [abilityMgr, dataAbilityUri, callerToken, &needToStartRequest]() -> void {
        std::cout<<"=========================================task start==================================="<<std::endl;
        EXPECT_TRUE(abilityMgr);
        std::vector<AppExecFwk::AbilityInfo> abilityInfos;
        auto bms = abilityMgr->GetBundleManager();
        EXPECT_TRUE(bms);
        bms->QueryAbilityInfosByUri(dataAbilityUri, abilityInfos);

        // wait
        auto waitMultiAppReturnRecord =
            abilityMgr->waitmultiAppReturnStorage_->AddRecord(callerToken);
        EXPECT_TRUE(waitMultiAppReturnRecord);
        auto requestUid = waitMultiAppReturnRecord->WaitForMultiAppSelectorReturn();

        // Get information about the application that needs to be started (needToStartRequest)
        abilityMgr->GetAbilityInfoFromBms(abilityInfos, callerToken,
            needToStartRequest, requestUid);
        std::cout<<"=========================================task end===================================="<<std::endl;
    };

    std::thread acquirc(task);

    // wait for acquirc run
    usleep(2000);
    int ref = abilityMgrServ_->TerminateAbility(selectorToken, -1, &requstWant);
    WaitAMS();
    EXPECT_EQ(ref, 0);

    acquirc.join();

    abilityMgrServ_->dataAbilityManager_->Acquire(needToStartRequest, false, callerToken, true);

    const std::string dataAbilityName(std::to_string(needToStartRequest.abilityInfo.applicationInfo.uid) + '.' +
        needToStartRequest.abilityInfo.bundleName + '.' + needToStartRequest.abilityInfo.name);
    auto dateAbilityRecord = abilityMgrServ_->dataAbilityManager_->dataAbilityRecordsLoading_.find(dataAbilityName);

    int size = abilityMgrServ_->dataAbilityManager_->dataAbilityRecordsLoading_.size();
    EXPECT_EQ(size, 1);
    EXPECT_TRUE(dateAbilityRecord != abilityMgrServ_->dataAbilityManager_->dataAbilityRecordsLoading_.end());
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: page ability start page ability
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_040, TestSize.Level1)
{
    int defValue = -1;
    Want want;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    radioAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = radioAbility->GetToken();

    // com.ix.clock is multi app
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int rootUid = 10012;
    int twinUid = 10013;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = rootUid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = twinUid;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    ElementName elementService("deviceId", "com.ix.clock", "ClockAbility");
    Want wantService;
    wantService.SetElement(elementService);
    request.want = wantService;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    auto ret = abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, radioAbility);
    EXPECT_EQ(ret, ERR_OK);

    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();

    Want requstWant;
    requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, radioAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, rootUid);

    WaitAMS();
    ret = abilityMgrServ_->TerminateAbility(selectorToken, defValue, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    auto clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    clockAbility1->SetAbilityState(AbilityState::ACTIVE);
    auto clockToken = clockAbility1->GetToken();
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, false);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: page ability start page ability
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_041, TestSize.Level1)
{
    int defValue = -1;
    Want want;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    radioAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = radioAbility->GetToken();

    // com.ix.clock is multi app
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int rootUid = 10012;
    int twinUid = 10013;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = rootUid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = twinUid;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    ElementName elementService("deviceId", "com.ix.clock", "ClockAbility");
    Want wantService;
    wantService.SetElement(elementService);
    request.want = wantService;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    auto ret = abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, radioAbility);
    EXPECT_EQ(ret, ERR_OK);

    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();

    Want requstWant;
    requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, radioAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    ret = abilityMgrServ_->TerminateAbility(selectorToken, defValue, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    auto clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    clockAbility1->SetAbilityState(AbilityState::ACTIVE);
    auto clockToken = clockAbility1->GetToken();
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, true);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Terminate the Ontology application when the Ontology and the clone
 *                  application are started at the same time (page ability -> page ability)
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_042, TestSize.Level1)
{
    int defValue = -1;
    Want want;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    radioAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = radioAbility->GetToken();

    // com.ix.clock is multi app
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int rootUid = 10012;
    int twinUid = 10013;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = rootUid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = twinUid;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    ElementName elementService("deviceId", "com.ix.clock", "ClockAbility");
    Want wantService;
    wantService.SetElement(elementService);
    request.want = wantService;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    auto ret = abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, radioAbility);
    EXPECT_EQ(ret, ERR_OK);

    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();

    Want requstWant;
    requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, radioAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    ret = abilityMgrServ_->TerminateAbility(selectorToken, defValue, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    auto clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    clockAbility1->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, true);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    WaitAMS();
    result = abilityMgrServ_->StartAbility(wantService, *abilityStartSetting, nullptr, defValue);
    EXPECT_EQ(result, ERR_OK);

    WaitAMS();
    auto clockAbility2 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility2);
    clockAbility2->SetAbilityState(AbilityState::ACTIVE);
    auto clockToken = clockAbility2->GetToken();
    EXPECT_EQ(clockAbility2->GetAbilityInfo().applicationInfo.isCloned, false);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().name, "ClockAbility");

    ret = abilityMgrServ_->TerminateAbility(clockToken, defValue, &wantService);
    EXPECT_EQ(ERR_OK, ret);
    WaitAMS();
    clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, true);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: When the ontology and the clone application are started
 *                  at the same time, terminate the clone application (page ability -> page ability)
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_043, TestSize.Level1)
{
    int defValue = -1;
    Want want;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto radioAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);
    radioAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = radioAbility->GetToken();

    // com.ix.clock is multi app
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int rootUid = 10012;
    int twinUid = 10013;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = rootUid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = twinUid;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    ElementName elementService("deviceId", "com.ix.clock", "ClockAbility");
    Want wantService;
    wantService.SetElement(elementService);
    request.want = wantService;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    auto ret = abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, radioAbility);
    EXPECT_EQ(ret, ERR_OK);

    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();

    Want requstWant;
    requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, radioAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    ret = abilityMgrServ_->TerminateAbility(selectorToken, defValue, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    auto clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    clockAbility1->SetAbilityState(AbilityState::ACTIVE);
    auto clockToken = clockAbility1->GetToken();
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, true);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    WaitAMS();
    result = abilityMgrServ_->StartAbility(wantService, *abilityStartSetting, nullptr, defValue);
    EXPECT_EQ(result, ERR_OK);

    WaitAMS();
    auto clockAbility2 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility2);
    clockAbility2->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().applicationInfo.isCloned, false);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().name, "ClockAbility");

    ret = abilityMgrServ_->TerminateAbility(clockToken, defValue, &wantService);
    EXPECT_EQ(ERR_OK, ret);
    WaitAMS();
    clockAbility2 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility2);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().applicationInfo.isCloned, false);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().name, "ClockAbility");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Terminate the Ontology application when the Ontology and the clone
 *                  application are started at the same time  (page ability -> service ability)
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_044, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    // com.ix.hiService is multi app
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::CONNECT_ABILITY_TYPE;
    abilityRequest1.connect = callback;
    abilityRequest1.want = serviceWant;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    auto ret = abilityMgrServ_->TerminateAbility(selectorToken, -1, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    std::string key = std::to_string(twinUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord1);

    WaitAMS();
    result = abilityMgrServ_->StartAbility(serviceWant);
    EXPECT_EQ(result, ERR_OK);

    WaitAMS();
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    auto clockToken = serviceRecord1->GetToken();
    EXPECT_TRUE(serviceRecord1);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.isCloned, false);

    abilityMgrServ_->OnAbilityRequestDone(clockToken, 3);

    WaitAMS();
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord1);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: When the ontology and the clone application are started
 *                  at the same time, terminate the clone application  (page ability -> service ability)
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_045, TestSize.Level1)
{
    int rootUid = 10011;
    int twinUid = 10010;

    Want callerWant;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicSAbility");
    callerWant.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(callerWant);
    EXPECT_EQ(result, ERR_OK);
    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbility->GetEventId());
    Want serviceWant;
    // com.ix.hiService is multi app
    ElementName elementService("deviceId", "com.ix.clock", "ClockService");
    serviceWant.SetElement(elementService);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    AbilityRequest abilityRequest1;
    abilityRequest1.callType = AbilityCallType::CONNECT_ABILITY_TYPE;
    abilityRequest1.connect = callback;
    abilityRequest1.want = serviceWant;
    abilityRequest1.callerToken = callerToken;
    abilityRequest1.requestUid = -1;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    auto bms = abilityMgrServ_->GetBundleManager();
    bms->QueryAbilityInfosForClone(serviceWant, abilityInfos);
    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, abilityRequest1, topAbility);
    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, selectorAbility->GetEventId());
    Want requstWant;
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, topAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    auto ret = abilityMgrServ_->TerminateAbility(selectorToken, -1, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    std::string key = std::to_string(twinUid) + "/" + elementService.GetURI();
    auto serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    auto serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord1);

    WaitAMS();
    result = abilityMgrServ_->StartAbility(serviceWant);
    EXPECT_EQ(result, ERR_OK);

    WaitAMS();
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    serviceRecord->SetAbilityState(AbilityState::ACTIVE);
    auto clockToken = serviceRecord->GetToken();
    EXPECT_TRUE(serviceRecord);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.uid, twinUid);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().applicationInfo.isCloned, true);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    serviceRecord1->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_TRUE(serviceRecord1);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.isCloned, false);

    WaitAMS();
    abilityMgrServ_->OnAbilityRequestDone(clockToken, 3);

    WaitAMS();
    key = std::to_string(twinUid) + "/" + elementService.GetURI();
    serviceRecord = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_FALSE(serviceRecord);
    key = std::to_string(rootUid) + "/" + elementService.GetURI();
    serviceRecord1 = abilityMgrServ_->GetServiceRecordByElementName(key);
    EXPECT_TRUE(serviceRecord1);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.uid, rootUid);
    EXPECT_EQ(serviceRecord1->GetAbilityInfo().applicationInfo.isCloned, false);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSelectedApplication
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: luncher ability start Radio ability
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_046, TestSize.Level1)
{
    Want want;
    ElementName element("deviceId", "com.ix.hiworld", "luncher");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto topAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = topAbility->GetToken();

    // Start the application more open
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int uid = 10003;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = uid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = uid+1;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    Want wantRadio;
    ElementName elementRadio("deviceId", "com.ix.hiRadio", "RadioAbility");
    wantRadio.SetElement(elementRadio);

    request.want = wantRadio;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, topAbility);

    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);

    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    auto selectorToken = selectorAbility->GetToken();

    auto abilityName = selectorAbility->GetWant().GetElement().GetAbilityName();
    EXPECT_EQ(abilityName, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
}

/*
 * Feature: AbilityManagerService
 * Function:StartMultiApplicationSelector
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: the ontology and the clone application are started
 *                  at the same time
 */
HWTEST_F(AbilityMgrModuleTest, StartSelectedApplication_047, TestSize.Level1)
{
    abilityMgrServ_->Init();
    int defValue = -1;
    Want want;
    ElementName element("deviceId", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);

    auto stackManager = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stackManager);
    auto musicAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbility);
    musicAbility->SetAbilityState(AbilityState::ACTIVE);
    auto callerToken = musicAbility->GetToken();

    // com.ix.clock is multi app
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;

    int rootUid = 10012;
    int twinUid = 10013;
    AppExecFwk::AbilityInfo theOne;
    theOne.applicationInfo.uid = rootUid;
    theOne.applicationInfo.isCloned = false;

    AppExecFwk::AbilityInfo theTwo;
    theOne.applicationInfo.uid = twinUid;
    theOne.applicationInfo.isCloned = true;

    abilityInfos.emplace_back(theOne);
    abilityInfos.emplace_back(theTwo);

    AbilityRequest request;
    ElementName elementService("deviceId", "com.ix.clock", "ClockAbility");
    Want wantService;
    wantService.SetElement(elementService);
    request.want = wantService;
    request.callType = AbilityCallType::START_ABILITY_TYPE;
    request.callerToken = callerToken;

    auto ret = abilityMgrServ_->StartMultiApplicationSelector(abilityInfos, request, musicAbility);
    EXPECT_EQ(ret, ERR_OK);

    WaitAMS();
    auto selectorAbility = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(selectorAbility);
    selectorAbility->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().bundleName, AbilityConfig::APPLICATION_SELECTOR_BUNDLE_NAME);
    EXPECT_EQ(selectorAbility->GetAbilityInfo().name, AbilityConfig::APPLICATION_SELECTOR_ABILITY_NAME);
    auto selectorToken = selectorAbility->GetToken();

    Want requstWant;
    requstWant = selectorAbility->GetWant();
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_CALLER_ABILITY_RECORD_ID, musicAbility->GetRecordId());
    requstWant.SetParam(AbilityConfig::APPLICATION_SELECTOR_RESULT_UID, twinUid);

    WaitAMS();
    ret = abilityMgrServ_->TerminateAbility(selectorToken, defValue, &requstWant);
    EXPECT_EQ(ERR_OK, ret);
    PacMap saveData;
    ret = abilityMgrServ_->AbilityTransitionDone(selectorToken, 1, saveData);
    EXPECT_EQ(ERR_OK, ret);

    WaitAMS();
    auto clockAbility1 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility1);
    clockAbility1->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().applicationInfo.isCloned, true);
    EXPECT_EQ(clockAbility1->GetAbilityInfo().name, "ClockAbility");

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    WaitAMS();
    result = abilityMgrServ_->StartAbility(wantService, *abilityStartSetting, nullptr, defValue);
    EXPECT_EQ(result, ERR_OK);

    WaitAMS();
    auto clockAbility2 = stackManager->GetCurrentTopAbility();
    EXPECT_TRUE(clockAbility2);
    clockAbility2->SetAbilityState(AbilityState::ACTIVE);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().applicationInfo.isCloned, false);
    EXPECT_EQ(clockAbility2->GetAbilityInfo().name, "ClockAbility");
}

/*
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: UninstallApp
 * EnvConditions: NA
 * CaseDescription: UninstallApp
 */
HWTEST_F(AbilityMgrModuleTest, UninstallApp_001, TestSize.Level1)
{
    abilityMgrServ_->pendingWantManager_->wantRecords_.clear();
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    EXPECT_NE(abilityMgrServ_->pendingWantManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)
            ->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId,
        wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 2);
    abilityMgrServ_->UninstallApp("bundleName3");
    WaitAMS();
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 2);
}

/*
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: UninstallApp
 * EnvConditions: NA
 * CaseDescription: UninstallApp
 */
HWTEST_F(AbilityMgrModuleTest, UninstallApp_002, TestSize.Level1)
{
    abilityMgrServ_->pendingWantManager_->wantRecords_.clear();
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    EXPECT_NE(abilityMgrServ_->pendingWantManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)
            ->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId,
        wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 2);
    abilityMgrServ_->UninstallApp("bundleName2");
    WaitAMS();
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 0);
}

/*
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: UninstallApp
 * EnvConditions: NA
 * CaseDescription: UninstallApp
 */
HWTEST_F(AbilityMgrModuleTest, UninstallApp_003, TestSize.Level1)
{
    abilityMgrServ_->pendingWantManager_->wantRecords_.clear();
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    EXPECT_NE(abilityMgrServ_->pendingWantManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)
            ->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName2", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId,
        wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 2);
    abilityMgrServ_->UninstallApp("bundleName1");
    WaitAMS();
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
}

/*
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: UninstallApp
 * EnvConditions: NA
 * CaseDescription: UninstallApp
 */
HWTEST_F(AbilityMgrModuleTest, UninstallApp_004, TestSize.Level1)
{
    abilityMgrServ_->pendingWantManager_->wantRecords_.clear();
    Want want1;
    ElementName element("device", "bundleName1", "abilityName1");
    want1.SetElement(element);
    Want want2;
    ElementName element2("device", "bundleName2", "abilityName2");
    want2.SetElement(element2);
    std::vector<Want> wants;
    wants.emplace_back(want1);
    wants.emplace_back(want2);
    WantSenderInfo wantSenderInfo = MakeWantSenderInfo(wants, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    EXPECT_NE(abilityMgrServ_->pendingWantManager_, nullptr);
    auto pendingRecord = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo.userId, wantSenderInfo, nullptr)
            ->AsObject());
    EXPECT_NE(pendingRecord, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
    Want want3;
    ElementName element3("device", "bundleName3", "abilityName2");
    want3.SetElement(element3);
    WantSenderInfo wantSenderInfo1 = MakeWantSenderInfo(want3, 0, 0);
    EXPECT_FALSE(((unsigned int)wantSenderInfo1.flags & (unsigned int)Flags::NO_BUILD_FLAG) != 0);
    auto pendingRecord1 = iface_cast<PendingWantRecord>(
        abilityMgrServ_->pendingWantManager_->GetWantSenderLocked(1, 1, wantSenderInfo1.userId,
        wantSenderInfo1, nullptr)->AsObject());
    EXPECT_NE(pendingRecord1, nullptr);
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 2);
    abilityMgrServ_->UninstallApp("bundleName3");
    WaitAMS();
    EXPECT_EQ((int)abilityMgrServ_->pendingWantManager_->wantRecords_.size(), 1);
}

/**
 * @tc.name: ability_mgr_service_test_028
 * @tc.desc: test MinimizeAbility
 * @tc.type: FUNC
 * @tc.require: AR000GJUND
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_028, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "1", appName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);
    abilityRequest.targetVersion = 7;

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);

    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "2", appName, bundleName);
    abilityRequest.targetVersion = 8;
    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND_NEW);
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

    int result = abilityMgrServ_->MinimizeAbility(abilityRecord2->GetToken());
    EXPECT_EQ(OHOS::AAFwk::AbilityState::BACKGROUNDING_NEW, abilityRecord2->GetAbilityState());
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/**
 * @tc.name: ability_mgr_service_test_029
 * @tc.desc: test CompleteBackgroundNew
 * @tc.type: FUNC
 * @tc.require: AR000GJUND
 */
HWTEST_F(AbilityMgrModuleTest, ability_mgr_service_test_029, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "1", appName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);
    abilityRequest.targetVersion = 7;

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);

    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + "2", appName, bundleName);
    abilityRequest.targetVersion = 8;
    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND_NEW);
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(2);
    abilityRecord2->SetScheduler(scheduler);

    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);
    mission->AddAbilityRecordToTop(abilityRecord2);
    abilityRecord2->SetMissionRecord(mission);

    auto stackManager_ = abilityMgrServ_->GetStackManager();
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::BACKGROUNDING_NEW);
    stackManager_->CompleteBackgroundNew(abilityRecord2);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::BACKGROUND_NEW, abilityRecord2->GetAbilityState());
}

}  // namespace AAFwk
}  // namespace OHOS
