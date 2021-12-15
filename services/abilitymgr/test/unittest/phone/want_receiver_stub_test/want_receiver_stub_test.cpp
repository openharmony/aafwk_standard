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
#include "iremote_proxy.h"
#include "want_receiver_stub_impl_mock.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class WantReceiverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel &data);
    sptr<WantReceiverStubImplMock> stub_ {nullptr};
};

void WantReceiverStubTest::SetUpTestCase(void)
{}
void WantReceiverStubTest::TearDownTestCase(void)
{}
void WantReceiverStubTest::TearDown()
{}

void WantReceiverStubTest::SetUp()
{
    stub_ = new WantReceiverStubImplMock();
}

void WantReceiverStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(WantReceiverStubImplMock::GetDescriptor());
}

/*
 * Feature: WantReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: WantReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub OnRemoteRequest
 * EnvConditions: Description abnormal
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    int res = stub_->OnRemoteRequest(IWantSender::WANT_SENDER_SEND, data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/*
 * Feature: WantReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub OnRemoteRequest
 * EnvConditions: Code is WANT_SENDER_SEND
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    SenderInfo info;
    WriteInterfaceToken(data);
    data.WriteParcelable(&info);

    int res = stub_->OnRemoteRequest(IWantSender::WANT_SENDER_SEND, data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: WantReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub SendInner
 * EnvConditions: Valid parameter
 * CaseDescription: Verify the function SendInner request is normal.
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_SendInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(1);

    int res = stub_->SendInner(data, reply);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: WantReceiverStub
 * Function: PerformReceiveInner
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub PerformReceiveInner
 * EnvConditions: Invalid parameter
 * CaseDescription: Verify the function PerformReceiveInner request is abnormal.
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_PerformReceiveInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    int res = stub_->PerformReceiveInner(data, reply);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: WantReceiverStub
 * Function: PerformReceiveInner
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub PerformReceiveInner
 * EnvConditions: Invalid parameter
 * CaseDescription: Verify the function PerformReceiveInner request is abnormal.
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_PerformReceiveInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    Want want;
    data.WriteParcelable(&want);

    int res = stub_->PerformReceiveInner(data, reply);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: WantReceiverStub
 * Function: PerformReceiveInner
 * SubFunction: NA
 * FunctionPoints: WantReceiverStub PerformReceiveInner
 * EnvConditions: Valid parameter
 * CaseDescription: Verify the function PerformReceiveInner request is normal.
 */
HWTEST_F(WantReceiverStubTest, WantReceiverStubTest_PerformReceiveInner_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    Want want;
    WantParams wantParams;
    int senddingUser = 1000;
    bool sticky = true;
    bool serialized = true;
    std::string bundleName = "com.ix.helloworld";
    int resultCode = 1;

    EXPECT_EQ(data.WriteParcelable(&want), true);
    data.WriteInt32(resultCode);
    data.WriteString16(Str8ToStr16(bundleName));
    EXPECT_EQ(data.WriteParcelable(&wantParams), true);
    data.WriteBool(serialized);
    data.WriteBool(sticky);
    data.WriteInt32(senddingUser);

    int res = stub_->PerformReceiveInner(data, reply);

    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS