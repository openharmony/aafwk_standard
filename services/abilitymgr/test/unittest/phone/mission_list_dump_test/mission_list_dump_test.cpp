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
#include "mission_list.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t MISSION_ID = 1024;
const int32_t RECORD_ID = 2048;
constexpr size_t SIZE_ONE = 1;
}  // namespace

class MissionListDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MissionListDumpTest::SetUpTestCase(void)
{}

void MissionListDumpTest::TearDownTestCase(void)
{}

void MissionListDumpTest::SetUp()
{}

void MissionListDumpTest::TearDown()
{}

/**
 * @tc.name: MissionListDump_DumpList_0100
 * @tc.desc: DumpList
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(MissionListDumpTest, MissionListDump_DumpList_0100, TestSize.Level0)
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    auto mission = std::make_shared<Mission>(MISSION_ID, abilityRecord);
    EXPECT_NE(mission, nullptr);

    auto missionList = std::make_shared<MissionList>();
    EXPECT_NE(missionList, nullptr);

    missionList->missions_.push_back(mission);

    std::vector<std::string> info;
    bool isClient = false;
    missionList->DumpList(info, isClient);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }
}

/**
 * @tc.name: MissionListDump_DumpStateByRecordId_0100
 * @tc.desc: DumpStateByRecordId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(MissionListDumpTest, MissionListDump_DumpStateByRecordId_0100, TestSize.Level0)
{
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

    std::vector<std::string> info;
    bool isClient = false;
    int32_t abilityRecordId = RECORD_ID;
    std::vector<std::string> params;
    missionList->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
