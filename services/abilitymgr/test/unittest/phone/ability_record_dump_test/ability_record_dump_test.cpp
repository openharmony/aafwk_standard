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
#include "ability_record.h"
#undef private
#undef protected
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t SIZE_ONE = 1;
}  // namespace

class AbilityRecordDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityRecordDumpTest::SetUpTestCase(void)
{}

void AbilityRecordDumpTest::TearDownTestCase(void)
{}

void AbilityRecordDumpTest::SetUp()
{}

void AbilityRecordDumpTest::TearDown()
{}

/**
 * @tc.name: AbilityRecordDump_DumpAbilityState_0100
 * @tc.desc: DumpAbilityState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityRecordDumpTest, AbilityRecordDump_DumpAbilityState_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityRecordDump_DumpAbilityState_0100 start");

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    std::vector<std::string> info;
    bool isClient = false;
    std::vector<std::string> params;
    abilityRecord->DumpAbilityState(info, isClient, params);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("AbilityRecordDump_DumpAbilityState_0100 end");
}

/**
 * @tc.name: AbilityRecordDump_DumpService_0100
 * @tc.desc: DumpService
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityRecordDumpTest, AbilityRecordDump_DumpService_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityRecordDump_DumpService_0100 start");

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    std::vector<std::string> info;
    bool isClient = false;
    abilityRecord->DumpService(info, isClient);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("AbilityRecordDump_DumpService_0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS
