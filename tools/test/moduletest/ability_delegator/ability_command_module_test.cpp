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
const std::string STRING_BUNDLENAME = "com.example.myapplication";
const std::string STRING_BUNDLENAME1 = "com.example.myapplication1";
const std::string CLASS = "class";
const std::string UNITTEST = "unittest";
const std::string TIME = "50";
}  // namespace

class AbilityCommandModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "test";
};

void AbilityCommandModuleTest::SetUpTestCase()
{}

void AbilityCommandModuleTest::TearDownTestCase()
{}

void AbilityCommandModuleTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityCommandModuleTest::TearDown()
{}

void AbilityCommandModuleTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ability_Command_Module_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -h -hhhhhhhhhhhhhhhhhhhhhh" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0100 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
        (char *)"-hhhhhhhhhhhhhhhhhhhhhh",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Module_Test_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -hhhhhhhhhhhhhhhhhhhhhh -h" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0200 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-hhhhhhhhhhhhhhhhhhhhhh",
        (char *)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -hhhhhhhhhhhhhhhhhhhhhh" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0300 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-hhhhhhhhhhhhhhhhhhhhhh",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -help -hhhhhhhhhhhhhhhhhhhhhh" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0400, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0400 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--help",
        (char *)"-hhhhhhhhhhhhhhhhhhhhhh",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_OK);
}

/**
 * @tc.number: Ability_Command_Module_Test_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -hhhhhhhhhhhhhhhhhhhhhh -help" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0500, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0500 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-hhhhhhhhhhhhhhhhhhhhhh",
        (char *)"-help",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -h" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0600, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0600 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-h",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -w 50" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0700, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0700 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p com.example.myapplication -s unittst JSUserTestRunner -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 50 -z 50" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0800, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0800 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-s",
        (char *)CLASS.c_str(),
        (char *)STRING_CLASS_NAME.c_str(),
        (char *)"-w",
        (char *)TIME.c_str(),
        (char *)"-z",
        (char *)TIME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsTestCommand(), OHOS::ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Command_Module_Test_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa test -p com.example.myapplication -s unittst JSUserTestRunner  -z 50 -s class
 *           ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010 -w 50" command.
 */
HWTEST_F(AbilityCommandModuleTest, Ability_Command_Module_Test_0900, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Command_Module_Test_0900 is called");
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-p",
        (char *)STRING_BUNDLENAME.c_str(),
        (char *)"-s",
        (char *)UNITTEST.c_str(),
        (char *)STRING_USER_TESTRUNNER.c_str(),
        (char *)"-z",
        (char *)TIME.c_str(),
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