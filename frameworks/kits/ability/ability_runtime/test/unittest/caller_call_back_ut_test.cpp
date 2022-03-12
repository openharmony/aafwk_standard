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
#define protected public
#include "caller_callback.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

class CallerCallBackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CallerCallBackTest::SetUpTestCase(void)
{}

void CallerCallBackTest::TearDownTestCase(void)
{}

void CallerCallBackTest::SetUp(void)
{}

void CallerCallBackTest::TearDown(void)
{}

/**
 * @tc.number: Caller_Call_Back_SetCallBack_0100
 * @tc.name: StartAbility
 * @tc.desc: Caller call back to process SetCallBack success.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_SetCallBack_0100, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;
    EXPECT_TRUE(callerCallBack.callback_ == nullptr);

    callerCallBack.SetCallBack([](const sptr<IRemoteObject> &) {});

    EXPECT_TRUE(callerCallBack.callback_ != nullptr);
}

/**
 * @tc.number: Caller_Call_Back_SetOnRelease_0100
 * @tc.name: SetOnRelease
 * @tc.desc: Caller call back to process SetOnRelease success.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_SetOnRelease_0100, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;
    EXPECT_TRUE(callerCallBack.onRelease_ == nullptr);

    callerCallBack.SetOnRelease([](const std::string &result) {
    });

    EXPECT_TRUE(callerCallBack.onRelease_ != nullptr);
}

/**
 * @tc.number: Caller_Call_Back_InvokeCallBack_0100
 * @tc.name: InvokeCallBack
 * @tc.desc: Caller call back to process InvokeCallBack success.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_InvokeCallBack_0100, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;
    EXPECT_EQ(callerCallBack.isCallBack_, false);

    callerCallBack.SetCallBack([](const sptr<IRemoteObject> &) {});

    callerCallBack.InvokeCallBack(nullptr);

    EXPECT_EQ(callerCallBack.isCallBack_, true);
}

/**
 * @tc.number: Caller_Call_Back_InvokeCallBack_0200
 * @tc.name: InvokeCallBack
 * @tc.desc: Caller call back to process InvokeCallBack fail because call back is null.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_InvokeCallBack_0200, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;
    EXPECT_EQ(callerCallBack.isCallBack_, false);

    callerCallBack.InvokeCallBack(nullptr);

    EXPECT_EQ(callerCallBack.isCallBack_, false);
}

/**
 * @tc.number: Caller_Call_Back_InvokeOnRelease_0100
 * @tc.name: InvokeOnRelease
 * @tc.desc: Caller call back to process InvokeOnRelease success.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_InvokeOnRelease_0100, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;

    callerCallBack.SetOnRelease([](const std::string &result) {
        EXPECT_TRUE("result" == result);
    });

    std::string key = "result";
    callerCallBack.InvokeOnRelease(key);
}

/**
 * @tc.number: Caller_Call_Back_InvokeOnRelease_0200
 * @tc.name: InvokeOnRelease
 * @tc.desc: Caller call back to process InvokeOnRelease fail because onRelease is null.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_InvokeOnRelease_0200, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;

    std::string key("");
    callerCallBack.InvokeOnRelease(key);

    EXPECT_TRUE("" == key);
}

/**
 * @tc.number: Caller_Call_Back_IsCallBack_0100
 * @tc.name: IsCallBack
 * @tc.desc: Caller call back to process IsCallBack success.
 */
HWTEST_F(CallerCallBackTest, Caller_Call_Back_IsCallBack_0100, Function | MediumTest | Level1)
{
    CallerCallBack callerCallBack;
    callerCallBack.isCallBack_ = true;

    bool ret = callerCallBack.IsCallBack();

    EXPECT_EQ(ret, true);
}
}  // namespace AppExecFwk
}  // namespace OHOS