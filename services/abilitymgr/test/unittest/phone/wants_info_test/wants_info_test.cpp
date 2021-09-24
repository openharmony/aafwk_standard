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
#include "sender_info.h"
#define private public
#define protected public
#include "wants_info.h"
#undef private
#undef protected
#include "want_receiver_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))
namespace {}  // namespace
class WantsInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static constexpr int TEST_WAIT_TIME = 100000;
    class CancelReceiver : public AAFwk::WantReceiverStub {
    public:
        int performReceiveCount = 0;
        void Send(const int32_t resultCode) override;
        void PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
            const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser) override;
    };

public:
};

void WantsInfoTest::CancelReceiver::Send(const int32_t resultCode)
{}
void WantsInfoTest::CancelReceiver::PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
    const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser)
{}

void WantsInfoTest::SetUpTestCase()
{}

void WantsInfoTest::TearDownTestCase()
{}

void WantsInfoTest::SetUp()
{}

void WantsInfoTest::TearDown()
{}

/*
 * @tc.number    : WantsInfoTest_0100
 * @tc.name      : Marshalling/UnMarshalling
 * @tc.desc      : 1.Marshalling/UnMarshalling
 */
HWTEST_F(WantsInfoTest, WantsInfoTest_0100, TestSize.Level1)
{
    WantsInfo info;
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicSAbility");
    want.SetElement(element);
    info.want = want;
    info.resolvedTypes = "nihao";
    Parcel parcel;
    info.Marshalling(parcel);
    auto unInfo = WantsInfo::Unmarshalling(parcel);
    EXPECT_EQ(unInfo->want.GetElement().GetBundleName(), "com.ix.hiMusic");
    EXPECT_EQ(unInfo->want.GetElement().GetAbilityName(), "MusicSAbility");
    EXPECT_EQ(unInfo->resolvedTypes, "nihao");
    delete unInfo;
}
}  // namespace AAFwk
}  // namespace OHOS