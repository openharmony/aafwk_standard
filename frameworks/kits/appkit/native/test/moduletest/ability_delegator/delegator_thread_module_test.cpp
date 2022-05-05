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

namespace {
bool callbackModuleFlag;
}

class DelegatorThreadModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DelegatorThreadModuleTest::SetUpTestCase()
{}

void DelegatorThreadModuleTest::TearDownTestCase()
{}

void DelegatorThreadModuleTest::SetUp()
{}

void DelegatorThreadModuleTest::TearDown()
{}

namespace {
void CallBackModuleFunc()
{
    HILOG_INFO("CallBackModuleFunc is called");
    callbackModuleFlag = true;
}
}

/**
 * @tc.number: Delegator_Thread_Module_Test_0100
 * @tc.name: Run
 * @tc.desc: Verify the Run is valid.
 */
HWTEST_F(DelegatorThreadModuleTest, Delegator_Thread_Module_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Module_Test_0100 is called");

    callbackModuleFlag = false;
    DelegatorThread threadObj(true);
    std::function<void()> fun = CallBackModuleFunc;
    EXPECT_TRUE(threadObj.Run(fun));
}

/**
 * @tc.number: Delegator_Thread_Module_Test_0200
 * @tc.name: GetThreadName
 * @tc.desc: Verify the GetThreadName.
 */
HWTEST_F(DelegatorThreadModuleTest, Delegator_Thread_Module_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Delegator_Thread_Module_Test_0200 is called");

    DelegatorThread threadObj(false);
    EXPECT_EQ(threadObj.GetThreadName(), "");
}