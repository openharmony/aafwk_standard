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
#include "ability_info.h"
#include "mission.h"
#include "mission_list.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName);

    Want want_ {};
    AbilityInfo abilityInfo_ {};
    ApplicationInfo appInfo_ {};
};

void MissionListTest::SetUpTestCase(void)
{}
void MissionListTest::TearDownTestCase(void)
{}
void MissionListTest::SetUp(void)
{}
void MissionListTest::TearDown(void)
{}

AbilityRequest MissionListTest::GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
    const std::string &appName, const std::string &bundleName)
{
    ElementName element(deviceName, abilityName, bundleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    ApplicationInfo appinfo;
    appinfo.name = appName;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(nullptr);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
    EXPECT_TRUE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_002, TestSize.Level1)
{
    // 1. create a mission
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(nullptr, "");
    // 2. create a mission list
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    // 3. add the mission
    missionList->AddMissionToTop(mission);
    // 4. try to add the mission again
    missionList->AddMissionToTop(mission);
    EXPECT_EQ(mission, missionList->GetTopMission());
    EXPECT_EQ(1, missionList->GetAllMissions().size());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_003, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission);
    EXPECT_EQ(mission, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_005, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_006, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_007, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    // 1. create and add mission1
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission1);
    // 2. create and add mission2
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission2);

    // 3. put agian mission1
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());
    EXPECT_EQ(2, missionList->GetAllMissions().size());
}

/*
 * Feature: MissionList
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMission
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMission
 */
HWTEST_F(MissionListTest, mission_list_remove_mission_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMission
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMission
 */
HWTEST_F(MissionListTest, mission_list_remove_mission_002, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(abilityRecord, "name1");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(mission, mission1);
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(abilityRecord, "name1");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(nullptr, mission1);
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(nullptr, mission1);
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByToken
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByToken
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_token_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(nullptr, missionList->GetAbilityRecordByToken(nullptr));
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByToken
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByToken
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_token_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    EXPECT_EQ(abilityRecord, missionList->GetAbilityRecordByToken(abilityRecord->GetToken()));
}

/*
 * Feature: MissionList
 * Function: RemoveMissionByAbilityRecord
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMissionByAbilityRecord
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMissionByAbilityRecord
 */
HWTEST_F(MissionListTest, mission_list_remove_ability_record_by_token_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    missionList->RemoveMissionByAbilityRecord(abilityRecord);

    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: IsEmpty
 * SubFunction: NA
 * FunctionPoints: MissionList IsEmpty
 * EnvConditions: NA
 * CaseDescription: Verify IsEmpty
 */
HWTEST_F(MissionListTest, mission_list_is_empty_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: IsEmpty
 * SubFunction: NA
 * FunctionPoints: MissionList IsEmpty
 * EnvConditions: NA
 * CaseDescription: Verify IsEmpty
 */
HWTEST_F(MissionListTest, mission_list_is_empty_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    EXPECT_FALSE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    EXPECT_EQ(MissionListType::CURRENT, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_002, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(MissionListType::CURRENT, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_003, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    EXPECT_EQ(MissionListType::DEFAULT_STANDARD, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_004, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    EXPECT_EQ(MissionListType::DEFAULT_SINGLE, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_005, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    EXPECT_EQ(MissionListType::LAUNCHER, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetLauncherRoot
 * SubFunction: NA
 * FunctionPoints: MissionList GetLauncherRoot
 * EnvConditions: NA
 * CaseDescription: Verify GetLauncherRoot
 */
HWTEST_F(MissionListTest, mission_list_get_launcher_root_001, TestSize.Level1)
{
    // 1. create mission1
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetLauncherRoot();
    auto mission = std::make_shared<Mission>(abilityRecord, "launcher");

    // 2. create mission2
    AppExecFwk::AbilityInfo abilityInfo1;
    Want want1;
    AppExecFwk::ApplicationInfo applicationInfo1;
    std::shared_ptr<AbilityRecord> abilityRecord1
        = std::make_shared<AbilityRecord>(want1, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    auto mission1 = std::make_shared<Mission>(abilityRecord1, "launcher");

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);
    missionList->AddMissionToTop(mission1);

    EXPECT_EQ(abilityRecord, missionList->GetLauncherRoot());
}
}  // namespace AAFwk
}  // namespace OHOS