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
#include "parcel.h"
#define private public
#define protected public
#include "lock_screen_white_list.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
namespace {
const std::string AMS_WHITE_LIST_FILE_PATH {"/data/ams_white_list/ams_lock_screen_white_list.json"};
}

namespace OHOS {
namespace AAFwk {
class LockScreenWhiteListTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<LockScreenWhiteList> lockScreenWhiteList_;
};

void LockScreenWhiteListTest::SetUpTestCase()
{}

void LockScreenWhiteListTest::TearDownTestCase()
{}

void LockScreenWhiteListTest::SetUp()
{
    lockScreenWhiteList_ = std::make_shared<LockScreenWhiteList>();
}

void LockScreenWhiteListTest::TearDown()
{
    lockScreenWhiteList_.reset();
}

/*
 * Feature: LockScreenWhiteList
 * Function: IsExistFile
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(LockScreenWhiteListTest, IsExistFile_0001, TestSize.Level1)
{
    std::string path;
    auto ret = lockScreenWhiteList_->IsExistFile(path);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: LockScreenWhiteList
 * Function: IsExistFile
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(LockScreenWhiteListTest, IsExistFile_0002, TestSize.Level1)
{
    std::string path = AMS_WHITE_LIST_FILE_PATH;
    auto ret = lockScreenWhiteList_->IsExistFile(path);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: LockScreenWhiteList
 * Function: FindBundleNameOnWhiteList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(LockScreenWhiteListTest, FindBundleNameOnWhiteList_0001, TestSize.Level1)
{
    std::string bundleName = "com.ix.hiMusic";
    bool isAwakenScreen = true;
    lockScreenWhiteList_->SetWhiteListInfo(bundleName, isAwakenScreen);
    auto ret = lockScreenWhiteList_->FindBundleNameOnWhiteList(bundleName, isAwakenScreen);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: LockScreenWhiteList
 * Function: FindBundleNameOnWhiteList
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(LockScreenWhiteListTest, FindBundleNameOnWhiteList_0002, TestSize.Level1)
{
    std::string bundleName = "";
    bool isAwakenScreen = false;
    auto ret = lockScreenWhiteList_->FindBundleNameOnWhiteList(bundleName, isAwakenScreen);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: LockScreenWhiteList
 * Function: SetWhiteListInfo
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(LockScreenWhiteListTest, SetWhiteListInfo_0001, TestSize.Level1)
{
    std::string bundleName = "com.ix.hiMusic";
    bool isAwakenScreen = true;
    auto ret = lockScreenWhiteList_->SetWhiteListInfo(bundleName, isAwakenScreen);
    EXPECT_EQ(ret, true);
    nlohmann::json jsonFile;
    EXPECT_EQ(lockScreenWhiteList_->GetWhiteListInfo(jsonFile), true);
}
}  // namespace AAFwk
}  // namespace OHOS