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
#include <algorithm>
#include <string>

#include "ability_parameter_container.h"
#include "iremote_proxy.h"
#include "ability_scheduler.h"
#include "mock_ability_connect_callback.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class AbilityParameterContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityParameterContainerTest::SetUpTestCase(void)
{}
void AbilityParameterContainerTest::TearDownTestCase(void)
{}
void AbilityParameterContainerTest::SetUp()
{}
void AbilityParameterContainerTest::TearDown()
{}

/*
 * Feature: AbilityParameterContainer
 * Function: AddParameter
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContainer AddParameter
 * EnvConditions: NA
 * CaseDescription: Verify that AddParameter is normal and abnormal
 */
HWTEST_F(AbilityParameterContainerTest, AbilityParameterContainer_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContainer_001 start";

    std::unique_ptr<AbilityParameterContainer> abilityParameterContainer =
        std::make_unique<AbilityParameterContainer>();
    int abilityRecordId = 1;
    EXPECT_EQ(false, abilityParameterContainer->IsExist(abilityRecordId));

    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;

    abilityParameterContainer->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContainer->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContainer_001 end";
}

/*
 * Feature: AbilityParameterContainer
 * Function: RemoveParameterByID
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContainer RemoveParameterByID
 * EnvConditions: NA
 * CaseDescription: Verify that RemoveParameterByID is normal and abnormal
 */
HWTEST_F(AbilityParameterContainerTest, AbilityParameterContainer_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContainer_002 start";

    std::unique_ptr<AbilityParameterContainer> abilityParameterContainer =
        std::make_unique<AbilityParameterContainer>();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;
    abilityParameterContainer->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContainer->IsExist(abilityRecordId));

    abilityParameterContainer->RemoveParameterByID(abilityRecordId);

    EXPECT_EQ(false, abilityParameterContainer->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContainer_002 end";
}

/*
 * Feature: AbilityParameterContainer
 * Function: GetAbilityRequestFromContainer
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContainer GetAbilityRequestFromContainer
 * EnvConditions: NA
 * CaseDescription: Verify that GetAbilityRequestFromContainer is normal and abnormal
 */
HWTEST_F(AbilityParameterContainerTest, AbilityParameterContainer_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContainer_003 start";

    std::unique_ptr<AbilityParameterContainer> abilityParameterContainer =
        std::make_unique<AbilityParameterContainer>();
    AbilityRequest abilityRequest;
    AbilityRequest abilityResult;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;
    abilityParameterContainer->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContainer->IsExist(abilityRecordId));

    abilityResult = abilityParameterContainer->GetAbilityRequestFromContainer(abilityRecordId);

    EXPECT_EQ(abilityResult.callerUid, abilityRequest.callerUid);
    GTEST_LOG_(INFO) << "AbilityParameterContainer_003 end";
}

/*
 * Feature: AbilityParameterContainer
 * Function: IsExist
 * SubFunction: NA
 * FunctionPoints: AbilityParameterContainer IsExist
 * EnvConditions: NA
 * CaseDescription: Verify that IsExist is normal and abnormal
 */
HWTEST_F(AbilityParameterContainerTest, AbilityParameterContainer_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityParameterContainer_004 start";

    std::unique_ptr<AbilityParameterContainer> abilityParameterContainer =
        std::make_unique<AbilityParameterContainer>();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.restart = true;
    abilityRequest.requestCode = 10;
    int abilityRecordId = 1;

    EXPECT_EQ(false, abilityParameterContainer->IsExist(abilityRecordId));

    abilityParameterContainer->AddParameter(abilityRecordId, abilityRequest);

    EXPECT_EQ(true, abilityParameterContainer->IsExist(abilityRecordId));
    GTEST_LOG_(INFO) << "AbilityParameterContainer_004 end";
}
}  // namespace AAFwk
}  // namespace OHOS