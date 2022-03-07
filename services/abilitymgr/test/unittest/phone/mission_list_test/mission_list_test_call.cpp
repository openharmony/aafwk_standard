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
#include "ability_info.h"
#include "mission.h"
#include "mission_list.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
    const std::string DEVICE_ID = "15010038475446345206a332922cb765";
    const std::string INVALID_DEVICE_ID = "88880038475446345206a332922c8888";
    const std::string BUNDLE_NAME = "testBundle";
    const std::string INVALID_BUNDLE_NAME = "invalidBundle";
    const std::string NAME = ".testMainAbility";
    const std::string INVALID_NAME = "invalid.testMainAbility";
}

namespace OHOS {
namespace AAFwk {
class MissionListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionListTest::SetUpTestCase(void)
{}
void MissionListTest::TearDownTestCase(void)
{}
void MissionListTest::SetUp(void)
{}
void MissionListTest::TearDown(void)
{}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_name_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.deviceId = DEVICE_ID;
    abilityInfo.bundleName = BUNDLE_NAME;
    abilityInfo.name = NAME;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    auto mission = std::make_shared<Mission>(0, abilityRecord, "launcher");
    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    AppExecFwk::ElementName localElement(DEVICE_ID, BUNDLE_NAME, NAME);
    EXPECT_EQ(abilityRecord, missionList->GetAbilityRecordByName(localElement));
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_name_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.deviceId = DEVICE_ID;
    abilityInfo.bundleName = BUNDLE_NAME;
    abilityInfo.name = NAME;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    auto mission = std::make_shared<Mission>(0, abilityRecord, "launcher");
    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    AppExecFwk::ElementName localElement(INVALID_DEVICE_ID, BUNDLE_NAME, NAME);
    EXPECT_EQ(nullptr, missionList->GetAbilityRecordByName(localElement));
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_name_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.deviceId = DEVICE_ID;
    abilityInfo.bundleName = BUNDLE_NAME;
    abilityInfo.name = NAME;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    auto mission = std::make_shared<Mission>(0, abilityRecord, "launcher");
    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    AppExecFwk::ElementName localElement(DEVICE_ID, INVALID_BUNDLE_NAME, NAME);
    EXPECT_EQ(nullptr, missionList->GetAbilityRecordByName(localElement));
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_name_004, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.deviceId = DEVICE_ID;
    abilityInfo.bundleName = BUNDLE_NAME;
    abilityInfo.name = NAME;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

    auto mission = std::make_shared<Mission>(0, abilityRecord, "launcher");
    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    AppExecFwk::ElementName localElement(DEVICE_ID, BUNDLE_NAME, INVALID_NAME);
    EXPECT_EQ(nullptr, missionList->GetAbilityRecordByName(localElement));
}
}  // namespace AAFwk
}  // namespace OHOS