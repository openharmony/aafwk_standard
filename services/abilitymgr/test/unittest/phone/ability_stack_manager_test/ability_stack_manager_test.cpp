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
#include "ability_stack_manager.h"
#include "ability_record.h"
#include "ability_manager_service.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "mock_ability_connect_callback.h"
#include "ability_scheduler_mock.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const std::string LANGUAGE = "locale";
const std::string LAYOUT = "layout";
const std::string FONTSIZE = "fontsize";
const std::string ORIENTATION = "orientation";
const std::string DENSITY = "density";
}  // namespace

class AbilityStackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void init();
    void OnStartabilityMs();
    AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName, const std::vector<std::string> config);

    void makeScene(const std::string &abilityName, const std::string &bundleName, AbilityInfo &abilityInfo, Want &want);

    std::shared_ptr<AbilityStackManager> stackManager_ {nullptr};
    AbilityRequest launcherAbilityRequest_ {};
    AbilityRequest musicAbilityRequest_ {};
    AbilityRequest musicTopAbilityRequest_ {};
    AbilityRequest musicSAbilityRequest_ {};
    AbilityRequest radioAbilityRequest_ {};
    AbilityRequest radioTonAbilityRequest_ {};
    Want want_ {};
    AbilityInfo abilityInfo_ {};
    ApplicationInfo appInfo_ {};
public:
    std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
};

void AbilityStackManagerTest::OnStartabilityMs()
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
        EXPECT_TRUE(abilityMs_->handler_);
        EXPECT_TRUE(abilityMs_->connectManager_);

        abilityMs_->connectManager_->SetEventHandler(abilityMs_->handler_);

        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(abilityMs_->dataAbilityManager_);

        abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs_->amsConfigResolver_);
        abilityMs_->amsConfigResolver_->Parse();

        abilityMs_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs_->pendingWantManager_);

        int userId = abilityMs_->GetUserId();
        abilityMs_->SetStackManager(userId, true);

        abilityMs_->eventLoop_->Run();

        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityStackManagerTest::SetUpTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}
void AbilityStackManagerTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityStackManagerTest::SetUp()
{
    init();
    abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartabilityMs();
    stackManager_ = std::make_shared<AbilityStackManager>(0);
    auto bms = abilityMs_->GetBundleManager();
    EXPECT_NE(bms, nullptr);
}

void AbilityStackManagerTest::TearDown()
{
    stackManager_.reset();
    abilityMs_->OnStop();
}

void AbilityStackManagerTest::init()
{
    std::vector<std::string> config;
    launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld", config);

    musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic", config);

    musicTopAbilityRequest_ = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic", config);

    musicSAbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic", config);

    radioAbilityRequest_ = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio", config);

    radioTonAbilityRequest_ = GenerateAbilityRequest("device", "RadioTonAbility", "radio", "com.ix.hiRadio", config);
}

void AbilityStackManagerTest::makeScene(
    const std::string &abilityName, const std::string &bundleName, AbilityInfo &abilityInfo, Want &want)
{
    if (bundleName == "com.ix.hiworld") {
        std::string entity = Want::ENTITY_HOME;
        want.AddEntity(entity);
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.applicationInfo.isLauncherApp = true;
        abilityInfo.process = "p";
    }

    if (bundleName == "com.ix.hiMusic") {
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.applicationInfo.isLauncherApp = false;

        if (abilityName == "MusicAbility") {
            abilityInfo.process = "p1";
            abilityInfo.launchMode = LaunchMode::STANDARD;
        }
        if (abilityName == "MusicTopAbility") {
            abilityInfo.process = "p1";
            abilityInfo.launchMode = LaunchMode::SINGLETOP;
        }
        if (abilityName == "MusicSAbility") {
            abilityInfo.process = "p2";
            abilityInfo.launchMode = LaunchMode::SINGLETON;
        }
    }

    if (bundleName == "com.ix.hiRadio") {
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.process = "p3";
        if (abilityName == "RadioAbility") {
            abilityInfo.launchMode = LaunchMode::STANDARD;
        }
        if (abilityName == "RadioTonAbility") {
            abilityInfo.launchMode = LaunchMode::SINGLETON;
        }
    }
}

AbilityRequest AbilityStackManagerTest::GenerateAbilityRequest(const std::string &deviceName,
    const std::string &abilityName, const std::string &appName, const std::string &bundleName,
    const std::vector<std::string> config)
{
    ElementName element(deviceName, abilityName, bundleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    ApplicationInfo appinfo;
    abilityInfo.visible = true;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.applicationName = appName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationInfo.name = appName;
    abilityInfo.configChanges = config;

    makeScene(abilityName, bundleName, abilityInfo, want);

    appinfo = abilityInfo.applicationInfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify get ability by token success
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_001, TestSize.Level1)
{
    stackManager_->Init();
    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto token = topAbility->GetToken();
    auto tokenAbility = stackManager_->GetAbilityRecordByToken(token);
    EXPECT_EQ(topAbility, tokenAbility);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify get ability by token fail
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_002, TestSize.Level1)
{
    Want want;
    AbilityInfo abilityInfo;
    ApplicationInfo appinfo;

    stackManager_->Init();
    stackManager_->StartAbility(launcherAbilityRequest_);

    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, appinfo);
    ability->Init();
    auto token = ability->GetToken();
    auto tokenAbility = stackManager_->GetAbilityRecordByToken(token);
    EXPECT_EQ(nullptr, tokenAbility);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify start launcher ability
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_003, TestSize.Level1)
{
    stackManager_->Init();
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ(nullptr, topAbility);

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(result, ERR_OK);
    topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(nullptr, stackManager_->GetCurrentTopAbility());
    AbilityInfo topAbilityInfo = topAbility->GetAbilityInfo();
    EXPECT_EQ("launcher", topAbilityInfo.applicationName);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify repeated start launcher ability
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_004, TestSize.Level1)
{
    stackManager_->Init();
    stackManager_->StartAbility(musicSAbilityRequest_);
    auto topAbility1 = stackManager_->GetCurrentTopAbility();
    topAbility1->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    stackManager_->StartAbility(musicSAbilityRequest_);
    auto topAbility2 = stackManager_->GetCurrentTopAbility();
    topAbility2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(topAbility1, topAbility2);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify no launcher ability GetTopMissionRecord
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_005, TestSize.Level1)
{
    stackManager_->Init();
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_EQ(nullptr, topMissionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify no launcher ability GetCurrentTopAbility
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_006, TestSize.Level1)
{
    stackManager_->Init();
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ(nullptr, topAbility);
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify repeated start launcher ability, GetTopMissionRecord
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_007, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord1 = stackManager_->GetTopMissionRecord();
    auto topAbility1 = stackManager_->GetCurrentTopAbility();
    topAbility1->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_NE(nullptr, topMissionRecord1);

    stackManager_->StartAbility(musicAbilityRequest_);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbility2 = stackManager_->GetCurrentTopAbility();
    topAbility2->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_NE(nullptr, topMissionRecord2);
    EXPECT_NE(topMissionRecord1->GetMissionRecordId(), topMissionRecord2->GetMissionRecordId());
}

/*
 * Feature: AbilityStackManager
 * Function: stack operate
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify launcher ability RemoveMissionRecordById
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_008, TestSize.Level1)
{
    stackManager_->Init();
    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topAbility1 = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(nullptr, topAbility1);
    auto topMissionRecord1 = stackManager_->GetTopMissionRecord();
    EXPECT_NE(nullptr, topMissionRecord1);
    auto topId = topMissionRecord1->GetMissionRecordId();
    bool ret = stackManager_->RemoveMissionRecordById(topId);
    EXPECT_EQ(true, ret);
    auto topAbility2 = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ(nullptr, topAbility2);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    EXPECT_EQ(nullptr, topMissionRecord2);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: LoadFirstAbility
 * FunctionPoints: NA
 * EnvConditions: top ability is null
 * CaseDescription: start the first ability. verify:
 *                   1. the MissionStack is launcher mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 1.
 *                   4. the result of StartAbility is ERRO_OK.
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_009, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(0, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord != nullptr);
    EXPECT_EQ(1, missionRecord->GetAbilityRecordCount());
    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto realAppinfo = topAbility->GetApplicationInfo();
    EXPECT_EQ("launcher", realAppinfo.name);
    EXPECT_EQ(0, result);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: LoadFirstAbility
 * FunctionPoints: NA
 * EnvConditions: top ability is null
 * CaseDescription: start the first ability not belong to launcher. verify:
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 1.
 *                   4. the result of StartAbility is ERRO_OK.
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_010, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(0, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(1, missionRecord->GetAbilityRecordCount());
    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto realAppinfo = topAbility->GetApplicationInfo();
    EXPECT_EQ("launcher", realAppinfo.name);
    EXPECT_EQ(0, result);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is launcher
 * CaseDescription: start the second ability belong to launcher. verify:
 *                   1. the MissionStack is launcher mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 2.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the state of top ability is INACTIVATING.
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_011, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(0, result);

    // verify
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(1, missionRecord->GetAbilityRecordCount());
    auto realAbilityinfo = secondTopAbility->GetAbilityInfo();
    EXPECT_EQ("MusicAbility", realAbilityinfo.name);

    auto state = firstTopAbility->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is launcher
 * CaseDescription: start the second ability not belong to launcher. verify:
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 1.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the state of top ability is INACTIVATING.
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_012, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(1, missionRecord->GetAbilityRecordCount());
    auto realAppinfo = secondTopAbility->GetApplicationInfo();
    EXPECT_EQ("music", realAppinfo.name);

    auto state = firstTopAbility->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is launcher
 * CaseDescription: resort the first ability. verify:
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 2.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the name of top ability is "secondAbility".
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_013, TestSize.Level1)
{
    // start first ability not belong to launcher.
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstAbility = stackManager_->GetCurrentTopAbility();
    firstAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start second ability not belong to launcher.
    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(0, result);
    auto secondAbility = stackManager_->GetCurrentTopAbility();
    secondAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start launcher ability.
    result = stackManager_->StartAbility(launcherAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(0, missionStack->GetMissionStackId());
    EXPECT_EQ(0, result);
    OHOS::AAFwk::AbilityState state = secondAbility->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // restart first ability not belong to launcher.
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // verify
    missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(2, missionRecord->GetAbilityRecordCount());
    topAbility = stackManager_->GetCurrentTopAbility();
    auto realAbilityInfo = topAbility->GetAbilityInfo();
    EXPECT_EQ("MusicTopAbility", realAbilityInfo.name);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is not launcher
 * CaseDescription: start a new ability from the common app first ability.verify:
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 2.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the name of top ability is "secondAbility".
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_014, TestSize.Level1)
{
    // start launcher ability
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto launcherTopAbility = stackManager_->GetCurrentTopAbility();
    launcherTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start common app the first ability from launcher
    result = stackManager_->StartAbility(musicAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(0, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto state = launcherTopAbility->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);

    // start the second ability from the common first ability
    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(0, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // verify
    missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(2, missionRecord->GetAbilityRecordCount());
    state = secondTopAbility->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);
    auto realAbilityInfo = thirdTopAbility->GetAbilityInfo();
    EXPECT_EQ("MusicTopAbility", realAbilityInfo.name);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is not launcher
 * CaseDescription: start launcher ability from the common app ability.verify:
 *                   1. the MissionStack is launcher mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 1.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the name of top ability is "MainAbility".
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_015, TestSize.Level1)
{
    // start common app the first ability from launcher
    stackManager_->Init();

    int result = stackManager_->StartAbility(musicAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start the second ability from the common first ability
    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(0, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start launcher ability
    result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto thirdopAbility = stackManager_->GetCurrentTopAbility();
    thirdopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // verify
    missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(0, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(1, missionRecord->GetAbilityRecordCount());
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    auto realAbilityInfo = thirdTopAbility->GetAbilityInfo();
    EXPECT_EQ("LauncherAbility", realAbilityInfo.name);
    auto realAppInfo = thirdTopAbility->GetApplicationInfo();
    EXPECT_EQ("launcher", realAppInfo.name);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is not launcher
 * CaseDescription: start two same ability.verify:
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 2.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the name of top ability is "MainAbility".
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_016, TestSize.Level1)
{
    // start common app the first ability from launcher
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start the same ability from the first ability
    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // verify
    missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(2, missionRecord->GetAbilityRecordCount());
    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto realAbilityInfo = topAbility->GetAbilityInfo();
    EXPECT_EQ("RadioAbility", realAbilityInfo.name);
    auto realAppInfo = topAbility->GetApplicationInfo();
    EXPECT_EQ("radio", realAppInfo.name);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: MoveMissionStackToTop, LoadFirstAbility, LoadAbility ChooseMissionRecord, Inactivate
 * FunctionPoints: NA
 * EnvConditions: top ability is not null, and top is not launcher
 * CaseDescription: start three ability. the bottom and the top are the same.
 *                   1. the MissionStack is common app mission stack
 *                   2. the mission record count is 1.
 *                   3. the ability record count is 3.
 *                   4. the result of StartAbility is ERRO_OK.
 *                   5. the name of top ability and bottom is "MainAbility".
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_017, TestSize.Level1)
{
    // start common app the first ability from launcher
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start the second ability from the first ability
    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(0, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start the same ability with the first ability
    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // verify
    missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(1, missionStack->GetMissionStackId());
    EXPECT_EQ(1, missionStack->GetMissionRecordCount());
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_EQ(3, missionRecord->GetAbilityRecordCount());

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto realAbilityInfo = topAbility->GetAbilityInfo();
    EXPECT_EQ("RadioAbility", realAbilityInfo.name);

    auto realAppInfo = topAbility->GetApplicationInfo();
    EXPECT_EQ("radio", realAppInfo.name);

    auto bottomAbility = missionRecord->GetBottomAbilityRecord();
    EXPECT_EQ("RadioAbility", realAbilityInfo.name);
}

/*
 * Feature: AbilityStackManager
 * Function: GetAbilityStackManagerUserId
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify get user id value
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_018, TestSize.Level1)
{
    EXPECT_EQ(stackManager_->GetAbilityStackManagerUserId(), 0);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager StartAbility
 * EnvConditions: NA
 * CaseDescription: launchMode is STANDARD, Startability and verify value
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_019, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result1);

    auto topAbility1 = stackManager_->GetCurrentTopAbility();
    topAbility1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result2 = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result2);

    auto topAbility2 = stackManager_->GetCurrentTopAbility();
    topAbility2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::vector<std::string> info;
    info.push_back("0");
    stackManager_->Dump(info);
    stackManager_->DumpStackList(info);
    info.push_back("1");
    stackManager_->DumpStack(stackManager_->GetTopMissionRecord()->GetMissionRecordId(), info);
    stackManager_->DumpMission(stackManager_->GetTopMissionRecord()->GetMissionRecordId(), info);
    stackManager_->DumpTopAbility(info);
    std::string s = "0";
    stackManager_->DumpWaittingAbilityQueue(s);

    EXPECT_NE(stackManager_->GetMissionRecordById(stackManager_->GetTopMissionRecord()->GetMissionRecordId()), nullptr);
    EXPECT_EQ(stackManager_->RemoveMissionRecordById(stackManager_->GetTopMissionRecord()->GetMissionRecordId()), true);
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: 1. ability record is nullptr cause TerminateAbility failed
 *                  2. Verify TerminateAbility succeeded
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_020, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto want = topAbility->GetWant();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<AbilityRecord> record = nullptr;
    auto nullToken = new Token(record);
    EXPECT_NE(0, stackManager_->TerminateAbility(nullToken, -1, &want));

    auto token = topAbility->GetToken();
    auto res = stackManager_->TerminateAbility(token, -1, &want);
    EXPECT_EQ(0, res);
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: isTerminating_ is true cause TerminateAbility success
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_021, TestSize.Level1)
{
    stackManager_->Init();
    int result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto want = topAbility->GetWant();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbility->SetTerminatingState();
    auto token = topAbility->GetToken();
    auto res = stackManager_->TerminateAbility(token, -1, &want);
    EXPECT_EQ(0, res);
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Terminate other Ability cause fail
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_022, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto want = topAbility->GetWant();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::string deviceName = "device";
    std::string abilityName = "otherAbility";
    std::string appName = "otherApp";
    std::string bundleName = "com.ix.other";
    std::vector<std::string> config;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, config);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto nullToken = new Token(record);
    EXPECT_NE(0, stackManager_->TerminateAbility(nullToken, -1, &want));
}

/*
 * Feature: AbilityStackManager
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: handler is nullptr cause dispatchActive failed
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_023, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::INITIAL);
    auto token = topAbility->GetToken();
    PacMap saveData;
    EXPECT_NE(stackManager_->AbilityTransitionDone(token, OHOS::AAFwk::ACTIVE, saveData), 0);

    std::shared_ptr<AbilityRecord> record = nullptr;
    auto nullToken = new Token(record);
    EXPECT_NE(stackManager_->AbilityTransitionDone(nullToken, OHOS::AAFwk::INACTIVE, saveData), 0);

    auto token1 = topAbility->GetToken();
    EXPECT_NE(stackManager_->AbilityTransitionDone(token1, OHOS::AAFwk::BACKGROUND, saveData), 0);

    auto token2 = topAbility->GetToken();
    EXPECT_NE(stackManager_->AbilityTransitionDone(token2, OHOS::AAFwk::INITIAL, saveData), 0);

    auto token3 = topAbility->GetToken();
    EXPECT_NE(stackManager_->AbilityTransitionDone(token3, OHOS::AAFwk::TERMINATING, saveData), 0);

    auto token4 = topAbility->GetToken();
    EXPECT_NE(stackManager_->AbilityTransitionDone(token4, OHOS::AAFwk::INACTIVE, saveData), 0);
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: start ability and remove ability record, verify terminal ability fail
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_024, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto want = topAbility->GetWant();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::string deviceName = "device";
    std::string abilityName = "otherAbility";
    std::string appName = "otherApp";
    std::string bundleName = "com.ix.other";
    std::vector<std::string> config;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, config);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = topAbility->GetToken();
    stackManager_->GetTopMissionRecord()->RemoveTopAbilityRecord();
    auto res = stackManager_->TerminateAbility(token, -1, &want);
    EXPECT_NE(0, res);
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: MissionRecord is nullptr cause TerminateAbility fail
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_025, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    auto want = topAbility->GetWant();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto token = topAbility->GetToken();
    std::shared_ptr<MissionRecord> mission = nullptr;
    topAbility->SetMissionRecord(mission);
    auto res = stackManager_->TerminateAbility(token, -1, &want);
    EXPECT_NE(0, res);

    topAbility->isLauncherAbility_ = true;
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: isLauncherAbility_ is true cause TerminateAbility failed
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_026, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto want = topAbility->GetWant();
    auto token = topAbility->GetToken();
    topAbility->isLauncherAbility_ = true;
    auto res = stackManager_->TerminateAbility(token, -1, &want);
    EXPECT_NE(0, res);
}

/*
 * Feature: AbilityStackManager
 * Function: GetTargetMissionStack
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager GetTargetMissionStack
 * EnvConditions: NA
 * CaseDescription: Verify get target mission stack value
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_027, TestSize.Level1)
{
    stackManager_->Init();
    Want want;
    want.AddEntity(Want::ENTITY_HOME);
    AbilityRequest request;
    request.want = want;
    request.abilityInfo.applicationInfo.isLauncherApp = true;
    EXPECT_EQ(stackManager_->launcherMissionStack_, stackManager_->GetTargetMissionStack(request));
    Want want1;
    AbilityRequest request1;
    want1.AddEntity(Want::ENTITY_VIDEO);
    request.want = want1;
    request.abilityInfo.applicationInfo.isLauncherApp = false;
    EXPECT_EQ(stackManager_->defaultMissionStack_, stackManager_->GetTargetMissionStack(request1));
}

/*
 * Feature: AbilityStackManager
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: handler is nullptr cause AttachAbilityThread fail
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_028, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::INITIAL);
    auto token = topAbility->GetToken();

    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    EXPECT_EQ(stackManager_->AttachAbilityThread(scheduler, token), ERR_OK);

    std::shared_ptr<AbilityRecord> record = nullptr;
    auto nullToken = new Token(record);
    EXPECT_EQ(stackManager_->AttachAbilityThread(scheduler, nullToken), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityStackManager
 * Function: AddWindowInfo
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager AddWindowInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddWindowInfo operation
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_029, TestSize.Level1)
{
    stackManager_->Init();
    int result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::INITIAL);
    auto token = topAbility->GetToken();

    std::shared_ptr<AbilityRecord> nullAbility = nullptr;
    auto nullToken = new Token(nullAbility);
    stackManager_->AddWindowInfo(nullToken, 1);
    EXPECT_EQ(static_cast<int>(stackManager_->windowTokenToAbilityMap_.size()), 0);
    topAbility->AddWindowInfo(1);
    stackManager_->AddWindowInfo(token, 1);
    EXPECT_EQ(static_cast<int>(stackManager_->windowTokenToAbilityMap_.size()), 0);
    topAbility->RemoveWindowInfo();
    stackManager_->AddWindowInfo(token, 1);
    EXPECT_EQ(static_cast<int>(stackManager_->windowTokenToAbilityMap_.size()), 1);
    stackManager_->AddWindowInfo(token, 2);
    EXPECT_EQ(static_cast<int>(stackManager_->windowTokenToAbilityMap_.size()), 1);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionStackToTop
 * SubFunction: NA
 * FunctionPoints: AbilityStackManager MoveMissionStackToTop
 * EnvConditions: NA
 * CaseDescription: MoveMissionStackToTop UT Exception case
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_030, TestSize.Level1)
{
    stackManager_->Init();
    int result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    int result1 = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result1);
    topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<MissionStack> mission = nullptr;
    stackManager_->MoveMissionStackToTop(mission);

    EXPECT_EQ(topAbility->GetAbilityInfo().name, "MusicAbility");
    EXPECT_EQ(stackManager_->currentMissionStack_, stackManager_->defaultMissionStack_);

    stackManager_->MoveMissionStackToTop(stackManager_->launcherMissionStack_);
    topAbility = stackManager_->GetCurrentTopAbility();

    EXPECT_EQ(topAbility->GetAbilityInfo().name, "LauncherAbility");
    EXPECT_EQ(stackManager_->currentMissionStack_, stackManager_->launcherMissionStack_);
}

/*
 * Feature: AbilityStackManager
 * Function: IsLauncherAbility
 * SubFunction: NA
 * FunctionPoints: IsLauncherAbility
 * EnvConditions: NA
 * CaseDescription: Verify that ability is a launcher
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_031, TestSize.Level1)
{
    Want want;
    std::string entity = Want::ENTITY_HOME;
    want.AddEntity(entity);

    std::string testAppName = "ability_stack_manager_test_app";
    AbilityInfo abilityInfo;
    abilityInfo.applicationName = testAppName;
    ApplicationInfo appinfo;
    appinfo.name = testAppName;
    appinfo.isLauncherApp = true;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    stackManager_->Init();
    auto topAbility = stackManager_->GetCurrentTopAbility();
    stackManager_->StartAbility(abilityRequest);
    EXPECT_EQ(stackManager_->IsLauncherAbility(abilityRequest), true);
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecentMissions
 * SubFunction: NA
 * FunctionPoints: GetRecentMissions
 * EnvConditions: NA
 * CaseDescription: Failed to verify getrecentmissions
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_033, TestSize.Level1)
{
    std::vector<AbilityMissionInfo> info;
    auto result = stackManager_->GetRecentMissions(-1, 0, info);
    EXPECT_EQ(ERR_INVALID_VALUE, result);

    result = stackManager_->GetRecentMissions(10, 10, info);
    EXPECT_EQ(ERR_INVALID_VALUE, result);

    stackManager_->defaultMissionStack_ = nullptr;
    result = stackManager_->GetRecentMissions(10, 1, info);
    EXPECT_EQ(ERR_NO_INIT, result);
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecentMissions
 * SubFunction: NA
 * FunctionPoints: GetRecentMissions
 * EnvConditions: NA
 * CaseDescription: Get all recent missions list
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_034, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::vector<AbilityMissionInfo> info;
    result = stackManager_->GetRecentMissions(10, 1, info);
    EXPECT_EQ(ERR_OK, result);

    EXPECT_EQ(static_cast<int>(info.size()), 1);
    EXPECT_EQ(info[0].id, thirdTopAbility->GetMissionRecord()->GetMissionRecordId());
    EXPECT_EQ("RadioAbility", info[0].topAbility.GetAbilityName());
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecentMissions
 * SubFunction: NA
 * FunctionPoints: GetRecentMissions
 * EnvConditions: NA
 * CaseDescription: Gets the list of recently active tasks
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_035, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::INITIAL);

    std::vector<AbilityMissionInfo> info;
    result = stackManager_->GetRecentMissions(10, 2, info);
    EXPECT_EQ(ERR_OK, result);

    EXPECT_EQ(static_cast<int>(info.size()), 1);
    EXPECT_EQ(info[0].id, thirdTopAbility->GetMissionRecord()->GetMissionRecordId());
    EXPECT_EQ("RadioAbility", info[0].topAbility.GetAbilityName());
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: MoveMissionToTop
 * EnvConditions: NA
 * CaseDescription: Failed to verify movemissiontotop
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_036, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->MoveMissionToTop(-1);
    EXPECT_EQ(ERR_INVALID_VALUE, result);

    stackManager_->launcherMissionStack_ = nullptr;
    auto result1 = stackManager_->MoveMissionToTop(10);
    EXPECT_EQ(ERR_NO_INIT, result1);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: MoveMissionToTop
 * EnvConditions: NA
 * CaseDescription: Failed to verify movemissiontotop
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_037, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = secondTopAbility->GetMissionRecord();
    mission->RemoveAbilityRecord(secondTopAbility);

    auto result1 = stackManager_->MoveMissionToTop(mission->GetMissionRecordId());
    EXPECT_EQ(MOVE_MISSION_FAILED, result1);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: MoveMissionToTop
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify movemissiontotop
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_038, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = thirdTopAbility->GetMissionRecord();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->MoveMissionToTop(mission->GetMissionRecordId());
    EXPECT_EQ(ERR_OK, result1);

    topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ("RadioAbility", topAbility->GetAbilityInfo().name);
}

/*
 * Feature: AbilityStackManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: If the current ability is active, if it is dead, it will return to the launcher and the state will
 * be init
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_039, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = thirdTopAbility->GetMissionRecord();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stackManager_->OnAbilityDied(topAbility);
    EXPECT_EQ(OHOS::AAFwk::INITIAL, topAbility->GetAbilityState());
    topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ("MusicSAbility", topAbility->GetAbilityInfo().name);
}

/*
 * Feature: AbilityStackManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: If the current ability is uninstall, if it is dead, delete record, it will return to the launcher
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_040, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = thirdTopAbility->GetMissionRecord();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbility->SetIsUninstallAbility();

    stackManager_->OnAbilityDied(topAbility);
    EXPECT_EQ(topAbility->GetMissionRecord(), nullptr);
    topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_EQ("RadioAbility", topAbility->GetAbilityInfo().name);
}

/*
 * Feature: AbilityStackManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: kill music process, music ability state is init, back to launcher
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_041, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = thirdTopAbility->GetMissionRecord();
}

/*
 * Feature: AbilityStackManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: Uninstall music process, delete music record, back to launcher
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_042, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicTopAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = thirdTopAbility->GetMissionRecord();

    stackManager_->UninstallApp("com.ix.hiMusic");
    // process died
    stackManager_->OnAbilityDied(thirdTopAbility);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbility);
    EXPECT_EQ("MusicAbility", topAbility->GetAbilityInfo().name);
    int size = stackManager_->defaultMissionStack_->GetMissionRecordCount();
    // handle is nullptr, not delete
    EXPECT_EQ(size, 1);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveMissionById
 * SubFunction: NA
 * FunctionPoints: RemoveMissionById
 * EnvConditions: NA
 * CaseDescription: Failed to verify removemissionbyid
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_043, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto launcherMissionId = firstTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto missionId = secondTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->RemoveMissionById(-1);
    EXPECT_EQ(ERR_INVALID_VALUE, result);

    result = stackManager_->RemoveMissionById(10);
    EXPECT_EQ(REMOVE_MISSION_ID_NOT_EXIST, result);

    result = stackManager_->RemoveMissionById(launcherMissionId);
    EXPECT_EQ(REMOVE_MISSION_LAUNCHER_DENIED, result);

    result = stackManager_->RemoveMissionById(missionId);
    EXPECT_EQ(ERR_OK, result);

    stackManager_->defaultMissionStack_ = nullptr;
    result = stackManager_->RemoveMissionById(missionId);
    EXPECT_EQ(ERR_NO_INIT, result);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveMissionById
 * SubFunction: NA
 * FunctionPoints: RemoveMissionById
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify removemissionbyid
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_044, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto missionId = secondTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->RemoveMissionById(missionId);
    EXPECT_EQ(ERR_OK, result);
    auto size = stackManager_->defaultMissionStack_->GetMissionRecordCount();

    EXPECT_EQ(size, 1);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: RemoveStack
 * EnvConditions: NA
 * CaseDescription: Failed to verify RemoveStack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_045, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->RemoveStack(-1);
    EXPECT_EQ(ERR_INVALID_VALUE, result);

    result = stackManager_->RemoveStack(10);
    EXPECT_EQ(REMOVE_STACK_ID_NOT_EXIST, result);

    result = stackManager_->RemoveStack(0);
    EXPECT_EQ(REMOVE_STACK_LAUNCHER_DENIED, result);

    stackManager_->missionStackList_.clear();
    result = stackManager_->RemoveStack(1);
    EXPECT_EQ(MISSION_STACK_LIST_IS_EMPTY, result);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: RemoveStack
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify RemoveStack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_046, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->RemoveStack(1);
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: RemoveStack
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify RemoveStack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_047, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(OHOS::AAFwk::ACTIVE, secondTopAbility->GetAbilityState());

    result = stackManager_->PowerOff();
    EXPECT_EQ(POWER_OFF_WAITING, result);
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: RemoveStack
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify RemoveStack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_048, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto firstMissionId = firstTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto secondMissionId = secondTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);
    auto thridMissionId = thirdTopAbility->GetMissionRecord()->GetMissionRecordId();

    EXPECT_EQ(secondMissionId, thridMissionId);
    EXPECT_NE(firstMissionId, secondMissionId);

    OHOS::sptr<AbilitySchedulerMock> scheduler(new AbilitySchedulerMock());

    thirdTopAbility->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);

    result = stackManager_->PowerOff();
    EXPECT_EQ(ERR_OK, result);

    auto missionInfo1 = stackManager_->GetMissionRecordFromAllStacks(firstMissionId);
    EXPECT_EQ(missionInfo1->GetTopAbilityRecord()->GetAbilityState(), OHOS::AAFwk::MOVING_BACKGROUND);
    auto missionInfo2 = stackManager_->GetMissionRecordFromAllStacks(secondMissionId);
    EXPECT_EQ(missionInfo1->GetTopAbilityRecord()->GetAbilityState(), OHOS::AAFwk::MOVING_BACKGROUND);

    auto recordVector = stackManager_->powerStorage_->GetPowerOffInActiveRecord();
    int size = recordVector.size();
    EXPECT_EQ(size, 1);  // Because we only focus on the top of the stack

    for (const auto &it : recordVector) {
        if (it.missionId == thridMissionId) {
            EXPECT_EQ(firstTopAbility->GetRecordId(), it.ability.lock()->GetRecordId());
        }
        if (it.missionId == secondMissionId) {
            EXPECT_EQ(secondTopAbility->GetRecordId(), it.ability.lock()->GetRecordId());
        }
    }

    result = stackManager_->PowerOn();
    EXPECT_EQ(ERR_OK, result);
    EXPECT_EQ(OHOS::AAFwk::INACTIVE, secondTopAbility->GetAbilityState());
}

/*
 * Feature: AbilityStackManager
 * Function: RemoveStack
 * SubFunction: NA
 * FunctionPoints: RemoveStack
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify RemoveStack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_049, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto firstMissionId = firstTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto secondMissionId = secondTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto thridMissionId = thirdTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(ERR_OK, result);
    auto fourthTopAbility = stackManager_->GetCurrentTopAbility();
    fourthTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);

    auto fourthMissionId = fourthTopAbility->GetMissionRecord()->GetMissionRecordId();

    EXPECT_EQ(secondMissionId, thridMissionId);
    EXPECT_NE(fourthMissionId, secondMissionId);

    OHOS::sptr<AbilitySchedulerMock> scheduler(new AbilitySchedulerMock());
    fourthTopAbility->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);

    result = stackManager_->PowerOff();
    EXPECT_EQ(ERR_OK, result);

    auto missionInfo1 = stackManager_->GetMissionRecordFromAllStacks(firstMissionId);
    EXPECT_EQ(missionInfo1->GetTopAbilityRecord()->GetAbilityState(), OHOS::AAFwk::MOVING_BACKGROUND);
    auto missionInfo2 = stackManager_->GetMissionRecordFromAllStacks(secondMissionId);
    EXPECT_EQ(missionInfo1->GetTopAbilityRecord()->GetAbilityState(), OHOS::AAFwk::MOVING_BACKGROUND);
    auto missionInfo3 = stackManager_->GetMissionRecordFromAllStacks(fourthMissionId);
    EXPECT_EQ(missionInfo1->GetTopAbilityRecord()->GetAbilityState(), OHOS::AAFwk::MOVING_BACKGROUND);

    auto recordVector = stackManager_->powerStorage_->GetPowerOffInActiveRecord();
    int size = recordVector.size();
    EXPECT_EQ(size, 2);

    for (const auto &it : recordVector) {
        if (it.missionId == firstMissionId) {
            EXPECT_TRUE(firstTopAbility->GetRecordId() == it.ability.lock()->GetRecordId());
        }
        if (it.missionId == thridMissionId) {
            EXPECT_TRUE(thirdTopAbility->GetRecordId() == it.ability.lock()->GetRecordId());
        }
    }
    result = stackManager_->PowerOn();
    EXPECT_EQ(ERR_OK, result);
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, fourthTopAbility->GetAbilityState());
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_050, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartLockMission(1000, 3, true, true);
    EXPECT_EQ(LOCK_MISSION_DENY_FAILED, result);

    auto result1 = stackManager_->StartLockMission(1000, -2, true, true);
    EXPECT_EQ(LOCK_MISSION_DENY_FAILED, result1);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_051, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();

    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    // can't repeat the lock
    auto resul2 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(LOCK_MISSION_DENY_FAILED, resul2);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_LOCKED);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_052, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto luncherMissionRecord = stackManager_->GetTopMissionRecord();
    auto luncherMisionRecordId = luncherMissionRecord->GetMissionRecordId();

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto musicMisionRecord = stackManager_->GetTopMissionRecord();

    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, luncherMisionRecordId, true, true);
    EXPECT_EQ(ERR_OK, result1);

    auto missionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_EQ(missionRecord->GetMissionRecordId(), luncherMisionRecordId);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_PINNED);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_053, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto musicMisionRecord = stackManager_->GetTopMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();

    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // 2
    std::vector<std::string> config;
    auto musicAbilityRequest2th =
        GenerateAbilityRequest("device", "MusicAbility2th", "music", "com.ix.hiMusic", config);
    result = stackManager_->StartAbility(musicAbilityRequest2th);
    EXPECT_EQ(0, result);
    auto musicMisionRecord2th = stackManager_->GetTopMissionRecord();
    auto musicMisionRecordId2th = musicMisionRecord->GetMissionRecordId();
    EXPECT_EQ(musicMisionRecordId, musicMisionRecordId2th);

    topAbilityRecord = stackManager_->GetCurrentTopAbility();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, musicMisionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    auto topRecord = stackManager_->GetCurrentTopAbility();
    auto abilityInfo = topRecord->GetAbilityInfo();
    EXPECT_EQ(abilityInfo.name, "MusicAbility2th");

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_LOCKED);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_054, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();
    EXPECT_NE(0, misionRecordId);
    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, false);
    EXPECT_EQ(LOCK_MISSION_DENY_FAILED, result1);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a mission and unlock
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_055, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();
    EXPECT_NE(0, misionRecordId);
    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_LOCKED);

    // unlock
    auto result2 = stackManager_->StartLockMission(1000, misionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result2);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a luncher mission and unlock
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_056, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();
    EXPECT_NE(0, misionRecordId);
    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, true, true);
    EXPECT_EQ(ERR_OK, result1);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_PINNED);

    // unlock
    auto result2 = stackManager_->StartLockMission(1000, misionRecordId, true, false);
    EXPECT_EQ(ERR_OK, result2);
}

/*
 * Feature: AbilityStackManager
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: StartLockMission
 * EnvConditions: NA
 * CaseDescription: lock a luncher mission and unlock
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_057, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();
    EXPECT_NE(0, misionRecordId);
    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ(lockState, LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_LOCKED);

    // unlock
    auto result2 = stackManager_->StartLockMission(1000, misionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result2);
}

/*
 * Feature: AbilityStackManager
 * Function: SetMissionDescriptionInfo
 * SubFunction: NA
 * FunctionPoints: SetMissionDescriptionInfo
 * EnvConditions: NA
 * CaseDescription: set details for a mission
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_058, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    Want want;
    AbilityInfo abilityInfo;
    ApplicationInfo appInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, appInfo);

    MissionDescriptionInfo missionDescriptionInfo;
    missionDescriptionInfo.label = "123456";
    missionDescriptionInfo.iconPath = "loacl";

    result = stackManager_->SetMissionDescriptionInfo(abilityRecord, missionDescriptionInfo);
    EXPECT_EQ(SET_MISSION_INFO_FAILED, result);

    result = stackManager_->SetMissionDescriptionInfo(topAbilityRecord, missionDescriptionInfo);
    EXPECT_EQ(ERR_OK, result);

    auto descriptionInfo = topMissionRecord->GetMissionDescriptionInfo();
    EXPECT_TRUE(descriptionInfo);

    EXPECT_TRUE(descriptionInfo->label == missionDescriptionInfo.label);
    EXPECT_TRUE(descriptionInfo->iconPath == missionDescriptionInfo.iconPath);
}

/*
 * Feature: AbilityStackManager
 * Function: SetMissionDescriptionInfo
 * SubFunction: NA
 * FunctionPoints: SetMissionDescriptionInfo
 * EnvConditions: NA
 * CaseDescription: can stop or can start mission state
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_059, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto isCanStart = stackManager_->CanStartInLockMissionState(launcherAbilityRequest_, topAbilityRecord);
    EXPECT_TRUE(isCanStart);

    // LOCK
    auto misionRecordId = topMissionRecord->GetMissionRecordId();
    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    // check can terminal
    auto isCanStop = stackManager_->CanStopInLockMissionState(topAbilityRecord);
    EXPECT_FALSE(isCanStop);

    // check start agin
    isCanStart = stackManager_->CanStartInLockMissionState(launcherAbilityRequest_, topAbilityRecord);
    EXPECT_FALSE(isCanStart);

    // unlock
    result1 = stackManager_->StartLockMission(1000, misionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result1);

    isCanStop = stackManager_->CanStopInLockMissionState(topAbilityRecord);
    EXPECT_TRUE(isCanStop);
}

/*
 * Feature: AbilityStackManager
 * Function: SetMissionDescriptionInfo
 * SubFunction: NA
 * FunctionPoints: SetMissionDescriptionInfo
 * EnvConditions: NA
 * CaseDescription: when the uid defferent
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_060, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto isCanStart = stackManager_->CanStartInLockMissionState(launcherAbilityRequest_, topAbilityRecord);
    EXPECT_TRUE(isCanStart);

    // LOCK
    auto misionRecordId = topMissionRecord->GetMissionRecordId();
    auto result1 = stackManager_->StartLockMission(99, misionRecordId, false, true);
    EXPECT_EQ(LOCK_MISSION_DENY_FAILED, result1);

    result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    // unlock defferent uid(99)
    result1 = stackManager_->StartLockMission(99, misionRecordId, false, false);
    EXPECT_EQ(UNLOCK_MISSION_DENY_FAILED, result1);

    result1 = stackManager_->StartLockMission(1000, misionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result1);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Start the floating window according to the parameters
 * EnvConditions: NA
 * CaseDescription: Start a floating window application
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_061, TestSize.Level1)
{
    stackManager_->Init();
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // defult mission id
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    // promary mission id
    abilityStartSetting->AddProperty(
        AbilityStartSetting::WINDOW_MODE_KEY, std::to_string(
            AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Start the floating window according to the parameters
 * EnvConditions: NA
 * CaseDescription:  a lunchr + Start a floating window application
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_062, TestSize.Level1)
{
    stackManager_->Init();

    // start a luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Start the floating window according to the parameters
 * EnvConditions: NA
 * CaseDescription: a defult ability + Start a floating window application
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_063, TestSize.Level1)
{
    stackManager_->Init();

    // start a ability
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Start the floating window according to the parameters
 * EnvConditions: NA
 * CaseDescription: When an application starts a floating window, it is put on the floating window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_064, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto redioMissionRecord = stackManager_->GetTopMissionRecord();
    auto redioAbilityRecord = redioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(redioMissionRecord);
    EXPECT_TRUE(redioAbilityRecord);
    redioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // radioAbility move to floating stack
    EXPECT_EQ(redioMissionRecord->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Start the floating window according to the parameters
 * EnvConditions: NA
 * CaseDescription: When an singlton application starts a floating window, it is ont put on the together
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_065, TestSize.Level1)
{
    stackManager_->Init();

    // singlton ability
    auto result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto redioMissionRecord = stackManager_->GetTopMissionRecord();
    auto redioAbilityRecord = redioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(redioMissionRecord);
    EXPECT_TRUE(redioAbilityRecord);
    redioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // radioAbility in the defult stack
    EXPECT_EQ(redioMissionRecord->GetMissionStack()->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToFloatingStack
 * SubFunction: NA
 * FunctionPoints: Move misson to floating window stack
 * EnvConditions: NA
 * CaseDescription: a ability move to floating window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_066, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto redioMissionRecord = stackManager_->GetTopMissionRecord();
    auto redioAbilityRecord = redioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(redioMissionRecord);
    EXPECT_TRUE(redioAbilityRecord);
    redioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = -10;

    auto ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.missionId = 13;
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToFloatingStack
 * SubFunction: NA
 * FunctionPoints: Move misson to floating window stack
 * EnvConditions: NA
 * CaseDescription: a ability move to floating window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_067, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;

    auto ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_NONE);

    auto stack = musicMissionRecord->GetMissionStack();
    EXPECT_EQ(stack->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToFloatingStack
 * SubFunction: NA
 * FunctionPoints: Move misson to floating window stack
 * EnvConditions: NA
 * CaseDescription: a luncher ability move to floating window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_068, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto launcherMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbilityRecord = launcherMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(launcherMissionRecord);
    EXPECT_TRUE(launcherAbilityRecord);
    launcherAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = launcherMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;

    auto ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN);

    auto stack = launcherMissionRecord->GetMissionStack();
    EXPECT_EQ(stack->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: ChangeFocusAbility
 * SubFunction: NA
 * FunctionPoints: Change Focus Ability
 * EnvConditions: NA
 * CaseDescription: Move focus from one application to another
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_069, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stactLuncher = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stactLuncher);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = topMissionRecord2->GetMissionStack();
    EXPECT_TRUE(stact);

    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(stactLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);

    // luncher get focus
    auto ref = stackManager_->ChangeFocusAbility(topAbilityRecord2->GetToken(), topAbilityRecord->GetToken());
    EXPECT_EQ(0, ref);
    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: ChangeFocusAbility
 * SubFunction: NA
 * FunctionPoints: Change Focus Ability
 * EnvConditions: NA
 * CaseDescription: Move focus from one application to another
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_070, TestSize.Level1)
{
    stackManager_->Init();
    auto ref = stackManager_->ChangeFocusAbility(nullptr, nullptr);
    EXPECT_EQ(ref, ERR_INVALID_VALUE);

    auto abilityrecord = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    auto abilityrecord1 = AbilityRecord::CreateAbilityRecord(radioAbilityRequest_);
    EXPECT_TRUE(abilityrecord);
    EXPECT_TRUE(abilityrecord1);
    abilityrecord->Init();
    abilityrecord1->Init();

    ref = stackManager_->ChangeFocusAbility(abilityrecord->GetToken(), abilityrecord1->GetToken());
    EXPECT_EQ(ref, CHANGE_FOCUS_ABILITY_FAILED);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToSplitScreenStack
 * SubFunction: NA
 * FunctionPoints: Move misson to Split window stack
 * EnvConditions: NA
 * CaseDescription: a ability move to Split window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_071, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto redioMissionRecord = stackManager_->GetTopMissionRecord();
    auto redioAbilityRecord = redioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(redioMissionRecord);
    EXPECT_TRUE(redioAbilityRecord);
    redioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = -10;

    MissionOption missionOptionOther;
    missionOptionOther.missionId = -11;

    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOptionOther);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.missionId = 13;
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;

    missionOptionOther.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOptionOther);
    EXPECT_EQ(ref, ERR_INVALID_DATA);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    missionOptionOther.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOptionOther);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOptionOther);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);

    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOptionOther);
    EXPECT_EQ(ref, ERR_INVALID_DATA);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToSplitScreenStack
 * SubFunction: NA
 * FunctionPoints: Move misson to Split window stack
 * EnvConditions: NA
 * CaseDescription: a ability move to Split window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_072, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption radioMissionOption;
    radioMissionOption.missionId = radioMissionRecord->GetMissionRecordId();
    radioMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, radioMissionOption);
    EXPECT_EQ(ref, ERR_NONE);

    auto stack = musicMissionRecord->GetMissionStack();
    EXPECT_EQ(stack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToSplitScreenStack
 * SubFunction: NA
 * FunctionPoints: Move misson to Split window stack
 * EnvConditions: NA
 * CaseDescription: a luncher ability move to Split window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_073, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto launcherMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbilityRecord = launcherMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(launcherMissionRecord);
    EXPECT_TRUE(launcherAbilityRecord);
    launcherAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption launcherMissionOption;
    launcherMissionOption.missionId = launcherMissionRecord->GetMissionRecordId();
    launcherMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, launcherMissionOption);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN);

    auto stack = launcherMissionRecord->GetMissionStack();
    EXPECT_EQ(stack->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToSplitScreenStack
 * SubFunction: NA
 * FunctionPoints: Move misson to Split window stack
 * EnvConditions: NA
 * CaseDescription: a luncher ability move to Split window stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_074, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto launcherMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbilityRecord = launcherMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(launcherMissionRecord);
    EXPECT_TRUE(launcherAbilityRecord);
    launcherAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = launcherMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    MissionOption primary;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(primary, missionOption);
    EXPECT_NE(ref, ERR_OK);

    auto stack = launcherMissionRecord->GetMissionStack();
    EXPECT_EQ(stack->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MinimizeMultiWindow
 * SubFunction: NA
 * FunctionPoints: Move the floating window to the background
 * EnvConditions: NA
 * CaseDescription: Move the floating window to the background
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_075, TestSize.Level1)
{
    stackManager_->Init();

    auto ref = stackManager_->MinimizeMultiWindow(-1);
    EXPECT_EQ(ref, MINIMIZE_MULTI_WINDOW_FAILED);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    ref = stackManager_->MinimizeMultiWindow(musicMissionRecord->GetMissionRecordId());
    EXPECT_EQ(ref, MINIMIZE_MULTI_WINDOW_FAILED);
}

/*
 * Feature: AbilityStackManager
 * Function: MinimizeMultiWindow
 * SubFunction: NA
 * FunctionPoints: Move the floating window to the background
 * EnvConditions: NA
 * CaseDescription: Move the floating window to the background
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_076, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::INACTIVE);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = topMissionRecord2->GetMissionStack();
    EXPECT_TRUE(stact);

    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto topStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(topStack);
    EXPECT_EQ(topStack->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    // luncher move to top
    auto ref = stackManager_->MinimizeMultiWindow(topMissionRecord2->GetMissionRecordId());
    EXPECT_EQ(0, ref);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MinimizeMultiWindow
 * SubFunction: NA
 * FunctionPoints: Move the floating window to the background
 * EnvConditions: NA
 * CaseDescription: Move the floating window to the background
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_077, TestSize.Level1)
{
    stackManager_->Init();

    // start luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start top ability
    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(radioMissionRecord);
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start float ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = topMissionRecord2->GetMissionStack();
    EXPECT_TRUE(stact);

    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto topStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(topStack);
    EXPECT_EQ(topStack->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    // ton ability move to top
    auto ref = stackManager_->MinimizeMultiWindow(topMissionRecord2->GetMissionRecordId());
    EXPECT_EQ(0, ref);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    // current top
    EXPECT_EQ(stact->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MaximizeMultiWindow
 * SubFunction: NA
 * FunctionPoints: Move the floating window to the default stack
 * EnvConditions: NA
 * CaseDescription: move the ability form float stack to default stack
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_078, TestSize.Level1)
{
    stackManager_->Init();

    auto ref = stackManager_->MaximizeMultiWindow(-1);
    EXPECT_EQ(ref, MAXIMIZE_MULTIWINDOW_NOT_EXIST);

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    ref = stackManager_->MaximizeMultiWindow(musicMissionRecord->GetMissionRecordId());
    EXPECT_EQ(ref, MAXIMIZE_MULTIWINDOW_NOT_EXIST);
}

/*
 * Feature: AbilityStackManager
 * Function: MaximizeMultiWindow
 * SubFunction: NA
 * FunctionPoints: Move the floating window to the background
 * EnvConditions: NA
 * CaseDescription: Move the floating window to the background
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_079, TestSize.Level1)
{
    stackManager_->Init();

    // start luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start top ability
    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(radioMissionRecord);
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start float ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = topMissionRecord2->GetMissionStack();
    EXPECT_TRUE(stact);
    auto stack1 = radioMissionRecord->GetMissionStack();
    EXPECT_TRUE(stact);

    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(stack1->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    stackManager_->isMultiWinMoving_ = false;
    auto topStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(topStack);
    EXPECT_EQ(topStack->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto ref = stackManager_->MaximizeMultiWindow(topMissionRecord2->GetMissionRecordId());
    EXPECT_EQ(0, ref);

    stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    // current top
    EXPECT_EQ(stact->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    auto topAbility = stackManager_->GetTopMissionRecord()->GetTopAbilityRecord();
    EXPECT_TRUE(topAbility);
}

/*
 * Feature: AbilityStackManager
 * Function: CloseMultiWindow
 * SubFunction: NA
 * FunctionPoints: Close multiple windows
 * EnvConditions: NA
 * CaseDescription: Can I exit the front desk by closing the floating window
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_080, TestSize.Level1)
{
    stackManager_->Init();

    auto ref = stackManager_->CloseMultiWindow(-1);
    EXPECT_EQ(ref, CLOSE_MULTI_WINDOW_FAILED);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(musicMissionRecord);
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    ref = stackManager_->CloseMultiWindow(musicMissionRecord->GetMissionRecordId());
    EXPECT_EQ(ref, CLOSE_MULTI_WINDOW_FAILED);

    result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    ref = stackManager_->CloseMultiWindow(musicMissionRecord->GetMissionRecordId());
    EXPECT_EQ(ref, CLOSE_MULTI_WINDOW_FAILED);
}

/*
 * Feature: AbilityStackManager
 * Function: CloseMultiWindow
 * SubFunction: NA
 * FunctionPoints: Close multiple windows
 * EnvConditions: NA
 * CaseDescription: Can I exit the front desk by closing the floating window
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_081, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(topMissionRecord);
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start top ability
    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start float ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = topMissionRecord2->GetMissionStack();
    EXPECT_TRUE(stact);
    auto stack1 = radioMissionRecord->GetMissionStack();
    EXPECT_TRUE(stact);

    EXPECT_EQ(stact->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(stack1->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto ref = stackManager_->CloseMultiWindow(topMissionRecord2->GetMissionRecordId());
    EXPECT_EQ(0, ref);

    auto topRecord = stackManager_->GetCurrentTopAbility();
    int state = topRecord->GetAbilityState();
    EXPECT_EQ(OHOS::AAFwk::INACTIVATING, state);
}

/*
 * Feature: AbilityStackManager
 * Function: RestartAbility
 * SubFunction: NA
 * FunctionPoints: Restart ability
 * EnvConditions: NA
 * CaseDescription: Restart ability
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_082, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);

    auto ability = std::make_shared<AbilityRecord>(want_, abilityInfo_, appInfo_);
    ability->Init();
    ability->SetAbilityState(AbilityState::ACTIVE);
    stackManager_->RestartAbility(ability);
    EXPECT_EQ(AbilityState::INACTIVATING, ability->GetAbilityState());

    ability->SetAbilityState(AbilityState::ACTIVATING);
    stackManager_->RestartAbility(ability);
    EXPECT_EQ(AbilityState::INACTIVATING, ability->GetAbilityState());
}

/*
 * Feature: AbilityStackManager
 * Function: RestartAbility
 * SubFunction: NA
 * FunctionPoints: Restart ability
 * EnvConditions: NA
 * CaseDescription: Restart ability
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_083, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    auto ability = std::make_shared<AbilityRecord>(want_, abilityInfo_, appInfo_);
    ability->Init();
    ability->SetAbilityState(AbilityState::INACTIVE);
    stackManager_->RestartAbility(ability);
    EXPECT_FALSE(ability->IsNewWant());

    ability->SetAbilityState(AbilityState::INACTIVATING);
    stackManager_->RestartAbility(ability);
    EXPECT_FALSE(ability->IsNewWant());
}

/*
 * Feature: AbilityStackManager
 * Function: RestartAbility
 * SubFunction: NA
 * FunctionPoints: Restart ability
 * EnvConditions: NA
 * CaseDescription: Restart ability
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_084, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    auto ability = std::make_shared<AbilityRecord>(want_, abilityInfo_, appInfo_);
    ability->Init();
    ability->SetAbilityState(AbilityState::BACKGROUND);
    stackManager_->RestartAbility(ability);
    EXPECT_EQ(AbilityState::TERMINATING, ability->GetAbilityState());

    ability->SetAbilityState(AbilityState::MOVING_BACKGROUND);
    stackManager_->RestartAbility(ability);
    EXPECT_EQ(AbilityState::TERMINATING, ability->GetAbilityState());
}

/*
 * Feature: AbilityStackManager
 * Function : GetMissionSnapshot
 * SubFunction : NA
 * FunctionPoints : Get Mission Snapshot
 * EnvConditions: NA
 * CaseDescription : Get Mission Snapshot
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_getMissionSnapshot_001, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    int32_t missionId = -1;
    MissionPixelMap missionPixelMap;
    auto ret = stackManager_->GetMissionSnapshot(missionId, missionPixelMap);

    EXPECT_TRUE(REMOVE_MISSION_ID_NOT_EXIST == ret);
}

/*
 * Feature: AbilityStackManager
 * Function : GetMissionSnapshot
 * SubFunction : NA
 * FunctionPoints : Get Mission Snapshot
 * EnvConditions: NA
 * CaseDescription : Get Mission Snapshot
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_getMissionSnapshot_002, TestSize.Level1)
{
    stackManager_->Init();
    EXPECT_TRUE(stackManager_);
    int32_t missionId = 0;
    MissionPixelMap missionPixelMap;
    stackManager_->missionStackList_.clear();
    auto ret = stackManager_->GetMissionSnapshot(missionId, missionPixelMap);

    EXPECT_TRUE(REMOVE_MISSION_ID_NOT_EXIST == ret);
}

/*
 * Feature: AbilityStackManager
 * Function: GenerateMissinOptionsOfSplitScreen
 * SubFunction: NA
 * FunctionPoints: creat splitscerenn mission option
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_087, TestSize.Level1)
{
    stackManager_->Init();

    MissionOption primary;
    primary.missionId = 0;
    primary.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    MissionOption secondary = primary;
    std::list<MissionOption> options;

    auto ref = stackManager_->GenerateMissinOptionsOfSplitScreen(primary, secondary, options);
    EXPECT_EQ(ERR_INVALID_DATA, ref);
}

/*
 * Feature: AbilityStackManager
 * Function: GenerateMissinOptionsOfSplitScreen
 * SubFunction: NA
 * FunctionPoints: creat splitscerenn mission option
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_088, TestSize.Level1)
{
    stackManager_->Init();

    // start top ability
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto radioMissionId = firstTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption primary;
    primary.missionId = radioMissionId;
    primary.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption secondary;
    std::list<MissionOption> options;

    auto ref = stackManager_->GenerateMissinOptionsOfSplitScreen(primary, secondary, options);
    EXPECT_EQ(ERR_OK, ref);

    int num = options.size();
    int optioNum = 2;
    EXPECT_EQ(optioNum, num);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToSplitScreenStack
 * SubFunction: NA
 * FunctionPoints: Move full screen application to split screen stack
 * EnvConditions: NA
 * CaseDescription: try Can it move when locked
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_089, TestSize.Level1)
{
    stackManager_->Init();

    // start top ability
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto radioMissionId = firstTopAbility->GetMissionRecord()->GetMissionRecordId();

    result = stackManager_->StartLockMission(1000, radioMissionId, false, true);
    EXPECT_EQ(ERR_OK, result);

    MissionOption primary;
    primary.missionId = radioMissionId;
    primary.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption secondary;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(primary, secondary);
    EXPECT_EQ(ERR_INVALID_DATA, ref);
}

/*
 * Feature: AbilityStackManager
 * Function: EmplaceMissionToStack
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_090, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_TRUE(stackManager_->launcherMissionStack_);
    auto luncherTopMission = stackManager_->launcherMissionStack_->GetTopMissionRecord();
    EXPECT_FALSE(luncherTopMission);

    stackManager_->EmplaceMissionToStack(topMissionRecord, stackManager_->launcherMissionStack_);
    luncherTopMission = stackManager_->launcherMissionStack_->GetTopMissionRecord();
    EXPECT_TRUE(luncherTopMission);
}

/*
 * Feature: AbilityStackManager
 * Function: UpdateMissionOption, GetFriendMissionBySplitScreen
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_091, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split screen
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);

    musicSAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stack);
    EXPECT_EQ(stack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);

    // radio move to the split screen
    auto  radioStack = firstTopAbility->GetMissionRecord()->GetMissionStack();
    EXPECT_TRUE(radioStack);
    EXPECT_EQ(radioStack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);

    auto friendMission = stackManager_->GetFriendMissionBySplitScreen(stack, topMissionRecord->GetMissionRecordId());
    EXPECT_TRUE(friendMission);
    EXPECT_TRUE(firstTopAbility->GetMissionRecord() == friendMission);

    EXPECT_TRUE(stackManager_->defaultMissionStack_);
    stackManager_->UpdateMissionOption(friendMission,
        stackManager_->defaultMissionStack_, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN);

    EXPECT_EQ(radioStack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: GetLatestSystemWindowMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_092, TestSize.Level1)
{
    stackManager_->Init();
    auto ref = stackManager_->GetLatestSystemWindowMode();
    EXPECT_EQ(ref, AAFwk::SystemWindowMode::DEFAULT_WINDOW_MODE);
}

/*
 * Feature: AbilityStackManager
 * Function: GetLatestSystemWindowMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_093, TestSize.Level1)
{
    stackManager_->Init();

    // start a ability
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();

    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;
    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);


    auto ref = stackManager_->GetLatestSystemWindowMode();
    EXPECT_EQ(ref, AAFwk::SystemWindowMode::FLOATING_WINDOW_MODE);
}

/*
 * Feature: AbilityStackManager
 * Function: GetLatestSystemWindowMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_094, TestSize.Level1)
{
#ifdef SUPPORT_GRAPHICS
    stackManager_->Init();
    stackManager_->curSysWindowMode_ = SystemWindowMode::SPLITSCREEN_WINDOW_MODE;

    std::string split = "split screen window mode";
    std::vector<std::string> infos;
    stackManager_->DumpWindowMode(infos);

    std::string::size_type index;
    for (auto &mode : infos) {
        index = mode.find(split);
        EXPECT_TRUE(index != std::string::npos);
    }
#endif
}

/*
 * Feature: AbilityStackManager
 * Function: JudgingTargetSystemWindowMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_095, TestSize.Level1)
{
    EXPECT_EQ(SystemWindowMode::SPLITSCREEN_WINDOW_MODE,
        stackManager_->JudgingTargetSystemWindowMode(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_EQ(SystemWindowMode::SPLITSCREEN_WINDOW_MODE,
        stackManager_->JudgingTargetSystemWindowMode(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    EXPECT_EQ(SystemWindowMode::FLOATING_WINDOW_MODE,
        stackManager_->JudgingTargetSystemWindowMode(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
}

/*
 * Feature: AbilityStackManager
 * Function: CheckSplitSrceenCondition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_096, TestSize.Level1)
{
    stackManager_->Init();

    // start a ability
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(topMissionRecord);
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto canMove = stackManager_->CheckSplitSrceenCondition(musicAbilityRequest_, topAbilityRecord);
    EXPECT_FALSE(canMove);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // return false
    canMove = stackManager_->CheckSplitSrceenCondition(musicAbilityRequest_, topAbilityRecord2);
    EXPECT_FALSE(canMove);
}

/*
 * Feature: AbilityStackManager
 * Function: CheckSplitSrceenCondition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_097, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // return true
    auto canMove = stackManager_->CheckSplitSrceenCondition(radioAbilityRequest_, topAbilityRecord);
    EXPECT_TRUE(canMove);

    // start singleton ability
    result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = topMissionRecord2->GetTopAbilityRecord();
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    canMove = stackManager_->CheckSplitSrceenCondition(radioAbilityRequest_, topAbilityRecord2);
    EXPECT_TRUE(canMove);
}

/*
 * Feature: AbilityStackManager
 * Function: HandleAbilityDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_098, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(topMissionRecord);
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // get setting
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    radioAbilityRequest_.startSetting = abilityStartSetting;

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto topStack = stackManager_->GetTopMissionRecord()->GetMissionStack();
    EXPECT_TRUE(topStack);
    EXPECT_EQ(topStack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);

    stackManager_->HandleAbilityDied(radioAbilityRecord);

    topStack = stackManager_->GetTopMissionRecord()->GetMissionStack();
    EXPECT_TRUE(topStack);
    EXPECT_EQ(topStack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MakeTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_099, TestSize.Level1)
{
    auto musicAbilityRecord = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);

    TerminatingAbility unit {nullptr, -1, nullptr};
    stackManager_->MakeTerminatingAbility(unit, musicAbilityRecord, DEFAULT_INVAL_VALUE, nullptr);
    EXPECT_EQ(unit.abilityRecord, musicAbilityRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: ConvertWindowModeState
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0100, TestSize.Level1)
{
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    auto str = stackManager_->ConvertWindowModeState(SystemWindowMode::DEFAULT_WINDOW_MODE);
    EXPECT_EQ(str, "default window mode");
    str = stackManager_->ConvertWindowModeState(SystemWindowMode::SPLITSCREEN_WINDOW_MODE);
    EXPECT_EQ(str, "split screen window mode");
    str = stackManager_->ConvertWindowModeState(SystemWindowMode::FLOATING_WINDOW_MODE);
    EXPECT_EQ(str, "floating window mode");
    str = stackManager_->ConvertWindowModeState(SystemWindowMode::FLOATING_AND_SPLITSCREEN_WINDOW_MODE);
    EXPECT_EQ(str, "floating and split screen window mode");
#endif
}

/*
 * Feature: AbilityStackManager
 * Function: SortPreMission
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0101, TestSize.Level1)
{
    auto mission = std::make_shared<MissionRecord>("test_one");
    auto nextMission = std::make_shared<MissionRecord>("test_two");

    auto preMission = nextMission->GetPreMissionRecord();
    EXPECT_TRUE(nullptr == preMission);

    stackManager_->SortPreMission(mission, nextMission);

    preMission = nextMission->GetPreMissionRecord();
    EXPECT_TRUE(preMission);
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecordBySplitScreenMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0102, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<AbilityRecord> targetAbilityRecord(nullptr);
    std::shared_ptr<MissionRecord> targetMissionRecord(nullptr);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;

    stackManager_->GetRecordBySplitScreenMode(musicAbilityRequest_, targetAbilityRecord, targetMissionRecord);

    EXPECT_TRUE(targetAbilityRecord);
    EXPECT_TRUE(targetMissionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecordBySplitScreenMode
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0103, TestSize.Level1)
{
    stackManager_->Init();

    // start sa ability
    auto result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<AbilityRecord> targetAbilityRecord(nullptr);
    std::shared_ptr<MissionRecord> targetMissionRecord(nullptr);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);

    musicSAbilityRequest_.startSetting = abilityStartSetting;

    stackManager_->GetRecordBySplitScreenMode(musicSAbilityRequest_, targetAbilityRecord, targetMissionRecord);

    EXPECT_TRUE(targetAbilityRecord);
    EXPECT_TRUE(targetMissionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: CheckMultiWindowCondition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0104, TestSize.Level1)
{
#ifdef SUPPORT_GRAPHICS
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::list<MissionOption> missionOptions;
    stackManager_->isMultiWinMoving_ = true;
    auto ref = stackManager_->CheckMultiWindowCondition(missionOptions);
    EXPECT_EQ(MOVE_MISSION_TO_STACK_MOVING_DENIED, ref);

    stackManager_->isMultiWinMoving_ = false;
    ref = stackManager_->CheckMultiWindowCondition(missionOptions);
    EXPECT_EQ(MOVE_MISSION_TO_STACK_OUT_OF_SIZE, ref);

    MissionOption mission;
    mission.missionId = 77;
    missionOptions.push_back(mission);

    ref = stackManager_->CheckMultiWindowCondition(missionOptions);
    EXPECT_EQ(MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION, ref);
#endif
}

/*
 * Feature: AbilityStackManager
 * Function: CheckMultiWindowCondition
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0105, TestSize.Level1)
{
#ifdef SUPPORT_GRAPHICS
    stackManager_->Init();
    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    std::list<MissionOption> missionOptions;
    MissionOption mission;
    mission.missionId = topMissionRecord->GetMissionRecordId();
    mission.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    missionOptions.push_back(mission);
    auto ref = stackManager_->CheckMultiWindowCondition(missionOptions);
    EXPECT_EQ(ERR_OK, ref);
#endif
}

/*
 * Feature: AbilityStackManager
 * Function: EmplaceMissionToStack
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0106, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto targetStack = stackManager_->GetTargetMissionStack(musicAbilityRequest_);
    EXPECT_TRUE(targetStack);
    stackManager_->EmplaceMissionToStack(topMissionRecord, targetStack);

    auto topMission = targetStack->GetTopMissionRecord();
    EXPECT_TRUE(topMission);
}

/*
 * Feature: AbilityStackManager
 * Function: SortRecentMissions
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0107, TestSize.Level1)
{
    stackManager_->Init();

    // Start two singleton applications in the floating window stack
    auto result = stackManager_->StartAbility(musicSAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // Music application at the bottom of the stack
    std::vector<MissionRecordInfo> missionInfos;
    for (auto &stack : stackManager_->missionStackList_) {
        EXPECT_TRUE(stack);
        stack->GetAllMissionInfo(missionInfos);
    }

    auto frontMission = missionInfos.back();
    EXPECT_EQ(frontMission.id, musicMissionRecord->GetMissionRecordId());

    // The value of who updates first is small
    radioMissionRecord->UpdateActiveTimestamp();
    musicMissionRecord->UpdateActiveTimestamp();

    stackManager_->SortRecentMissions(missionInfos);

    // The radio application is at the bottom of the stack
    frontMission = missionInfos.front();
    EXPECT_EQ(frontMission.id, radioMissionRecord->GetMissionRecordId());
}

/*
 * Feature: AbilityStackManager
 * Function: SortAndGetLastActiveAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0108, TestSize.Level1)
{
    auto musicAbilityRecord = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    auto musicSaAbilityRecord = AbilityRecord::CreateAbilityRecord(musicSAbilityRequest_);
    auto radioAbilityRecord = AbilityRecord::CreateAbilityRecord(radioAbilityRequest_);

    EXPECT_TRUE(musicAbilityRecord);
    EXPECT_TRUE(musicSaAbilityRecord);
    EXPECT_TRUE(radioAbilityRecord);

    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    musicSaAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::BACKGROUND);

    TerminatingAbility musicTerminateAbility;
    musicTerminateAbility.abilityRecord = musicAbilityRecord;

    TerminatingAbility musicSaTerminateAbility;
    musicSaTerminateAbility.abilityRecord = musicSaAbilityRecord;

    TerminatingAbility radioSaTerminateAbility;
    radioSaTerminateAbility.abilityRecord = radioAbilityRecord;

    std::list<TerminatingAbility> terminateLists;
    terminateLists.emplace_back(musicTerminateAbility);
    terminateLists.emplace_back(musicSaTerminateAbility);
    terminateLists.emplace_back(radioSaTerminateAbility);

    std::shared_ptr<AbilityRecord> lastActiveAbility(nullptr);

    stackManager_->SortAndGetLastActiveAbility(terminateLists, lastActiveAbility);
    EXPECT_TRUE(lastActiveAbility);
    auto backAbility = terminateLists.back();
    EXPECT_TRUE(lastActiveAbility == backAbility.abilityRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: MakeTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0109, TestSize.Level1)
{
    auto musicAbilityRecord = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);

    TerminatingAbility unit {nullptr, -1, nullptr};
    stackManager_->MakeTerminatingAbility(unit, musicAbilityRecord, DEFAULT_INVAL_VALUE, nullptr);
    EXPECT_EQ(unit.abilityRecord, musicAbilityRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: GetTargetChangeType
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0110, TestSize.Level1)
{
    stackManager_->Init();

    int ACTIVE = 0;
    int DEFAULT = 2;

    bool isMissionChanged {false};
    bool isStackChanged {false};
    bool isCurrentFull {false};
    bool isTargetFull {false};

    std::shared_ptr<AbilityRecord> needBackgroundAbility {nullptr};

    auto ref = stackManager_->GetTargetChangeType(isMissionChanged, isStackChanged, isCurrentFull, isTargetFull,
        nullptr, nullptr, needBackgroundAbility);
    EXPECT_EQ(DEFAULT, ref);

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto targetAbility = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);

    isMissionChanged = true;
    isStackChanged = true;
    ref = stackManager_->GetTargetChangeType(isMissionChanged, isStackChanged, isCurrentFull, isTargetFull,
        nullptr, targetAbility, needBackgroundAbility);
    EXPECT_EQ(ACTIVE, ref);
    EXPECT_FALSE(needBackgroundAbility);

    isMissionChanged = true;
    isStackChanged = true;
    isCurrentFull = true;
    ref = stackManager_->GetTargetChangeType(isMissionChanged, isStackChanged, isCurrentFull, isTargetFull,
        nullptr, targetAbility, needBackgroundAbility);
    EXPECT_EQ(ACTIVE, ref);
    EXPECT_FALSE(needBackgroundAbility);
}


/*
 * Feature: AbilityStackManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0111, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto topMisionRecord = stackManager_->GetTopMissionRecord();
    auto misionRecordId = topMisionRecord->GetMissionRecordId();

    auto topAbilityRecord = topMisionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecord->SetEventId(17);

    auto result1 = stackManager_->StartLockMission(1000, misionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result1);

    auto lockState = stackManager_->GetMissionLockModeState();
    EXPECT_EQ((LockMissionContainer::LockMissionState)lockState,
        LockMissionContainer::LockMissionState::LOCK_MISSION_STATE_LOCKED);

    stackManager_->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, topAbilityRecord->GetEventId());
}

/*
 * Feature: AbilityStackManager
 * Function: TerminateAbilityLocked, RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0112, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split screen
    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption radioMissionOption;
    radioMissionOption.missionId = radioMissionRecord->GetMissionRecordId();
    radioMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, radioMissionOption);
    EXPECT_EQ(ref, ERR_NONE);

    EXPECT_TRUE(musicAbilityRecord);
    EXPECT_TRUE(radioAbilityRecord);

    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    TerminatingAbility musicTerminateAbility;
    musicTerminateAbility.abilityRecord = musicAbilityRecord;

    TerminatingAbility radioSaTerminateAbility;
    radioSaTerminateAbility.abilityRecord = radioAbilityRecord;

    std::list<TerminatingAbility> terminateLists;
    terminateLists.emplace_back(musicTerminateAbility);
    terminateLists.emplace_back(radioSaTerminateAbility);

    stackManager_->TerminateAbilityLocked(terminateLists);

    int terminalNum = 2;
    int size = terminateLists.size();
    EXPECT_EQ(terminalNum, size);
}

/*
 * Feature: AbilityStackManager
 * Function: CompleteMoveMissionToStack
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0113, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(DEFAULT_MISSION_STACK_ID, currentStack->GetMissionStackId());

    auto targetStack = stackManager_->GetOrCreateMissionStack(SPLIT_SCREEN_MISSION_STACK_ID, true);
    EXPECT_TRUE(targetStack);

    stackManager_->CompleteMoveMissionToStack(radioMissionRecord, targetStack);

    currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(SPLIT_SCREEN_MISSION_STACK_ID, currentStack->GetMissionStackId());
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecentMissionsLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0114, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0115, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split screen
    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption radioMissionOption;
    radioMissionOption.missionId = radioMissionRecord->GetMissionRecordId();
    radioMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, radioMissionOption);
    EXPECT_EQ(ref, ERR_NONE);

    stackManager_->isMultiWinMoving_ = false;
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto moveref = stackManager_->MoveMissionToEnd(musicAbilityRecord->GetToken(), true);
    EXPECT_EQ(moveref, ERR_NONE);

    EXPECT_TRUE(musicAbilityRecord->IsToEnd());
    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, currentStack->GetMissionStackId());
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0116, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split screen
    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption radioMissionOption;
    radioMissionOption.missionId = radioMissionRecord->GetMissionRecordId();
    radioMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, radioMissionOption);
    EXPECT_EQ(ref, ERR_NONE);

    stackManager_->isMultiWinMoving_ = false;
    auto moveref = stackManager_->MoveMissionToEnd(radioAbilityRecord->GetToken(), true);
    EXPECT_EQ(moveref, ERR_NONE);

    EXPECT_TRUE(radioAbilityRecord->IsToEnd());
    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, currentStack->GetMissionStackId());
}

/*
 * Feature: AbilityStackManager
 * Function: DispatchLifecycle
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0117, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicTopAbility = stackManager_->GetCurrentTopAbility();
    musicTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    radioAbilityRequest_.startSetting = abilityStartSetting;


    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);

    musicTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    auto ref = stackManager_->DispatchLifecycle(musicTopAbility, radioAbility, false);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AbilityStackManager
 * Function: DispatchLifecycle
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0118, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicTopAbility = stackManager_->GetCurrentTopAbility();
    musicTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    radioTonAbilityRequest_.startSetting = abilityStartSetting;

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbility);

    musicTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    auto ref = stackManager_->DispatchLifecycle(musicTopAbility, radioAbility, false);
    EXPECT_EQ(ERR_OK, ref);

    EXPECT_TRUE(musicTopAbility->GetInMovingState());
}

/*
 * Feature: AbilityStackManager
 * Function: GetRecentMissionsLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0119, TestSize.Level1)
{
    stackManager_->Init();

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto musicMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicAbilityRecord = musicMissionRecord->GetTopAbilityRecord();
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioTonAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split screen
    MissionOption missionOption;
    missionOption.missionId = musicMissionRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    MissionOption radioMissionOption;
    radioMissionOption.missionId = radioMissionRecord->GetMissionRecordId();
    radioMissionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY;
    auto ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, radioMissionOption);
    EXPECT_EQ(ref, ERR_NONE);

    stackManager_->isMultiWinMoving_ = false;
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto moveref = stackManager_->MoveMissionToEnd(musicAbilityRecord->GetToken(), true);
    EXPECT_EQ(moveref, ERR_NONE);

    EXPECT_TRUE(musicAbilityRecord->IsToEnd());
    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, currentStack->GetMissionStackId());

    // get recent list
    std::vector<AbilityMissionInfo> recentList;
    int getMaxNum = 1;
    int flag = RECENT_WITH_EXCLUDED;
    stackManager_->GetRecentMissionsLocked(getMaxNum, flag, recentList);

    int size = recentList.size();
    EXPECT_EQ(size, 1);
}

/*
 * Feature: AbilityStackManager
 * Function: GetMissionRecordAndAbilityRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0120, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<AbilityRecord> targetAbilityRecord(nullptr);
    std::shared_ptr<MissionRecord> targetMissionRecord(nullptr);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;

    stackManager_->GetMissionRecordAndAbilityRecord(
        musicAbilityRequest_, topAbilityRecord, targetAbilityRecord, targetMissionRecord);

    EXPECT_TRUE(targetAbilityRecord);
    EXPECT_TRUE(targetMissionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: GetMissionRecordAndAbilityRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0121, TestSize.Level1)
{
    stackManager_->Init();

    auto result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<AbilityRecord> targetAbilityRecord(nullptr);
    std::shared_ptr<MissionRecord> targetMissionRecord(nullptr);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);

    musicAbilityRequest_.startSetting = abilityStartSetting;

    stackManager_->GetMissionRecordAndAbilityRecord(
        musicAbilityRequest_, topAbilityRecord, targetAbilityRecord, targetMissionRecord);

    EXPECT_TRUE(targetAbilityRecord);
    EXPECT_TRUE(targetMissionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: GetTargetMissionStackBySetting
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0122, TestSize.Level1)
{
    stackManager_->Init();

    auto stack = stackManager_->GetTargetMissionStackBySetting(musicAbilityRequest_);
    EXPECT_TRUE(stackManager_->defaultMissionStack_ == stack);
    EXPECT_EQ(stack->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: GetTargetMissionStackBySetting
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0123, TestSize.Level1)
{
    stackManager_->Init();

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto stack = stackManager_->GetTargetMissionStackBySetting(musicAbilityRequest_);
    EXPECT_TRUE(stack);

    EXPECT_EQ(stack->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: GetTargetMissionStackBySetting
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0124, TestSize.Level1)
{
    stackManager_->Init();

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto stack = stackManager_->GetTargetMissionStackBySetting(musicAbilityRequest_);
    EXPECT_TRUE(stack);

    EXPECT_EQ(stack->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AbilityStackManager
 * Function: MoveMissionAndAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0125, TestSize.Level1)
{
    stackManager_->Init();

    auto currentAbility =  AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    auto targetAbility = AbilityRecord::CreateAbilityRecord(radioAbilityRequest_);
    auto targetMissionRecord = std::make_shared<MissionRecord>("test");

    MissionOption option;
    option.missionId = 2;
    option.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    targetMissionRecord->SetMissionOption(option);

    stackManager_->MoveMissionAndAbility(currentAbility, targetAbility, targetMissionRecord);

    auto ability = targetMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(ability);

    EXPECT_TRUE(ability == targetAbility);
    EXPECT_TRUE(currentAbility == targetAbility->GetPreAbilityRecord());
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbilityAsSpecialLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0126, TestSize.Level1)
{
    stackManager_->Init();

    // start a luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto stact = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(stact);
    EXPECT_EQ(stact->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility, StartAbilityAsSpecialLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0127, TestSize.Level1)
{
    stackManager_->Init();

    // start a luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topMissionRecord);
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
}

/*
 * Feature: AbilityStackManager
 * Function: StartAbility, StartAbilityLifeCycle
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0128, TestSize.Level1)
{
    stackManager_->Init();

    // start a luncher
    auto result = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(0, result);
    auto topMissionRecord = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(topMissionRecord);
    auto topAbilityRecord = topMissionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    result = stackManager_->StartAbility(radioAbilityRequest_);
    EXPECT_EQ(0, result);
    auto radioMissionRecord = stackManager_->GetTopMissionRecord();
    auto radioAbilityRecord = radioMissionRecord->GetTopAbilityRecord();
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<MissionRecord> targetMissionRecord;

    stackManager_->GetMissionRecordAndAbilityRecord(musicAbilityRequest_, radioAbilityRecord, targetAbilityRecord,
        targetMissionRecord);
    EXPECT_TRUE(targetAbilityRecord);
    EXPECT_TRUE(targetMissionRecord);

    auto lifeCycleRef = stackManager_->StartAbilityLifeCycle(nullptr, radioAbilityRecord, targetAbilityRecord);
    EXPECT_EQ(0, lifeCycleRef);
}

/*
 * Feature: AbilityStackManager
 * Function: CompleteActive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0129, TestSize.Level1)
{
    stackManager_->Init();

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto ability = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    EXPECT_TRUE(ability);

    ability->SetMovingBackgroundFlag(true);

    stackManager_->CompleteActive(ability);
    EXPECT_EQ(ability->GetAbilityState(), AbilityState::ACTIVE);
}

/*
 * Feature: AbilityStackManager
 * Function: CompleteInactive
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0130, TestSize.Level1)
{
    stackManager_->Init();

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto ability = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    EXPECT_TRUE(ability);

    ability->SetMovingBackgroundFlag(true);

    stackManager_->CompleteInactive(ability);

    EXPECT_EQ(ability->GetAbilityState(), AbilityState::MOVING_BACKGROUND);
}


/*
 * Feature: AbilityStackManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0131, TestSize.Level1)
{
    stackManager_->Init();

    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    // Set floating window identification
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    EXPECT_TRUE(abilityStartSetting);
    musicAbilityRequest_.startSetting = abilityStartSetting;

    auto ability = AbilityRecord::CreateAbilityRecord(musicAbilityRequest_);
    EXPECT_TRUE(ability);

    ability->SetAbilityState(AbilityState::MOVING_BACKGROUND);

    ability->SetMovingBackgroundFlag(true);

    stackManager_->CompleteBackground(ability);

    EXPECT_EQ(ability->GetAbilityState(), AbilityState::BACKGROUND);
}

/*
 * Feature: AbilityStackManager
 * Function: GetMaxHoldMissionsByStackId
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0132, TestSize.Level1)
{
    StackSetting stackOne;
    stackOne.stackId = (AAFwk::STACK_ID)1;
    stackOne.isSyncVisual = false;
    stackOne.maxHoldMission = 10;

    StackSetting stackTwo;
    stackTwo.stackId = (AAFwk::STACK_ID)2;
    stackTwo.isSyncVisual = true;
    stackTwo.maxHoldMission = 11;

    StackSetting stackThree;
    stackThree.stackId = (AAFwk::STACK_ID)3;
    stackThree.isSyncVisual = false;
    stackThree.maxHoldMission = 12;

    stackManager_->stackSettings_.push_back(stackOne);
    stackManager_->stackSettings_.push_back(stackTwo);
    stackManager_->stackSettings_.push_back(stackThree);

    EXPECT_EQ(12, stackManager_->GetMaxHoldMissionsByStackId(stackThree.stackId));
    EXPECT_EQ(11, stackManager_->GetMaxHoldMissionsByStackId(stackTwo.stackId));
    EXPECT_EQ(10, stackManager_->GetMaxHoldMissionsByStackId(stackOne.stackId));
    EXPECT_EQ(DEFAULT_INVAL_VALUE, stackManager_->GetMaxHoldMissionsByStackId(-1));
}

/*
 * Feature: AbilityStackManager
 * Function: SupportSyncVisualByStackId
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_operating_0133, TestSize.Level1)
{
    StackSetting stackOne;
    stackOne.stackId = (AAFwk::STACK_ID)1;
    stackOne.isSyncVisual = false;

    StackSetting stackTwo;
    stackTwo.stackId = (AAFwk::STACK_ID)2;
    stackTwo.isSyncVisual = true;

    StackSetting stackThree;
    stackThree.stackId = (AAFwk::STACK_ID)4;
    stackThree.isSyncVisual = false;

    stackManager_->stackSettings_.push_back(stackOne);
    stackManager_->stackSettings_.push_back(stackTwo);
    stackManager_->stackSettings_.push_back(stackThree);

    EXPECT_TRUE(stackManager_->SupportSyncVisualByStackId(stackThree.stackId));
    EXPECT_TRUE(stackManager_->SupportSyncVisualByStackId(stackTwo.stackId));
    EXPECT_FALSE(stackManager_->SupportSyncVisualByStackId(stackOne.stackId));
}

/*
 * Feature: AbilityStackManager
 * Function: CheckMissionRecordInWhiteList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_CheckMissionRecordInWhiteList_0001, TestSize.Level1)
{
    std::shared_ptr<MissionRecord> mission = nullptr;
    auto ret = stackManager_->CheckMissionRecordInWhiteList(mission);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: AbilityStackManager
 * Function: CheckMissionRecordInWhiteList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_CheckMissionRecordInWhiteList_0002, TestSize.Level1)
{
    stackManager_->Init();
    auto ret = stackManager_->SetShowOnLockScreenLocked("com.ix.hiMusic", true);
    EXPECT_EQ(ret, ERR_OK);
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(result, ERR_OK);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(missionStack);
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord);
    missionStack->missionStackId_ = 3;
    ret = stackManager_->CheckMissionRecordInWhiteList(missionRecord);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: AbilityStackManager
 * Function: CheckMissionRecordInWhiteList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_CheckMissionRecordInWhiteList_0003, TestSize.Level1)
{
    stackManager_->Init();
    auto ret = stackManager_->SetShowOnLockScreenLocked("com.ix.hiMusic", true);
    EXPECT_EQ(ret, ERR_OK);
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(result, ERR_OK);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(missionStack);
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord);
    missionStack->missionStackId_ = 2;
    ret = stackManager_->CheckMissionRecordInWhiteList(missionRecord);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: AbilityStackManager
 * Function: DeleteMissionRecordInStackOnLockScreen
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_DeleteMissionRecordInStackOnLockScreen_0001, TestSize.Level1)
{
    std::shared_ptr<MissionRecord> mission;
    auto ret = stackManager_->DeleteMissionRecordInStackOnLockScreen(mission);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: AbilityStackManager
 * Function: DeleteMissionRecordInStackOnLockScreen
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_DeleteMissionRecordInStackOnLockScreen_0002, TestSize.Level1)
{
    std::shared_ptr<MissionRecord> mission = nullptr;
    auto ret = stackManager_->DeleteMissionRecordInStackOnLockScreen(mission);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: AbilityStackManager
 * Function: DeleteMissionRecordInStackOnLockScreen
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_DeleteMissionRecordInStackOnLockScreen_0003, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(result, ERR_OK);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(missionStack);
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord);
    missionStack->missionStackId_ = 2;
    auto ret = stackManager_->DeleteMissionRecordInStackOnLockScreen(missionRecord);
    EXPECT_EQ(ret, true);
    missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_FALSE(missionRecord);
}

/*
 * Feature: AbilityStackManager
 * Function: SetShowOnLockScreenLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_SetShowOnLockScreenLocked_0001, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);

    auto ret = stackManager_->SetShowOnLockScreenLocked("com.ix.hiMusic", true);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: AbilityStackManager
 * Function: SetPowerOffRecordWhenLockScreen
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_SetPowerOffRecordWhenLockScreen_0001, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(missionStack);
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord);
    auto topAbilityRecord = missionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    stackManager_->lockScreenMissionStack_ = missionStack;

    auto powerStorage = std::make_shared<PowerStorage>();
    EXPECT_TRUE(powerStorage);
    stackManager_->SetPowerOffRecordWhenLockScreen(powerStorage);
    EXPECT_EQ(topAbilityRecord->GetPowerState(), true);
}

/*
 * Feature: AbilityStackManager
 * Function: GetLockScreenRootAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_GetLockScreenRootAbility_0001, TestSize.Level1)
{
    stackManager_->lockScreenMissionStack_ = nullptr;
    auto ret = stackManager_->GetLockScreenRootAbility();
    EXPECT_EQ(ret, nullptr);
}

/*
 * Feature: AbilityStackManager
 * Function: GetLockScreenRootAbility
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityStackManagerTest, ability_stack_manager_GetLockScreenRootAbility_0002, TestSize.Level1)
{
    stackManager_->Init();
    auto result = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, result);
    auto missionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(missionStack);
    auto missionRecord = missionStack->GetTopMissionRecord();
    EXPECT_TRUE(missionRecord);
    auto topAbilityRecord = missionRecord->GetTopAbilityRecord();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecord->SetLockScreenRoot();
    stackManager_->lockScreenMissionStack_ = missionStack;

    auto ret = stackManager_->GetLockScreenRootAbility();
    EXPECT_TRUE(ret);
}
}  // namespace AAFwk
}  // namespace OHOS
