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

#define protected public
#include "ability_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

class AaCommandScreenTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "screen";
};

void AaCommandScreenTest::SetUpTestCase()
{}

void AaCommandScreenTest::TearDownTestCase()
{}

void AaCommandScreenTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandScreenTest::TearDown()
{}

void AaCommandScreenTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_Screen_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen xxx" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen -x" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen -xxx" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen --x" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen --xxx" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen -h" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen --help" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--help",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_SCREEN);
}

/**
 * @tc.number: Aa_Command_Screen_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa screen -p" command.
 */
HWTEST_F(AaCommandScreenTest, Aa_Command_Screen_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_SCREEN);
}
