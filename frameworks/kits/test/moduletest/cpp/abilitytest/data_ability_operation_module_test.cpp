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

#include <climits>
#include <gtest/gtest.h>
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"
#include "data_ability_operation.h"
#include "data_ability_operation_builder.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
class DataAbilityOperationModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityOperationModuleTest::SetUpTestCase(void)
{}

void DataAbilityOperationModuleTest::TearDownTestCase(void)
{}

void DataAbilityOperationModuleTest::SetUp(void)
{}

void DataAbilityOperationModuleTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_DataAbilityOperation_InsertBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_InsertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_InsertBuilder_0100";
    std::shared_ptr<NativeRdb::ValuesBucket> values = std::make_shared<NativeRdb::ValuesBucket>();
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
    std::shared_ptr<DataAbilityOperation> operation =
            DataAbilityOperation::NewInsertBuilder(uri)->WithValuesBucket(values)->Build();
    EXPECT_NE(operation, nullptr);

    bool isInsertOperation = operation->IsInsertOperation();
    EXPECT_TRUE(isInsertOperation);

    std::shared_ptr<Uri> operationUri = operation->GetUri();
    EXPECT_EQ(operationUri->ToString(), "dataability://com.example.myapplication5.DataAbilityTest");

    int type = operation->GetType();
    EXPECT_EQ(type, DataAbilityOperation::TYPE_INSERT);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_InsertBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_UpdateBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_UpdateBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_UpdateBuilder_0100";
    std::shared_ptr<NativeRdb::ValuesBucket> values = std::make_shared<NativeRdb::ValuesBucket>();
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
    std::shared_ptr<DataAbilityOperation> operation = DataAbilityOperation::NewUpdateBuilder(uri)
                                                              ->WithValuesBucket(values)
                                                              ->WithPredicatesBackReference(0, 0)
                                                              ->WithPredicates(predicates)
                                                              ->WithInterruptionAllowed(true)
                                                              ->Build();
    EXPECT_TRUE(operation == nullptr); // ValuesBucket is empty.
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_UpdateBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_AssertBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_AssertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_AssertBuilder_0100";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
    std::shared_ptr<DataAbilityOperation> operation =
            DataAbilityOperation::NewAssertBuilder(uri)->WithExpectedCount(2)->Build();
    EXPECT_NE(operation, nullptr);

    bool isAssertOperation = operation->IsAssertOperation();
    EXPECT_TRUE(isAssertOperation);

    int count = operation->GetExpectedCount();
    EXPECT_EQ(count, 2);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_AssertBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_DeleteBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_DeleteBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_DeleteBuilder_0100";
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    EXPECT_NE(predicates, nullptr);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewDeleteBuilder(uri);
    EXPECT_NE(builder, nullptr);
    std::shared_ptr<DataAbilityOperation> operation = builder->WithPredicates(predicates)->Build();
    EXPECT_NE(operation, nullptr);
    bool isDeleteOperation = operation->IsDeleteOperation();
    EXPECT_TRUE(isDeleteOperation);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_DeleteBuilder_0100";
}

}  // namespace AppExecFwk
}  // namespace OHOS