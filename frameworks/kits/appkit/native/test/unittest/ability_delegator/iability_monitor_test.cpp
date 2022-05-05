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
#include <map>
#include <string>
#include <thread>
#include <iostream>

#include "ability_lifecycle_executor.h"

#define private public
#include "iability_monitor.h"
#include "ability_manager_client.h"
#undef private

#include "hilog_wrapper.h"
#include "mock_ability_delegator_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace {
const std::string ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string PROPERTY_ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string PROPERTY_ABILITY_NAME1 = "com.example.myapplication.MainAbility1";
}

class IabilityMonitorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
};

void IabilityMonitorTest::SetUpTestCase()
{}

void IabilityMonitorTest::TearDownTestCase()
{}

void IabilityMonitorTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void IabilityMonitorTest::TearDown()
{}

void IabilityMonitorTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Iability_Monitor_Test_0100
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Iability_Monitor_Test_0100 is called");

    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    EXPECT_FALSE(iabilityMonitor.Match(nullptr));
}

/**
 * @tc.number: Iability_Monitor_Test_0200
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Iability_Monitor_Test_0200 is called");

    IAbilityMonitor iabilityMonitor("");
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_FALSE(iabilityMonitor.Match(proterty));
}

/**
 * @tc.number: Iability_Monitor_Test_0300
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Iability_Monitor_Test_0300 is called");

    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_TRUE(iabilityMonitor.Match(proterty));
}
