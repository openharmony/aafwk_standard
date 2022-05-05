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

#include "shell_cmd_result.h"
#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const std::string SHELLCOMMANDDRESULT = "shell cmd result AAAAAAAAAAAAAAAAAAAAA";
const std::string CHANGESHELLCOMMANDDRESULT = "shell cmd result BBBBBBBBBBBBBBBBBB";
const int EXITCODE = 15;
const int CHANGEEXITCODE = 100;
}  // namespace

class ShellCmdResultTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ShellCmdResultTest::SetUpTestCase()
{}

void ShellCmdResultTest::TearDownTestCase()
{}

void ShellCmdResultTest::SetUp()
{}

void ShellCmdResultTest::TearDown()
{}

/**
 * @tc.number: Shell_Cmd_Result_Test_0100
 * @tc.name: GetExitCode and GetStdResult
 * @tc.desc: Verify the GetExitCode and GetStdResult.
 */
HWTEST_F(ShellCmdResultTest, Shell_Cmd_Result_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Shell_Cmd_Result_Test_0100 is called");
    ShellCmdResult shellCmd(EXITCODE, SHELLCOMMANDDRESULT);
    
    EXPECT_EQ(shellCmd.GetExitCode(), EXITCODE);
    EXPECT_EQ(shellCmd.GetStdResult(), SHELLCOMMANDDRESULT);
}

/**
 * @tc.number: Shell_Cmd_Result_Test_0200
 * @tc.name: SetExitCode and GetExitCode
 * @tc.desc: Verify the SetExitCode and GetExitCode.
 */
HWTEST_F(ShellCmdResultTest, Shell_Cmd_Result_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Shell_Cmd_Result_Test_0200 is called");
    ShellCmdResult shellCmd(EXITCODE, SHELLCOMMANDDRESULT);
    shellCmd.SetExitCode(CHANGEEXITCODE);

    EXPECT_EQ(shellCmd.GetExitCode(), CHANGEEXITCODE);
}

/**
 * @tc.number: Shell_Cmd_Result_Test_0300
 * @tc.name: SetStdResult and GetStdResult
 * @tc.desc: Verify the SetStdResult and GetStdResult.
 */
HWTEST_F(ShellCmdResultTest, Shell_Cmd_Result_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Shell_Cmd_Result_Test_0300 is called");
    ShellCmdResult shellCmd(EXITCODE, SHELLCOMMANDDRESULT);
    shellCmd.SetStdResult(CHANGESHELLCOMMANDDRESULT);

    EXPECT_EQ(shellCmd.GetStdResult(), CHANGESHELLCOMMANDDRESULT);
}
