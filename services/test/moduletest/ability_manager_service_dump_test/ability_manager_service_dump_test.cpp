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
#undef private
#undef protected
#include "mock_app_mgr_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID = 100;
const size_t SIZE_ZERO = 0;
const std::string STRING_PROCESS_NAME = "process_name";
}  // namespace

class AbilityManagerServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
    std::shared_ptr<AbilityManagerService> abilityMs_;
    std::unique_ptr<MockAppMgrClient> mockAppMgrClient_;
};

void AbilityManagerServiceDumpTest::SetUpTestCase()
{}

void AbilityManagerServiceDumpTest::TearDownTestCase()
{}

void AbilityManagerServiceDumpTest::SetUp()
{
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    EXPECT_NE(abilityMs_, nullptr);
    EXPECT_NE(abilityMs_->appScheduler_, nullptr);

    mockAppMgrClient_ = std::make_unique<MockAppMgrClient>();
    EXPECT_NE(mockAppMgrClient_, nullptr);

    abilityMs_->appScheduler_->appMgrClient_ = std::move(mockAppMgrClient_);
    EXPECT_NE(abilityMs_->appScheduler_->appMgrClient_, nullptr);
}

void AbilityManagerServiceDumpTest::TearDown()
{
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

/**
 * @tc.name: AbilityManagerService_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    std::vector<RunningProcessInfo> info;
    auto result = abilityMs_->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AbilityManagerService_DumpSysFuncInit_0100
 * @tc.desc: DumpSysFuncInit
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysFuncInit_0100, TestSize.Level1)
{
    abilityMs_->DumpSysFuncInit();
    EXPECT_GT(abilityMs_->dumpsysFuncMap_.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysInner_0100
 * @tc.desc: DumpSysInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysMissionListInner_0100
 * @tc.desc: DumpSysMissionListInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysMissionListInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysMissionListInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysAbilityInner_0100
 * @tc.desc: DumpSysAbilityInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysAbilityInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysAbilityInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysStateInner_0100
 * @tc.desc: DumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysStateInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysStateInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysPendingInner_0100
 * @tc.desc: DumpSysPendingInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysPendingInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysPendingInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DumpSysProcess_0100
 * @tc.desc: DumpSysProcess
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DumpSysProcess_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DumpSysProcess(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_DataDumpSysStateInner_0100
 * @tc.desc: DataDumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_DataDumpSysStateInner_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;
    abilityMs_->DataDumpSysStateInner(args, info, isClient, isUserID, USER_ID);
    EXPECT_GT(info.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerService_OnAppStateChanged_0100
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerServiceDumpTest, AbilityManagerService_OnAppStateChanged_0100, TestSize.Level1)
{
    abilityMs_->connectManager_ = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(abilityMs_->connectManager_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    abilityMs_->currentMissionListManager_ = std::make_shared<MissionListManager>(USER_ID);
    EXPECT_NE(abilityMs_->currentMissionListManager_, nullptr);
    abilityMs_->currentMissionListManager_->terminateAbilityList_.push_back(abilityRecord);

    abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
    EXPECT_NE(abilityMs_->dataAbilityManager_, nullptr);

    AppInfo info;
    info.processName = STRING_PROCESS_NAME;
    info.state = AppState::TERMINATED;
    abilityMs_->OnAppStateChanged(info);

    abilityRecord = abilityMs_->currentMissionListManager_->terminateAbilityList_.front();
    EXPECT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetAppState(), AppState::TERMINATED);
}
}  // namespace AAFwk
}  // namespace OHOS
