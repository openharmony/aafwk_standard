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
#include "pending_want_manager.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t CODE = 1024;
const int32_t UID = 20000000;
constexpr size_t SIZE_ONE = 1;
}  // namespace

class PendingWantManagerDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void PendingWantManagerDumpTest::SetUpTestCase(void)
{}

void PendingWantManagerDumpTest::TearDownTestCase(void)
{}

void PendingWantManagerDumpTest::SetUp()
{}

void PendingWantManagerDumpTest::TearDown()
{}

/**
 * @tc.name: PendingWantManagerDump_DumpByRecordId_0100
 * @tc.desc: DumpByRecordId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(PendingWantManagerDumpTest, PendingWantManagerDump_DumpByRecordId_0100, TestSize.Level1)
{
    HILOG_INFO("PendingWantManagerDump_DumpByRecordId_0100 start");

    auto pendingManager = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager, nullptr);

    auto pendingKey = std::make_shared<PendingWantKey>();
    EXPECT_NE(pendingKey, nullptr);
    pendingKey->SetCode(CODE);

    auto pendingWantRecord = std::make_shared<PendingWantRecord>(pendingManager, UID, nullptr, pendingKey);
    EXPECT_NE(pendingWantRecord, nullptr);
    pendingManager->wantRecords_ = {{pendingKey, pendingWantRecord.get()}};

    std::vector<std::string> info;
    std::string args = std::to_string(CODE);
    pendingManager->DumpByRecordId(info, args);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("PendingWantManagerDump_DumpByRecordId_0100 end");
}

/**
 * @tc.name: PendingWantManagerDump_Dump_0100
 * @tc.desc: Dump
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(PendingWantManagerDumpTest, PendingWantManagerDump_Dump_0100, TestSize.Level1)
{
    HILOG_INFO("PendingWantManagerDump_Dump_0100 start");

    auto pendingManager = std::make_shared<PendingWantManager>();
    EXPECT_NE(pendingManager, nullptr);

    std::vector<std::string> info;
    pendingManager->Dump(info);
    EXPECT_EQ(info.size(), SIZE_ONE);

    HILOG_INFO("PendingWantManagerDump_Dump_0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS
