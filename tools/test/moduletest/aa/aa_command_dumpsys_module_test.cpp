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

#include "ability_command.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_SEPERATOR = "\n";
const std::string STRING_RECORD_ID = "0";
const size_t SIZE_ONE = 1;
}  // namespace

class AaCommandDumpsysModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::string cmd_ = "dump";
};

void AaCommandDumpsysModuleTest::SetUpTestCase()
{}

void AaCommandDumpsysModuleTest::TearDownTestCase()
{}

void AaCommandDumpsysModuleTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AaCommandDumpsysModuleTest::TearDown()
{}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -a" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-a",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --all" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--all",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -l" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-l",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --mission-list" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--mission-list",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -i" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-i",
        (char *)STRING_RECORD_ID.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GE(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --ability" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--ability",
        (char *)STRING_RECORD_ID.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GE(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -e" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --extension" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--extension",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -p" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GE(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --pending" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1000, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--pending",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GE(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -r" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-r",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --process" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--process",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -d" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-d",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --data" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--data",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump -c" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-c",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}

/**
 * @tc.number: Aa_Command_Dumpsys_ModuleTest_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump --client" command.
 */
HWTEST_F(AaCommandDumpsysModuleTest, Aa_Command_Dumpsys_ModuleTest_1600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--client",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    auto result = cmd.ExecCommand();
    EXPECT_NE(result, HELP_MSG_DUMPSYS);

    std::vector<std::string> lines;
    SplitStr(result, " ", lines);
    EXPECT_GT(lines.size(), SIZE_ONE);
}
