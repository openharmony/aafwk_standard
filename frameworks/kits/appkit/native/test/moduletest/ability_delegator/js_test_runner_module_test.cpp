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
#include "js_test_runner.h"
#include "ability_delegator.h"
#include "ability_manager_client.h"
#include "ability_runtime/context/context_impl.h"
#undef private

#define protected public
#include "js_runtime.h"
#undef protected

#include "ability_constants.h"
#include "ability_delegator_registry.h"
#include "app_loader.h"
#include "event_runner.h"
#include "hilog_wrapper.h"
#include "napi/native_common.h"
#include "mock_ability_delegator_stub.h"
#include "mock_test_observer_stub.h"
#include "ohos_application.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime::Constants;
using namespace OHOS::RunnerRuntime;

namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationjsmodule";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunnerjsmodule";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE =
    "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010_jsmodule";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "35";
const std::string REPORT_FINISH_MSG = "report finish message";
}

class JsTestRunnerModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;
};

void JsTestRunnerModuleTest::SetUpTestCase()
{}

void JsTestRunnerModuleTest::TearDownTestCase()
{}

void JsTestRunnerModuleTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void JsTestRunnerModuleTest::TearDown()
{}

void JsTestRunnerModuleTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub);

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Js_Test_Runner_Module_Test_0100
 * @tc.name: ReportFinished
 * @tc.desc: Verify the ReportFinished is valid.
 */
HWTEST_F(JsTestRunnerModuleTest, Js_Test_Runner_Module_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Js_Test_Runner_Module_Test_0100 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
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

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs);

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportFinished(REPORT_FINISH_MSG);
    
    EXPECT_TRUE(MockAbilityDelegatorStub::finishFlag_);
}

/**
 * @tc.number: Js_Test_Runner_Module_Test_0200
 * @tc.name: ReportStatus
 * @tc.desc: Verify the ReportStatus is valid.
 */
HWTEST_F(JsTestRunnerModuleTest, Js_Test_Runner_Module_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Js_Test_Runner_Module_Test_0200 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
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

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportStatus(REPORT_FINISH_MSG);
    
    EXPECT_TRUE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}