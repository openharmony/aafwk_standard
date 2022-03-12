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
#include "app_mgr_service.h"
#undef private
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
}  // namespace

class AppMgrServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceDumpTest::SetUpTestCase(void)
{}

void AppMgrServiceDumpTest::TearDownTestCase(void)
{}

void AppMgrServiceDumpTest::SetUp()
{}

void AppMgrServiceDumpTest::TearDown()
{}

/**
 * @tc.name: AppMgrServiceDump_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    HILOG_INFO("AppMgrServiceDump_GetProcessRunningInfosByUserId_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    auto result = appMgrServiceInner->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, ERR_OK);

    HILOG_INFO("AppMgrServiceDump_GetProcessRunningInfosByUserId_0100 end");
}

/**
 * @tc.name: AppMgrServiceDump_GetProcessRunningInfosByUserId_0200
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_GetProcessRunningInfosByUserId_0200, TestSize.Level1)
{
    HILOG_INFO("AppMgrServiceDump_GetProcessRunningInfosByUserId_0200 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    appMgrService->handler_ = std::make_shared<AMSEventHandler>(
        EventRunner::Create("AppMgrServiceDumpTest"), appMgrService->appMgrServiceInner_);

    std::vector<RunningProcessInfo> info;
    auto result = appMgrService->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, ERR_OK);

    HILOG_INFO("AppMgrServiceDump_GetProcessRunningInfosByUserId_0200 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
