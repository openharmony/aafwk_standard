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
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#include "ams_configuration_parameter.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "os_account_info.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class MissionDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MissionDumpTest::SetUpTestCase()
{}

void MissionDumpTest::TearDownTestCase()
{}

void MissionDumpTest::SetUp()
{}

void MissionDumpTest::TearDown()
{}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_Mission_List_001, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    EXPECT_TRUE(missionList);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "NORMAL";
    missionListManager->DumpMissionList(info, isClient, args);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("Current");
    EXPECT_TRUE(idx != string::npos);

    std::string::size_type idx1;
    idx1 = result.find("lockedState");
    EXPECT_TRUE(idx1 != string::npos);

    std::string::size_type idx2;
    idx2 = result.find("AbilityRecord");
    EXPECT_TRUE(idx2 != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_Mission_List_002, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    EXPECT_TRUE(missionList);
    missionListManager->launcherList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "LAUNCHER";
    missionListManager->DumpMissionList(info, isClient, args);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("launcher");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("lockedState");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_Mission_List_003, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    EXPECT_TRUE(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "DEFAULT_STANDARD";
    missionListManager->DumpMissionList(info, isClient, args);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("stand");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("lockedState");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_Mission_List_004, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    EXPECT_TRUE(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "DEFAULT_SINGLE";
    missionListManager->DumpMissionList(info, isClient, args);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("single");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("lockedState");
    EXPECT_TRUE(idx != string::npos);

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionListByRecordId
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_MissionList_ByRecordId_001, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);
    abilityRecord->recordId_ = 101;

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    EXPECT_TRUE(missionList);
    missionListManager->launcherList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::vector<std::string> params;
    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecord->recordId_, params);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionListByRecordId
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_MissionList_ByRecordId_002, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);
    abilityRecord->recordId_ = 101;

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    EXPECT_TRUE(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::vector<std::string> params;
    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecord->recordId_, params);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionListByRecordId
 * SubFunction: NA
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(MissionDumpTest, Dump_MissionList_ByRecordId_003, TestSize.Level1)
{
    int userId = 100;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    EXPECT_TRUE(missionListManager);
    missionListManager->Init();

    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    int requestCode = -1;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, requestCode);
    EXPECT_TRUE(abilityRecord);
    abilityRecord->recordId_ = 101;

    int32_t id = 1;
    std::string missionName = "missionName";
    auto mission = std::make_shared<Mission>(id, abilityRecord, missionName);
    EXPECT_TRUE(mission);

    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    EXPECT_TRUE(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionList->AddMissionToTop(mission);

    std::vector<std::string> info;
    bool isClient = true;
    std::vector<std::string> params;
    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecord->recordId_, params);
    EXPECT_TRUE(info.size() != 0);

    std::string result = "";
    std::string::size_type idx;
    for (auto it : info) {
        result += it + "\n";
    }

    idx = result.find("AbilityRecord");
    EXPECT_TRUE(idx != string::npos);
}
}  // namespace AAFwk
}  // namespace OHOS
