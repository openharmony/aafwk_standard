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

#define private public
#include "want_receiver_proxy.h"
#undef private

#include <gtest/gtest.h>
#include "want_receiver_stub_mock.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class WantReceiverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<WantReceiverProxy> proxy_{nullptr};
    sptr<WantReceiverStubMock> mock_{nullptr};
};

void WantReceiverProxyTest::SetUpTestCase(void)
{}
void WantReceiverProxyTest::TearDownTestCase(void)
{}
void WantReceiverProxyTest::TearDown()
{}
void WantReceiverProxyTest::SetUp()
{
    mock_ = new WantReceiverStubMock();
    proxy_ = std::make_shared<WantReceiverProxy>(mock_);
}

/*
 * Feature: WantReceiverProxy
 * Function: Send
 * SubFunction: NA
 * FunctionPoints: WantReceiverProxy Send
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of Send is normal
 */
HWTEST_F(WantReceiverProxyTest, WantReceiverProxyTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &WantReceiverStubMock::InvokeSendRequest));
    int32_t resultCode = 0;
    proxy_->Send(resultCode);

    EXPECT_EQ(IWantReceiver::WANT_RECEIVER_SEND, mock_->code_);
}

/*
 * Feature: WantReceiverProxy
 * Function: Send
 * SubFunction: NA
 * FunctionPoints: WantReceiverProxy Send
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of PerformReceive is normal
 */
HWTEST_F(WantReceiverProxyTest, WantReceiverProxyTest_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &WantReceiverStubMock::InvokeSendRequest));
    const Want want;
    const WantParams wantParams;
    std::string data = "hello world";
    proxy_->PerformReceive(want, 0, data, wantParams, true, true, 0);

    EXPECT_EQ(IWantReceiver::WANT_RECEIVER_PERFORM_RECEIVE, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS