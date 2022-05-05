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
#include "delegator_thread.h"
#undef private

#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class DelegatorThreadTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DelegatorThreadTest::SetUpTestCase()
{}

void DelegatorThreadTest::TearDownTestCase()
{}

void DelegatorThreadTest::SetUp()
{}

void DelegatorThreadTest::TearDown()
{}

namespace {
void CallBackFunc()
{
    HILOG_INFO("CallBackFunc is called");
}
}

/**
 * @tc.number: Delegator_Thread_Test_0100
 * @tc.name: Run
 * @tc.desc: Verify the Run para task is null and result is invalid.
 */
HWTEST_F(DelegatorThreadTest, Delegator_Thread_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Test_0100 is called");

    DelegatorThread threadObj(true);
    std::function<void()> ff = nullptr;
    EXPECT_FALSE(threadObj.Run(ff));
}

/**
 * @tc.number: Delegator_Thread_Test_0200
 * @tc.name: Run
 * @tc.desc: Verify the Run para handler_ is null and result is invalid.
 */
HWTEST_F(DelegatorThreadTest, Delegator_Thread_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Test_0200 is called");

    DelegatorThread threadObj(true);
    threadObj.handler_ = nullptr;
    std::function<void()> ff = CallBackFunc;
    EXPECT_FALSE(threadObj.Run(ff));
}

/**
 * @tc.number: Delegator_Thread_Test_0300
 * @tc.name: Run
 * @tc.desc: Verify the Run is valid.
 */
HWTEST_F(DelegatorThreadTest, Delegator_Thread_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Test_0300 is called");

    DelegatorThread threadObj(true);
    std::function<void()> ff = CallBackFunc;
    EXPECT_TRUE(threadObj.Run(ff));
}

/**
 * @tc.number: Delegator_Thread_Test_0400
 * @tc.name: GetThreadName
 * @tc.desc: Verify the GetThreadName.
 */
HWTEST_F(DelegatorThreadTest, Delegator_Thread_Test_0400, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Test_0400 is called");

    DelegatorThread threadObj(false);
    EXPECT_EQ(threadObj.GetThreadName(), "");
}