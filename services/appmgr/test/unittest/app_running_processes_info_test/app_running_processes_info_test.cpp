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

#define private public
#include "app_mgr_service_inner.h"
#undef private

#include <limits>
#include <gtest/gtest.h>
#include "iremote_object.h"
#include "refbase.h"
#include "application_info.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "ability_info.h"
#include "ability_running_record.h"
#include "hilog_wrapper.h"
#include "mock_app_scheduler.h"
#include "mock_ability_token.h"
#include "mock_app_spawn_client.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_iapp_state_callback.h"
#include "mock_bundle_manager.h"
#include "mock_application.h"

using namespace testing::ext;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::Return;
using testing::SetArgReferee;

namespace OHOS {
namespace AppExecFwk {
class AppRunningProcessesInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    static const std::string GetTestProcessName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAppName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }
    static int GetTestUid()
    {
        // a valid inner uid value which is not border value.
        const static int VALID_UID_VALUE = 1010;
        return VALID_UID_VALUE;
    }

    std::shared_ptr<AppRunningRecord> GetTestAppRunningRecord();
    sptr<IAppScheduler> GetMockedAppSchedulerClient() const;
    std::shared_ptr<AppRunningRecord> StartLoadAbility(const sptr<IRemoteObject> &token,
        const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
        const pid_t newPid) const;
    sptr<MockAbilityToken> GetMockToken() const
    {
        return mock_token_;
    }

protected:
    std::shared_ptr<AbilityRunningRecord> testAbilityRecord_;
    sptr<IAppScheduler> client_;
    sptr<MockAppScheduler> mockAppSchedulerClient_;
    std::shared_ptr<AppRunningRecord> testAppRecord_;
    std::unique_ptr<AppMgrServiceInner> service_;
    sptr<MockAbilityToken> mock_token_;
    sptr<BundleMgrService> mockBundleMgr;
};

void AppRunningProcessesInfoTest::SetUpTestCase()
{}

void AppRunningProcessesInfoTest::TearDownTestCase()
{}

void AppRunningProcessesInfoTest::SetUp()
{
    mockAppSchedulerClient_ = new (std::nothrow) MockAppScheduler();
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    mock_token_ = new (std::nothrow) MockAbilityToken();
    client_ = iface_cast<IAppScheduler>(mockAppSchedulerClient_.GetRefPtr());
    mockBundleMgr = new (std::nothrow) BundleMgrService();
    service_->SetBundleManager(mockBundleMgr);
}

void AppRunningProcessesInfoTest::TearDown()
{
    testAbilityRecord_.reset();
    testAppRecord_.reset();
}

sptr<IAppScheduler> AppRunningProcessesInfoTest::GetMockedAppSchedulerClient() const
{
    if (client_) {
        return client_;
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AppRunningProcessesInfoTest::GetTestAppRunningRecord()
{
    if (!testAppRecord_) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        appInfo->name = GetTestAppName();
        testAppRecord_ = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
        testAppRecord_->SetApplicationClient(GetMockedAppSchedulerClient());
        auto abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = GetTestAbilityName();
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "module789";
        testAppRecord_->AddModule(appInfo, abilityInfo, GetMockToken(), hapModuleInfo, nullptr);
    }
    return testAppRecord_;
}

std::shared_ptr<AppRunningRecord> AppRunningProcessesInfoTest::StartLoadAbility(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const pid_t newPid) const
{
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    service_->SetAppSpawnClient(mockClientPtr);
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(newPid), Return(ERR_OK)));

    service_->LoadAbility(token, nullptr, abilityInfo, appInfo, nullptr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);

    EXPECT_TRUE(record);
    auto clent = GetMockedAppSchedulerClient();
    record->SetApplicationClient(clent);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), newPid);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    return record;
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    RunningProcessInfo info;
    service_->GetRunningProcessInfoByToken(GetMockToken(), info);
    EXPECT_TRUE(info.processName_ == GetTestProcessName());
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    int uid = 0;
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->uid = uid;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    sptr<MockApplication> mockApplication(new MockApplication());
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockApplication);
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockApplication, ScheduleLaunchApplication(_, _))
        .Times(1)
        .WillOnce(Invoke(mockApplication.GetRefPtr(), &MockApplication::LaunchApplication));
    Configuration config;
    record->LaunchApplication(config);
    mockApplication->Wait();

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    std::vector<RunningProcessInfo> info;
    size_t infoCount {0};
    auto res = service_->GetAllRunningProcesses(info);
    EXPECT_TRUE (res == ERR_OK);
    EXPECT_TRUE(info.size() == infoCount);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat two apprunningrecords, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_003, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    int uid = 0;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    sptr<MockApplication> mockApplication(new MockApplication());
    sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockApplication);
    record->SetApplicationClient(client);
    EXPECT_CALL(*mockApplication, ScheduleLaunchApplication(_, _))
        .Times(1)
        .WillOnce(Invoke(mockApplication.GetRefPtr(), &MockApplication::LaunchApplication));
    Configuration config;
    record->LaunchApplication(config);
    mockApplication->Wait();

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(0);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "2";
    abilityInfo2->applicationInfo.uid = uid;
    auto appInfo2 = std::make_shared<ApplicationInfo>();
    appInfo2->name = GetTestAppName() + "2";
    appInfo2->uid = uid;
    BundleInfo bundleInfo2;
    HapModuleInfo hapModuleInfo2;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo2, appInfo2, bundleInfo2, hapModuleInfo2));
    EXPECT_TRUE(service_ != nullptr);
    auto mock_token = new (std::nothrow) MockAbilityToken();
    auto record2 = service_->CreateAppRunningRecord(
        mock_token, nullptr, appInfo2, abilityInfo2, GetTestProcessName() + "2", bundleInfo2, hapModuleInfo2, nullptr);
    EXPECT_TRUE(record != nullptr);
    record2->SetUid(uid);

    std::vector<RunningProcessInfo> info;
    size_t infoCount {0};
    auto res = service_->GetAllRunningProcesses(info);
    EXPECT_TRUE (res == ERR_OK);
    EXPECT_TRUE(info.size() == infoCount);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecords, set record state, call query function.
 */
HWTEST_F(AppRunningProcessesInfoTest, UpdateAppRunningRecord_004, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    RunningProcessInfo info;
    service_->appRunningManager_->GetRunningProcessInfoByToken(GetMockToken(), info);
    EXPECT_TRUE(info.processName_ == GetTestProcessName());
}
}  // namespace AppExecFwk
}  // namespace OHOS