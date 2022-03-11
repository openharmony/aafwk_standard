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
#define protected public
#include "ability_stack_manager.h"
#include "screenshot_handler.h"
#include "screenshot_response.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "mock_ability_connect_callback.h"
#include "ability_scheduler_mock.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {

class ScreenShotHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ScreenshotHandler> screenShotHandler_;
    std::shared_ptr<ScreenShotResponse> screenShotResponse_;
};

void ScreenShotHandlerTest::SetUpTestCase(void)
{}

void ScreenShotHandlerTest::TearDownTestCase(void)
{}

void ScreenShotHandlerTest::SetUp()
{
    screenShotHandler_ = std::make_shared<ScreenshotHandler>();
    screenShotResponse_ = std::make_shared<ScreenShotResponse>();
}

void ScreenShotHandlerTest::TearDown()
{
    screenShotHandler_.reset();
    screenShotResponse_.reset();
}

/*
 * Feature: ScreenShotHandlerTest
 * Function: OnWindowShot
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get Screen Shot
 */
HWTEST_F(ScreenShotHandlerTest, ability_screenshot_response_001, TestSize.Level1)
{
    OHOS::WMImageInfo info;
    info.width = 1;
    info.size = 5;
    info.height = 2;
    info.format = 3;
    info.data = nullptr;
    screenShotResponse_->OnWindowShot(info);
    OHOS::WMImageInfo infos = screenShotResponse_->GetImageInfo();
    EXPECT_EQ(infos.width, static_cast<uint32_t>(1));
    EXPECT_EQ(infos.size, static_cast<uint32_t>(5));
    EXPECT_EQ(infos.height, static_cast<uint32_t>(2));
    EXPECT_EQ(infos.format, static_cast<uint32_t>(3));
    EXPECT_EQ(infos.data, nullptr);
}

/*
 * Feature: ScreenShotHandlerTest
 * Function: ScreenshotHandler::GetImageInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get Image Info
 */
HWTEST_F(ScreenShotHandlerTest, ability_screenshot_handler_001, TestSize.Level1)
{
    int missionId = 0;
    OHOS::WMImageInfo info;
    info.width = 1;
    info.size = 5;
    info.height = 2;
    info.format = 3;
    info.data = nullptr;
    screenShotHandler_->screenShot_.emplace(missionId, info);
    EXPECT_EQ(static_cast<uint32_t>(screenShotHandler_->screenShot_.size()), static_cast<uint32_t>(1));
    auto imageInfo = screenShotHandler_->GetImageInfo(missionId);
    EXPECT_EQ(static_cast<uint32_t>(screenShotHandler_->screenShot_.size()), static_cast<uint32_t>(1));
    EXPECT_EQ(imageInfo.width, static_cast<uint32_t>(1));
    EXPECT_EQ(imageInfo.size, static_cast<uint32_t>(5));
    EXPECT_EQ(imageInfo.height, static_cast<uint32_t>(2));
    EXPECT_EQ(imageInfo.format, static_cast<uint32_t>(3));
    EXPECT_EQ(imageInfo.data, nullptr);
}

/*
 * Feature: ScreenShotHandlerTest
 * Function: ScreenshotHandler::RemoveImageInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: remove Image Info
 */
HWTEST_F(ScreenShotHandlerTest, ability_screenshot_handler_002, TestSize.Level1)
{
    int missionId = 0;
    OHOS::WMImageInfo info;
    info.width = 1;
    info.size = 5;
    info.height = 2;
    info.format = 3;
    info.data = nullptr;
    screenShotHandler_->screenShot_.emplace(missionId, info);
    EXPECT_EQ(static_cast<uint32_t>(screenShotHandler_->screenShot_.size()), static_cast<uint32_t>(1));
    screenShotHandler_->RemoveImageInfo(missionId);
    EXPECT_EQ(static_cast<uint32_t>(screenShotHandler_->screenShot_.size()), static_cast<uint32_t>(0));
}
}  // namespace AAFwk
}  // namespace OHOS
