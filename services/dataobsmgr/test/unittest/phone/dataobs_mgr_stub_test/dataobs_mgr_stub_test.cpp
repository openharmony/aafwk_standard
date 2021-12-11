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
#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_data_obs_mgr_stub.h"

#include "dataobs_mgr_proxy.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
class DataObsManagerStubTest : public testing::Test {
public:
    DataObsManagerStubTest() = default;
    virtual ~DataObsManagerStubTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsManagerStubTest::SetUpTestCase(void)
{}
void DataObsManagerStubTest::TearDownTestCase(void)
{}
void DataObsManagerStubTest::SetUp()
{}
void DataObsManagerStubTest::TearDown()
{}

/*
 * Feature: DataObsManagerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub OnRemoteRequest is normal.
 */
HWTEST_F(DataObsManagerStubTest, AaFwk_DataObsManagerStubTest_OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_OnRemoteRequest_0100 start";
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    const int testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    uint32_t code = IDataObsMgr::NOTIFY_CHANGE + 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        GTEST_LOG_(ERROR) << "---------- WriteInterfaceToken(data) retval is false end";
        return;
    }

    const int retval = dataobs->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(testVal, retval);
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_OnRemoteRequest_0100 end";
}

/*
 * Feature: DataObsManagerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub OnRemoteRequest is normal.
 */
HWTEST_F(DataObsManagerStubTest, AaFwk_DataObsManagerStubTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_RegisterObserver_0100 start";
    const int testVal1 = static_cast<int>(NO_ERROR);
    const int testVal2 = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    uint32_t code = IDataObsMgr::REGISTER_OBSERVER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        GTEST_LOG_(ERROR) << "---------- WriteInterfaceToken(data) retval is false end";
        return;
    }
    if (!data.WriteParcelable(uri.get())) {
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable(uri) retval is false end";
        return;
    }
    if (dataObserver == nullptr) {
        return;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable(dataObserver->AsObject()) retval is false end";
        return;
    }

    EXPECT_CALL(*dataobs, RegisterObserver(testing::_, testing::_)).Times(1).WillOnce(testing::Return(testVal2));

    const int retval1 = dataobs->OnRemoteRequest(code, data, reply, option);
    const int retval2 = reply.ReadInt32();

    EXPECT_EQ(testVal1, retval1);
    EXPECT_EQ(testVal2, retval2);
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerStub
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub UnregisterObserver is normal.
 */
HWTEST_F(DataObsManagerStubTest, AaFwk_DataObsManagerStubTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_UnregisterObserver_0100 start";
    const int testVal1 = static_cast<int>(NO_ERROR);
    const int testVal2 = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    uint32_t code = IDataObsMgr::UNREGISTER_OBSERVER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        GTEST_LOG_(ERROR) << "---------- WriteInterfaceToken(data) retval is false end";
        return;
    }
    if (!data.WriteParcelable(uri.get())) {
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable(uri) retval is false end";
        return;
    }
    if (dataObserver == nullptr) {
        return;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable(dataObserver->AsObject()) retval is false end";
        return;
    }

    EXPECT_CALL(*dataobs, UnregisterObserver(testing::_, testing::_)).Times(1).WillOnce(testing::Return(testVal2));

    const int retval1 = dataobs->OnRemoteRequest(code, data, reply, option);
    const int retval2 = reply.ReadInt32();

    EXPECT_EQ(testVal1, retval1);
    EXPECT_EQ(testVal2, retval2);
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerStub
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub NotifyChange is normal.
 */
HWTEST_F(DataObsManagerStubTest, AaFwk_DataObsManagerStubTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_NotifyChange_0100 start";
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const int testVal1 = static_cast<int>(NO_ERROR);
    const int testVal2 = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    uint32_t code = IDataObsMgr::NOTIFY_CHANGE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        GTEST_LOG_(ERROR) << "---------- WriteInterfaceToken(data) retval is false end";
        return;
    }
    if (!data.WriteParcelable(uri.get())) {
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable(uri) retval is false end";
        return;
    }

    EXPECT_CALL(*dataobs, NotifyChange(testing::_)).Times(1).WillOnce(testing::Return(testVal2));

    const int retval1 = dataobs->OnRemoteRequest(code, data, reply, option);
    const int retval2 = reply.ReadInt32();

    EXPECT_EQ(testVal1, retval1);
    EXPECT_EQ(testVal2, retval2);
    GTEST_LOG_(INFO) << "AaFwk_DataObsManagerStubTest_NotifyChange_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS