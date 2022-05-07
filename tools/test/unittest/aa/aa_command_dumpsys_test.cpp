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

namespace {
const std::string STRING_USER_ID = "100";
const std::string UNKNOWN_OPTION_MSG = "error: unknown option.\n";
}  // namespace

class AaCommandDumpsysTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "dump";
};

void AaCommandDumpsysTest::SetUpTestCase()
{}

void AaCommandDumpsysTest::TearDownTestCase()
{}

void AaCommandDumpsysTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandDumpsysTest::TearDown()
{}

void AaCommandDumpsysTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_Dumpsys_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump xxx" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -x" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), UNKNOWN_OPTION_MSG + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -xxx" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), UNKNOWN_OPTION_MSG + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --x" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), UNKNOWN_OPTION_MSG + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --xxx" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), UNKNOWN_OPTION_MSG + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -h" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --help" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--help",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -a" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-a",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --all" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1000, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--all",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -l" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-l",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --mission-list" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--mission-list",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -i" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-i",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), UNKNOWN_OPTION_MSG + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --ability" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--ability",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -e" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --extension" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--extension",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -p" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --pending" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--pending",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -r" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_1900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-r",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --process" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2000, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--process",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -d" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-d",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --data" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--data",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "");
}

/**
 * @tc.number: Aa_Command_Dumpsys_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -u" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-u",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --userId" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--userId",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -c" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-c",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMPSYS);
}

/**
 * @tc.number: Aa_Command_Dumpsys_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --client" command.
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AaCommandDumpsysTest, Aa_Command_Dumpsys_2600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--client",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMPSYS);
}
