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
#include "main_thread.h"
#undef private

#include "ability_constants.h"
#include "ability_delegator_registry.h"
#include "ability_runtime/context/context_impl.h"
#include "app_loader.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "mock_ability_delegator_stub.h"
#include "ohos_application.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime::Constants;
using namespace OHOS::AAFwk;

namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplication5";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner3";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE =
    "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010_6";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "60";
}

class AbilityDelegatorRegistryTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;
};

void AbilityDelegatorRegistryTest::SetUpTestCase()
{}

void AbilityDelegatorRegistryTest::TearDownTestCase()
{}

void AbilityDelegatorRegistryTest::SetUp()
{}

void AbilityDelegatorRegistryTest::TearDown()
{}

void AbilityDelegatorRegistryTest::MakeMockObjects() const
{}

/**
 * @tc.number: Ability_Delegator_Registry_Test_0100
 * @tc.name: RegisterInstance and GetAbilityDelegator and GetArguments
 * @tc.desc: Verify the RegisterInstance and GetAbilityDelegator and GetArguments.
 */
HWTEST_F(AbilityDelegatorRegistryTest, Ability_Delegator_Registry_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Ability_Delegator_Registry_Test_0100 is called");

    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }
    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(nullptr, abilityArgs, true);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(nullptr, std::move(testRunner), nullptr);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs);

    EXPECT_EQ(AbilityDelegatorRegistry::GetAbilityDelegator(), abilityDelegator);
    EXPECT_EQ(AbilityDelegatorRegistry::GetArguments(), abilityArgs);
}
