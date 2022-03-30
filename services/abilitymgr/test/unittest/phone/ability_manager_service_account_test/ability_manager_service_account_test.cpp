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
#include "ability_stack_manager.h"
#include "ability_connect_manager.h"
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
#include "os_account_manager.h"
#include "os_account_info.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AccountSA;
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
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t ERROR_USER_ID_U256 = 256;
}  // namespace
class AbilityManagerServiceAccountTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
    AbilityRequest abilityRequest_ {};
};
static OsAccountInfo osAccountInfo_ = OsAccountInfo();
static int new_user_id_;
void AbilityManagerServiceAccountTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest SetUpTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    AccountSA::OsAccountManager::CreateOsAccount("testAccount", OsAccountType::NORMAL, osAccountInfo_);
    new_user_id_ = osAccountInfo_.GetLocalId();
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
    WaitUntilTaskFinished();
    GTEST_LOG_(INFO) << "Create new user. UserId: "<<new_user_id_;
}

void AbilityManagerServiceAccountTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest TearDownTestCase called";
    abilityMs_->OnStop();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
    AccountSA::OsAccountManager::RemoveOsAccount(new_user_id_);
}

void AbilityManagerServiceAccountTest::SetUp()
{}

void AbilityManagerServiceAccountTest::TearDown()
{}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: 100 user and 101 user for StartAbility
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_001 start";
    abilityMs_->StartUser(USER_ID_U100);
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    abilityMs_->StartUser(new_user_id_);
    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }

    result = abilityMs_->StartAbility(want, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start standard PageAbility with 100 user authentication StartAbility interface
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_002 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiworld", "WorldService");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
        token = topAbility->GetToken();
    }

    Want want1;
    ElementName element1("", "com.ix.hiMusic", "hiMusic");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StartAbility(want1, token, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result1);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: use 100 user and 101 user to start StartAbility in abilityStartSetting mode
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_003 start";
    AbilityStartSetting abilityStartSetting ;
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiMusic", "hiMusic");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, abilityStartSetting, nullptr, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    abilityMs_->StartUser(new_user_id_);
    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    result = abilityMs_->StartAbility(want, abilityStartSetting, nullptr, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: use 100 user and 101 user to start StartAbility in abilityStartOptions mode
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_004 start";
    StartOptions abilityStartOptions;
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiMusic", "hiMusic");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    abilityMs_->StartUser(new_user_id_);
    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: use 0 Standard user to start StartAbility in abilityStartOptions mode
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_005 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, 0, -1);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_005 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 0 user authentication StartAbility interface
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_006 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, 0, -1);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_006 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with 100 user authentication StartAbility interface
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_007 start";
    abilityMs_->StartUser(USER_ID_U100);
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, USER_ID_U100, -1);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_007 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 100 user authentication StartAbility interface
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_008 start";
    abilityMs_->StartUser(USER_ID_U100);
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, USER_ID_U100, -1);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_008 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with error user authentication StartAbility interface
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_009 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, ERROR_USER_ID_U256, -1);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_009 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication StartAbility interface
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_010 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    StartOptions abilityStartOptions;
    auto result = abilityMs_->StartAbility(want, abilityStartOptions, nullptr, ERROR_USER_ID_U256, -1);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_010 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: 100 users StartAbility start 101 users' app in non-concurrent mode
 * StartAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbility_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_011 start";
    abilityMs_->StartUser(USER_ID_U100);
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("device", "ohos.samples.clock", "ohos.samples.clock.default");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbility_011 end";
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Start standard TerminateAbility with 100 user
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_TerminateAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_TerminateAbility_001 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }

    auto result1 = abilityMs_->TerminateAbility(token, -1, &want);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result1);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_TerminateAbility_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with 0 user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_001 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 0 user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_002 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with 100 user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_003 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1, -1, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 100 user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_004 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundData", "hiBackgroundData");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1, -1, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single PageAbility with error user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_005 start";
    Want want;
    ElementName element("", "com.ix.hiSingleMusicInfo", "SingleMusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1, -1, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_005 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication StartAbilityInner interface
 * StartAbilityInner parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StartAbilityInner_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_006 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1, -1, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StartAbilityInner_006 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.user id is U100
 * 1.callback is nullptr, connectAbility failed
 * 2.ability type is page, connectAbility failed
 * 3.ability type is service and callback is not nullptr, connectAbility success
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_001 start";
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, nullptr, nullptr, USER_ID_U100);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Want want1;
    ElementName element1("", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->ConnectAbility(want1, callback, nullptr, USER_ID_U100);
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);

    auto result2 = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    EXPECT_EQ(result2, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.user id is new user id
 * 1.callback is nullptr, connectAbility failed
 * 2.ability type is page, connectAbility failed
 * 3.ability type is service and callback is not nullptr, connectAbility success
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_002 start";
    abilityMs_->StartUser(new_user_id_);
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, nullptr, nullptr, new_user_id_);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Want want1;
    ElementName element1("", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->ConnectAbility(want1, callback, nullptr, new_user_id_);
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);

    auto result2 = abilityMs_->ConnectAbility(want, callback, nullptr, new_user_id_);
    EXPECT_EQ(result2, ERR_OK);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 0 user authentication ConnectAbility interface
 * ConnectAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_003 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with 100 user authentication ConnectAbility interface
 * ConnectAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_004 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Start single ServiceAbility with error user authentication ConnectAbility interface
 * ConnectAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_005 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_005 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: 100 users ConnectAbility start 101 users' app in non-concurrent mode
 * ConnectAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ConnectAbility_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_006 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, new_user_id_);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ConnectAbility_006 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.token is nullptr, ScheduleConnectAbilityDone failed
 * 3.ability record is nullptr, ScheduleConnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_001 start";
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->GetServiceMap();
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

    auto result3 = abilityMs_->ScheduleConnectAbilityDone(service->GetToken(), callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result3, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.ability type is not service, ScheduleConnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleConnectAbilityDone_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_002 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleConnectAbilityDone(token, callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is new user id
 * 2.token is nullptr, ScheduleConnectAbilityDone failed
 * 3.ability record is nullptr, ScheduleConnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleConnectAbilityDone_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_003 start";
    abilityMs_->StartUser(new_user_id_);
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(new_user_id_)->GetServiceMap();
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

    auto result3 = abilityMs_->ScheduleConnectAbilityDone(service->GetToken(), callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result3, ERR_OK);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is new user id
 * 2.ability type is not service, ScheduleConnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleConnectAbilityDone_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_004 start";
    abilityMs_->StartUser(new_user_id_);
    auto topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleConnectAbilityDone(token, callback->AsObject());
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleConnectAbilityDone_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.token is nullptr, ScheduleDisconnectAbilityDone failed
 * 3.ability record is nullptr, ScheduleDisconnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleDisconnectAbilityDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_001 start";
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->GetServiceMap();
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

    auto result3 = abilityMs_->ScheduleDisconnectAbilityDone(service->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result3, CONNECTION_NOT_EXIST);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.ability type is not service, ScheduleDisconnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleDisconnectAbilityDone_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_002 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleDisconnectAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is new user id
 * 2.ability type is not service, ScheduleDisconnectAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleDisconnectAbilityDone_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_003 start";
    abilityMs_->StartUser(new_user_id_);
    auto topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleDisconnectAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleDisconnectAbilityDone_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.token is nullptr, ScheduleCommandAbilityDone failed
 * 3.ability record is nullptr, ScheduleCommandAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleCommandAbilityDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_001 start";
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->GetServiceMap();
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

    auto result3 = abilityMs_->ScheduleCommandAbilityDone(service->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result3, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is U100
 * 2.ability type is not service, ScheduleCommandAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleCommandAbilityDone_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_002 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleCommandAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is new user id
 * 2.token is nullptr, ScheduleCommandAbilityDone failed
 * 3.ability record is nullptr, ScheduleCommandAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleCommandAbilityDone_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_003 start";
    abilityMs_->StartUser(new_user_id_);
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(result, ERR_OK);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(new_user_id_)->GetServiceMap();
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

    auto result3 = abilityMs_->ScheduleCommandAbilityDone(service->GetToken());
    WaitUntilTaskFinished();
    EXPECT_EQ(result3, ERR_OK);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.the user id is new user id
 * 2.ability type is not service, ScheduleCommandAbilityDone failed
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_ScheduleCommandAbilityDone_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_004 start";
    abilityMs_->StartUser(new_user_id_);
    auto topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(new_user_id_)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result1 = abilityMs_->ScheduleCommandAbilityDone(token);
    WaitUntilTaskFinished();
    EXPECT_EQ(result1, TARGET_ABILITY_NOT_SERVICE);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_ScheduleCommandAbilityDone_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: the user id is U100, verify StopServiceAbility results
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StopServiceAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_001 start";
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->GetServiceMap();
    EXPECT_EQ(1, static_cast<int>(serviceMap.size()));
    for (auto &it : serviceMap) {
        EXPECT_EQ(it.first, element.GetURI());
    }
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StopServiceAbility(want1, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE, result1);

    auto result2 = abilityMs_->StopServiceAbility(want, USER_ID_U100);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result2);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: the user id is new user id, verify StopServiceAbility results
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StopServiceAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_002 start";
    abilityMs_->StartUser(new_user_id_);
    Want want;
    ElementName element("", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, new_user_id_, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    auto serviceMap = abilityMs_->GetConnectManagerByUserId(new_user_id_)->GetServiceMap();
    EXPECT_EQ(1, static_cast<int>(serviceMap.size()));
    for (auto &it : serviceMap) {
        EXPECT_EQ(it.first, element.GetURI());
    }
    auto service = serviceMap.at(element.GetURI());
    service->SetAbilityState(AAFwk::AbilityState::ACTIVE);

    Want want1;
    ElementName element1("device", "com.ix.hiMusic", "MusicAbility");
    want1.SetElement(element1);
    auto result1 = abilityMs_->StopServiceAbility(want1, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE, result1);

    auto result2 = abilityMs_->StopServiceAbility(want, new_user_id_);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result2);
    abilityMs_->StartUser(USER_ID_U100);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with 0 user authentication StopServiceAbility interface
 * StopServiceAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StopServiceAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_003 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    result = abilityMs_->StopServiceAbility(want, 0);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with 100 user authentication StopServiceAbility interface
 * StopServiceAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StopServiceAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_004 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result);
    result = abilityMs_->StopServiceAbility(want, USER_ID_U100);
    EXPECT_EQ(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Use single ServiceAbility with error user authentication StopServiceAbility interface
 * StopServiceAbility parameter
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_StopServiceAbility_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_005 start";
    Want want;
    ElementName element("", "com.ix.hiBackgroundMusic", "hiBackgroundMusic");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto result = abilityMs_->ConnectAbility(want, callback, nullptr, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    result = abilityMs_->StopServiceAbility(want, ERROR_USER_ID_U256);
    EXPECT_NE(OHOS::ERR_OK, result);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_StopServiceAbility_005 end";
}

/*
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 * EnvConditions: NA
 * CaseDescription: Verify function MinimizeAbility
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_MinimizeAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_MinimizeAbility_001 start";
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result);

    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
        token = topAbility->GetToken();
    }
    auto resultFunction = abilityMs_->MinimizeAbility(token, true);
    EXPECT_EQ(resultFunction, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_MinimizeAbility_001 end";
}

/*
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify function AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_AttachAbilityThread_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_AttachAbilityThread_001 start";
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    auto resultFunction = abilityMs_->AttachAbilityThread(scheduler, token);
    EXPECT_EQ(resultFunction, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_AttachAbilityThread_001 end";
}

/*
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify function OnAbilityRequestDone
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_OnAbilityRequestDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_OnAbilityRequestDone_001 start";
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result);
    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        token = topAbility->GetToken();
    }
    abilityMs_->OnAbilityRequestDone(token, 2);
    EXPECT_EQ(topAbility->GetAbilityState(), OHOS::AAFwk::AbilityState::FOREGROUNDING_NEW);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_OnAbilityRequestDone_001 end";
}

/*
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 * EnvConditions: NA
 * CaseDescription: Verify function KillProcess
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_KillProcess_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_KillProcess_001 start";
    // default user
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(ERR_OK, result);
    auto resultFunction = abilityMs_->KillProcess("bundle");
    EXPECT_EQ(ERR_OK, resultFunction);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_KillProcess_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: AbilityTransitionDone failed due to empty token or scheduler
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_AbilityTransitionDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_AbilityTransitionDone_001 start";
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want, USER_ID_U100, -1);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);
    topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();

    sptr<IRemoteObject> token = nullptr;
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUNDING_NEW);
        token = topAbility->GetToken();
    }
    PacMap saveData;
    auto res1 = abilityMs_->AbilityTransitionDone(token, ABILITY_STATE_FOREGROUND_NEW, saveData);
    EXPECT_EQ(OHOS::ERR_OK, res1);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_AbilityTransitionDone_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 * EnvConditions: NA
 * CaseDescription: Verify function TerminateAbilityResult
 */
HWTEST_F(AbilityManagerServiceAccountTest, Account_TerminateAbilityResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_TerminateAbilityResult_001 start";
    Want want;
    ElementName element("", "com.ix.accountService", "accountService");
    want.SetElement(element);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();

    auto result2 = abilityMs_->ConnectAbility(want, callback, nullptr, USER_ID_U100);
    EXPECT_EQ(result2, ERR_OK);

    sptr<IRemoteObject> token1 = nullptr;
    auto serviceMapIter = abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->serviceMap_.find(element.GetURI());

    if (serviceMapIter != abilityMs_->GetConnectManagerByUserId(USER_ID_U100)->serviceMap_.end()) {
        token1 = serviceMapIter->second->GetToken();
    }

    auto result1 = abilityMs_->TerminateAbilityResult(token1, serviceMapIter->second->GetStartId());
    EXPECT_EQ(OHOS::ERR_OK, result1);
    GTEST_LOG_(INFO) << "AbilityManagerServiceAccountTest Account_TerminateAbilityResult_001 end";
}
}  // namespace AAFwk
}  // namespace OHOS