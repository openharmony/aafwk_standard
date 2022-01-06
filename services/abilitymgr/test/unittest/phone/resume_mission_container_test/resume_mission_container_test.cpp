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
#include "resume_mission_container.h"
#undef private
#include "ability_manager_service.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionRecordTest::SetUpTestCase(void)
{}
void MissionRecordTest::TearDownTestCase(void)
{}
void MissionRecordTest::SetUp(void)
{}
void MissionRecordTest::TearDown(void)
{}

/*
 * Feature: ResumeMissionContainer
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: Resume Mission Container
 * EnvConditions:NA
 * CaseDescription: Resume Mission Container UT.
 */
HWTEST_F(MissionRecordTest, Save_01, TestSize.Level1)
{

    auto eventLoop = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto handler =
        std::make_shared<AbilityEventHandler>(eventLoop, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    auto resumeMissionContainer = std::make_shared<ResumeMissionContainer>(handler);
    resumeMissionContainer->missionMaps_.clear();
    std::string bundleName = "com.ix.aafwk.moduletest";
    auto mission = std::make_shared<MissionRecord>(bundleName);
    resumeMissionContainer->Save(mission);
    EXPECT_TRUE(1 == resumeMissionContainer->missionMaps_.size());
}

/*
 * Feature: ResumeMissionContainer
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: Resume Mission Container
 * EnvConditions:NA
 * CaseDescription: Resume Mission Container UT.
 */
HWTEST_F(MissionRecordTest, Resume_01, TestSize.Level1)
{
    auto eventLoop = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto handler =
        std::make_shared<AbilityEventHandler>(eventLoop, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    auto resumeMissionContainer = std::make_shared<ResumeMissionContainer>(handler);
    resumeMissionContainer->missionMaps_.clear();
    std::string bundleName = "com.ix.aafwk.moduletest";
    auto mission = std::make_shared<MissionRecord>(bundleName);

    auto missionId = mission->GetMissionRecordId();
    std::shared_ptr<MissionRecord> backup;
    backup = std::make_shared<MissionRecord>(mission);
    resumeMissionContainer->missionMaps_.emplace(missionId, backup);

    resumeMissionContainer->Resume(mission);
    EXPECT_TRUE(0 == resumeMissionContainer->missionMaps_.size());
}

/*
 * Feature: ResumeMissionContainer
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: Resume Mission Container
 * EnvConditions:NA
 * CaseDescription: Resume Mission Container UT.
 */
HWTEST_F(MissionRecordTest, Remove_01, TestSize.Level1)
{
    auto eventLoop = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto handler =
        std::make_shared<AbilityEventHandler>(eventLoop, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    auto resumeMissionContainer = std::make_shared<ResumeMissionContainer>(handler);
    resumeMissionContainer->missionMaps_.clear();
    std::string bundleName = "com.ix.aafwk.moduletest";
    auto mission = std::make_shared<MissionRecord>(bundleName);

    auto missionId = mission->GetMissionRecordId();
    std::shared_ptr<MissionRecord> backup;
    backup = std::make_shared<MissionRecord>(mission);
    resumeMissionContainer->missionMaps_.emplace(missionId, backup);

    resumeMissionContainer->Remove(missionId);
    EXPECT_TRUE(0 == resumeMissionContainer->missionMaps_.size());
}

/*
 * Feature: ResumeMissionContainer
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: Resume Mission Container
 * EnvConditions:NA
 * CaseDescription: Resume Mission Container UT.
 */
HWTEST_F(MissionRecordTest, IsResume_01, TestSize.Level1)
{
    auto eventLoop = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto handler =
        std::make_shared<AbilityEventHandler>(eventLoop, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    auto resumeMissionContainer = std::make_shared<ResumeMissionContainer>(handler);
    resumeMissionContainer->missionMaps_.clear();
    std::string bundleName = "com.ix.aafwk.moduletest";
    auto mission = std::make_shared<MissionRecord>(bundleName);

    auto missionId = mission->GetMissionRecordId();
    std::shared_ptr<MissionRecord> backup;
    backup = std::make_shared<MissionRecord>(mission);
    resumeMissionContainer->missionMaps_.emplace(missionId, backup);

    EXPECT_EQ(true, resumeMissionContainer->IsResume(missionId));
}

/*
 * Feature: ResumeMissionContainer
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: Resume Mission Container
 * EnvConditions:NA
 * CaseDescription: Resume Mission Container UT.
 */
HWTEST_F(MissionRecordTest, IsResume_02, TestSize.Level1)
{
    auto eventLoop = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto handler =
        std::make_shared<AbilityEventHandler>(eventLoop, OHOS::DelayedSingleton<AbilityManagerService>::GetInstance());
    auto resumeMissionContainer = std::make_shared<ResumeMissionContainer>(handler);
    resumeMissionContainer->missionMaps_.clear();
    std::string bundleName = "com.ix.aafwk.moduletest";
    auto mission = std::make_shared<MissionRecord>(bundleName);
    auto missionId = mission->GetMissionRecordId();

    EXPECT_EQ(false, resumeMissionContainer->IsResume(missionId));
}

}  // namespace AAFwk
}  // namespace OHOS