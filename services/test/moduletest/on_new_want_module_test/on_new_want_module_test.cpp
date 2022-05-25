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

#include <string_view>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#define private public
#include "ability_manager_service.h"
#include "mission_list_manager.h"
#undef private
#include "ability_util.h"
#include "mission_list.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::iface_cast;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::Return;

namespace OHOS {
namespace AAFwk {
class OnNewWantModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    AbilityInfo CreateAbilityInfo(const std::string &name, const std::string &appName, const std::string &bundleName,
        const std::string &moduleName, bool mode);
    ApplicationInfo CreateAppInfo(const std::string &appName, const std::string &name);
    inline static std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
};

AbilityInfo OnNewWantModuleTest::CreateAbilityInfo(const std::string &name, const std::string &appName,
    const std::string &bundleName, const std::string &moduleName, bool mode)
{
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.name = name;
    abilityInfo.applicationName = appName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.applicationInfo.bundleName = bundleName;
    abilityInfo.applicationName = "com.ix.hiMusic";
    abilityInfo.applicationInfo.name = "com.ix.hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    if (mode) {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    } else {
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }

    return abilityInfo;
}

ApplicationInfo OnNewWantModuleTest::CreateAppInfo(const std::string &appName, const std::string &bundleName)
{
    ApplicationInfo appInfo;
    appInfo.name = appName;
    appInfo.bundleName = bundleName;

    return appInfo;
}

void OnNewWantModuleTest::SetUpTestCase()
{
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
}

void OnNewWantModuleTest::TearDownTestCase()
{
    abilityMgrServ_->OnStop();
}

void OnNewWantModuleTest::SetUp()
{}

void OnNewWantModuleTest::TearDown()
{}

/*
 * Feature: MissionListManager
 * Function: Startability
 * SubFunction: NA
 * FunctionPoints: NA
 * CaseDescription:Start the singleton ability multiple times, and the ability is marked with newwant
 */
HWTEST_F(OnNewWantModuleTest, Startability_001, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);

    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";
    std::string moduleName = "entry";

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility", "entry");
    want.SetElement(element);

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, appName, bundleName, moduleName, true);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND);

    std::string missionName = AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);

    missionListManager->Init();
    missionListManager->launcherList_->AddMissionToTop(mission);
    missionListManager->StartAbility(abilityRequest);

    EXPECT_TRUE(abilityRecord->IsNewWant());
}

/*
 * Feature: MissionListManager
 * Function: Startability
 * SubFunction: NA
 * FunctionPoints: NA
 * CaseDescription:Start standard ability multiple times, ability has no newwant mark
 */
HWTEST_F(OnNewWantModuleTest, Startability_002, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);

    std::string abilityName = "MusicAbility";
    std::string appName = "test_app";
    std::string bundleName = "com.ix.hiMusic";
    std::string moduleName = "entry";

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility", "entry");
    want.SetElement(element);

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = CreateAbilityInfo(abilityName, appName, bundleName, moduleName, false);
    abilityRequest.appInfo = CreateAppInfo(appName, bundleName);

    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::FOREGROUND);

    std::string missionName = AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);

    missionListManager->Init();
    missionListManager->launcherList_->AddMissionToTop(mission);
    missionListManager->StartAbility(abilityRequest);

    EXPECT_EQ(abilityRecord->IsNewWant(), false);
}
}  // namespace AAFwk
}  // namespace OHOS
