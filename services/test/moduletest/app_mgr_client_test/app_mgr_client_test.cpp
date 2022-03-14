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

#include "app_mgr_client.h"
#include "configuration.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AppMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrClientTest::SetUpTestCase(void)
{}

void AppMgrClientTest::TearDownTestCase(void)
{}

void AppMgrClientTest::SetUp()
{}

void AppMgrClientTest::TearDown()
{}

/**
 * @tc.name: AppMgrClient_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    HILOG_INFO("AppMgrClient_GetProcessRunningInfosByUserId_0100 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::vector<RunningProcessInfo> info;
    result = appMgrClient->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    HILOG_INFO("info.size() = %{public}zu", info.size());

    HILOG_INFO("AppMgrClient_GetProcessRunningInfosByUserId_0100 end");
}

/**
 * @tc.name: AppMgrClient_GetConfiguration_0100
 * @tc.desc: GetConfiguration
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetConfiguration_0100, TestSize.Level1)
{
    HILOG_INFO("AppMgrClient_GetConfiguration_0100 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    Configuration config;
    result = appMgrClient->GetConfiguration(config);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    HILOG_INFO("AppMgrClient_GetConfiguration_0100 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
