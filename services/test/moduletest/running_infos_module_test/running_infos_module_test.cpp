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

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t MOCK_MAIN_USER_ID = 100;

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
}  // namespace
class RunningInfosModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void OnStartAms();
    void OnStopAms();
    Want CreateWant(const std::string &abilityName, const std::string &bundleName);

    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
    inline static MockAppMgrClient *mockAppMgrClient_ = nullptr;
};

Want RunningInfosModuleTest::CreateWant(const std::string &abilityName, const std::string &bundleName)
{
    ElementName element;
    element.SetDeviceID("device");
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return want;
}

void RunningInfosModuleTest::OnStartAms()
{
    if (abilityMgrServ_) {
        if (abilityMgrServ_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMgrServ_->state_ = ServiceRunningState::STATE_RUNNING;
        abilityMgrServ_->useNewMission_ = true;

        abilityMgrServ_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMgrServ_->eventLoop_);

        abilityMgrServ_->handler_ = std::make_shared<AbilityEventHandler>(abilityMgrServ_->eventLoop_, abilityMgrServ_);
        EXPECT_TRUE(abilityMgrServ_->handler_);

        // init user controller.
        abilityMgrServ_->userController_ = std::make_shared<UserController>();
        EXPECT_TRUE(abilityMgrServ_->userController_);
        abilityMgrServ_->userController_->Init();
        int userId = MOCK_MAIN_USER_ID;
        abilityMgrServ_->userController_->SetCurrentUserId(userId);
        abilityMgrServ_->InitConnectManager(userId, true);
        abilityMgrServ_->InitDataAbilityManager(userId, true);
        abilityMgrServ_->InitPendWantManager(userId, true);
        abilityMgrServ_->systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(abilityMgrServ_->systemDataAbilityManager_);

        abilityMgrServ_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMgrServ_->amsConfigResolver_);
        abilityMgrServ_->amsConfigResolver_->Parse();
        abilityMgrServ_->useNewMission_ = abilityMgrServ_->amsConfigResolver_->IsUseNewMission();

        abilityMgrServ_->SetStackManager(userId, true);
        abilityMgrServ_->InitMissionListManager(userId, true);
        abilityMgrServ_->connectManager_->SetEventHandler(abilityMgrServ_->handler_);
        abilityMgrServ_->eventLoop_->Run();
        auto topAbility = abilityMgrServ_->GetListManagerByUserId(MOCK_MAIN_USER_ID)->GetCurrentTopAbilityLocked();
        if (topAbility) {
            topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
        }
        WaitUntilTaskFinished();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void RunningInfosModuleTest::OnStopAms()
{
    abilityMgrServ_->eventLoop_.reset();
    abilityMgrServ_->handler_.reset();
    abilityMgrServ_->state_ = ServiceRunningState::STATE_NOT_START;
}

void RunningInfosModuleTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    mockAppMgrClient_ = new MockAppMgrClient();
    if (mockAppMgrClient_) {
        appScheduler->appMgrClient_.reset(mockAppMgrClient_);
        GTEST_LOG_(INFO) << "mock appMgrClient_ ok";
    }
}

void RunningInfosModuleTest::TearDownTestCase()
{
    delete mockAppMgrClient_;
    mockAppMgrClient_ = nullptr;
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void RunningInfosModuleTest::SetUp()
{
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartAms();
}

void RunningInfosModuleTest::TearDown()
{
    OnStopAms();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start page ability, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1);
    Want want = CreateWant(abilityName, bundleName);

    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(1);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {1};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_002, TestSize.Level1)
{
    std::string abilityName = "ServiceAbility";
    std::string bundleName = "com.ix.hiService";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(1);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {1};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start launcher, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_003, TestSize.Level1)
{
    std::string abilityName = "com.ohos.launcher.MainAbility";
    std::string bundleName = "com.ohos.launcher";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(1);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {1};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two page abilities, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_004, TestSize.Level1)
{
    std::string abilityName = "MusicAbility";
    std::string bundleName = "com.ix.hiMusic";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(2);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMgrServ_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    std::string abilityName2 = "MusicAbilityOther";
    std::string bundleName2 = "com.ix.hiMusicOther";
    want = CreateWant(abilityName2, bundleName2);
    auto result2 = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(2);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {2};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName2);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        EXPECT_TRUE(infos[1].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::FOREGROUND_NEW));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two service abilities, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_005, TestSize.Level1)
{
    std::string abilityName = "ServiceAbility";
    std::string bundleName = "com.ix.hiService";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(2);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    std::string abilityName2 = "ServiceAbilityOther";
    std::string bundleName2 = "com.ix.hiServiceOther";
    want = CreateWant(abilityName2, bundleName2);
    auto result2 = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(2);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {2};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        EXPECT_TRUE(infos[1].ability.GetAbilityName() == abilityName2);
        EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two launcher abilities, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetAbilityRunningInfos_006, TestSize.Level1)
{
    std::string abilityName = "com.ohos.launcher.MainAbility";
    std::string bundleName = "com.ohos.launcher";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(2);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMgrServ_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::FOREGROUND_NEW);

    std::string abilityName2 = "com.ohos.launcher.MainAbilityOther";
    std::string bundleName2 = "com.ohos.launcherOther";
    want = CreateWant(abilityName2, bundleName2);
    auto result2 = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(2);

    std::vector<AbilityRunningInfo> infos;
    abilityMgrServ_->GetAbilityRunningInfos(infos);
    size_t infoCount {2};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == abilityName2);
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        EXPECT_TRUE(infos[1].ability.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::FOREGROUND_NEW));
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    std::string abilityName = "hiExtension";
    std::string bundleName = "com.ix.hiExtension";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(1);

    std::vector<ExtensionRunningInfo> infos;
    int upperLimit = 10;
    abilityMgrServ_->GetExtensionRunningInfos(upperLimit, infos);
    size_t infoCount {1};
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].extension.GetAbilityName() == abilityName);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start service abilities, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetExtensionRunningInfos_002, TestSize.Level1)
{
    std::string abilityName = "hiExtension";
    std::string bundleName = "com.ix.hiExtension";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(2);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    std::string abilityName2 = "hiExtensionOther";
    std::string bundleName2 = "com.ix.hiExtension";
    want = CreateWant(abilityName2, bundleName2);
    auto result2 = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _)).Times(2);

    std::vector<ExtensionRunningInfo> infos;
    int upperLimit = 10;
    abilityMgrServ_->GetExtensionRunningInfos(upperLimit, infos);
    size_t infoCount {2};
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].extension.GetAbilityName() == abilityName);
        EXPECT_TRUE(infos[1].extension.GetAbilityName() == abilityName2);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints:query process running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, mock object, call query function.
 */
HWTEST_F(RunningInfosModuleTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    std::string abilityName = "hiExtension";
    std::string bundleName = "com.ix.hiExtension";
    EXPECT_CALL(*mockAppMgrClient_, LoadAbility(_, _, _, _, _)).Times(1);
    Want want = CreateWant(abilityName, bundleName);
    auto result = abilityMgrServ_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);
    EXPECT_CALL(*mockAppMgrClient_, GetAllRunningProcesses(_)).Times(1);

    std::vector<RunningProcessInfo> infos;
    auto ret = abilityMgrServ_->GetProcessRunningInfos(infos);
    EXPECT_EQ(OHOS::ERR_OK, ret);
}
}  // namespace AAFwk
}  // namespace OHOS