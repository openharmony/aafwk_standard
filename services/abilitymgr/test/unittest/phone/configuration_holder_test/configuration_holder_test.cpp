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
#include "configuration_holder.h"
#undef private
#undef protected

#include "mock_configuration_holder.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class ConfigurationHolderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AppExecFwk::MockConfigurationHolder> mockConfigurationHolder_;
};

void ConfigurationHolderTest::SetUpTestCase(void)
{}
void ConfigurationHolderTest::TearDownTestCase(void)
{}
void ConfigurationHolderTest::SetUp(void)
{
    mockConfigurationHolder_ = std::make_shared<AppExecFwk::MockConfigurationHolder>();
}
void ConfigurationHolderTest::TearDown(void)
{}

/*
 * Feature: ConfigurationHolder
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Update Configuration
 */
HWTEST_F(ConfigurationHolderTest, UpdateConfiguration_001, TestSize.Level1)
{
    std::string name = "layout";
    auto config = std::make_shared<DummyConfiguration>(name);
    auto config1 = std::make_shared<DummyConfiguration>();
    mockConfigurationHolder_->baseConfiguration_ = config1;
    mockConfigurationHolder_->baseConfiguration_->testInfostr_ = "orientation";
    mockConfigurationHolder_->UpdateConfiguration(config);

    EXPECT_EQ(config, mockConfigurationHolder_->GetConfiguration());
}

/*
 * Feature: ConfigurationHolder
 * Function: ProcessConfigurationChange
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationHolderTest, ProcessConfigurationChange_001, TestSize.Level1)
{
    mockConfigurationHolder_->baseConfiguration_ = nullptr;
    EXPECT_EQ(false, mockConfigurationHolder_->ProcessConfigurationChange());
}

/*
 * Feature: ConfigurationHolder
 * Function: ProcessConfigurationChange
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change
 */
HWTEST_F(ConfigurationHolderTest, ProcessConfigurationChange_002, TestSize.Level1)
{
    std::string name = "layout";
    auto config = std::make_shared<DummyConfiguration>(name);
    mockConfigurationHolder_->baseConfiguration_ = config;
    mockConfigurationHolder_->baseConfiguration_->testInfostr_ = "orientation";
    EXPECT_EQ(false, mockConfigurationHolder_->ProcessConfigurationChange());
}

/*
 * Feature: ConfigurationHolder
 * Function: ProcessConfigurationChangeInner
 * SubFunction: Process Configuration Change Inner
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Process Configuration Change Inner
 */
HWTEST_F(ConfigurationHolderTest, ProcessConfigurationChangeInner_001, TestSize.Level1)
{
    std::string name = "layout";
    auto config = std::make_shared<DummyConfiguration>(name);
    auto config1 = std::make_shared<DummyConfiguration>();
    mockConfigurationHolder_->baseConfiguration_ = config1;
    mockConfigurationHolder_->baseConfiguration_->testInfostr_ = "orientation";
    EXPECT_EQ(false, mockConfigurationHolder_->ProcessConfigurationChangeInner(config));
    EXPECT_EQ(config, mockConfigurationHolder_->GetConfiguration());
}

}  // namespace AAFwk
}  // namespace OHOS