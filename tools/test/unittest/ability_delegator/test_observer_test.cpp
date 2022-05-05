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
#undef protected
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string MSG = "observer msg";
const int RESULT_CODE = 20;
const int64_t TIMEOUT = 50;
const std::string CMD = "ls -l";
}  // namespace

class TestObserverTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;
    static void thfunc(TestObserver &observer);
};

void TestObserverTest::SetUpTestCase()
{}

void TestObserverTest::TearDownTestCase()
{}

void TestObserverTest::SetUp()
{}

void TestObserverTest::TearDown()
{}

void TestObserverTest::thfunc(TestObserver &observer)
{
    HILOG_INFO("thfunc");
    sleep(1);
    HILOG_INFO("after sleep 1s thfunc");
    observer.isFinished_ = true;
}

/**
 * @tc.number: Test_Observer_Test_0100
 * @tc.name: TestFinished
 * @tc.desc: Verify the TestFinished.
 */
HWTEST_F(TestObserverTest, Test_Observer_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Test_0100 is called");
    TestObserver observer;
    observer.TestFinished(MSG.c_str(), RESULT_CODE);
    EXPECT_TRUE(observer.isFinished_);
}

/**
 * @tc.number: Test_Observer_Test_0200
 * @tc.name: ExecuteShellCommand
 * @tc.desc: Verify the ExecuteShellCommand.
 */
HWTEST_F(TestObserverTest, Test_Observer_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Test_0200 is called");
    TestObserver observer;
    EXPECT_NE(observer.ExecuteShellCommand(CMD.c_str(), TIMEOUT).stdResult.size(), 0);
}

/**
 * @tc.number: Test_Observer_Test_0300
 * @tc.name: WaitForFinish
 * @tc.desc: Verify the WaitForFinish.
 */
HWTEST_F(TestObserverTest, Test_Observer_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Test_0300 is called");
    TestObserver observer;
    std::thread th(&TestObserverTest::thfunc, std::ref(observer));
    bool ret = observer.WaitForFinish(5000);
    th.join();
    EXPECT_TRUE(ret);
}