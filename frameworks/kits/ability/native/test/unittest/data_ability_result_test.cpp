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
#include "data_ability_result.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

static const int COUNT_NULL = 0;
static const int SET_COUNT = 1;
static const Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
class DataAbilityResultTest : public testing::Test {
public:
    DataAbilityResultTest()
    {}
    ~DataAbilityResultTest()
    {}

    std::shared_ptr<DataAbilityResult> Base_ = nullptr;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityResultTest::SetUpTestCase(void)
{}

void DataAbilityResultTest::TearDownTestCase(void)
{}

void DataAbilityResultTest::SetUp(void)
{
    Base_ = std::make_shared<DataAbilityResult>(COUNT_NULL);
}

void DataAbilityResultTest::TearDown(void)
{}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_DataAbilityResult0100, Function | MediumTest | Level1)
{
    DataAbilityResult test1(SET_COUNT);
    EXPECT_EQ(SET_COUNT, test1.GetCount());
    EXPECT_EQ(std::string(""), test1.GetUri().ToString());

    DataAbilityResult test2(uri);
    EXPECT_EQ(COUNT_NULL, test2.GetCount());
    EXPECT_EQ(uri.ToString(), test2.GetUri().ToString());

    DataAbilityResult test3(uri, SET_COUNT);
    EXPECT_EQ(SET_COUNT, test3.GetCount());
    EXPECT_EQ(uri.ToString(), test3.GetUri().ToString());

    *Base_ = test3;
    if (Base_) {
        Parcel in;
        Base_->Marshalling(in);
        DataAbilityResult test4(in);
        EXPECT_EQ(SET_COUNT, test4.GetCount());
        EXPECT_EQ(uri.ToString(), test4.GetUri().ToString());
    }
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_DataAbilityResult0200, Function | MediumTest | Level1)
{
    DataAbilityResult test1(COUNT_NULL);
    EXPECT_EQ(COUNT_NULL, test1.GetCount());
    EXPECT_EQ(std::string(""), test1.GetUri().ToString());

    Uri zero("");
    DataAbilityResult test2(zero);
    EXPECT_EQ(COUNT_NULL, test2.GetCount());
    EXPECT_EQ(zero.ToString(), test2.GetUri().ToString());

    DataAbilityResult test3(uri, SET_COUNT);
    EXPECT_EQ(SET_COUNT, test3.GetCount());
    EXPECT_EQ(uri.ToString(), test3.GetUri().ToString());

    Parcel in;
    test3.Marshalling(in);
    DataAbilityResult test4(in);
    EXPECT_EQ(SET_COUNT, test4.GetCount());
    EXPECT_EQ(uri.ToString(), test4.GetUri().ToString());
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_Get0100, Function | MediumTest | Level1)
{
    DataAbilityResult test(uri, SET_COUNT);
    EXPECT_EQ(SET_COUNT, test.GetCount());
    EXPECT_EQ(uri.ToString(), test.GetUri().ToString());
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_Get0200, Function | MediumTest | Level1)
{
    EXPECT_EQ(true, Base_->GetUri().ToString().empty());
    EXPECT_EQ(COUNT_NULL, Base_->GetCount());
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_CreateFromParcel0100, Function | MediumTest | Level1)
{
    Parcel in;
    Base_->Marshalling(in);
    DataAbilityResult *ptr = Base_->CreateFromParcel(in);

    if (ptr != nullptr) {
        EXPECT_EQ(true, ptr->GetUri().ToString().empty());
        EXPECT_EQ(COUNT_NULL, ptr->GetCount());
    }
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_ToString0100, Function | MediumTest | Level1)
{
    std::string search = "DataAbilityResult(";
    std::size_t pos = 0;
    std::size_t result = 0;

    DataAbilityResult dataAbilityResult(uri, SET_COUNT);
    std::string str = dataAbilityResult.ToString();

    result = str.find(search, pos);
    EXPECT_EQ(result, pos);
    if (result != std::string::npos) {
        pos += search.length() - 1;
    }

    search = std::string("uri=") + uri.ToString() + std::string(" ");
    result = str.find(search, pos);

    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, pos);
    if (result != std::string::npos) {
        pos += search.length();
    }

    search = std::string("count=") + std::to_string(dataAbilityResult.GetCount()) + std::string(")");
    result = str.find(search, pos);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, pos);
    if (result != std::string::npos) {
        pos += search.length() + 1;
    }

    EXPECT_EQ(str.length(), pos);
}

/**
 * @tc.number:
 * @tc.name:
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_ToString0200, Function | MediumTest | Level1)
{

    std::string search = "DataAbilityResult(";
    std::size_t pos = 0;
    std::size_t result = 0;

    DataAbilityResult dataAbilityResult(COUNT_NULL);
    std::string str = dataAbilityResult.ToString();

    result = str.find(search, pos);
    EXPECT_EQ(result, pos);
    if (result != std::string::npos) {
        pos += search.length() - 1;
    }

    search = std::string("uri=") + dataAbilityResult.GetUri().ToString() + std::string(" ");
    result = str.find(search, pos);

    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, pos);
    if (result != std::string::npos) {
        pos += search.length();
    }

    search = std::string("count=") + std::to_string(dataAbilityResult.GetCount()) + std::string(")");
    result = str.find(search, pos);
    EXPECT_NE(result, std::string::npos);
    EXPECT_GE(result, pos);
    if (result != std::string::npos) {
        pos += search.length() + 1;
    }

    EXPECT_EQ(str.length(), pos);
}

/**
 * @tc.number:
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc: marshalling
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_Parcelable_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<DataAbilityResult> ResultIn_ = std::make_shared<DataAbilityResult>(COUNT_NULL);
    if (ResultIn_ == nullptr) {
        return;
    }

    Parcel in;
    ResultIn_->Marshalling(in);
    std::shared_ptr<DataAbilityResult> ResultOut_(DataAbilityResult::Unmarshalling(in));

    if (ResultOut_ != nullptr) {
        EXPECT_EQ(ResultIn_->GetUri().ToString(), ResultOut_->GetUri().ToString());
        EXPECT_EQ(ResultIn_->GetCount(), ResultOut_->GetCount());
    }
}

/**
 * @tc.number:
 * @tc.name: Marshalling/Unmarshalling
 * @tc.desc:
 */
HWTEST_F(DataAbilityResultTest, AppExecFwk_DataAbilityResult_Parcelable_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<DataAbilityResult> ResultIn_ = std::make_shared<DataAbilityResult>(uri);
    if (ResultIn_ == nullptr) {
        return;
    }

    Parcel in;
    ResultIn_->Marshalling(in);
    std::shared_ptr<DataAbilityResult> ResultOut_(DataAbilityResult::Unmarshalling(in));

    if (ResultOut_ != nullptr) {
        EXPECT_EQ(ResultIn_->GetUri().ToString(), ResultOut_->GetUri().ToString());
        EXPECT_EQ(ResultIn_->GetCount(), ResultOut_->GetCount());
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
