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

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"
#include "data_ability_operation.h"
#include "data_ability_operation_builder.h"
#include <gtest/gtest.h>
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
static const std::string URI = "dataability://ohos.demo.TestDataAbilityOperation";
class DataAbilityOperationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void CreateBuilder(int type, std::shared_ptr<Uri> &uri);
    std::shared_ptr<DataAbilityOperationBuilder> builder = nullptr;
};

void DataAbilityOperationTest::SetUpTestCase(void)
{}

void DataAbilityOperationTest::TearDownTestCase(void)
{}

void DataAbilityOperationTest::SetUp(void)
{}

void DataAbilityOperationTest::TearDown(void)
{
    builder = nullptr;
}

void DataAbilityOperationTest::CreateBuilder(int type, std::shared_ptr<Uri> &uri)
{
    builder = std::make_shared<DataAbilityOperationBuilder>(type, uri);
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Create_0100
 * @tc.name: Constructor
 * @tc.desc: Test the dataabilityoperation object.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Create_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0100 start";
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = std::make_shared<DataAbilityOperation>();
    EXPECT_EQ(dataAbilityOperation->GetUri(), nullptr);
    EXPECT_EQ(dataAbilityOperation->GetType(), 0);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Create_0200
 * @tc.name: Constructor
 * @tc.desc: Test the dataabilityoperation object.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Create_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0200 start";
    Parcel in;
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = std::make_shared<DataAbilityOperation>(in);
    EXPECT_EQ(dataAbilityOperation->GetUri(), nullptr);
    EXPECT_EQ(dataAbilityOperation->GetType(), -1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Create_0300
 * @tc.name: Constructor
 * @tc.desc: Test the dataabilityoperation object.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Create_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0300 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(nullptr, uri);
    EXPECT_EQ(dataAbilityOperation->GetUri()->ToString(), URI);
    EXPECT_EQ(dataAbilityOperation->GetType(), 0);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Create_0400
 * @tc.name: Constructor
 * @tc.desc: Test the dataabilityoperation object.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Create_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0400 start";
    Parcel in;
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = std::make_shared<DataAbilityOperation>(in);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::unique_ptr<DataAbilityOperation> operation = std::make_unique<DataAbilityOperation>(dataAbilityOperation, uri);
    EXPECT_EQ(operation->GetUri()->ToString(), URI);
    EXPECT_EQ(operation->GetType(), -1);
    dataAbilityOperation.reset();
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Create_0500
 * @tc.name: Constructor/GetUri
 * @tc.desc: Test the dataabilityoperation object.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Create_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0500 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(DataAbilityOperation::TYPE_INSERT, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetUri()->ToString(), URI);
    EXPECT_EQ(dataAbilityOperation->GetType(), 1);
    dataAbilityOperation.reset();
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Create_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_NewInsertBuilder_0100
 * @tc.name: NewInsertBuilder
 * @tc.desc: Test whether the return value of NewInsertBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewInsertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewInsertBuilder_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);

    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewInsertBuilder(uri);
    EXPECT_NE(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewInsertBuilder_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_NewInsertBuilder_0200
 * @tc.name: NewInsertBuilder
 * @tc.desc: Test whether the return value of NewInsertBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewInsertBuilder_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewInsertBuilder_0200 start";
    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewInsertBuilder(nullptr);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewInsertBuilder_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_NewUpdateBuilder_0100
 * @tc.name: NewUpdateBuilder
 * @tc.desc: Test whether the return value of NewUpdateBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewUpdateBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewUpdateBuilder_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);

    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewUpdateBuilder(uri);
    EXPECT_NE(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewUpdateBuilder_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_NewUpdateBuilder_0200
 * @tc.name: NewUpdateBuilder
 * @tc.desc: Test whether the return value of NewUpdateBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewUpdateBuilder_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewUpdateBuilder_0200 start";
    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewUpdateBuilder(nullptr);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewUpdateBuilder_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_NewDeleteBuilder_0100
 * @tc.name: NewDeleteBuilder
 * @tc.desc: Test whether the return value of NewDeleteBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewDeleteBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewDeleteBuilder_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);

    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewDeleteBuilder(uri);
    EXPECT_NE(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewDeleteBuilder_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_NewDeleteBuilder_0200
 * @tc.name: NewDeleteBuilder
 * @tc.desc: Test whether the return value of NewDeleteBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewDeleteBuilder_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewDeleteBuilder_0200 start";
    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewDeleteBuilder(nullptr);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewDeleteBuilder_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_NewAssertBuilder_0100
 * @tc.name: NewAssertBuilder
 * @tc.desc: Test whether the return value of NewAssertBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewAssertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewAssertBuilder_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);

    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewAssertBuilder(uri);
    EXPECT_NE(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewAssertBuilder_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_NewAssertBuilder_0200
 * @tc.name: NewAssertBuilder
 * @tc.desc: Test whether the return value of NewAssertBuilder is DataAbilityOperationBuilder
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_NewAssertBuilder_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewAssertBuilder_0200 start";
    std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewAssertBuilder(nullptr);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_NewAssertBuilder_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Test whether the return value of GetType is 3
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetType_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetType(), 3);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetType_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetValuesBucket_0100
 * @tc.name: GetValuesBucket
 * @tc.desc: Test whether the return value of GetValuesBucket is nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetValuesBucket_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucket_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetValuesBucket(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucket_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetValuesBucket_0200
 * @tc.name: GetValuesBucket
 * @tc.desc: Test whether the return value of GetValuesBucket is not nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetValuesBucket_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucket_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<NativeRdb::ValuesBucket> values = std::make_shared<NativeRdb::ValuesBucket>();
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation =
        DataAbilityOperation::NewAssertBuilder(uri)->WithValuesBucket(values)->Build();
    EXPECT_NE(dataAbilityOperation->GetValuesBucket(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucket_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetExpectedCount_0100
 * @tc.name: GetExpectedCount
 * @tc.desc: Test whether the return value of GetExpectedCount is 0
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetExpectedCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetExpectedCount_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetExpectedCount(), 0);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetExpectedCount_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetExpectedCount_0200
 * @tc.name: GetExpectedCount
 * @tc.desc: Test whether the return value of GetExpectedCount is 10
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetExpectedCount_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetExpectedCount_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation =
        DataAbilityOperation::NewAssertBuilder(uri)->WithExpectedCount(10)->Build();
    EXPECT_EQ(dataAbilityOperation->GetExpectedCount(), 10);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetExpectedCount_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0100
 * @tc.name: GetDataAbilityPredicates
 * @tc.desc: Test whether the return value of GetDataAbilityPredicates is nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetDataAbilityPredicates(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0200
 * @tc.name: GetDataAbilityPredicates
 * @tc.desc: Test whether the return value of GetDataAbilityPredicates is not nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation =
        DataAbilityOperation::NewAssertBuilder(uri)->WithPredicates(predicates)->Build();
    EXPECT_NE(dataAbilityOperation->GetDataAbilityPredicates(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicates_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetValuesBucketReferences_0100
 * @tc.name: GetValuesBucketReferences
 * @tc.desc: Test whether the return value of GetValuesBucketReferences is nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetValuesBucketReferences_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucketReferences_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_EQ(dataAbilityOperation->GetValuesBucketReferences(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucketReferences_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetValuesBucketReferences_0200
 * @tc.name: GetValuesBucketReferences
 * @tc.desc: Test whether the return value of GetValuesBucketReferences is not nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetValuesBucketReferences_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucketReferences_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<NativeRdb::ValuesBucket> backReferences = std::make_shared<NativeRdb::ValuesBucket>();
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation =
        DataAbilityOperation::NewAssertBuilder(uri)->WithValueBackReferences(backReferences)->Build();
    EXPECT_NE(dataAbilityOperation->GetValuesBucketReferences(), nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetValuesBucketReferences_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0100
 * @tc.name: GetDataAbilityPredicatesBackReferences
 * @tc.desc: Test whether the return value of GetDataAbilityPredicatesBackReferences is empty
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    std::map<int, int> map = dataAbilityOperation->GetDataAbilityPredicatesBackReferences();
    EXPECT_TRUE(map.empty());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0200
 * @tc.name: GetDataAbilityPredicatesBackReferences
 * @tc.desc: Test whether the value of the return value is 5
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation =
        DataAbilityOperation::NewAssertBuilder(uri)->WithPredicatesBackReference(2, 5)->Build();
    std::map<int, int> map = dataAbilityOperation->GetDataAbilityPredicatesBackReferences();
    std::map<int, int>::iterator iter;
    iter = map.find(2);
    EXPECT_EQ(iter->second, 5);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_GetDataAbilityPredicatesBackReferences_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsInsertOperation_0100
 * @tc.name: IsInsertOperation
 * @tc.desc: Test whether the return value of IsInsertOperation is true
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsInsertOperation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInsertOperation_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_INSERT, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_TRUE(dataAbilityOperation->IsInsertOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInsertOperation_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsInsertOperation_0200
 * @tc.name: IsInsertOperation
 * @tc.desc: Test whether the return value of IsInsertOperation is false
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsInsertOperation_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInsertOperation_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_UPDATE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_FALSE(dataAbilityOperation->IsInsertOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInsertOperation_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsUpdateOperation_0100
 * @tc.name: IsUpdateOperation
 * @tc.desc: Test whether the return value of IsUpdateOperation is true
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsUpdateOperation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsUpdateOperation_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_UPDATE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_TRUE(dataAbilityOperation->IsUpdateOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsUpdateOperation_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsUpdateOperation_0200
 * @tc.name: IsUpdateOperation
 * @tc.desc: Test whether the return value of IsUpdateOperation is false
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsUpdateOperation_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsUpdateOperation_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_FALSE(dataAbilityOperation->IsUpdateOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsUpdateOperation_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsDeleteOperation_0100
 * @tc.name: IsDeleteOperation
 * @tc.desc: Test whether the return value of IsDeleteOperation is true
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsDeleteOperation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsDeleteOperation_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_TRUE(dataAbilityOperation->IsDeleteOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsDeleteOperation_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsDeleteOperation_0200
 * @tc.name: IsDeleteOperation
 * @tc.desc: Test whether the return value of IsDeleteOperation is false
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsDeleteOperation_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsDeleteOperation_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_ASSERT, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_FALSE(dataAbilityOperation->IsDeleteOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsDeleteOperation_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_IsAssertOperation_0100
 * @tc.name: IsAssertOperation
 * @tc.desc: Test whether the return value of IsAssertOperation is true
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsAssertOperation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsAssertOperation_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_ASSERT, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_TRUE(dataAbilityOperation->IsAssertOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsAssertOperation_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsAssertOperation_0200
 * @tc.name: IsAssertOperation
 * @tc.desc: Test whether the return value of IsAssertOperation is false
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsAssertOperation_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsAssertOperation_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_FALSE(dataAbilityOperation->IsAssertOperation());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsAssertOperation_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsInterruptionAllowed_0100
 * @tc.name: IsInterruptionAllowed
 * @tc.desc: Test whether the return value of IsInterruptionAllowed is false
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsInterruptionAllowed_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInterruptionAllowed_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);

    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_FALSE(dataAbilityOperation->IsInterruptionAllowed());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInterruptionAllowed_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_IsInterruptionAllowed_0200
 * @tc.name: IsInterruptionAllowed
 * @tc.desc: Test whether the return value of IsInterruptionAllowed is true
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperation_IsInterruptionAllowed_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInterruptionAllowed_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    builder = builder->WithInterruptionAllowed(true);
    EXPECT_NE(builder, nullptr);
    std::unique_ptr<DataAbilityOperation> dataAbilityOperation = std::make_unique<DataAbilityOperation>(builder);
    EXPECT_TRUE(dataAbilityOperation->IsInterruptionAllowed());
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_IsInterruptionAllowed_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_CreateFromParcel_0100
 * @tc.name: CreateFromParcel
 * @tc.desc: Test whether the return value of CreateFromParcel is not nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_CreateFromParcel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_CreateFromParcel_0100 start";
    Parcel in;
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = DataAbilityOperation::CreateFromParcel(in);
    EXPECT_NE(dataAbilityOperation, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_CreateFromParcel_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_Marshalling_0100
 * @tc.name: Marshalling
 * @tc.desc: Validation serialization.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Marshalling_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Marshalling_0100 start";

    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = DataAbilityOperation::NewAssertBuilder(uri)
                                                                     ->WithInterruptionAllowed(true)
                                                                     ->WithPredicatesBackReference(0, 0)
                                                                     ->Build();
    std::map<int, int> references = dataAbilityOperation->GetDataAbilityPredicatesBackReferences();
    for (int i = 0; i < 3 * 1024 * 1024 + 1; i++) {
        references.insert(std::make_pair(i, i));
    }

    Parcel out;
    dataAbilityOperation->Marshalling(out);

    EXPECT_TRUE(out.GetReadableBytes() < 3 * 1024 * 1024);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Marshalling_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperation_Unmarshalling_0100
 * @tc.name: Unmarshalling
 * @tc.desc: Validation serialization.
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperation_Unmarshalling_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Unmarshalling_0100 start";

    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    std::shared_ptr<DataAbilityOperation> dataAbilityOperation = DataAbilityOperation::NewAssertBuilder(uri)
                                                                     ->WithInterruptionAllowed(true)
                                                                     ->WithPredicatesBackReference(0, 0)
                                                                     ->Build();
    Parcel in;
    dataAbilityOperation->Marshalling(in);
    DataAbilityOperation *pDataAbilityOperation = DataAbilityOperation::Unmarshalling(in);
    std::map<int, int> references = dataAbilityOperation->GetDataAbilityPredicatesBackReferences();
    EXPECT_TRUE(references.size() == 1);
    delete pDataAbilityOperation;
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_Unmarshalling_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithValuesBucket_0100
 * @tc.name: WithValuesBucket
 * @tc.desc: Test whether the return value of WithValuesBucket is nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithValuesBucket_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithValuesBucket_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::shared_ptr<NativeRdb::ValuesBucket> values = std::make_shared<NativeRdb::ValuesBucket>();
    builder = builder->WithValuesBucket(values);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithValuesBucket_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithPredicates_0100
 * @tc.name: WithPredicates
 * @tc.desc: Test whether the return value of WithPredicates is nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithPredicates_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithPredicates_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_INSERT, uri);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = std::make_shared<NativeRdb::DataAbilityPredicates>();
    builder = builder->WithPredicates(predicates);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithPredicates_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithExpectedCount_0100
 * @tc.name: WithExpectedCount
 * @tc.desc: Test whether the return value of WithExpectedCount is nullptr
 */
HWTEST_F(
    DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithExpectedCount_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithExpectedCount_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_INSERT, uri);
    builder = builder->WithExpectedCount(1);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithExpectedCount_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithPredicatesBackReference_0100
 * @tc.name: WithPredicatesBackReference
 * @tc.desc: Test whether the return value of WithPredicatesBackReference is nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithPredicatesBackReference_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithPredicatesBackReference_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_INSERT, uri);
    builder = builder->WithPredicatesBackReference(1, 1);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithPredicatesBackReference_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithValueBackReferences_0100
 * @tc.name: WithValueBackReferences
 * @tc.desc: Test whether the return value of WithValueBackReferences is nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithValueBackReferences_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithValueBackReferences_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(DataAbilityOperation::TYPE_DELETE, uri);
    std::shared_ptr<NativeRdb::ValuesBucket> backReferences = std::make_shared<NativeRdb::ValuesBucket>();
    builder = builder->WithValueBackReferences(backReferences);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithValueBackReferences_0100 end";
}
/**
 * @tc.number: AaFwk_DataAbilityOperationBuilder_WithInterruptionAllowed_0100
 * @tc.name: WithInterruptionAllowed
 * @tc.desc: Test whether the return value of WithInterruptionAllowed is nullptr
 */
HWTEST_F(DataAbilityOperationTest, AaFwk_DataAbilityOperationBuilder_WithInterruptionAllowed_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithInterruptionAllowed_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(URI);
    CreateBuilder(0, uri);
    builder = builder->WithInterruptionAllowed(false);
    EXPECT_EQ(builder, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperationBuilder_WithInterruptionAllowed_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS