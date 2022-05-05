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
#include "ability_manager_client.h"
#undef private
#include "shell_command_result.h"
#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string SHELLCOMMANDDRESULT = "shell cmd result";
const int EXITCODE = 15;
}  // namespace

class ShellCommandResultTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ShellCommandResultTest::SetUpTestCase()
{}

void ShellCommandResultTest::TearDownTestCase()
{}

void ShellCommandResultTest::SetUp()
{}

void ShellCommandResultTest::TearDown()
{}

/**
 * @tc.number: Shell_Command_Result_Test_0100
 * @tc.name: Marshalling and Unmarshalling
 * @tc.desc: Verify the Marshalling and Unmarshalling.
 */
HWTEST_F(ShellCommandResultTest, Shell_Command_Result_Test_0100, Function | MediumTest | Level1)
{
    ShellCommandResult shellCmd;
    shellCmd.exitCode = EXITCODE;
    shellCmd.stdResult = SHELLCOMMANDDRESULT;
    Parcel parcel;
    EXPECT_TRUE(shellCmd.Marshalling(parcel));
    EXPECT_NE(shellCmd.Unmarshalling(parcel), nullptr);
}

/**
 * @tc.number: Shell_Command_Result_Test_0200
 * @tc.name: Marshalling and ReadFromParcel
 * @tc.desc: Verify the Marshalling and ReadFromParcel.
 */
HWTEST_F(ShellCommandResultTest, Shell_Command_Result_Test_0200, Function | MediumTest | Level1)
{
    ShellCommandResult shellCmd;
    shellCmd.exitCode = EXITCODE;
    shellCmd.stdResult = SHELLCOMMANDDRESULT;
    Parcel parcel;
    EXPECT_TRUE(shellCmd.Marshalling(parcel));
    shellCmd.ReadFromParcel(parcel);
    EXPECT_EQ(shellCmd.exitCode, EXITCODE);
    EXPECT_EQ(shellCmd.stdResult, SHELLCOMMANDDRESULT);
}

