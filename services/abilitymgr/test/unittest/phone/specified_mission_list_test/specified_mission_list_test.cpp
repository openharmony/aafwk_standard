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
#include "mission_list.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class SpecifiedMissionListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SpecifiedMissionListTest::SetUpTestCase(void)
{}
void SpecifiedMissionListTest::TearDownTestCase(void)
{}
void SpecifiedMissionListTest::SetUp(void)
{}
void SpecifiedMissionListTest::TearDown(void)
{}

/*
 * Feature: MissionList
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(SpecifiedMissionListTest, mission_list_GetMissionBySpecifiedFlag_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    Want want;
    auto list = missionList->GetMissionBySpecifiedFlag(want, "flag");

    EXPECT_EQ(nullptr, list);
}

/*
 * Feature: MissionList
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(SpecifiedMissionListTest, mission_list_GetMissionBySpecifiedFlag_002, TestSize.Level1)
{
    // 1. create mission1
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetLauncherRoot();
    std::shared_ptr<Mission> mission = nullptr;

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto list = missionList->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(nullptr, list);
}

/*
 * Feature: MissionList
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(SpecifiedMissionListTest, mission_list_GetMissionBySpecifiedFlag_003, TestSize.Level1)
{
    // 1. create mission1
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetLauncherRoot();
    abilityRecord = nullptr;
    std::shared_ptr<Mission> mission;

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto list = missionList->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(nullptr, list);
}

/*
 * Feature: MissionList
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(SpecifiedMissionListTest, mission_list_GetMissionBySpecifiedFlag_004, TestSize.Level1)
{
    // 1. create mission1
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetLauncherRoot();
    abilityRecord->SetSpecifiedFlag("flag");
    std::shared_ptr<Mission> mission;

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto list = missionList->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, list);
}
}  // namespace AAFwk
}  // namespace OHOS
