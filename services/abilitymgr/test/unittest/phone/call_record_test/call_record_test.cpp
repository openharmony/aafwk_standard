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
#include "call_record.h"

using namespace testing::ext;
using OHOS::sptr;

namespace OHOS {
namespace AAFwk {
class CallRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CallRecordTest::SetUpTestCase(void)
{}
void CallRecordTest::TearDownTestCase(void)
{}
void CallRecordTest::TearDown()
{}

void CallRecordTest::SetUp()
{}

/*
 * Feature: WindowInfo
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallRecordTest, AAFWK_Call_Record, TestSize.Level1)
{
    static bool value = false;

    auto function = [] (const wptr<IRemoteObject> &) { value = true;};
    AbilityCallRecipient abilityCallRecipient(function);

    wptr<IRemoteObject> object = nullptr;
    abilityCallRecipient.OnRemoteDied(object);
    EXPECT_EQ(value, true);
}
}  // namespace AAFwk
}  // namespace OHOS