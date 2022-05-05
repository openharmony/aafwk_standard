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
#include "ability_command.h"
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"
#include "hilog_wrapper.h"
#include "mock_ability_manager_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_CLASS_NAME = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string STRING_USER_TESTRUNNER = "JSUserTestRunner";
const std::string STRING_PACKAGENAME = "com.example.myapplication";
const std::string STRING_PACKAGENAME1 = "com.example.myapplication1";
const std::string STRING_BUNDLENAME = "com.example.myapplication";
const std::string STRING_MODULENAME = "com.example.myapplication.MyApplication";
const std::string CLASS = "class";
const std::string UNITTEST = "unittest";
const std::string UNITTEST1 = "unittest1";
const std::string TIME = "20";
const std::string ANYKEY = "123";
const std::string ANYVALUE = "999999999";
}  // namespace

class AbilityCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "test";
};

void AbilityCommandTest::SetUpTestCase()
{}

void AbilityCommandTest::TearDownTestCase()
{}

void AbilityCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityCommandTest::TearDown()
{}

void AbilityCommandTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ability_Command_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0100 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -zxmy" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0200 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-zxmy",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -h" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0300 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -help" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0400, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0400 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-help",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0500, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0500 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0600, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0600 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-s",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s 123456 aaaaaaaaaaaaaa" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0700, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0700 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-s",
        (char *)"123456",
        (char *)"aaaaaaaaaaaaaa",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -l" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0800, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0800 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-l",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_0900, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_0900 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s unittest" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1000, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1000 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -s class" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1100 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittest" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1200, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1200 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s class" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1300, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1300 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittest JSUserTestRunner" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1400, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1400 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1500, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1500 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1600, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1600 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1700, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1700 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1800, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1800 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_PACKAGENAME1.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)"20",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst1 JSUserTestRunner -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_1900, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_1900 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_PACKAGENAME1.c_str(),
        (char *)"-s",
        (char *)UNITTEST1.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)"20",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication1 -s unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2000, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2000 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_PACKAGENAME1.c_str(),
        (char *)"-s",
        (char *)UNITTEST1.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p1 com.example.myapplication -s unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2100 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b1",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p1 com.example.myapplication -s1 unittst1 JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2200, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2200 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s1",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2300, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2300 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-p",
        (char *)STRING_PACKAGENAME.c_str(),
        (char *)"-m",
        (char *)STRING_MODULENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -b -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2400, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2400 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-p",
        (char *)"-m",
        (char *)STRING_MODULENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2500, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2500 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-p",
        (char *)STRING_PACKAGENAME.c_str(),
        (char *)"-m",
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Test_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -b com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 20" command.
 */
HWTEST_F(AbilityCommandTest, Ability_Command_Test_2600, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Test_2600 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-b",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-p",
        (char *)STRING_PACKAGENAME.c_str(),
        (char *)"-m",
        (char *)STRING_MODULENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-s",
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}
