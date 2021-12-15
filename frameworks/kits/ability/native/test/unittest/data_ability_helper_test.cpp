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

#include "gtest/gtest.h"

#include "mock_ability_manager_client_for_data_ability_observer.h"
#include "mock_ability_scheduler_for_observer.h"
#include "mock_ability_manager_client.h"
#include "context.h"
#include "ability_context.h"
#include "data_ability_helper.h"

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class DataAbilityHelperForObserverTest : public testing::Test {
public:
    DataAbilityHelperForObserverTest()
    {}
    virtual ~DataAbilityHelperForObserverTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityHelperForObserverTest::SetUpTestCase(void)
{}

void DataAbilityHelperForObserverTest::TearDownTestCase(void)
{
    MockAbilitySchedulerTools::GetInstance()->SetMockStatus(false);
}

void DataAbilityHelperForObserverTest::SetUp(void)
{}

void DataAbilityHelperForObserverTest::TearDown(void)
{
    MockAbilitySchedulerTools::DestoryInstance();
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0100
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    // Test to AbilityThread interface
    auto returnGetFileTypes = [&](const Uri &uri, const std::string &mimeTypeFilter) {
        std::vector<std::string> matchedMIMEs;
        matchedMIMEs.push_back("test1");
        matchedMIMEs.push_back("test2");
        matchedMIMEs.push_back("test3");
        return matchedMIMEs;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetFileTypes(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetFileTypes));

    dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0200
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    // Test to AbilityThread interface
    auto returnGetFileTypes = [&](const Uri &uri, const std::string &mimeTypeFilter) {
        std::vector<std::string> matchedMIMEs;
        matchedMIMEs.push_back("test1");
        matchedMIMEs.push_back("test2");
        matchedMIMEs.push_back("test3");
        return matchedMIMEs;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetFileTypes(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetFileTypes));

    dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0100
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string mode("modetest");
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0200
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mode("modetest");
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0100
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    // Test to AbilityThread interface
    auto returnInsert = [&](const Uri &uri, const NativeRdb::ValuesBucket &val) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Insert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnInsert));

    dataAbilityHelper->Insert(*uri, val);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0200
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    // Test to AbilityThread interface
    auto returnInsert = [&](const Uri &uri, const NativeRdb::ValuesBucket &val) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Insert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnInsert));

    dataAbilityHelper->Insert(*uri, val);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0100
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnUpdate = [&](const Uri &uri, const NativeRdb::ValuesBucket &val,
        const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Update(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnUpdate));

    dataAbilityHelper->Update(*uri, val, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0200
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnUpdate = [&](const Uri &uri, const NativeRdb::ValuesBucket &val,
        const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Update(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnUpdate));

    dataAbilityHelper->Update(*uri, val, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0100
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnDelete = [&](const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Delete(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDelete));

    dataAbilityHelper->Delete(*uri, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0200
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnDelete = [&](const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Delete(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDelete));

    dataAbilityHelper->Delete(*uri, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0100
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) {
            std::shared_ptr<NativeRdb::AbsSharedResultSet> set =
            std::make_shared<NativeRdb::AbsSharedResultSet>("resultset");
            return set;
        };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0200
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) {
            std::shared_ptr<NativeRdb::AbsSharedResultSet> set =
            std::make_shared<NativeRdb::AbsSharedResultSet>("resultset");
            return set;
        };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnGetType = [&](const Uri &uri) {
        std::string type("Type1");
        return type;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetType(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetType));

    dataAbilityHelper->GetType(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0200
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnGetType = [&](const Uri &uri) {
        std::string type("Type1");
        return type;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetType(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetType));

    dataAbilityHelper->GetType(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    std::string mode("modetest");
    auto returnOpenRawFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenRawFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenRawFile));

    dataAbilityHelper->OpenRawFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0200
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    std::string mode("modetest");
    auto returnOpenRawFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenRawFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenRawFile));

    dataAbilityHelper->OpenRawFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    PacMap extras;
    auto returnReload = [&](const Uri &uri, const PacMap &extras) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Reload(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnReload));

    dataAbilityHelper->Reload(*uri, extras);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0200
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    PacMap extras;
    auto returnReload = [&](const Uri &uri, const PacMap &extras) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Reload(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnReload));

    dataAbilityHelper->Reload(*uri, extras);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;
    auto returnBatchInsert = [&](const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), BatchInsert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnBatchInsert));

    dataAbilityHelper->BatchInsert(*uri, values);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0200
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;
    auto returnBatchInsert = [&](const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), BatchInsert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnBatchInsert));

    dataAbilityHelper->BatchInsert(*uri, values);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnNormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), NormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnNormalizeUri));

    dataAbilityHelper->NormalizeUri(*uri);
    
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0200
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnNormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), NormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnNormalizeUri));

    dataAbilityHelper->NormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0100 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnDenormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), DenormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDenormalizeUri));

    dataAbilityHelper->DenormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0200
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0200 start";
    
    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnDenormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), DenormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDenormalizeUri));

    dataAbilityHelper->DenormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS