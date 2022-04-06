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
#include "pending_want_manager.h"
#include "pending_want_record.h"
#undef private
#undef protected
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"
#include "os_account_manager.h"

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
namespace {
const int32_t MOCK_MAIN_USER_ID = 100;
const int32_t ERROR_USER_ID_U256 = 256;
}  // namespace
class AbilityMgrModuleAccountTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WaitAMS();
    bool MockAppClent();
    Want CreateWant(const std::string &abilityName, const std::string &bundleName);
    AbilityInfo CreateAbilityInfo(const std::string &name, const std::string &appName, const std::string &bundleName);
    void CreateAbilityRequest(const std::string &abilityName, const std::string bundleName, Want &want,
        std::shared_ptr<MissionStack> &curMissionStack, sptr<IRemoteObject> &recordToken);
    void MockServiceAbilityLoadHandlerInner(bool &testResult, const std::string &bundleName,
        const std::string &abilityName, sptr<IRemoteObject> &testToken);
    ApplicationInfo CreateAppInfo(const std::string &appName, const std::string &name);
    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
    inline static std::shared_ptr<MockAppMgrClient> mockAppMgrClient_ {nullptr};
    sptr<MockAbilityScheduler> scheduler_ {nullptr};
    inline static bool doOnce_ = false;
};

bool AbilityMgrModuleAccountTest::MockAppClent()
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

Want AbilityMgrModuleAccountTest::CreateWant(const std::string &abilityName, const std::string &bundleName)
{
    ElementName element;
    element.SetDeviceID("");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return want;
}

AbilityInfo AbilityMgrModuleAccountTest::CreateAbilityInfo(
    const std::string &name, const std::string &appName, const std::string &bundleName)
{
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.name = name;
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    return abilityInfo;
}

ApplicationInfo AbilityMgrModuleAccountTest::CreateAppInfo(const std::string &appName, const std::string &bundleName)
{
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

void AbilityMgrModuleAccountTest::CreateAbilityRequest(const std::string &abilityName, const std::string bundleName,
    Want &want, std::shared_ptr<MissionStack> &curMissionStack, sptr<IRemoteObject> &recordToken)
{
    Want want2 = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest2;
    abilityRequest2.want = want2;
    abilityRequest2.abilityInfo.type = OHOS::AppExecFwk::AbilityType::PAGE;
    abilityRequest2.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest2);
    abilityRecord2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    recordToken = abilityRecord2->GetToken();

    want = CreateWant(abilityName, bundleName);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo =
        CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.abilityInfo.type = OHOS::AppExecFwk::AbilityType::SERVICE;
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    abilityMgrServ_->RemoveAllServiceRecord();
}

void AbilityMgrModuleAccountTest::MockServiceAbilityLoadHandlerInner(
    bool &testResult, const std::string &bundleName, const std::string &abilityName, sptr<IRemoteObject> &testToken)
{
    auto mockHandler = [&testResult, &bundleName, &abilityName, &testToken](const sptr<IRemoteObject> &token,
                           const sptr<IRemoteObject> &preToken,
                           const AbilityInfo &abilityInfo,
                           const ApplicationInfo &appInfo,
                           const Want &want) {
        testToken = token;
        testResult = !!testToken && abilityInfo.bundleName == bundleName && abilityInfo.name == abilityName &&
                     appInfo.bundleName == bundleName;
        return AppMgrResultCode::RESULT_OK;
    };

    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Invoke(mockHandler));
}

void AbilityMgrModuleAccountTest::WaitAMS()
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

void AbilityMgrModuleAccountTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest SetUpTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    mockAppMgrClient_ = std::make_shared<MockAppMgrClient>();
    abilityMgrServ_->OnStart();
    abilityMgrServ_->StartUser(MOCK_MAIN_USER_ID);
}

void AbilityMgrModuleAccountTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest TearDownTestCase called";
    abilityMgrServ_->OnStop();
    mockAppMgrClient_.reset();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityMgrModuleAccountTest::SetUp()
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest SetUp called";
    scheduler_ = new MockAbilityScheduler();
    if (!doOnce_) {
        doOnce_ = true;
        MockAppClent();
    }
    WaitAMS();
}

void AbilityMgrModuleAccountTest::TearDown()
{}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_001
 * SubFunction: NA
 * FunctionPoints: Start single PageAbility with 0 user authentication StartAbilityInner interface
 * EnvConditions: NA
 * CaseDescription: terminate ability.
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_001 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);

    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_001 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_002
 * SubFunction: NA
 * FunctionPoints: Start single ServiceAbility with 0 user authentication StartAbilityInner interface
 * EnvConditions: NA
 * CaseDescription: terminate ability.
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_002 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_002 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_003
 * SubFunction: NA
 * FunctionPoints: Start single PageAbility with 100 user authentication StartAbilityInner interface
 * EnvConditions: NA
 * CaseDescription: terminate ability.
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_003 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1, -1, MOCK_MAIN_USER_ID);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_003 start";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_004
 * SubFunction: NA
 * FunctionPoints: Start single ServiceAbility with 100 user authentication StartAbilityInner interface
 * EnvConditions: NA
 * CaseDescription: terminate ability.
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_004, TestSize.Level1)
{
    Want want;
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_004 start";
    ElementName element("", "com.ix.hiBackgroundData", "hiBackgroundData");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1, -1, MOCK_MAIN_USER_ID);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_004 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_005
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with error user authentication StartAbilityInner interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_005 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1, -1, ERROR_USER_ID_U256);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_005 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbilityInner_006
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication StartAbilityInner interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbilityInner_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_006 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    auto result = abilityMgrServ_->StartAbilityInner(want, nullptr, -1, -1, ERROR_USER_ID_U256);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbilityInner_006 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_001
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with 0 user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_001 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, 0, -1);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_001 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_002
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 0 user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_002 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, 0, -1);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_002 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_003
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with 100 user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_003 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, MOCK_MAIN_USER_ID, -1);
    WaitAMS();
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_003 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_004
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 100 user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_004 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, MOCK_MAIN_USER_ID, -1);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_004 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_005
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with error user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_005 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, ERROR_USER_ID_U256, -1);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_005 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_006
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication StartAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_006 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMgrServ_->StartAbility(want, abilityStartOptions, nullptr, ERROR_USER_ID_U256, -1);
    WaitAMS();
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_006 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_007
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Specify a user to start a page ability
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_007 start";
    auto topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    int result = abilityMgrServ_->StartAbility(want, MOCK_MAIN_USER_ID, -1);
    EXPECT_EQ(ERR_OK, result);

    topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    auto resultFunction = abilityMgrServ_->AttachAbilityThread(scheduler, token);
    EXPECT_EQ(resultFunction, ERR_OK);

    topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        topAbility->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::FOREGROUNDING_NEW, topAbility->GetAbilityState());

    AppInfo appInfo;
    appInfo.processName = topAbility->GetAbilityInfo().process;
    appInfo.state = AppState::FOREGROUND;
    abilityMgrServ_->OnAppStateChanged(appInfo);
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(topAbility->GetToken(),
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, saveData);

    WaitAMS();
    EXPECT_EQ(OHOS::AAFwk::AbilityState::FOREGROUND_NEW, topAbility->GetAbilityState());
    testing::Mock::AllowLeak(scheduler);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_007 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StartAbility_006
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Do not specify a user to start a page ability
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StartAbility_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_008 start";
    auto topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    int result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(ERR_OK, result);

    topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);
    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(1);
    auto resultFunction = abilityMgrServ_->AttachAbilityThread(scheduler, token);
    EXPECT_EQ(resultFunction, ERR_OK);

    topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(_, _)).Times(1);
    abilityMgrServ_->OnAbilityRequestDone(
        topAbility->GetToken(), (int32_t)OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::FOREGROUNDING_NEW, topAbility->GetAbilityState());

    AppInfo appInfo;
    appInfo.processName = topAbility->GetAbilityInfo().process;
    appInfo.state = AppState::FOREGROUND;
    abilityMgrServ_->OnAppStateChanged(appInfo);
    PacMap saveData;
    abilityMgrServ_->AbilityTransitionDone(topAbility->GetToken(),
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, saveData);

    WaitAMS();
    EXPECT_EQ(OHOS::AAFwk::AbilityState::FOREGROUND_NEW, topAbility->GetAbilityState());
    testing::Mock::AllowLeak(scheduler);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StartAbility_008 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_ConnectAbility_001
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 0 user authentication ConnectAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_ConnectAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_001 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_001 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_ConnectAbility_002
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 100 user authentication ConnectAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_ConnectAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_002 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, MOCK_MAIN_USER_ID);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_002 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_ConnectAbility_003
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication ConnectAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_ConnectAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_003 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_003 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_ConnectAbility_004
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Specify a user to bind a Serviceability
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_ConnectAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_004 start";
    std::string abilityName = "AccountServiceTest";
    std::string bundleName = "com.ix.hiAccountService";

    Want want;
    std::shared_ptr<MissionStack> curMissionStack;
    sptr<IRemoteObject> recordToken;
    CreateAbilityRequest(abilityName, bundleName, want, curMissionStack, recordToken);

    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));

    bool testResult = false;
    sptr<IRemoteObject> testToken;
    MockServiceAbilityLoadHandlerInner(testResult, bundleName, abilityName, testToken);

    int result = abilityMgrServ_->ConnectAbility(want, callback, recordToken, MOCK_MAIN_USER_ID);
    EXPECT_EQ(OHOS::ERR_OK, result);

    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());
    std::shared_ptr<AbilityRecord> record =
        abilityMgrServ_->GetConnectManagerByUserId(MOCK_MAIN_USER_ID)->GetServiceRecordByToken(testToken);
    EXPECT_TRUE(record);
    ElementName element;

    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectRecordList().front();
    EXPECT_TRUE(connectRecord);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(3);
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
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_004 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_ConnectAbility_005
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Do not specify a user to bind a Serviceability
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_ConnectAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_005 start";
    std::string abilityName = "AccountServiceTest";
    std::string bundleName = "com.ix.hiAccountService";

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
    EXPECT_EQ(OHOS::ERR_OK, result);

    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->connectMap_.size());
    EXPECT_EQ((std::size_t)1, abilityMgrServ_->connectManager_->serviceMap_.size());
    std::shared_ptr<AbilityRecord> record =
        abilityMgrServ_->GetConnectManagerByUserId(MOCK_MAIN_USER_ID)->GetServiceRecordByToken(testToken);
    EXPECT_TRUE(record);
    ElementName element;

    std::shared_ptr<ConnectionRecord> connectRecord = record->GetConnectRecordList().front();
    EXPECT_TRUE(connectRecord);

    sptr<MockAbilityScheduler> scheduler = new MockAbilityScheduler();
    EXPECT_TRUE(scheduler);

    EXPECT_CALL(*mockAppMgrClient_, UpdateAbilityState(_, _)).Times(3);
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
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_ConnectAbility_005 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StopServiceAbility_001
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with 0 user authentication StopServiceAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StopServiceAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_001 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);
    result = abilityMgrServ_->StopServiceAbility(want, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    testing::Mock::AllowLeak(stub);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_001 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StopServiceAbility_002
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with 100 user authentication StopServiceAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StopServiceAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_002 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, MOCK_MAIN_USER_ID);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*stub, OnAbilityDisconnectDone(_, _)).Times(1);
    result = abilityMgrServ_->StopServiceAbility(want, MOCK_MAIN_USER_ID);
    EXPECT_EQ(OHOS::ERR_OK, result);
    testing::Mock::AllowLeak(stub);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_002 end";
}

/*
 * Feature: AaFwk
 * Function: AbilityMgrModuleAccountTest_StopServiceAbility_003
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with error user authentication StopServiceAbility interface
 */
HWTEST_F(AbilityMgrModuleAccountTest, AbilityMgrModuleAccountTest_StopServiceAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_003 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    sptr<MockAbilityConnectCallbackStub> stub(new MockAbilityConnectCallbackStub());
    const sptr<AbilityConnectionProxy> callback(new AbilityConnectionProxy(stub));
    auto result = abilityMgrServ_->ConnectAbility(want, callback, nullptr, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    result = abilityMgrServ_->StopServiceAbility(want, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityMgrModuleAccountTest AbilityMgrModuleAccountTest_StopServiceAbility_003 end";
}
}  // namespace AAFwk
}  // namespace OHOS
