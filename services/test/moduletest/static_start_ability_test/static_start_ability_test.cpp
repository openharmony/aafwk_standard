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
const std::string COM_OHOS_Test = "com.ohos.test";
const std::string MAIN_ABILITY = ".MainAbility";
const std::string MY_APPLICATION = ".MyApplication";
const std::string DESCRIPTION = "$string:mainability_description";
const std::string LABEL = "$string:label";
const std::string ICON = "$media:icon";
const std::string THEME = "mytheme";
const std::string KIND = "page";
const std::string PROCESS = "p10";
const std::string READ_PERMISSION = "readPermission";
const std::string WRITE_PERMISSION = "writePermission";
const std::string RESOURCE_PATH = "/data/app/com.ohos.test";
const std::string SRC_PATH = "/resources/base/profile";
const std::string SRC_LANGUAGE = "C++";
const uint32_t NUMBER_ONE = 10001;
const uint32_t NUMBER_TWO = 10002;
const uint32_t NUMBER_THREE = 10003;
}  // namespace
class StaticStartAbilityTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool MockAppClent();
    void SetActive();
    void WaitAMS();
    Want CreateWant(const std::string &abilityName, const std::string &bundleName);
    std::shared_ptr<AbilityRecord> GreatePageAbility(const std::string &abilityName, const std::string &bundleName);

    inline static std::shared_ptr<MockAppMgrClient> mockAppMgrClient_ {nullptr};
    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
    inline static BundleMgrService *bundleObject_ {nullptr};
    std::shared_ptr<AbilityStackManager> stackManager_ {nullptr};
    sptr<MockAbilityScheduler> scheduler_ {nullptr};
    inline static bool doOnce_ = false;  // In order for mock to execute once
};

void StaticStartAbilityTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    mockAppMgrClient_ = std::make_shared<MockAppMgrClient>();

    if (StaticStartAbilityTest::abilityMgrServ_) {
    if (StaticStartAbilityTest::abilityMgrServ_->state_ == ServiceRunningState::STATE_RUNNING) {
        return;
    }
    StaticStartAbilityTest::abilityMgrServ_->state_ = ServiceRunningState::STATE_RUNNING;
    StaticStartAbilityTest::abilityMgrServ_->eventLoop_ =
        AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->eventLoop_);

    StaticStartAbilityTest::abilityMgrServ_->handler_ =std::make_shared<AbilityEventHandler>(
        StaticStartAbilityTest::abilityMgrServ_->eventLoop_, StaticStartAbilityTest::abilityMgrServ_);
    StaticStartAbilityTest::abilityMgrServ_->connectManager_ = std::make_shared<AbilityConnectManager>();
    StaticStartAbilityTest::abilityMgrServ_->connectManagers_.emplace(0,
        StaticStartAbilityTest::abilityMgrServ_->connectManager_);
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->handler_);
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->connectManager_);

    StaticStartAbilityTest::abilityMgrServ_->connectManager_->
        SetEventHandler(StaticStartAbilityTest::abilityMgrServ_->handler_);

    StaticStartAbilityTest::abilityMgrServ_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
    StaticStartAbilityTest::abilityMgrServ_->dataAbilityManagers_.emplace(0,
        StaticStartAbilityTest::abilityMgrServ_->dataAbilityManager_);
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->dataAbilityManager_);

    StaticStartAbilityTest::abilityMgrServ_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->amsConfigResolver_);
    StaticStartAbilityTest::abilityMgrServ_->amsConfigResolver_->Parse();

    StaticStartAbilityTest::abilityMgrServ_->currentMissionListManager_ = std::make_shared<MissionListManager>(0);
    StaticStartAbilityTest::abilityMgrServ_->currentMissionListManager_->Init();

    StaticStartAbilityTest::abilityMgrServ_->pendingWantManager_ = std::make_shared<PendingWantManager>();
    EXPECT_TRUE(StaticStartAbilityTest::abilityMgrServ_->pendingWantManager_);

    StaticStartAbilityTest::abilityMgrServ_->eventLoop_->Run();

    GTEST_LOG_(INFO) << "OnStart success";
    return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void StaticStartAbilityTest::TearDownTestCase(void)
{
    abilityMgrServ_->OnStop();
    mockAppMgrClient_.reset();
}

void StaticStartAbilityTest::SetUp(void)
{
    scheduler_ = new MockAbilityScheduler();
    if (!doOnce_) {
        doOnce_ = true;
        MockAppClent();
    }
    WaitAMS();
}

void StaticStartAbilityTest::TearDown(void)
{}

bool StaticStartAbilityTest::MockAppClent()
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

void StaticStartAbilityTest::SetActive()
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

void StaticStartAbilityTest::WaitAMS()
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

Want StaticStartAbilityTest::CreateWant(const std::string &abilityName, const std::string &bundleName)
{
    ElementName element;
    element.SetDeviceID("");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return want;
}

std::shared_ptr<AbilityRecord> StaticStartAbilityTest::GreatePageAbility(
    const std::string &abilityName, const std::string &bundleName)
{
    Want want = CreateWant(abilityName, bundleName);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    int testRequestCode = 1;
    SetActive();
    abilityMgrServ_->StartAbility(want, 0, testRequestCode);
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

/*
 * Feature: AaFwk
 * Function: ability manager service
 * SubFunction: StartAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test the abilityInfo query in StartAbility.
 */
HWTEST_F(StaticStartAbilityTest, StaticStartAbility_001, TestSize.Level1)
{
    std::string abilityName = "com.ohos.test.MainAbility";
    std::string bundleName = "com.ohos.test";
    abilityMgrServ_->RemoveAllServiceRecord();
    Want want = CreateWant(abilityName, bundleName);
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1).WillOnce(Return(AppMgrResultCode::RESULT_OK));
    int testRequestCode = 1;
    SetActive();
    abilityMgrServ_->StartAbility(want, 0, testRequestCode);
    WaitAMS();

    auto stack = abilityMgrServ_->GetStackManager();
    EXPECT_TRUE(stack);
    std::shared_ptr<AbilityRecord> record = stack->GetCurrentTopAbility();
    record->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_TRUE(record);
    AbilityInfo abilityInfo = record->GetAbilityInfo();
    abilityMgrServ_->RemoveAllServiceRecord();
    EXPECT_EQ(abilityInfo.name, COM_OHOS_Test + MAIN_ABILITY);
    EXPECT_EQ(abilityInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.applicationInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.applicationName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.applicationInfo.name, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.bundleName, COM_OHOS_Test);
    EXPECT_EQ(abilityInfo.moduleName, MY_APPLICATION);
    EXPECT_EQ(abilityInfo.kind, KIND);
    EXPECT_EQ(abilityInfo.process, PROCESS);
    EXPECT_EQ(abilityInfo.labelId, NUMBER_ONE);
    EXPECT_EQ(abilityInfo.label, LABEL);
    EXPECT_EQ(abilityInfo.descriptionId, NUMBER_TWO);
    EXPECT_EQ(abilityInfo.description, DESCRIPTION);
    EXPECT_EQ(abilityInfo.iconId, NUMBER_THREE);
    EXPECT_EQ(abilityInfo.iconPath, ICON);
    EXPECT_EQ(abilityInfo.theme, THEME);
    EXPECT_EQ(abilityInfo.readPermission, READ_PERMISSION);
    EXPECT_EQ(abilityInfo.writePermission, WRITE_PERMISSION);
    EXPECT_EQ(abilityInfo.resourcePath, RESOURCE_PATH);
    EXPECT_EQ(abilityInfo.srcPath, SRC_PATH);
    EXPECT_EQ(abilityInfo.srcLanguage, SRC_LANGUAGE);
    EXPECT_FALSE(abilityInfo.multiUserShared);
    EXPECT_FALSE(abilityInfo.supportPipMode);
    EXPECT_FALSE(abilityInfo.grantPermission);
    EXPECT_FALSE(abilityInfo.isLauncherAbility);
    EXPECT_FALSE(abilityInfo.isNativeAbility);
    EXPECT_FALSE(abilityInfo.enabled);
    EXPECT_FALSE(abilityInfo.formEnabled);
}
}  // namespace AAFwk
}  // namespace OHOS