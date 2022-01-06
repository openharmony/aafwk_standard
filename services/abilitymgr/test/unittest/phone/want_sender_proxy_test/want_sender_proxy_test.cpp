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
#include "want_sender_proxy.h"
#undef private

#include <gtest/gtest.h>
#include "want_sender_stub_mock.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class WantSenderProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<WantSenderProxy> proxy_ {nullptr};
    sptr<WantSenderStubMock> mock_ {nullptr};
};

void WantSenderProxyTest::SetUpTestCase(void)
{}
void WantSenderProxyTest::TearDownTestCase(void)
{}
void WantSenderProxyTest::TearDown()
{}
void WantSenderProxyTest::SetUp()
{
    mock_ = new WantSenderStubMock();
    proxy_ = std::make_shared<WantSenderProxy>(mock_);
}

/*
 * Feature: WantSendProxy
 * Function: Send
 * SubFunction: NA
 * FunctionPoints: WantSendProxy Send
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of Send is normal
 */
HWTEST_F(WantSenderProxyTest, WantSenderProxyTest_001, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &WantSenderStubMock::InvokeSendRequest));
    SenderInfo senderInfo;
    proxy_->Send(senderInfo);

    EXPECT_EQ(IWantSender::WANT_SENDER_SEND, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS