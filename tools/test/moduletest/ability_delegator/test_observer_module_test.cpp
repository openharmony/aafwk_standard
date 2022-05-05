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

#include <iostream>
#include <thread>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "system_time.h"

#define private public
#include "test_observer.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string MSG = "observer msg";
const int RESULT_CODE = 20;
const int64_t TIMEOUT = 50;
const std::string CMD = "ls -l";
}  // namespace

class TestObserverModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void TestObserverModuleTest::SetUpTestCase()
{}

void TestObserverModuleTest::TearDownTestCase()
{}

void TestObserverModuleTest::SetUp()
{}

void TestObserverModuleTest::TearDown()
{}

/**
 * @tc.number: Test_Observer_Module_Test_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump" command.
 */
HWTEST_F(TestObserverModuleTest, Test_Observer_Module_Test_0100, Function | MediumTest | Level1)
{
    TestObserver observer;
    observer.TestFinished(MSG.c_str(), RESULT_CODE);
    EXPECT_TRUE(observer.isFinished_);
}

/**
 * @tc.number: Aa_Ability_Command_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa dump xxx" command.
 */
HWTEST_F(TestObserverModuleTest, Test_Observer_Module_Test_0200, Function | MediumTest | Level1)
{
    TestObserver observer;
    EXPECT_NE(observer.ExecuteShellCommand(CMD.c_str(), TIMEOUT).stdResult.size(), 0);
}

