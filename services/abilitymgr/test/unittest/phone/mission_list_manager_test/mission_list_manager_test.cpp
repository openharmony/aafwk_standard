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
#include "ability_info.h"
#include "mission.h"
#include "mission_list_manager.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionListManagerTest::SetUpTestCase(void)
{}
void MissionListManagerTest::TearDownTestCase(void)
{}
void MissionListManagerTest::SetUp(void)
{}
void MissionListManagerTest::TearDown(void)
{}

/*
 * Feature: MissionListManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartAbility
 */
HWTEST_F(MissionListManagerTest, StartAbility_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);

    std::shared_ptr<AbilityRecord> currentTopAbility;
    std::shared_ptr<AbilityRecord> callerAbility;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;

    auto result = missionListManager->StartAbility(currentTopAbility, callerAbility, abilityRequest);

    EXPECT_EQ(0, result);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->launcherList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;

    ability->SetSpecifiedFlag("flag");
    missionListManager->defaultSingleList_->AddMissionToTop(mission);
    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->launcherList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->defaultStandardList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    ability->SetIsNewWant(false);

    AbilityRequest abilityRequest;
    abilityRequest.callerToken = ability->GetToken();
    missionListManager->EnqueueWaittingAbility(abilityRequest);
    missionListManager->defaultStandardList_->AddMissionToTop(mission);

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->IsNewWant(), true);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetIsNewWant(false);

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->IsNewWant(), false);
    missionListManager.reset();
}
}  // namespace AAFwk
}  // namespace OHOS
