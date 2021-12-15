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
#include <memory>
#include "mock_dataobs_mgr_client.h"
#include "dataobs_mgr_proxy.h"
#define private public
#include "mock_dataobs_mgr_service.h"
#include "data_ability_observer_proxy.h"
#include "data_ability_observer_stub.h"
#include "mock_data_obs_manager_onchange_callback.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class DataObsMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsMgrClientTest::SetUpTestCase(void)
{}
void DataObsMgrClientTest::TearDownTestCase(void)
{}
void DataObsMgrClientTest::SetUp()
{}
void DataObsMgrClientTest::TearDown()
{}

/*
 * Feature: DataObsMgrClient.
 * Function: The RegisterObserver function of dataobsmgrservice was called.
 * SubFunction: DataObsMgrService::RegisterObserver is called.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_RegisterObserver_0100 start";

    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<DataAbilityObserverProxy> dataAbilityObserverProxy(new (std::nothrow) DataAbilityObserverProxy(callBack));

    MockDataObsMgrClient::GetInstance();

    EXPECT_CALL(*((MockDataObsMgrService *)(DataObsMgrClient::GetInstance()->remoteObject_).GetRefPtr()),
        RegisterObserverCall(testing::_, testing::_))
        .Times(1);
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/25");
    MockDataObsMgrClient::GetInstance()->RegisterObserver(uri, dataAbilityObserverProxy);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->remoteObject_);
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_RegisterObserver_0100 end";
}
/*
 * Feature: DataObsMgrClient.
 * Function: The unregisterObserve function of dataobsmgrservice was called.
 * SubFunction: DataObsMgrService::UnregisterObserve is called.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_UnregisterObserver_0100 start";

    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    sptr<DataAbilityObserverProxy> dataAbilityObserverProxy(new (std::nothrow) DataAbilityObserverProxy(callBack));

    MockDataObsMgrClient::GetInstance();

    EXPECT_CALL(*((MockDataObsMgrService *)(DataObsMgrClient::GetInstance()->remoteObject_).GetRefPtr()),
        UnregisterObserverCall(testing::_, testing::_))
        .Times(1);

    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/25");
    MockDataObsMgrClient::GetInstance()->UnregisterObserver(uri, dataAbilityObserverProxy);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->remoteObject_);
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrClient.
 * Function: The NotifyChange function of dataobsmgrservice was called.
 * SubFunction: DataObsMgrService::NotifyChange is called.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_NotifyChange_0100 start";

    MockDataObsMgrClient::GetInstance()->Connect();

    EXPECT_CALL(*((MockDataObsMgrService *)(DataObsMgrClient::GetInstance()->remoteObject_).GetRefPtr()),
        NotifyChangeCall(testing::_))
        .Times(1);

    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/25");
    MockDataObsMgrClient::GetInstance()->NotifyChange(uri);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->remoteObject_);
    GTEST_LOG_(INFO) << "DataObsMgrClientTest_DataObsMgrClient_NotifyChange_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS