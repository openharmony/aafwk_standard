/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "ams_mgr_scheduler.h"
#undef private
#include "app_log_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int DISPLAY_ID = 1024;
}  // namespace

class AmsMgrSchedulerDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AmsMgrSchedulerDumpTest::SetUpTestCase(void)
{}

void AmsMgrSchedulerDumpTest::TearDownTestCase(void)
{}

void AmsMgrSchedulerDumpTest::SetUp()
{}

void AmsMgrSchedulerDumpTest::TearDown()
{}

/**
 * @tc.name: AmsMgrSchedulerDump_GetConfiguration_0100
 * @tc.desc: GetConfiguration
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AmsMgrSchedulerDumpTest, AmsMgrSchedulerDump_GetConfiguration_0100, TestSize.Level0)
{
    APP_LOGI("AmsMgrSchedulerDump_GetConfiguration_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    auto config = std::make_shared<Configuration>();
    config->defaultDisplayId_ = DISPLAY_ID;
    appMgrServiceInner->configuration_ = config;

    auto amsEventHandler =
        std::make_shared<AMSEventHandler>(EventRunner::Create("AmsMgrSchedulerDumpTest"), appMgrServiceInner);
    EXPECT_NE(amsEventHandler, nullptr);

    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(appMgrServiceInner, amsEventHandler);
    EXPECT_NE(amsMgrScheduler, nullptr);
    EXPECT_EQ(amsMgrScheduler->IsReady(), true);

    Configuration configFromAmsMgrScheduler;
    auto result = amsMgrScheduler->GetConfiguration(configFromAmsMgrScheduler);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(configFromAmsMgrScheduler.defaultDisplayId_, config->defaultDisplayId_);

    APP_LOGI("AmsMgrSchedulerDump_GetConfiguration_0100 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
