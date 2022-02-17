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
#include "ability_manager_errors.h"
#include "ability_event_handler.h"
#include "mock_ability_scheduler.h"
#include "mock_app_mgr_client.h"
#include "mock_app_scheduler.h"
#include "ability_manager_service.h"
#include "mock_window_manager.h"
#include "screenshot_handler.h"
#undef private
#undef protected
#include <thread>
#include "common_event.h"
#include "common_event_manager.h"
#include "mock_bundle_mgr.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "os_account_manager.h"
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string LANGUAGE = "locale";
const std::string LAYOUT = "layout";
const std::string FONTSIZE = "fontsize";
const std::string ORIENTATION = "orientation";
const std::string DENSITY = "density";
}  // namespace
class AbilityStackModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void OnStartabilityMs(std::shared_ptr<AbilityManagerService> abilityMs);
    Want CreateWant(const std::string &entity);
    AbilityInfo CreateAbilityInfo(const std::string &name, const std::string &appName, const std::string &bundleName);
    ApplicationInfo CreateAppInfo(const std::string &appName, const std::string &bundleName);
    AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName);
    void makeScene(const std::string &abilityName, const std::string &bundleName, AbilityInfo &abilityInfo, Want &want);

public:
    std::shared_ptr<AbilityStackManager> stackManager_ {nullptr};
    inline static BundleMgrService *bundleObject_ {nullptr};
    OHOS::sptr<MockAbilityScheduler> mockScheduler_ = nullptr;
};

void AbilityStackModuleTest::OnStartabilityMs(std::shared_ptr<AbilityManagerService> abilityMs)
{
    if (abilityMs) {
        if (abilityMs->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }
        abilityMs->state_ = ServiceRunningState::STATE_RUNNING;
        abilityMs->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs->eventLoop_);
        abilityMs->handler_ = std::make_shared<AbilityEventHandler>(abilityMs->eventLoop_, abilityMs);
        abilityMs->connectManager_ = std::make_shared<AbilityConnectManager>();
        abilityMs->connectManagers_.emplace(0, abilityMs->connectManager_);
        EXPECT_TRUE(abilityMs->handler_);
        EXPECT_TRUE(abilityMs->connectManager_);
        abilityMs->connectManager_->SetEventHandler(abilityMs->handler_);
        abilityMs->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        abilityMs->dataAbilityManagers_.emplace(0, abilityMs->dataAbilityManager_);
        EXPECT_TRUE(abilityMs->dataAbilityManager_);

        abilityMs->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs->amsConfigResolver_);
        abilityMs->amsConfigResolver_->Parse();
        abilityMs->kernalAbilityManager_ = std::make_shared<KernalAbilityManager>(0);
        abilityMs->currentMissionListManager_ = std::make_shared<MissionListManager>(0);
        abilityMs->currentMissionListManager_->Init();
        abilityMs->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs->pendingWantManager_);
        int userId = abilityMs->GetUserId();
        abilityMs->SetStackManager(userId, true);
        abilityMs->stackManagers_.emplace(0, abilityMs->GetStackManager());
        abilityMs->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
        EXPECT_TRUE(abilityMs->systemAppManager_);
        abilityMs->eventLoop_->Run();
        return;
    }
    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityStackModuleTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    if (!bundleObject_) {
        bundleObject_ = new (std::nothrow) BundleMgrService();
        if (bundleObject_) {
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
                OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject_);
        }
    }
}

void AbilityStackModuleTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    if (bundleObject_) {
        delete bundleObject_;
        bundleObject_ = nullptr;
    }
}

void AbilityStackModuleTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp start";
    auto ams = DelayedSingleton<AbilityManagerService>::GetInstance();
    auto bms = ams->GetBundleManager();
    OnStartabilityMs(ams);
    stackManager_ = ams->GetStackManager();
    EXPECT_TRUE(stackManager_);
    EXPECT_NE(bms, nullptr);

    if (mockScheduler_ == nullptr) {
        mockScheduler_ = new MockAbilityScheduler();
    }
    GTEST_LOG_(INFO) << "SetUp end";
}

void AbilityStackModuleTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown start";
    auto ams = DelayedSingleton<AbilityManagerService>::GetInstance();
    ams->stackManagers_.clear();
    ams->currentStackManager_ = nullptr;
    stackManager_ = nullptr;
    ams->OnStop();
    GTEST_LOG_(INFO) << "TearDown end";
}

AbilityRequest AbilityStackModuleTest::GenerateAbilityRequest(const std::string &deviceName,
    const std::string &abilityName, const std::string &appName, const std::string &bundleName)
{
    GTEST_LOG_(INFO) << "GenerateAbilityRequest start ability :" << abilityName;
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

    makeScene(abilityName, bundleName, abilityInfo, want);

    appinfo = abilityInfo.applicationInfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void AbilityStackModuleTest::makeScene(
    const std::string &abilityName, const std::string &bundleName, AbilityInfo &abilityInfo, Want &want)
{
    GTEST_LOG_(INFO) << "makeScene";
    if (bundleName == "com.ix.hiworld") {
        std::string entity = Want::ENTITY_HOME;
        want.AddEntity(entity);
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.applicationInfo.isLauncherApp = true;
        abilityInfo.process = "p";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }

    if (bundleName == "com.ix.hiMusic") {
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.applicationInfo.isLauncherApp = false;

        if (abilityName == "MusicAbility" || abilityName == "MusicAbility2th") {
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
        if (abilityName == "RadioTopAbility") {
            abilityInfo.launchMode = LaunchMode::SINGLETOP;
        }
        if (abilityName == "RadioSAbility") {
            abilityInfo.launchMode = LaunchMode::SINGLETON;
        }
    }
}

Want AbilityStackModuleTest::CreateWant(const std::string &entity)
{
    GTEST_LOG_(INFO) << "CreateWant";
    Want want;
    if (!entity.empty()) {
        want.AddEntity(entity);
    }
    return want;
}

AbilityInfo AbilityStackModuleTest::CreateAbilityInfo(
    const std::string &name, const std::string &appName, const std::string &bundleName)
{
    GTEST_LOG_(INFO) << "CreateAbilityInfo";
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    return abilityInfo;
}

ApplicationInfo AbilityStackModuleTest::CreateAppInfo(const std::string &appName, const std::string &bundleName)
{
    GTEST_LOG_(INFO) << "CreateAppInfo";
    AbilityInfo abilityInfo;
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

/*
 * Feature: AaFwk
 * Function:GetMissionSnapshot
 * SubFunction: Get Mission Snapshot
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Get Mission Snapshot
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_getMissionSnapshot_001, TestSize.Level1)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    WindowManagerServiceMock *mockWindowManager = new WindowManagerServiceMock();
    // set mock
    stackManager_->screenshotHandler_->windowMS_ = mockWindowManager;
    sptr<PromiseWMSImageInfo> promise = new PromiseWMSImageInfo();
    auto infos = [promise](int32_t id) -> sptr<PromiseWMSImageInfo> {
        return promise;
    };
    EXPECT_CALL(*mockWindowManager, ShotWindow)
        .Times(AtLeast(1))
        .WillOnce(Invoke(infos));

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    EXPECT_TRUE(stackManager_->missionStackList_.size() != 0);

    int32_t missionId = 0;
    MissionPixelMap missionPixelMap;
    std::thread([promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        WMSImageInfo imageInfo;
        imageInfo.width = 2;
        imageInfo.height = 3;
        imageInfo.format = 10;
        promise->Resolve(imageInfo);
    }).detach();
    auto ret = stackManager_->GetMissionSnapshot(missionId, missionPixelMap);

    EXPECT_TRUE(missionPixelMap.topAbility.abilityName_ == "LauncherAbility");
    EXPECT_TRUE(missionPixelMap.topAbility.bundleName_ == "com.ix.hiworld");
    EXPECT_TRUE(missionPixelMap.imageInfo.width == 2);
    EXPECT_TRUE(missionPixelMap.imageInfo.height== 3);
    EXPECT_TRUE(missionPixelMap.imageInfo.format == 10);
    EXPECT_TRUE(ERR_OK == ret);
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: start launcher ability
 * FunctionPoints: launcher mission stack
 * EnvConditions: NA
 * CaseDescription: start launcher ability when current launcher mission stack empty.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_001 start";
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityInfo abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    ApplicationInfo appInfo = CreateAppInfo(bundleName, bundleName);
    Want want = CreateWant(Want::ENTITY_HOME);

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appInfo;
    abilityRequest.appInfo.isLauncherApp = true;  // launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(curMissionStack);
    stackManager_->StartAbility(abilityRequest);
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    EXPECT_TRUE(curMissionStack->GetTopMissionRecord() != nullptr);
    EXPECT_EQ(1, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    EXPECT_STREQ(abilityName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().name.c_str());
    EXPECT_STREQ(bundleName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().bundleName.c_str());

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_001 end";
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: start launcher ability
 * FunctionPoints: launcher mission stack
 * EnvConditions: NA
 * CaseDescription: not add new ability to current mission record when start the same launcher ability twice.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_002 start";
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant(Want::ENTITY_HOME);
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    abilityRequest.appInfo.isLauncherApp = true;  // launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    EXPECT_EQ(true, stackManager_->waittingAbilityQueue_.empty());

    int result = stackManager_->StartAbility(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    EXPECT_EQ(1, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());

    result = stackManager_->StartAbility(abilityRequest);  // same launcher ability
    // not change current mission stack
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    // not add new mission to current mission stack
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    // not add new ability to current mission record
    EXPECT_EQ(1, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    EXPECT_STREQ(abilityName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().name.c_str());
    EXPECT_STREQ(bundleName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().bundleName.c_str());
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_002 end";
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: start launcher ability
 * FunctionPoints: launcher mission stack
 * EnvConditions: NA
 * CaseDescription: add new mission to current mission stack whene start the different launcher ability.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_003 start";
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";
    int index = 1;

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant(Want::ENTITY_HOME);
    std::string strInd = std::to_string(index);
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + strInd, bundleName + strInd, bundleName + strInd);
    abilityRequest.appInfo = CreateAppInfo(bundleName + strInd, bundleName + strInd);
    abilityRequest.appInfo.isLauncherApp = true;  // launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    AbilityRequest abilityRequest2;
    abilityRequest2.want = CreateWant(Want::ENTITY_HOME);
    index++;
    strInd = std::to_string(index);
    abilityRequest2.abilityInfo = CreateAbilityInfo(abilityName + strInd, bundleName + strInd, bundleName + strInd);
    abilityRequest2.appInfo = CreateAppInfo(bundleName + strInd, bundleName + strInd);
    abilityRequest2.appInfo.isLauncherApp = true;  // another launcher ability
    abilityRequest2.abilityInfo.applicationInfo = abilityRequest2.appInfo;

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    int result = stackManager_->StartAbility(abilityRequest);
    usleep(1000);
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    curMissionStack->GetTopAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    result = stackManager_->StartAbility(abilityRequest2);
    // not change current mission stack
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    // add new mission to current mission stack
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    EXPECT_EQ(2, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    EXPECT_STREQ((abilityName + strInd).c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().name.c_str());
    EXPECT_STREQ(
        (bundleName + strInd).c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().bundleName.c_str());
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_003 end";
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: start non-launcher ability
 * FunctionPoints: non-launcher mission stack
 * EnvConditions: NA
 * CaseDescription: change current mission stack when start non-launcher ability.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_004 start";

    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";
    int index = 1;
    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant(Want::ENTITY_HOME);
    std::string strInd = std::to_string(index);
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName + strInd, bundleName + strInd, bundleName + strInd);
    abilityRequest.appInfo = CreateAppInfo(bundleName + strInd, bundleName + strInd);
    abilityRequest.appInfo.isLauncherApp = true;  // launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    AbilityRequest abilityRequest2;
    abilityRequest2.want = CreateWant("");
    strInd = std::to_string(++index);
    abilityRequest2.abilityInfo = CreateAbilityInfo(abilityName + strInd, bundleName + strInd, bundleName + strInd);
    abilityRequest2.appInfo = CreateAppInfo(bundleName + strInd, bundleName + strInd);
    abilityRequest2.appInfo.isLauncherApp = false;  // non-launcher ability
    abilityRequest2.abilityInfo.applicationInfo = abilityRequest2.appInfo;

    int result = stackManager_->StartAbility(abilityRequest);
    EXPECT_EQ(result, ERR_OK);

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    curMissionStack->GetTopAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    result = stackManager_->StartAbility(abilityRequest2);
    EXPECT_EQ(result, ERR_OK);

    curMissionStack = stackManager_->GetCurrentMissionStack();
    // change current mission stack
    EXPECT_EQ(DEFAULT_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    // add new mission to current mission stack
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    EXPECT_EQ(1, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    EXPECT_STREQ((abilityName + strInd).c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().name.c_str());
    EXPECT_STREQ(
        (bundleName + strInd).c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().bundleName.c_str());
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_004 end";
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: terminate launcher ability
 * FunctionPoints: launcher mission stack
 * EnvConditions: NA
 * CaseDescription: verify unique launcher ability can't be terminated.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_005 start";
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant(Want::ENTITY_HOME);
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    abilityRequest.appInfo.isLauncherApp = true;  // launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    int result = stackManager_->StartAbility(abilityRequest);
    usleep(1000);
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    curMissionStack->GetTopAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    result = stackManager_->TerminateAbility(curMissionStack->GetTopAbilityRecord()->GetToken(), -1, nullptr);
    EXPECT_EQ(TERMINATE_LAUNCHER_DENIED, result);
    // not change current mission stack
    EXPECT_EQ(LAUNCHER_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    EXPECT_EQ(1, curMissionStack->GetMissionRecordCount());
    // not change current ability state
    EXPECT_TRUE(curMissionStack->GetTopAbilityRecord() != nullptr);
    EXPECT_EQ(OHOS::AAFwk::AbilityState::ACTIVE, curMissionStack->GetTopAbilityRecord()->GetAbilityState());
    EXPECT_STREQ(abilityName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().name.c_str());
    EXPECT_STREQ(bundleName.c_str(), curMissionStack->GetTopAbilityRecord()->GetAbilityInfo().bundleName.c_str());

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_005 end";
}

/*
 * Feature: AaFwk
 * Function: ability stack management
 * SubFunction: start and terminate non-launcher ability
 * FunctionPoints: single top
 * EnvConditions: NA
 * CaseDescription: start and terminate multiple non-launcher ability(single application).
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_006 start";
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    std::string appSuffix = "1";
    std::vector<AbilityRequest> abilityRequests;
    for (int i = 0; i < 3; i++) {
        AbilityRequest abilityRequest;
        abilityRequest.want = CreateWant("");
        std::string abilitySuffix = appSuffix + std::to_string(i + 1);
        abilityRequest.abilityInfo =
            CreateAbilityInfo(abilityName + abilitySuffix, bundleName + appSuffix, bundleName + appSuffix);
        abilityRequest.abilityInfo.launchMode = LaunchMode::STANDARD;
        abilityRequest.appInfo = CreateAppInfo(bundleName + appSuffix, bundleName + appSuffix);
        abilityRequest.appInfo.isLauncherApp = false;  // non-launcher ability
        abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

        abilityRequests.push_back(abilityRequest);
    }

    // start "ability_name11"
    int result = stackManager_->StartAbility(abilityRequests[0]);
    usleep(1000);
    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    EXPECT_EQ(DEFAULT_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    std::shared_ptr<AbilityRecord> currentTopAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_STREQ("ability_name11", currentTopAbilityRecord->GetAbilityInfo().name.c_str());
    currentTopAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    // start "ability_name12"
    result = stackManager_->StartAbility(abilityRequests[1]);
    usleep(1000);
    EXPECT_EQ(DEFAULT_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    currentTopAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_STREQ("ability_name12", currentTopAbilityRecord->GetAbilityInfo().name.c_str());
    currentTopAbilityRecord->GetPreAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    currentTopAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(2, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());

    // start "ability_name12"
    result = stackManager_->StartAbility(abilityRequests[1]);
    usleep(1000);
    currentTopAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_STREQ("ability_name12", currentTopAbilityRecord->GetAbilityInfo().name.c_str());
    // EXPECT_STREQ("ability_name11",
    currentTopAbilityRecord->GetPreAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    currentTopAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(3, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());

    // start "ability_name13"
    result = stackManager_->StartAbility(abilityRequests[2]);
    usleep(1000);
    currentTopAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_STREQ("ability_name13", currentTopAbilityRecord->GetAbilityInfo().name.c_str());
    currentTopAbilityRecord->GetPreAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    currentTopAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(4, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());

    // start "ability_name12"
    result = stackManager_->StartAbility(abilityRequests[1]);
    usleep(1000);
    currentTopAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_STREQ("ability_name12", currentTopAbilityRecord->GetAbilityInfo().name.c_str());
    EXPECT_STREQ("ability_name13", currentTopAbilityRecord->GetPreAbilityRecord()->GetAbilityInfo().name.c_str());
    currentTopAbilityRecord->GetPreAbilityRecord()->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    currentTopAbilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    EXPECT_EQ(5, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());

    // terminate stack top ability "ability_name12"
    currentTopAbilityRecord->lifecycleDeal_ = nullptr;
    Want want;
    result = stackManager_->TerminateAbility(currentTopAbilityRecord->GetToken(), -1, &want);
    EXPECT_EQ(DEFAULT_MISSION_STACK_ID, curMissionStack->GetMissionStackId());
    EXPECT_EQ(4, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    currentTopAbilityRecord = curMissionStack->GetTopAbilityRecord();
    EXPECT_TRUE(currentTopAbilityRecord != nullptr);
    EXPECT_STREQ("ability_name13", currentTopAbilityRecord->GetAbilityInfo().name.c_str());

    // terminate stack bottom ability
    auto bottomAbility = curMissionStack->GetTopMissionRecord()->abilities_.back();
    result = stackManager_->TerminateAbility(bottomAbility->GetToken(), -1, &want);
    EXPECT_EQ(3, curMissionStack->GetTopMissionRecord()->GetAbilityRecordCount());
    currentTopAbilityRecord = curMissionStack->GetTopAbilityRecord();
    EXPECT_TRUE(currentTopAbilityRecord != nullptr);
    EXPECT_STREQ("ability_name13", currentTopAbilityRecord->GetAbilityInfo().name.c_str());

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_006 end";
}

/*
 * Feature: AaFwk
 * Function: attach ability
 * SubFunction: attach ability thread
 * FunctionPoints: update ability state
 * EnvConditions: NA
 * CaseDescription: update ability state when attach ability.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_007 start";

    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);
    abilityRequest.appInfo.isLauncherApp = false;  // non-launcher ability
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVE);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    OHOS::sptr<MockAbilityScheduler> abilityScheduler(new MockAbilityScheduler());

    EXPECT_TRUE(abilityRecord->GetToken());

    EXPECT_CALL(*abilityScheduler, AsObject()).WillRepeatedly(Return(nullptr));
    stackManager_->AttachAbilityThread(abilityScheduler, abilityRecord->GetToken());

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_007 end";
}

/*
 * Feature: AaFwk
 * Function: ability state transition
 * SubFunction: ability state transition done
 * FunctionPoints: ability state transition(TERMINATING->INITIAL)
 * EnvConditions: NA
 * CaseDescription: complete ability state transition(TERMINATING->INITIAL).
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_008 start";

    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->lifecycleDeal_ = nullptr;
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::TERMINATING);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    std::shared_ptr<AbilityEventHandler> handler =
        std::make_shared<AbilityEventHandler>(nullptr, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->handler_ = handler;

    PacMap saveData;
    int result =
        stackManager_->AbilityTransitionDone(abilityRecord->GetToken(), OHOS::AAFwk::AbilityState::INITIAL, saveData);
    EXPECT_EQ(OHOS::ERR_OK, result);

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_008 end";
}

/*
 * Feature: AaFwk
 * Function: add Window
 * SubFunction: add window for ability
 * FunctionPoints: add window for ability
 * EnvConditions: NA
 * CaseDescription: add window for ability.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_009 start";

    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    stackManager_->AddWindowInfo(abilityRecord->GetToken(), 1);
    EXPECT_TRUE(abilityRecord->GetWindowInfo() != nullptr);

    GTEST_LOG_(INFO) << "AbilityStackModuleTest ability_stack_test_009 end";
}

/*
 * Feature: AaFwk
 * Function: app state callback
 * SubFunction: OnAbilityRequestDone
 * FunctionPoints: OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: activate ability(ABILITY_STATE_FOREGROUND).
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_010, TestSize.Level1)
{
    std::string abilityName = "ability_name";
    std::string bundleName = "com.ix.aafwk.moduletest";

    AbilityRequest abilityRequest;
    abilityRequest.want = CreateWant("");
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, bundleName, bundleName);
    abilityRequest.appInfo = CreateAppInfo(bundleName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    abilityRecord->SetScheduler(scheduler);

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    std::shared_ptr<MissionRecord> mission = std::make_shared<MissionRecord>(bundleName);
    mission->AddAbilityRecordToTop(abilityRecord);

    std::shared_ptr<MissionStack> curMissionStack = stackManager_->GetCurrentMissionStack();
    curMissionStack->AddMissionRecordToTop(mission);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);
    stackManager_->OnAbilityRequestDone(
        abilityRecord->GetToken(), static_cast<int32_t>(OHOS::AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND));
}

/*
 * Feature: AbilityStackManager
 * Function:  PowerOff
 * SubFunction: NA
 * FunctionPoints: PowerOff
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify PowerOff
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_011, TestSize.Level1)
{
    AbilityRequest launcherAbilityRequest_ =
        GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");

    stackManager_->StartAbility(launcherAbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(firstTopAbility);

    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto transactionDoneCaller = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteInactive(firstTopAbility);
    };

    auto transactionDoneCaller2 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteBackground(firstTopAbility);
    };
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_))
        .Times(testing::AtLeast(2))
        .WillOnce(testing::Invoke(transactionDoneCaller))
        .WillOnce(testing::Invoke(transactionDoneCaller2));
    EXPECT_CALL(*scheduler, ScheduleSaveAbilityState()).Times(AtLeast(1));

    firstTopAbility->SetScheduler(scheduler);

    int result = stackManager_->PowerOff();
    EXPECT_EQ(ERR_OK, result);

    EXPECT_NE(stackManager_->powerStorage_, nullptr);
    EXPECT_EQ(OHOS::AAFwk::BACKGROUND, firstTopAbility->GetAbilityState());

    auto recordVector = stackManager_->powerStorage_->GetPowerOffInActiveRecord();
    EXPECT_TRUE(recordVector.empty());
}

/*
 * Feature: AbilityStackManager
 * Function:  PowerOn
 * SubFunction: NA
 * FunctionPoints: PowerOn
 * EnvConditions: NA
 * CaseDescription: Succeeded to verify PowerOn
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_012, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    stackManager_->StartAbility(launcherAbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);

    auto transactionDoneCaller = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteBackground(firstTopAbility);
    };

    auto transactionDoneCaller2 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteActive(firstTopAbility);
    };

    auto transactionDoneCaller3 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteInactive(firstTopAbility);
    };

    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));

    firstTopAbility->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, AsObject())
        .Times(testing::AtLeast(1))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(nullptr));

    EXPECT_CALL(*scheduler, ScheduleSaveAbilityState()).Times(testing::AtLeast(3));

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_))
        .Times(testing::AtLeast(3))
        .WillOnce(testing::Invoke(transactionDoneCaller))
        .WillOnce(testing::Invoke(transactionDoneCaller2))
        .WillOnce(testing::Invoke(transactionDoneCaller3))
        .WillOnce(testing::Invoke(transactionDoneCaller));
    EXPECT_CALL(*scheduler, ScheduleSaveAbilityState()).Times(AtLeast(1));

    firstTopAbility->SetScheduler(scheduler);

    auto transactionDoneCaller2_1 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteInactive(secondTopAbility);
    };

    auto transactionDoneCaller2_2 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteBackground(secondTopAbility);
    };

    auto transactionDoneCaller2_3 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteActive(secondTopAbility);
    };

    OHOS::sptr<MockAbilityScheduler> scheduler2(new MockAbilityScheduler());

    EXPECT_CALL(*scheduler2, AsObject()).WillRepeatedly(Return(nullptr));

    secondTopAbility->SetScheduler(scheduler2);
    EXPECT_CALL(*scheduler2, AsObject())
        .Times(testing::AtLeast(1))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(nullptr));

    EXPECT_CALL(*scheduler2, ScheduleSaveAbilityState()).Times(testing::AtLeast(3));

    EXPECT_CALL(*scheduler2, ScheduleAbilityTransaction(testing::_, testing::_))
        .Times(testing::AtLeast(3))
        .WillOnce(testing::Invoke(transactionDoneCaller2_1))
        .WillOnce(testing::Invoke(transactionDoneCaller2_2))
        .WillOnce(testing::Invoke(transactionDoneCaller2_3));
    EXPECT_CALL(*scheduler2, ScheduleSaveAbilityState()).Times(AtLeast(1));

    secondTopAbility->SetScheduler(scheduler2);

    int result = stackManager_->PowerOff();
    EXPECT_EQ(ERR_OK, result);

    EXPECT_EQ(OHOS::AAFwk::BACKGROUND, firstTopAbility->GetAbilityState());
    EXPECT_EQ(OHOS::AAFwk::BACKGROUND, secondTopAbility->GetAbilityState());

    auto recordVector = stackManager_->powerStorage_->GetPowerOffInActiveRecord();
    int size = recordVector.size();
    EXPECT_EQ(size, 1);

    for (const auto &it : recordVector) {
        EXPECT_EQ(it.ability.lock()->GetRecordId(), firstTopAbility->GetRecordId());
        EXPECT_EQ(it.ability.lock()->GetAbilityState(), OHOS::AAFwk::BACKGROUND);
    }

    result = stackManager_->PowerOn();
    firstTopAbility->Activate();
    secondTopAbility->Activate();
    EXPECT_EQ(ERR_OK, result);

    // end last move to background EXPECT_EQ(OHOS::AAFwk::BACKGROUND, firstTopAbility->GetAbilityState())
    EXPECT_EQ(OHOS::AAFwk::ACTIVE, secondTopAbility->GetAbilityState());

    testing::Mock::AllowLeak(scheduler);
    testing::Mock::AllowLeak(scheduler2);
}

/*
 * Feature: AaFwk
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: lock a mission , It's locked mission top ability will be active
 * EnvConditions: NA
 * CaseDescription: lock a mission
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_013, TestSize.Level1)
{
    int sysUid = 1000;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(sysUid))
        .WillOnce(Return(sysUid));

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    stackManager_->StartAbility(launcherAbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // lock LauncherAbility
    auto launcherMisionRecord = firstTopAbility->GetMissionRecord();
    auto launcherMisionRecordId = launcherMisionRecord->GetMissionRecordId();
    auto result = stackManager_->StartLockMission(sysUid, launcherMisionRecordId, true, true);
    EXPECT_EQ(ERR_OK, result);

    auto topRecord = stackManager_->GetCurrentTopAbility();
    auto abilityInfo = topRecord->GetAbilityInfo();
    EXPECT_EQ(abilityInfo.name, "LauncherAbility");
    topRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // unlock
    result = stackManager_->StartLockMission(sysUid, launcherMisionRecordId, true, false);
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AaFwk
 * Function: StartLockMissions
 * SubFunction: NA
 * FunctionPoints: lock a mission , It's locked mission top ability will be active
 * EnvConditions: NA
 * CaseDescription: multiple mission lock someone
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_014, TestSize.Level1)
{
    int sysUid = 1000;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(sysUid))
        .WillOnce(Return(sysUid));

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2th", "launcher", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto redioAbilityRequest_ = GenerateAbilityRequest("device", "RadioSAbility", "Radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(redioAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto transactionDoneCaller = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteActive(secondTopAbility);
    };
    auto transactionDoneCaller2 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteInactive(secondTopAbility);
    };

    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    secondTopAbility->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_))
        .Times(testing::AtLeast(2))
        .WillOnce(testing::Invoke(transactionDoneCaller))
        .WillOnce(testing::Invoke(transactionDoneCaller2));

    auto transactionDoneCaller3 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteInactive(thirdTopAbility);
    };
    auto transactionDoneCaller4 = [&](const Want &want, const LifeCycleStateInfo &targetState) {
        stackManager_->CompleteActive(thirdTopAbility);
    };

    OHOS::sptr<MockAbilityScheduler> scheduler2(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler2, AsObject()).WillRepeatedly(Return(nullptr));
    thirdTopAbility->SetScheduler(scheduler2);
    EXPECT_CALL(*scheduler2, ScheduleAbilityTransaction(testing::_, testing::_))
        .Times(testing::AtLeast(2))
        .WillOnce(testing::Invoke(transactionDoneCaller3))
        .WillOnce(testing::Invoke(transactionDoneCaller4));
    EXPECT_CALL(*scheduler2, ScheduleSaveAbilityState()).Times(AtLeast(1));

    secondTopAbility->SetAbilityState(OHOS::AAFwk::INACTIVE);
    auto musicMisionRecord = secondTopAbility->GetMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();

    auto result = stackManager_->StartLockMission(sysUid, musicMisionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result);

    auto topRecord = stackManager_->GetCurrentTopAbility();
    auto abilityInfo = topRecord->GetAbilityInfo();
    EXPECT_EQ(abilityInfo.name, "MusicAbility2th");

    // unlock
    result = stackManager_->StartLockMission(sysUid, musicMisionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AaFwk
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: lock a mission , It's locked mission top ability will be active
 * EnvConditions: NA
 * CaseDescription: when a misson locked other mission do not to statr ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_015, TestSize.Level1)
{
    int userUid = 10;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(userUid))
        .WillOnce(Return(userUid));

    // start a SINGLETON ability
    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // lock
    auto musicMisionRecord = stackManager_->GetTopMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();
    auto result = stackManager_->StartLockMission(userUid, musicMisionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result);

    // not can start a ability
    auto redioAbilityRequest_ = GenerateAbilityRequest("device", "RedioAbility", "music", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(redioAbilityRequest_);
    EXPECT_EQ(LOCK_MISSION_STATE_DENY_REQUEST, ref);

    auto topAbility = stackManager_->GetCurrentTopAbility();
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    // unlock
    result = stackManager_->StartLockMission(userUid, musicMisionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result);

    // can start a ability
    ref = stackManager_->StartAbility(redioAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function: StartLockMission
 * SubFunction: NA
 * FunctionPoints: lock a mission , It's locked mission top ability will be active
 * EnvConditions: NA
 * CaseDescription: when a mission locked, you cannot delete the last one
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_016, TestSize.Level1)
{
    int userUid = 10;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(userUid))
        .WillOnce(Return(userUid));

    // start a SINGLETON ability
    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // lock
    auto musicMisionRecord = stackManager_->GetTopMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();
    auto result = stackManager_->StartLockMission(userUid, musicMisionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result);

    result = stackManager_->TerminateAbility(secondTopAbility->GetToken(), -1, nullptr);
    EXPECT_EQ(LOCK_MISSION_STATE_DENY_REQUEST, result);

    // unlock
    result = stackManager_->StartLockMission(userUid, musicMisionRecordId, false, false);
    EXPECT_EQ(ERR_OK, result);

    result = stackManager_->TerminateAbility(secondTopAbility->GetToken(), -1, nullptr);
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AaFwk
 * Function: StartLockMissions
 * SubFunction: NA
 * FunctionPoints: lock a mission , It's locked mission top ability will be active
 * EnvConditions: NA
 * CaseDescription: lock a  multi ability mission
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_017, TestSize.Level1)
{
    int sysUid = 1000;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(sysUid))
        .WillOnce(Return(sysUid));

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "launcher", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2th", "launcher", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto redioAbilityRequest_ = GenerateAbilityRequest("device", "RadioSAbility", "Radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(redioAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto thirdTopAbility = stackManager_->GetCurrentTopAbility();
    thirdTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    firstTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);

    auto musicMisionRecord = secondTopAbility->GetMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();

    auto result = stackManager_->StartLockMission(sysUid, musicMisionRecordId, true, true);
    EXPECT_EQ(ERR_OK, result);

    auto topRecord = stackManager_->GetCurrentTopAbility();
    auto abilityInfo = topRecord->GetAbilityInfo();
    EXPECT_EQ(abilityInfo.name, "MusicAbility2th");

    // unlock
    result = stackManager_->StartLockMission(sysUid, musicMisionRecordId, true, false);
    EXPECT_EQ(ERR_OK, result);
}

/*
 * Feature: AaFwk
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: Execute when application dies
 * EnvConditions: NA
 * CaseDescription: When the system attribute is changed, if the listener is not registered, the application needs to be
 * restarted
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_018, TestSize.Level1)
{
    int userUid = 10;
    EXPECT_CALL(*bundleObject_, GetUidByBundleName(_, _))
        .Times(AtLeast(2))
        .WillOnce(Return(userUid))
        .WillOnce(Return(userUid));

    // start a SINGLETON ability
    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(musicAbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // lock
    auto musicMisionRecord = stackManager_->GetTopMissionRecord();
    auto musicMisionRecordId = musicMisionRecord->GetMissionRecordId();
    auto result = stackManager_->StartLockMission(userUid, musicMisionRecordId, false, true);
    EXPECT_EQ(ERR_OK, result);

    // let ability die
    stackManager_->OnAbilityDied(secondTopAbility);
    auto state = stackManager_->lockMissionContainer_->IsLockedMissionState();
    EXPECT_FALSE(state);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start music ability standard
 *                  2. start radio ability standard
 *                  3. radio ability died
 *                  4. start music ability standard again
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_025, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability standard
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start music ability again
    auto musicAbilityRequest2 = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest2);
    EXPECT_EQ(ERR_OK, ref);

    // attach radio ability
    EXPECT_CALL(*mockScheduler_, ScheduleRestoreAbilityState(_)).Times(1);
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
    radioAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start radio ability singleton
 *                  2. start music ability standard
 *                  3. radio ability died
 *                  4. start radio ability singleton again
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_026, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability singleton
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start radio ability singleton again
    auto radioAbilityReques2 = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityReques2);
    EXPECT_EQ(ERR_OK, ref);

    // attach radio ability
    EXPECT_CALL(*mockScheduler_, ScheduleRestoreAbilityState(_)).Times(1);
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
    radioAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start music ability standard
 *                  2. start radio ability singletop
 *                  3. radio ability died
 *                  4. start radio ability singletop again
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_027, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability singletop
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start radio ability singleton again
    auto radioAbilityReques2 = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityReques2);
    EXPECT_EQ(ERR_OK, ref);

    // attach radio ability
    EXPECT_CALL(*mockScheduler_, ScheduleRestoreAbilityState(_)).Times(1);
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
    radioAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start radio ability singleton
 *                  2. start music ability standard
 *                  3. radio ability died
 *                  4. move misstion to top which has radio ability
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_028, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability singleton
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // move mission to top, radio ability singleton
    auto missionId = radioAbility->GetMissionRecord()->GetMissionRecordId();
    stackManager_->MoveMissionToTop(missionId);

    // attach radio ability
    EXPECT_CALL(*mockScheduler_, ScheduleRestoreAbilityState(_)).Times(1);
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToTop
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start radio ability standard
 *                  2. start music ability standard
 *                  3. start music ability singleton
 *                  4. radio ability died
 *                  5. move mission to top which has radio ability
 *                  6. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_029, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability standard
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability singleton
    auto musicSAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicSAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicSAbility = stackManager_->GetCurrentTopAbility();
    musicSAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // move mission to top
    auto missionId = radioAbility->GetMissionRecord()->GetMissionRecordId();
    stackManager_->MoveMissionToTop(missionId);

    // attach radio ability
    EXPECT_CALL(*mockScheduler_, ScheduleRestoreAbilityState(_)).Times(1);
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
    radioAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start radio ability singleton
 *                  2. start music ability standard
 *                  3. radio ability died
 *                  4. move mission which at top to end
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_030, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability singleton
    auto radioSAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioSAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioSAbility = stackManager_->GetCurrentTopAbility();
    radioSAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability standard
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbility = stackManager_->GetCurrentTopAbility();
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioSAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioSAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // move mission to end
    stackManager_->MoveMissionToEnd(musicAbility->GetToken(), false);

    // attach radio ability
    stackManager_->AttachAbilityThread(mockScheduler_, radioSAbility->GetToken());
    radioSAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: Resume mission record
 * EnvConditions: NA
 * CaseDescription: 1. start radio ability standard
 *                  2. start music ability singleton
 *                  3. radio ability died
 *                  4. move mission which at top to end
 *                  5. resume radio ability
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_031, TestSize.Level1)
{
    // launcher ability
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto launcherAbility = stackManager_->GetCurrentTopAbility();
    launcherAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability standard
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbility = stackManager_->GetCurrentTopAbility();
    radioAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // music ability singleton
    auto musicSAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicSAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicSAbility = stackManager_->GetCurrentTopAbility();
    musicSAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // set mock scheduler
    EXPECT_CALL(*mockScheduler_, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbility->SetScheduler(mockScheduler_);

    // radio ability die
    stackManager_->OnAbilityDied(radioAbility);

    // set top ability active
    auto topAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_NE(topAbility->GetAbilityInfo().name, "RadioAbility");
    topAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // move mission to end
    stackManager_->MoveMissionToEnd(musicSAbility->GetToken(), false);

    // attach radio ability
    stackManager_->AttachAbilityThread(mockScheduler_, radioAbility->GetToken());
    radioAbility->SetScheduler(nullptr);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: Floating window and full screen movement
 * EnvConditions: NA
 * CaseDescription: When a full screen application and a floating window application are displayed, move the floating
 * window application to the background
 */

HWTEST_F(AbilityStackModuleTest, ability_stack_test_032, TestSize.Level1)
{
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecord1 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "Music", "launcher", "com.ix.hiMusic");
    // start float ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    musicAbilityRequest_.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, ref);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stackManager_->isMultiWinMoving_ = false;
    // test case
    EXPECT_EQ(topMissionRecord1->GetMissionStack()->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(topMissionRecord2->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    topAbilityRecord2->SetScheduler(scheduler);

    // Notify application executionon ConfigurationUpdated
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(testing::AtLeast(1));
    // test fun
    ref = stackManager_->MoveMissionToEnd(topAbilityRecord2->GetToken(), true);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToEnd
 * SubFunction: NA
 * FunctionPoints: Floating window and full screen movement
 * EnvConditions: NA
 * CaseDescription: When a full screen application and a floating window application are displayed, move the floating
 * window application to the background
 */

HWTEST_F(AbilityStackModuleTest, ability_stack_test_033, TestSize.Level1)
{
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    OHOS::sptr<MockAbilityScheduler> schedulerRadio(new MockAbilityScheduler());

    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecord1 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // radio ability standard
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecordRadio);
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest_ = GenerateAbilityRequest("device", "Music", "launcher", "com.ix.hiMusic");
    // start float ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    musicAbilityRequest_.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(musicAbilityRequest_);
    EXPECT_EQ(0, ref);
    auto topMissionRecord2 = stackManager_->GetTopMissionRecord();
    auto topAbilityRecord2 = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecord2);
    EXPECT_TRUE(topAbilityRecord2);
    topAbilityRecord2->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // test case
    EXPECT_EQ(topMissionRecord1->GetMissionStack()->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(topMissionRecordRadio->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(topMissionRecord2->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    stackManager_->isMultiWinMoving_ = false;

    // Music anility  scheduler
    topAbilityRecord2->SetScheduler(scheduler);
    EXPECT_CALL(*scheduler, AsObject()).Times(testing::AtLeast(1));
    // Notify application executionon ConfigurationUpdated
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(testing::AtLeast(1));
    // test fun
    ref = stackManager_->MoveMissionToEnd(topAbilityRecord2->GetToken(), true);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility by setting info, satrt a Split screen or floating window
 * EnvConditions: NA
 * CaseDescription: Start a floating window in singleton mode?
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_034, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // singleton ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");

    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecordRadio);
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topMissionRecordRadio->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_FALSE(stackManager_->isMultiWinMoving_);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility by setting info, satrt a Split screen or floating window
 * EnvConditions: NA
 * CaseDescription: Start a normal full screen stack when the top of the stack is a floating window
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_035, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // singleton ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");

    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecordRadio);
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start defult ability, put in float stack
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);

    auto musicAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbility);
    musicAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);
    EXPECT_EQ(musicAbility->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility by setting info, satrt a Split screen or floating window
 * EnvConditions: NA
 * CaseDescription: Start a singleton full screen stack when the top of the stack is a floating window
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_036, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // singleton ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");

    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecordRadio);
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start defult singleton ability, put in float stack
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicTonMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicTonAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicTonMissionRecord);
    EXPECT_TRUE(musicTonAbility);
    musicTonAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(musicTonMissionRecord->GetMissionStack()->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility by setting info, satrt a Split screen or floating window
 * EnvConditions: NA
 * CaseDescription: Start a floating window based on a single column full screen stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_037, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start defult singleton ability, put in float stack
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicTonMissionRecord = stackManager_->GetTopMissionRecord();
    auto musicTonAbility = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicTonMissionRecord);
    EXPECT_TRUE(musicTonAbility);
    musicTonAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // singleton ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topMissionRecordRadio);
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topMissionRecordRadio->GetMissionStack()->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToFloatingStack
 * SubFunction: Try moving other applications to the floating window stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move the luncher to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_038, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);

    // move luncher, will fail
    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN);

    EXPECT_NE(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToFloatingStack
 * SubFunction: Try moving other applications to the floating window stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move the defult ability to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_039, TestSize.Level1)
{
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // Starting another normal application should start on the floating window stack
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(radioAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToFloatingStack
 * SubFunction: Try moving other applications to the floating window stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move a single instance of the common application to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_040, TestSize.Level1)
{
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // Starting another singletop application should start on the floating window stack
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // To be discussed
    EXPECT_EQ(radioAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MoveMissionToFloatingStack
 * SubFunction: Try moving other applications to the floating window stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move a floating window to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_041, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // singleton ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");

    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));

    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // move float window
    MissionOption missionOption;
    missionOption.missionId = topAbilityRecordRadio->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION);
}

/*
 * Feature: AaFwk
 * Function: MinimizeMultiWindow and MaximizeMultiWindow
 * SubFunction: moving other applications to the floating window stack or Full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move the luncher to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_042, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // try move luncher to float stack
    ref = stackManager_->MinimizeMultiWindow(topAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(MINIMIZE_MULTI_WINDOW_FAILED, ref);
    ref = stackManager_->MaximizeMultiWindow(topAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(MAXIMIZE_MULTIWINDOW_NOT_EXIST, ref);
}

/*
 * Feature: AaFwk
 * Function: MinimizeMultiWindow and MaximizeMultiWindow
 * SubFunction: moving other applications to the floating window stack or Full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move the singleton ability to the floating window stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_043, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordlauncher = stackManager_->GetCurrentTopAbility();
    topAbilityRecordlauncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    stackManager_->isMultiWinMoving_ = false;

    // SetScheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecord->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);

    OHOS::sptr<MockAbilityScheduler> schedulerluncher(new MockAbilityScheduler());
    EXPECT_CALL(*schedulerluncher, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordlauncher->SetScheduler(schedulerluncher);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    EXPECT_FALSE(topAbilityRecord->IsToEnd());

    ref = stackManager_->MinimizeMultiWindow(topAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(ERR_OK, ref);

    EXPECT_TRUE(topAbilityRecord->IsToEnd());
}

/*
 * Feature: AaFwk
 * Function: MinimizeMultiWindow and MaximizeMultiWindow
 * SubFunction: moving other applications to the floating window stack or Full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move the float ability to the defult stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_044, TestSize.Level1)
{
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    // SetScheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecord->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, NotifyMultiWinModeChanged(testing::_, testing::_)).Times(1);
    EXPECT_CALL(*scheduler, NotifyTopActiveAbilityChanged(testing::_)).Times(2);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    ref = stackManager_->MaximizeMultiWindow(topAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(ERR_OK, ref);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MinimizeMultiWindow and MaximizeMultiWindow
 * SubFunction: moving other applications to the floating window stack or Full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Operate when there are two floating window
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_045, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord1 = stackManager_->GetCurrentTopAbility();
    topAbilityRecord1->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(radioAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::BACKGROUND);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // SetScheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    radioAbilityRecord->SetScheduler(scheduler);

    // top float ability move to defult stack
    ref = stackManager_->MaximizeMultiWindow(radioAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(ERR_OK, ref);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function: MinimizeMultiWindow and MaximizeMultiWindow
 * SubFunction: moving other applications to the floating window stack or Full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: When the floating window application calls MinimizeMultiWindow, it will switch to the background
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_046, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordlauncher = stackManager_->GetCurrentTopAbility();
    topAbilityRecordlauncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(radioAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::BACKGROUND);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // SetScheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    musicAbilityRecord->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);

    OHOS::sptr<MockAbilityScheduler> schedulerluncher(new MockAbilityScheduler());
    EXPECT_CALL(*schedulerluncher, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordlauncher->SetScheduler(schedulerluncher);

    auto getFocusChangeFlag = [](bool flag) { EXPECT_TRUE(flag); };

    EXPECT_CALL(*schedulerluncher, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(getFocusChangeFlag));

    EXPECT_FALSE(musicAbilityRecord->IsToEnd());

    ref = stackManager_->MinimizeMultiWindow(radioAbilityRecord->GetMissionRecordId());
    EXPECT_EQ(ERR_OK, ref);

    EXPECT_TRUE(musicAbilityRecord->IsToEnd());
}

/*
 * Feature: AaFwk
 * Function:ChangeFocusAbility
 * SubFunction: Change the focus state of two active states
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Switch from the focus of the floating window to the luncher
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_047, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    stackManager_->CompleteActive(topAbilityRecordLuncher);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    stackManager_->CompleteActive(topAbilityRecordRadio);

    // set luncher scheduler
    OHOS::sptr<MockAbilityScheduler> schedulerluncher(new MockAbilityScheduler());
    EXPECT_CALL(*schedulerluncher, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordLuncher->SetScheduler(schedulerluncher);

    auto getFocusChangeFlag = [](bool flag) { EXPECT_TRUE(flag); };
    auto loseFocusChangeFlag = [](bool flag) { EXPECT_FALSE(flag); };

    EXPECT_CALL(*schedulerluncher, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(getFocusChangeFlag));

    // set float windows scheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordRadio->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(loseFocusChangeFlag));

    // make sure both are active
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    // luncher get the focus
    ref = stackManager_->ChangeFocusAbility(topAbilityRecordRadio->GetToken(), topAbilityRecordLuncher->GetToken());
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function:ChangeFocusAbility
 * SubFunction: Change the focus state of two active states
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Switch focus between two floating windows
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_048, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    stackManager_->CompleteActive(topAbilityRecordLuncher);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    stackManager_->CompleteActive(topAbilityRecordRadio);

    // in float stact
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    stackManager_->CompleteActive(musicAbilityRecord);

    // Make sure to start in the floating window stack
    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // set MusicAbility scheduler
    OHOS::sptr<MockAbilityScheduler> schedulerMusic(new MockAbilityScheduler());
    EXPECT_CALL(*schedulerMusic, AsObject()).WillRepeatedly(Return(nullptr));
    musicAbilityRecord->SetScheduler(schedulerMusic);

    auto loseFocusChangeFlag = [](bool flag) { EXPECT_FALSE(flag); };

    EXPECT_CALL(*schedulerMusic, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(loseFocusChangeFlag));

    // set RadioAbility scheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordRadio->SetScheduler(scheduler);

    // make sure both are active
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    // RadioAbility get the focus
    ref = stackManager_->ChangeFocusAbility(musicAbilityRecord->GetToken(), topAbilityRecordRadio->GetToken());
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function:ChangeFocusAbility
 * SubFunction: Change the focus state of two active states
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:The application of floating window gets the focus
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_049, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    stackManager_->CompleteActive(topAbilityRecordLuncher);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    stackManager_->CompleteActive(topAbilityRecordRadio);

    // set luncher scheduler
    OHOS::sptr<MockAbilityScheduler> schedulerluncher(new MockAbilityScheduler());
    EXPECT_CALL(*schedulerluncher, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordLuncher->SetScheduler(schedulerluncher);

    auto getFocusChangeFlag = [](bool flag) { EXPECT_TRUE(flag); };
    auto loseFocusChangeFlag = [](bool flag) { EXPECT_FALSE(flag); };

    EXPECT_CALL(*schedulerluncher, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(getFocusChangeFlag))
        .WillOnce(testing::Invoke(loseFocusChangeFlag));

    // set float windows scheduler
    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordRadio->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, NotifyTopActiveAbilityChanged(testing::_))
        .Times(testing::AtLeast(1))
        .WillOnce(testing::Invoke(loseFocusChangeFlag))
        .WillOnce(testing::Invoke(getFocusChangeFlag));

    // make sure both are active
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    // luncher get the focus
    ref = stackManager_->ChangeFocusAbility(topAbilityRecordRadio->GetToken(), topAbilityRecordLuncher->GetToken());
    EXPECT_EQ(ERR_OK, ref);

    // make sure both are active
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    // Click the floating window again to get the focus
    ref = stackManager_->ChangeFocusAbility(topAbilityRecordLuncher->GetToken(), topAbilityRecordRadio->GetToken());
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function:GetFloatingMissions
 * SubFunction: Get the information of floating window application
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Get the information of floating window application..
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_050, TestSize.Level1)
{
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // Starting another singletop application should start on the floating window stack
    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // get missions
    std::vector<AbilityMissionInfo> list;
    stackManager_->GetFloatingMissions(list);
    int size = list.size();
    EXPECT_EQ(size, 1);

    AbilityMissionInfo info = list.front();
    EXPECT_EQ(info.baseAbility.abilityName_, "MusicAbility");
    EXPECT_EQ(info.topAbility.abilityName_, "MusicTopAbility");
}

/*
 * Feature: AaFwk
 * Function:GetFloatingMissions
 * SubFunction: Get the information of floating window application
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Get the information of floating window application
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_051, TestSize.Level1)
{
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // Starting another singletop application should start on the floating window stack
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    std::vector<AbilityMissionInfo> list;
    stackManager_->GetFloatingMissions(list);
    int size = list.size();
    EXPECT_EQ(size, 1);

    AbilityMissionInfo info = list.front();
    EXPECT_EQ(info.baseAbility.abilityName_, "MusicAbility");
    EXPECT_EQ(info.topAbility.abilityName_, "MusicTopAbility");
}

/*
 * Feature: AaFwk
 * Function:GetFloatingMissions
 * SubFunction: Get the information of floating window application
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Get the information of floating window application
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_052, TestSize.Level1)
{
    auto musicTonAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicTonAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    topAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    MissionOption missionOption;
    missionOption.missionId = topAbilityRecord->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToFloatingStack(missionOption);
    EXPECT_EQ(ref, ERR_OK);

    EXPECT_EQ(topAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // Starting another singletop application should start on the floating window stack
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto musicAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(musicAbilityRecord);
    musicAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(musicAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto radioAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(radioAbilityRecord);
    radioAbilityRecord->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(radioAbilityRecord->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    std::vector<AbilityMissionInfo> list;
    stackManager_->GetFloatingMissions(list);
    int size = list.size();
    EXPECT_EQ(size, 1);

    AbilityMissionInfo info = list.front();
    EXPECT_EQ(info.baseAbility.abilityName_, "MusicAbility");
    EXPECT_EQ(info.topAbility.abilityName_, "RadioAbility");
}

/*
 * Feature: AaFwk
 * Function:GetFloatingMissions
 * SubFunction: Get the information of floating window application
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Get the information of floating window application
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_053, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::vector<AbilityMissionInfo> list;
    stackManager_->GetFloatingMissions(list);
    int size = list.size();
    EXPECT_EQ(size, 1);

    AbilityMissionInfo info = list.front();
    EXPECT_EQ(info.topAbility.abilityName_, "RadioAbility");
    EXPECT_EQ(info.baseAbility.abilityName_, "RadioAbility");
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionById
 * SubFunction: delete mission by id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: 01
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_054, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music2AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music2AbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    GTEST_LOG_(INFO) << "firstTopAbility :" << firstTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "secondTopAbility :" << secondTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "topAbilityRecordRadio :" << topAbilityRecordRadio->GetMissionRecordId();

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(secondTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    auto missionID = firstTopAbility->GetMissionRecordId();
    ref = stackManager_->RemoveMissionById(missionID);
    EXPECT_EQ(ERR_OK, ref);

    auto missionRecord = stackManager_->GetMissionRecordById(missionID);
    EXPECT_TRUE(missionRecord == nullptr);

    // luncher stack move to top
    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(currentStack != nullptr);
    EXPECT_EQ(currentStack->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function:OnAbilityDied
 * SubFunction: ability died
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:02
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_055, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music2AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music2AbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    GTEST_LOG_(INFO) << "firstTopAbility :" << firstTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "secondTopAbility :" << secondTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "topAbilityRecordRadio :" << topAbilityRecordRadio->GetMissionRecordId();

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(secondTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    // let ability died
    stackManager_->OnAbilityDied(firstTopAbility);

    EXPECT_TRUE(secondTopAbility != nullptr);
    EXPECT_EQ(firstTopAbility->GetAbilityState(), OHOS::AAFwk::INITIAL);
}

/*
 * Feature: AaFwk
 * Function:OnAbilityDied
 * SubFunction: ability died
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:03 Delete the defult stack and pull it up again
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_056, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music2AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music2AbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    GTEST_LOG_(INFO) << "firstTopAbility :" << firstTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "secondTopAbility :" << secondTopAbility->GetMissionRecordId();
    GTEST_LOG_(INFO) << "topAbilityRecordRadio :" << topAbilityRecordRadio->GetMissionRecordId();

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(secondTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);

    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    firstTopAbility->SetScheduler(scheduler);

    EXPECT_CALL(*scheduler, ScheduleRestoreAbilityState(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*scheduler, ScheduleAbilityTransaction(testing::_, testing::_)).Times(1);
    // let ability died
    stackManager_->OnAbilityDied(firstTopAbility);

    EXPECT_TRUE(secondTopAbility != nullptr);
    EXPECT_EQ(firstTopAbility->GetAbilityState(), OHOS::AAFwk::INITIAL);

    // restart
    ref = stackManager_->StartAbility(music1AbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionById
 * SubFunction: delete mission by id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: 04 remove radioAbility mission.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_057, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    auto missionID = topAbilityRecordRadio->GetMissionRecordId();
    ref = stackManager_->RemoveMissionById(missionID);
    EXPECT_EQ(ERR_OK, ref);

    auto missionRecord = stackManager_->GetMissionRecordById(missionID);
    EXPECT_TRUE(missionRecord == nullptr);

    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(currentStack != nullptr);
    EXPECT_EQ(currentStack->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionById
 * SubFunction: delete mission by id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: 05 ability died RadioAbility.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_058, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    // let ability died
    stackManager_->OnAbilityDied(topAbilityRecordRadio);

    EXPECT_EQ(topAbilityRecordRadio->GetAbilityState(), OHOS::AAFwk::INITIAL);
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionById
 * SubFunction: delete mission by id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: 06 restart RadioAbility, it will be call onrestore
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_059, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    // float flag
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), FLOATING_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    OHOS::sptr<MockAbilityScheduler> scheduler(new MockAbilityScheduler());
    EXPECT_CALL(*scheduler, AsObject()).WillRepeatedly(Return(nullptr));
    topAbilityRecordRadio->SetScheduler(scheduler);

    // let ability died
    stackManager_->OnAbilityDied(topAbilityRecordRadio);

    auto currentStack = stackManager_->GetCurrentMissionStack();
    EXPECT_TRUE(currentStack != nullptr);
    EXPECT_EQ(currentStack->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);

    EXPECT_EQ(topAbilityRecordRadio->GetAbilityState(), OHOS::AAFwk::INITIAL);

    // restart
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
}

/*
 * Feature: AaFwk
 * Function:IsSplitScreenStack
 * SubFunction: Is Split Screen Stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Is Split Screen Stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_060, TestSize.Level1)
{
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto musicId = topAbilityRecordMusic->GetMissionRecord()->GetMissionRecordId();
    GTEST_LOG_(INFO) << "musicId :" << musicId;
    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_TRUE(stackManager_->IsExistSplitScreenStack());
    EXPECT_TRUE(stackManager_->IsSplitScreenStack(topAbilityRecordRadio->GetMissionStackId()));
    EXPECT_TRUE(stackManager_->IsSplitScreenStack(topAbilityRecordMusic->GetMissionStackId()));

    auto missionId = topAbilityRecordRadio->GetMissionRecord()->GetMissionRecordId();
    GTEST_LOG_(INFO) << "radioId :" << missionId;

    ref = stackManager_->RemoveMissionById(missionId);
    EXPECT_EQ(ERR_OK, ref);

    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_FALSE(topAbilityRecord);
}

/*
 * Feature: AaFwk
 * Function:TerminateAbility
 * SubFunction: Remove split screen ability, active ability is full screen stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Remove split screen ability, active ability is full screen stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_061, TestSize.Level1)
{
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "music", "com.ix.hiMusic");
    auto ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    int radioStackId = 4;
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    topAbilityRecordRadio->SetMissionStackId(radioStackId);

    EXPECT_TRUE(stackManager_->IsExistSplitScreenStack());
    EXPECT_TRUE(stackManager_->IsSplitScreenStack(topAbilityRecordRadio->GetMissionStackId()));

    ref = stackManager_->TerminateAbility(topAbilityRecordRadio->GetToken(), -1, nullptr);
    EXPECT_EQ(ERR_OK, ref);

    auto topAbilityRecord = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecord);
    auto stackId = topAbilityRecord->GetMissionStackId();
    EXPECT_TRUE(stackManager_->IsFullScreenStack(stackId));
}

/*
 * Feature: AaFwk
 * Function:MoveMissionToTop
 * SubFunction: move mission to top
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: move mission to top,view split screen stack information
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_062, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "musicTop", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto missionId = abilityRecordWorld->GetMissionRecord()->GetMissionRecordId();
    stackManager_->isMultiWinMoving_ = false;
    stackManager_->MoveMissionToTop(missionId);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), abilityRecordWorld);
}

/*
 * Feature: AaFwk
 * Function:MoveMissionToTop
 * SubFunction: move mission to top
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: move mission to top,view split screen stack information
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_063, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "musicTop", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto phoneAbilityRequest = GenerateAbilityRequest("device", "PhoneAbility1", "hiPhone", "com.ix.hiPhone");
    ref = stackManager_->StartAbility(phoneAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordPhone = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordPhone);
    topAbilityRecordPhone->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto missionId = topAbilityRecordMusic->GetMissionRecord()->GetMissionRecordId();
    stackManager_->isMultiWinMoving_ = false;
    stackManager_->MoveMissionToTop(missionId);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), topAbilityRecordMusic);
}

/*
 * Feature: AaFwk
 * Function:MoveMissionToEnd
 * SubFunction: move mission to end
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: move mission to end,view split screen stack information
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_064, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto phoneAbilityRequest = GenerateAbilityRequest("device", "PhoneAbility1", "hiPhone", "com.ix.hiPhone");
    ref = stackManager_->StartAbility(phoneAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordPhone = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordPhone);
    topAbilityRecordPhone->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "musicTop", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);

    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stackManager_->isMultiWinMoving_ = false;
    sptr<Token> token = topAbilityRecordMusic->GetToken();
    EXPECT_TRUE(token);
    stackManager_->MoveMissionToEnd(token, true);

    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), abilityRecordWorld);
}

/*
 * Feature: AaFwk
 * Function:MoveMissionToEnd
 * SubFunction: move mission to end
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: move mission to end,view split screen stack information
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_065, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto phoneAbilityRequest = GenerateAbilityRequest("device", "PhoneAbility1", "hiPhone", "com.ix.hiPhone");
    ref = stackManager_->StartAbility(phoneAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordPhone = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordPhone);
    topAbilityRecordPhone->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "musicTop", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    stackManager_->isMultiWinMoving_ = false;
    sptr<Token> token = topAbilityRecordMusic->GetToken();
    EXPECT_TRUE(token);
    stackManager_->MoveMissionToEnd(token, true);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), abilityRecordWorld);
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionRecordById
 * SubFunction:Remove Mission Record By Id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Remove Mission Record By Id
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_067, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start float ability
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto musicAbilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(musicAbilityStartSetting);
    musicAbilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    musicAbilityRequest.startSetting = musicAbilityStartSetting;
    stackManager_->isMultiWinMoving_ = false;
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    auto missionRecordMusic = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto missionId = abilityRecordMusic->GetMissionRecordId();
    ref = stackManager_->RemoveMissionRecordById(missionId);
    EXPECT_EQ(true, ref);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), nullptr);
}

/*
 * Feature: AaFwk
 * Function:RemoveMissionRecordById
 * SubFunction:Remove Mission Record By Id
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Remove Mission Record By Id
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_068, TestSize.Level1)
{
    stackManager_->isMultiWinMoving_ = false;
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<MissionStack> targetStack = stackManager_->GetTargetMissionStack(musicTopAbilityRequest);
    EXPECT_EQ(stackManager_->currentMissionStack_, targetStack);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);

    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(stackManager_->curSysWindowMode_, SystemWindowMode::SPLITSCREEN_WINDOW_MODE);

    auto missionId = topAbilityRecordRadio->GetMissionRecordId();
    ref = stackManager_->RemoveMissionRecordById(missionId);
    EXPECT_EQ(true, ref);

    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), topAbilityRecordMusic);
}

/*
 * Feature: AaFwk
 * Function:RemoveStack
 * SubFunction:Remove Stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Remove Stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_069, TestSize.Level1)
{
    stackManager_->isMultiWinMoving_ = false;
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    std::shared_ptr<MissionStack> targetStack = stackManager_->GetTargetMissionStack(musicTopAbilityRequest);
    EXPECT_EQ(stackManager_->currentMissionStack_, targetStack);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);

    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto topMissionRecordRadio = stackManager_->GetTopMissionRecord();
    EXPECT_TRUE(topMissionRecordRadio);

    EXPECT_EQ(stackManager_->curSysWindowMode_, SystemWindowMode::SPLITSCREEN_WINDOW_MODE);

    auto stackId = topMissionRecordRadio->GetMissionStack()->GetMissionStackId();
    stackManager_->RemoveStack(stackId);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), abilityRecordWorld);
}

/*
 * Feature: AaFwk
 * Function:OnTimeOut
 * SubFunction:Start ability timeout, the previous ability state changes to Active->Inactve->Active
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:Start ability timeout, the previous ability state changes to Active->Inactve->Active
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_070, TestSize.Level1)
{
    stackManager_->isMultiWinMoving_ = false;
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    stackManager_->ActiveTopAbility(topAbilityRecordRadio);

    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), topAbilityRecordMusic);
    EXPECT_EQ(false, stackManager_->IsExistSplitScreenStack());
}

/*
 * Feature: AaFwk
 * Function:OnAbilityDied
 * SubFunction: ability died
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:ability died
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_071, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(music1AbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music2AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(music2AbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    // start split ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
    EXPECT_EQ(secondTopAbility->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);

    // let ability died
    stackManager_->OnAbilityDied(firstTopAbility);

    EXPECT_TRUE(secondTopAbility != nullptr);
    EXPECT_EQ(firstTopAbility->GetAbilityState(), OHOS::AAFwk::INITIAL);
}

/*
 * Feature: AaFwk
 * Function:OnAbilityDied
 * SubFunction: ability died
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Delete the split stack and pull it up again
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_072, TestSize.Level1)
{
    auto launcherAbilityRequest_ = GenerateAbilityRequest("device", "LauncherAbility", "launcher", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(launcherAbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordLuncher = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordLuncher);
    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music1AbilityRequest_ = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music1AbilityRequest_);
    auto firstTopAbility = stackManager_->GetCurrentTopAbility();
    firstTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto music2AbilityRequest_ = GenerateAbilityRequest("device", "MusicAbility2", "music", "com.ix.hiMusic");
    stackManager_->StartAbility(music2AbilityRequest_);
    auto secondTopAbility = stackManager_->GetCurrentTopAbility();
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    // start split ability
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;

    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordLuncher->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    firstTopAbility->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    secondTopAbility->SetAbilityState(OHOS::AAFwk::ACTIVE);

    EXPECT_EQ(topAbilityRecordLuncher->GetMissionStackId(), LAUNCHER_MISSION_STACK_ID);
    EXPECT_EQ(firstTopAbility->GetMissionStackId(), DEFAULT_MISSION_STACK_ID);
    EXPECT_EQ(secondTopAbility->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);
    EXPECT_EQ(topAbilityRecordRadio->GetMissionStackId(), SPLIT_SCREEN_MISSION_STACK_ID);

    // let ability died
    stackManager_->OnAbilityDied(firstTopAbility);

    EXPECT_TRUE(firstTopAbility != nullptr);
    EXPECT_EQ(firstTopAbility->GetAbilityState(), OHOS::AAFwk::INITIAL);

    // restart
    ref = stackManager_->StartAbility(music1AbilityRequest_);
    EXPECT_EQ(ERR_OK, ref);
    EXPECT_EQ(stackManager_->GetCurrentTopAbility(), firstTopAbility);
    EXPECT_EQ(stackManager_->GetTopAbilityOfFullScreen(), firstTopAbility);
}

/*
 * Feature: AaFwk
 * Function:MoveMissionToSplitScreenStack
 * SubFunction: Move Mission To Split Screen Stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Move Mission To Split Screen Stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_073, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start float ability
    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    auto musicAbilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(musicAbilityStartSetting);
    musicAbilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING));
    musicAbilityRequest.startSetting = musicAbilityStartSetting;
    stackManager_->isMultiWinMoving_ = false;
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    MissionOption missionOption;
    missionOption.missionId = abilityRecordMusic->GetMissionRecord()->GetMissionRecordId();
    missionOption.winModeKey = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
    ref = stackManager_->MoveMissionToSplitScreenStack(missionOption, missionOption);
    EXPECT_EQ(ref, ERR_INVALID_DATA);
}

/*
 * Feature: AaFwk
 * Function:CompleteInactive
 * SubFunction: Complete Inactive
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Complete Inactive
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_074, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    EXPECT_EQ(AbilityState::INACTIVATING, abilityRecordWorld->currentState_);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordRadio->SetMovingBackgroundFlag(true);
    topAbilityRecordRadio->SetInMovingState(false);
    stackManager_->CompleteInactive(topAbilityRecordRadio);
    EXPECT_EQ(false, topAbilityRecordRadio->IsMovingBackground());
}

/*
 * Feature: AaFwk
 * Function:CompleteActive
 * SubFunction: Complete Active
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Complete Active
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_075, TestSize.Level1)
{
    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    auto ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVATING);
    topAbilityRecordRadio->SetInMovingState(false);
    stackManager_->CompleteActive(topAbilityRecordRadio);
    EXPECT_EQ(AbilityState::ACTIVE, topAbilityRecordRadio->GetAbilityState());
}

/*
 * Feature: AaFwk
 * Function:CompleteMoveMissionToStack
 * SubFunction: Complete Move Mission To Stack
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: Complete Move Mission To Stack
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_076, TestSize.Level1)
{
    // start launcher
    auto worldAbilityRequest = GenerateAbilityRequest("device", "WorldAbility", "world", "com.ix.hiworld");
    auto ref = stackManager_->StartAbility(worldAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordWorld = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordWorld);
    abilityRecordWorld->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicTopAbilityRequest = GenerateAbilityRequest("device", "MusicTopAbility", "musicTop", "com.ix.hiTopMusic");
    ref = stackManager_->StartAbility(musicTopAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordMusic);
    topAbilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);

    // start split ability
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioSAbility", "radio", "com.ix.hiRadio");
    auto abilityStartSetting = AbilityStartSetting::GetEmptySetting();
    EXPECT_TRUE(abilityStartSetting);
    abilityStartSetting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY,
        std::to_string(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY));
    radioAbilityRequest.startSetting = abilityStartSetting;
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto mission = abilityRecordMusic->GetMissionRecord();
    auto stack = mission->GetMissionStack()->GetMissionStackId();
    EXPECT_TRUE(stackManager_->IsFullScreenStack(stack));

    auto targetStack = stackManager_->GetTargetMissionStack(radioAbilityRequest);

    stackManager_->CompleteMoveMissionToStack(mission, targetStack);

    auto mission1 = abilityRecordMusic->GetMissionRecord();
    auto stack1 = mission->GetMissionStack()->GetMissionStackId();
    EXPECT_TRUE(stackManager_->IsSplitScreenStack(stack1));
}

/*
 * Feature: AaFwk
 * Function:CheckMissionRecordInWhiteList
 * SubFunction: CheckMissionRecordInWhiteList
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: check all ability record in mission record are all on the white list.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_077, TestSize.Level1)
{
    stackManager_->SetShowOnLockScreen("com.ix.hiMusic", true);
    stackManager_->SetShowOnLockScreen("com.ix.hiRadio", false);
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto callerToken = topAbilityRecordRadio->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = abilityRecordMusic->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    auto mission = abilityRecordMusic->GetMissionRecord();
    EXPECT_TRUE(mission);
    EXPECT_FALSE(stackManager_->CheckMissionRecordInWhiteList(mission));
}

/*
 * Feature: AaFwk
 * Function:CheckMissionRecordInWhiteList
 * SubFunction: CheckMissionRecordInWhiteList
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: check all ability record in mission record are all on the white list.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_078, TestSize.Level1)
{
    stackManager_->SetShowOnLockScreen("com.ix.hiMusic", true);
    stackManager_->SetShowOnLockScreen("com.ix.hiRadio", true);
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto callerToken = topAbilityRecordRadio->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = abilityRecordMusic->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    auto mission = abilityRecordMusic->GetMissionRecord();
    EXPECT_TRUE(mission);
    EXPECT_TRUE(stackManager_->CheckMissionRecordInWhiteList(mission));
}

/*
 * Feature: AaFwk
 * Function:DeleteMissionRecordInStackOnLockScreen
 * SubFunction: DeleteMissionRecordInStackOnLockScreen
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: delete mission record in stack on lock screen.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_079, TestSize.Level1)
{
    stackManager_->SetShowOnLockScreen("com.ix.hiMusic", true);
    stackManager_->SetShowOnLockScreen("com.ix.hiRadio", false);
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto callerToken = topAbilityRecordRadio->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = abilityRecordMusic->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    auto mission = abilityRecordMusic->GetMissionRecord();
    EXPECT_TRUE(mission);
    EXPECT_FALSE(stackManager_->DeleteMissionRecordInStackOnLockScreen(mission));
}

/*
 * Feature: AaFwk
 * Function:DeleteMissionRecordInStackOnLockScreen
 * SubFunction: DeleteMissionRecordInStackOnLockScreen
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: delete mission record in stack on lock screen.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_080, TestSize.Level1)
{
    stackManager_->SetShowOnLockScreen("com.ix.hiMusic", true);
    stackManager_->SetShowOnLockScreen("com.ix.hiRadio", true);
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioAbility", "radio", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto callerToken = topAbilityRecordRadio->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = abilityRecordMusic->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    auto mission = abilityRecordMusic->GetMissionRecord();
    auto stack = mission->GetMissionStack();
    EXPECT_TRUE(mission);
    EXPECT_TRUE(stack);
    EXPECT_TRUE(stackManager_->DeleteMissionRecordInStackOnLockScreen(mission));
    EXPECT_TRUE(topAbilityRecordRadio->GetLockScreenState());
    EXPECT_FALSE(abilityRecordMusic->GetLockScreenState());
    EXPECT_FALSE(stack->IsTopMissionRecord(mission));
    EXPECT_TRUE(stack->GetMissionRecordCount() == 0);
}

/*
 * Feature: AaFwk
 * Function:RestoreMissionRecordOnLockScreen and UpdatePowerOffRecord
 * SubFunction: RestoreMissionRecordOnLockScreen and UpdatePowerOffRecord
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: StartAbility in lock screen stack.
 */
HWTEST_F(AbilityStackModuleTest, ability_stack_test_081, TestSize.Level1)
{
    stackManager_->SetShowOnLockScreen("com.ix.hiMusic", true);
    stackManager_->SetShowOnLockScreen("com.ix.hiRadio", true);
    auto radioAbilityRequest = GenerateAbilityRequest("device", "RadioTopAbility", "radio", "com.ix.hiRadio");
    auto ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    auto callerToken = topAbilityRecordRadio->GetToken();
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    EXPECT_TRUE(handler);
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());
    auto radioMission = topAbilityRecordRadio->GetMissionRecord();
    auto radioStack = radioMission->GetMissionStack();
    EXPECT_TRUE(radioMission);
    EXPECT_TRUE(radioStack);
    EXPECT_TRUE(radioStack->IsEqualStackId(DEFAULT_MISSION_STACK_ID));

    auto musicAbilityRequest = GenerateAbilityRequest("device", "MusicSAbility", "music", "com.ix.hiMusic");
    ref = stackManager_->StartAbility(musicAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    auto abilityRecordMusic = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(abilityRecordMusic);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = abilityRecordMusic->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    auto musicMission = abilityRecordMusic->GetMissionRecord();
    auto musicStack = musicMission->GetMissionStack();
    EXPECT_TRUE(musicMission);
    EXPECT_TRUE(musicStack);
    EXPECT_TRUE(musicStack->IsEqualStackId(DEFAULT_MISSION_STACK_ID));
    stackManager_->PowerOff();
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    abilityRecordMusic->SetAbilityState(OHOS::AAFwk::BACKGROUND);
    handler->RemoveEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, abilityRecordMusic->GetEventId());
    handler->RemoveEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());
    stackManager_->UpdateLockScreenState(true);
    EXPECT_TRUE(stackManager_->IsLockScreenState());
    ref = stackManager_->StartAbility(radioAbilityRequest);
    EXPECT_EQ(ERR_OK, ref);
    topAbilityRecordRadio = stackManager_->GetCurrentTopAbility();
    EXPECT_TRUE(topAbilityRecordRadio);
    topAbilityRecordRadio->SetAbilityState(OHOS::AAFwk::ACTIVE);
    callerToken = topAbilityRecordRadio->GetToken();
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, topAbilityRecordRadio->GetEventId());
    radioMission = topAbilityRecordRadio->GetMissionRecord();
    radioStack = radioMission->GetMissionStack();
    EXPECT_TRUE(radioMission);
    EXPECT_TRUE(radioStack);
    EXPECT_TRUE(radioStack->IsEqualStackId(LOCK_SCREEN_STACK_ID));
    stackManager_->TerminateAbility(topAbilityRecordRadio->GetToken(), DEFAULT_INVAL_VALUE, nullptr);
    radioMission = topAbilityRecordRadio->GetMissionRecord();
    radioStack = radioMission->GetMissionStack();
    EXPECT_TRUE(radioStack->IsEqualStackId(DEFAULT_MISSION_STACK_ID));
}
}  // namespace AAFwk
}  // namespace OHOS
