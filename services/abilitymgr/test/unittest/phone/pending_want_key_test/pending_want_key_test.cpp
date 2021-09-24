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
#include "pending_want_key.h"
#undef private
#include "want.h"
#include "wants_info.h"

constexpr int PENDING_WANT_TYPE = 1;
const std::string PENDING_WANT_BUNDLENAME = "bundleName";
const std::string PENDING_WANT_WHO = "who";
constexpr int PENDING_WANT_REQUESTCODE = 2;
const std::string PENDING_WANT_REQUESTRESLOVEDTYPE = "requestResolvedType";
constexpr int PENDING_WANT_FLAGS = 10;
constexpr int PENDING_WANT_CODE = 20;
constexpr int PENDING_WANT_USERID = 123456;

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class PendingWantKeyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PendingWantKeyTest::SetUpTestCase(void)
{}
void PendingWantKeyTest::TearDownTestCase(void)
{}
void PendingWantKeyTest::SetUp(void)
{}
void PendingWantKeyTest::TearDown(void)
{}

/*
 * @tc.number    : SetType_0100
 * @tc.name      : set type
 * @tc.desc      : Set type, use GetType to verify whether the type value is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetType_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetType(PENDING_WANT_TYPE);
    EXPECT_EQ(PENDING_WANT_TYPE, amsPendingWantKey->GetType());
}

/*
 * @tc.number    : SetBundleName_0100
 * @tc.name      : set BundleName
 * @tc.desc      : Set BundleName, use GetBundleName to verify whether the BundleName  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetBundleName_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetBundleName(PENDING_WANT_BUNDLENAME);
    EXPECT_EQ(PENDING_WANT_BUNDLENAME, amsPendingWantKey->GetBundleName());
}

/*
 * @tc.number    : SetRequestWho_0100
 * @tc.name      : set RequestWho
 * @tc.desc      : Set RequestWho, use GetWho to verify whether the RequestWho is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetRequestWho_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetRequestWho(PENDING_WANT_WHO);
    EXPECT_EQ(PENDING_WANT_WHO, amsPendingWantKey->GetRequestWho());
}

/*
 * @tc.number    : SetRequestCode_0100
 * @tc.name      : set RequestCode
 * @tc.desc      : Set RequestCode, use GetRequestCode to verify whether the RequestCode  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetRequestCode_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetRequestCode(PENDING_WANT_REQUESTCODE);
    EXPECT_EQ(PENDING_WANT_REQUESTCODE, amsPendingWantKey->GetRequestCode());
}

/*
 * @tc.number    : SetRequestWant_0100
 * @tc.name      : set RequestWant
 * @tc.desc      : Set RequestWant, use GetRequestWant to verify whether the RequestWant  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetRequestWant_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    Want requestWant;
    OHOS::AAFwk::Operation operation;
    std::string bundleName = "ohos.pending.want.key.test";
    operation.SetBundleName(bundleName);
    requestWant.SetOperation(operation);
    amsPendingWantKey->SetRequestWant(requestWant);
    Want requestWant_ = amsPendingWantKey->GetRequestWant();
    EXPECT_EQ(bundleName, requestWant_.GetOperation().GetBundleName());
}

/*
 * @tc.number    : SetRequestResolvedType_0100
 * @tc.name      : set request resolved Type
 * @tc.desc      : Set RequestResolvedType, use GetRequestResolvedType to verify whether the RequestResolvedType  is
 * set successfully
 */
HWTEST_F(PendingWantKeyTest, SetRequestResolvedType_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetRequestResolvedType(PENDING_WANT_REQUESTRESLOVEDTYPE);
    EXPECT_EQ(PENDING_WANT_REQUESTRESLOVEDTYPE, amsPendingWantKey->GetRequestResolvedType());
}

/*
 * @tc.number    : SetAllWantsInfos_0100
 * @tc.name      : set All WantsInfos
 * @tc.desc      : Set AllWantsInfos, use GetAllWantsInfos to verify whether the AllWantsInfos  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetAllWantsInfos_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->allWantsInfos_.clear();
    std::vector<WantsInfo> allWantsInfos;
    WantsInfo wantsInfo;

    wantsInfo.resolvedTypes = "resolvedTypes";
    allWantsInfos.push_back(wantsInfo);
    amsPendingWantKey->SetAllWantsInfos(allWantsInfos);
    std::vector<WantsInfo> wantsInfo_ = amsPendingWantKey->GetAllWantsInfos();
    EXPECT_EQ(wantsInfo.resolvedTypes, wantsInfo_.front().resolvedTypes);
}

/*
 * @tc.number    : SetFlags_0100
 * @tc.name      : set Flags
 * @tc.desc      : Set Flags, use GetFlags to verify whether the Flags  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetFlags_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetFlags(PENDING_WANT_FLAGS);
    EXPECT_EQ(PENDING_WANT_FLAGS, amsPendingWantKey->GetFlags());
}

/*
 * @tc.number    : SetCode_0100
 * @tc.name      : set Code
 * @tc.desc      : Set Code, use GetCode to verify whether the Code  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetCode_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetCode(PENDING_WANT_CODE);
    EXPECT_EQ(PENDING_WANT_CODE, amsPendingWantKey->GetCode());
}

/*
 * @tc.number    : SetUserId_0100
 * @tc.name      : set UserId
 * @tc.desc      : Set UserId, use GetUserId to verify whether the UserId  is set successfully
 */
HWTEST_F(PendingWantKeyTest, SetUserId_0100, TestSize.Level1)
{
    std::unique_ptr<PendingWantKey> amsPendingWantKey = std::make_unique<PendingWantKey>();

    amsPendingWantKey->SetUserId(PENDING_WANT_USERID);
    EXPECT_EQ(PENDING_WANT_USERID, amsPendingWantKey->GetUserId());
}
}  // namespace AAFwk
}  // namespace OHOS