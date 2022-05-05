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
#include <string>
#include <map>

#define private public
#include "ability_delegator_args.h"
#undef private

#include "hilog_wrapper.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-b";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplication";
const std::string KEY_TEST_MODULE_NAME = "-m";
const std::string VALUE_TEST_MODULE_NAME = "com.example.myapplication.MyApplication";
const std::string KEY_TEST_PACKAGE_NAME = "-p";
const std::string VALUE_TEST_PACKAGE_NAME = "com.example.myapplication";
const std::string CHANGE_VALUE_TEST_BUNDLE_NAME = "com.example.myapplication1";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner";
const std::string CHANGE_VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner1";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string CHANGE_VALUE_TEST_CASE =
    "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction00101";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "50";
const std::string CHANGE_VALUE_TEST_WAIT_TIMEOUT = "80";
const std::string SET_VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationset";
const std::string KEY_TEST_DEBUG = "-D";
const std::string VALUE_TEST_DEBUG_DEFAULT = "true";
}  // namespace

class AbilityDelegatorArgsTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityDelegatorArgsTest::SetUpTestCase()
{}

void AbilityDelegatorArgsTest::TearDownTestCase()
{}

void AbilityDelegatorArgsTest::SetUp()
{}

void AbilityDelegatorArgsTest::TearDown()
{}

/**
 * @tc.number: Ability_Delegator_Args_Test_0100
 * @tc.name: GetTestBundleName and GetTestModuleName and GetTestPackageName and
 *           GetTestRunnerClassName and GetTestCaseName and GetTestRunnerClassName and FindDebugFlag.
 * @tc.desc: Verify the GetTestBundleName and GetTestModuleName and GetTestPackageName and
 *           GetTestRunnerClassName and GetTestCaseName and GetTestRunnerClassName and FindDebugFlag.
 */
HWTEST_F(AbilityDelegatorArgsTest, Ability_Delegator_Args_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Delegator_Args_Test_0100 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_MODULE_NAME, VALUE_TEST_MODULE_NAME);
    paras.emplace(KEY_TEST_PACKAGE_NAME, VALUE_TEST_PACKAGE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }
    
    AbilityDelegatorArgs delegatorArgs(want);
    std::map<std::string, std::string>::iterator iter = delegatorArgs.params_.find(KEY_TEST_WAIT_TIMEOUT);
    std::string value_timeout = iter->second;

    EXPECT_EQ(delegatorArgs.GetTestBundleName(), VALUE_TEST_BUNDLE_NAME);
    EXPECT_EQ(delegatorArgs.GetTestModuleName(), VALUE_TEST_MODULE_NAME);
    EXPECT_EQ(delegatorArgs.GetTestPackageName(), VALUE_TEST_PACKAGE_NAME);
    EXPECT_EQ(delegatorArgs.GetTestRunnerClassName(), VALUE_TEST_RUNNER_CLASS);
    EXPECT_EQ(delegatorArgs.GetTestCaseName(), VALUE_TEST_CASE);
    EXPECT_FALSE(delegatorArgs.FindDebugFlag());
    EXPECT_EQ(value_timeout, VALUE_TEST_WAIT_TIMEOUT);
}

/**
 * @tc.number: Ability_Delegator_Args_Test_0200
 * @tc.name: GetParamValue and FindDebugFlag.
 * @tc.desc: Verify the GetParamValue and FindDebugFlag.
 */
HWTEST_F(AbilityDelegatorArgsTest, Ability_Delegator_Args_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Delegator_Args_Test_0200 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_MODULE_NAME, VALUE_TEST_MODULE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);
    paras.emplace(KEY_TEST_DEBUG, VALUE_TEST_DEBUG_DEFAULT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }
    
    AbilityDelegatorArgs delegatorArgs(want);
    std::map<std::string, std::string>::iterator iter = delegatorArgs.params_.find(KEY_TEST_WAIT_TIMEOUT);
    std::string value_timeout = iter->second;

    EXPECT_EQ(delegatorArgs.GetParamValue(KEY_TEST_MODULE_NAME), VALUE_TEST_MODULE_NAME);
    EXPECT_EQ(delegatorArgs.GetParamValue(KEY_TEST_RUNNER_CLASS), VALUE_TEST_RUNNER_CLASS);
    EXPECT_EQ(delegatorArgs.GetParamValue(KEY_TEST_CASE), VALUE_TEST_CASE);
    EXPECT_TRUE(delegatorArgs.FindDebugFlag());
    EXPECT_EQ(value_timeout, VALUE_TEST_WAIT_TIMEOUT);
}
