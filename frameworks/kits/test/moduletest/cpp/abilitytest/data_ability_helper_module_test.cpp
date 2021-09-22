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
#include "gtest/gtest.h"
#include "mock_ability_test.h"
#include "mock_ability_manager_client.h"
#include "mock_ability_thread_for_data_observer.h"
#include "context.h"
#include "ability_context.h"
#include "data_ability_helper.h"

namespace OHOS {
namespace AppExecFwk {

using namespace testing::ext;
REGISTER_AA(MockAbilityTest)
/*
 * Parameters:
 * Action
 * Entity
 * Flag
 * ElementName
 */
class DataAbilityHelperTest : public testing::Test {
public:
    DataAbilityHelperTest(){};
    virtual ~DataAbilityHelperTest(){};

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
};

void DataAbilityHelperTest::SetUpTestCase(void)
{}

void DataAbilityHelperTest::TearDownTestCase(void)
{}

void DataAbilityHelperTest::SetUp(void)
{}

void DataAbilityHelperTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Insert with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Insert_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0100 start";

    sptr<AppExecFwk::AbilityThread> thread = nullptr;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("22", 22);
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");

    int value = dataAbilityHelper->Insert(urivalue, val);

    EXPECT_EQ(value, 20);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: Insert with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Insert_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0200 start";

    sptr<AppExecFwk::AbilityThread> thread = nullptr;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("22", 20);
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");

    int value = dataAbilityHelper->Insert(urivalue, val);

    EXPECT_EQ(value, 20);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Update with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Update_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    NativeRdb::ValuesBucket val;
    val.PutInt("22", 22);
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    NativeRdb::DataAbilityPredicates predicates;
    int value = dataAbilityHelper->Update(urivalue, val, predicates);
    EXPECT_EQ(value, 33);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: Update with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Update_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0200 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    NativeRdb::ValuesBucket val;
    val.PutInt("22", 22);
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    NativeRdb::DataAbilityPredicates predicates;
    int value = dataAbilityHelper->Update(urivalue, val, predicates);
    EXPECT_EQ(value, 33);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: GetFileTypes with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_GetFileTypes_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    std::string mimeTypeFilter("mimeTypeFiltertest");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::vector<std::string> result = dataAbilityHelper->GetFileTypes(urivalue, mimeTypeFilter);

    std::vector<std::string> list;
    list.push_back("Type1");
    list.push_back("Type2");
    list.push_back("Type3");

    for (size_t i = 0; i < result.size(); i++) {
        EXPECT_STREQ(result.at(i).c_str(), list.at(i).c_str());
    }

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: GetFileTypes with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_GetFileTypes_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::string mimeTypeFilter("mimeTypeFiltertest");
    std::vector<std::string> result = dataAbilityHelper->GetFileTypes(urivalue, mimeTypeFilter);

    std::vector<std::string> list;
    list.push_back("Type1");
    list.push_back("Type2");
    list.push_back("Type3");

    for (unsigned int i = 0; i < result.size(); i++) {
        EXPECT_STREQ(result.at(i).c_str(), list.at(i).c_str());
    }

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Openfile with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_OpenFile_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10?test/te.txt");
    std::string mode("r");
    int fd = dataAbilityHelper->OpenFile(urivalue, mode);
    EXPECT_NE(fd, -1);

    std::string result = "123456";
    FILE *file = fdopen(fd, "r");
    EXPECT_NE(file, nullptr);

    int strSize = 7;
    std::string str("");
    str.resize(strSize);
    if (!feof(file)) {
        fgets(&str[0], strSize, file);
    }
    string stringstr(str);
    EXPECT_STREQ(stringstr.c_str(), result.c_str());

    fclose(file);
    system("rm /dataability_openfile_test.txt");

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_Test_0200
 * @tc.name: DataAbilityHelper Insert
 * @tc.desc: Openfile with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_OpenFile_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10?test/te.txt");
    std::string mode("r");
    int fd = dataAbilityHelper->OpenFile(urivalue, mode);
    EXPECT_NE(fd, -1);

    std::string result = "123456";
    FILE *file = fdopen(fd, "r");
    EXPECT_NE(file, nullptr);

    int strSize = 7;
    std::string str("");
    str.resize(strSize);
    if (!feof(file)) {
        fgets(&str[0], strSize, file);
    }
    string stringstr(str);
    EXPECT_STREQ(stringstr.c_str(), result.c_str());

    fclose(file);
    system("rm /dataability_openfile_test.txt");
    
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: OpenRawFile with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_OpenRawFile_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::string mode("modetest");
    int fd = dataAbilityHelper->OpenRawFile(urivalue, mode);

    EXPECT_EQ(fd, 122);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_Test_0200
 * @tc.name: DataAbilityHelper Insert
 * @tc.desc: OpenRawFile with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_OpenRawFile_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::string mode("modetest");
    int fd = dataAbilityHelper->OpenRawFile(urivalue, mode);

    EXPECT_EQ(fd, 122);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: BatchInsert with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_BatchInsert_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::vector<NativeRdb::ValuesBucket> values;
    int fd = dataAbilityHelper->BatchInsert(urivalue, values);

    EXPECT_EQ(fd, 115);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_Test_0200
 * @tc.name: DataAbilityHelper Insert
 * @tc.desc: BatchInsert with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_BatchInsert_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::vector<NativeRdb::ValuesBucket> values;
    int fd = dataAbilityHelper->BatchInsert(urivalue, values);

    EXPECT_EQ(fd, 115);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Reload with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Reload_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    PacMap extras;
    int fd = dataAbilityHelper->Reload(urivalue, extras);

    EXPECT_EQ(fd, true);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: Reload with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Reload_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    PacMap extras;
    int fd = dataAbilityHelper->Reload(urivalue, extras);

    EXPECT_EQ(fd, true);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Delete with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Delete_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    NativeRdb::DataAbilityPredicates predicates;
    int index = dataAbilityHelper->Delete(urivalue, predicates);

    EXPECT_EQ(index, 234);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: Delete with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Delete_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    NativeRdb::DataAbilityPredicates predicates;
    int index = dataAbilityHelper->Delete(urivalue, predicates);

    EXPECT_EQ(index, 234);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_Test_0200 end";
}
/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_Test_0100
 * @tc.name: DataAbilityHelper 
 * @tc.desc: Query with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Query_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");

    std::vector<std::string> columns;
    columns.push_back("string1");
    columns.push_back("string2");
    columns.push_back("string3");
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_NE(nullptr, dataAbilityHelper->Query(urivalue, columns, predicates));

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_Test_0200
 * @tc.name: DataAbilityHelper Insert
 * @tc.desc: Query with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_Query_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");

    std::vector<std::string> columns;
    columns.push_back("string1");
    columns.push_back("string2");
    columns.push_back("string3");
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_NE(nullptr, dataAbilityHelper->Query(urivalue, columns, predicates));

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Gettype with DataAbilityHelper which created with uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_GetType_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::string type = dataAbilityHelper->GetType(urivalue);

    EXPECT_STREQ(type.c_str(), "Type1");

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_Test_0200
 * @tc.name: DataAbilityHelper
 * @tc.desc: GetType with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_GetType_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::string type = dataAbilityHelper->GetType(urivalue);

    EXPECT_STREQ(type.c_str(), "Type1");

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_Test_0100
 * @tc.name: DataAbilityHelper NormalizeUri
 * @tc.desc: NormalizeUri with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_NormalizeUri_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri type("");
    type = dataAbilityHelper->NormalizeUri(urivalue);

    EXPECT_STREQ(type.ToString().c_str(), urivalue.ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_Test_0200
 * @tc.name: DataAbilityHelper NormalizeUri
 * @tc.desc: NormalizeUri with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_NormalizeUri_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri type("");
    type = dataAbilityHelper->NormalizeUri(urivalue);

    EXPECT_STREQ(type.ToString().c_str(), urivalue.ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_Test_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_Test_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: DenormalizeUri with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_DenormalizeUri_Test_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_Test_0100 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri type("");
    type = dataAbilityHelper->DenormalizeUri(urivalue);

    EXPECT_STREQ(type.ToString().c_str(), urivalue.ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_Test_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_Test_0200
 * @tc.name: DataAbilityHelper DenormalizeUri
 * @tc.desc: DenormalizeUri with DataAbilityHelper which created without uri.
 */
HWTEST_F(DataAbilityHelperTest, AaFwk_DataAbilityHelper_DenormalizeUri_Test_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_Test_0200 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);

    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri type("");
    type = dataAbilityHelper->DenormalizeUri(urivalue);

    EXPECT_STREQ(type.ToString().c_str(), urivalue.ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_Test_0200 end";
}


}  // namespace AppExecFwk
}  // namespace OHOS