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

#include "hilog_wrapper.h"
#define private public
#include "mission_list_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const std::string STRING_EMPTY = "";
const std::string STRING_PROCESS_NAME = "process_name";
constexpr size_t SIZE_ONE = 1;
const int32_t RECORD_ID = 1024;
const int32_t MISSION_ID = 2048;
}  // namespace

class MissionListManagerDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MissionListManagerDumpTest::SetUpTestCase(void)
{}

void MissionListManagerDumpTest::TearDownTestCase(void)
{}

void MissionListManagerDumpTest::SetUp()
{}

void MissionListManagerDumpTest::TearDown()
{}

/**
 * @tc.name: MissionListManager_DumpMissionList_0100
 * @tc.desc: DumpMissionList
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(MissionListManagerDumpTest, MissionListManager_DumpMissionList_0100, TestSize.Level1)
{
    HILOG_INFO("MissionListManager_DumpMissionList_0100 start");

    auto missionListManager = std::make_shared<MissionListManager>(USER_ID);
    EXPECT_NE(missionListManager, nullptr);

    std::vector<std::string> info;
    bool isClient = false;
    std::string args = STRING_EMPTY;
    missionListManager->DumpMissionList(info, isClient, args);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("MissionListManager_DumpMissionList_0100 end");
}

/**
 * @tc.name: MissionListManager_DumpMissionListByRecordId_0100
 * @tc.desc: DumpMissionListByRecordId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(MissionListManagerDumpTest, MissionListManager_DumpMissionListByRecordId_0100, TestSize.Level1)
{
    HILOG_INFO("MissionListManager_DumpMissionListByRecordId_0100 start");

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->recordId_ = RECORD_ID;

    auto mission = std::make_shared<Mission>(MISSION_ID, abilityRecord);
    EXPECT_NE(mission, nullptr);

    auto missionList = std::make_shared<MissionList>();
    EXPECT_NE(missionList, nullptr);
    missionList->missions_.push_back(mission);

    auto missionListManager = std::make_shared<MissionListManager>(USER_ID);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->defaultStandardList_ = missionList;

    std::vector<std::string> info;
    bool isClient = false;
    int32_t abilityRecordId = RECORD_ID;
    std::vector<std::string> params;

    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecordId, params);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("MissionListManager_DumpMissionListByRecordId_0100 end");
}

/**
 * @tc.name: MissionListManager_OnAppStateChanged_0100
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(MissionListManagerDumpTest, MissionListManager_OnAppStateChanged_0100, TestSize.Level1)
{
    HILOG_INFO("MissionListManager_OnAppStateChanged_0100 start");

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    auto missionListManager = std::make_shared<MissionListManager>(USER_ID);
    EXPECT_NE(missionListManager, nullptr);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);

    AppInfo info;
    info.processName = STRING_PROCESS_NAME;
    info.state = AppState::TERMINATED;
    missionListManager->OnAppStateChanged(info);

    abilityRecord = missionListManager->terminateAbilityList_.front();
    EXPECT_NE(abilityRecord, nullptr);
    EXPECT_EQ(abilityRecord->GetAppState(), AppState::TERMINATED);

    HILOG_INFO("MissionListManager_OnAppStateChanged_0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS
