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
#include "mock_data_obs_manager_onchange_callback.h"
#define private public
#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_interface.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class DataAbilityObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityObserverStubTest::SetUpTestCase(void)
{}
void DataAbilityObserverStubTest::TearDownTestCase(void)
{}
void DataAbilityObserverStubTest::SetUp()
{}
void DataAbilityObserverStubTest::TearDown()
{}

/*
 * Feature: DataAbilityObserverStub.
 * Function: DataAbilityObserverStub::UnregisterObserverInner is called.
 * SubFunction: UnregisterObserverInner.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataAbilityObserverStubTest, DataAbilityObserverStub_remoteDescriptor_001, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> mockDataObsManagerOnChangeStub(new MockDataObsManagerOnChangeCallBack());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(Str8ToStr16(std::string("descrip_test")));

    EXPECT_CALL(*mockDataObsManagerOnChangeStub, OnChange()).Times(0);
    int res = mockDataObsManagerOnChangeStub->OnRemoteRequest(
        IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}
/*
 * Feature: DataAbilityObserverStub.
 * Function: DataAbilityObserverStub::UnregisterObserverInner is called.
 * SubFunction: UnregisterObserverInner.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataAbilityObserverStubTest, DataAbilityObserverStub_OnChange_001, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> mockDataObsManagerOnChangeStub(new MockDataObsManagerOnChangeCallBack());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor());

    EXPECT_CALL(*mockDataObsManagerOnChangeStub, OnChange()).Times(1);

    int res = mockDataObsManagerOnChangeStub->OnRemoteRequest(
        IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option);
    EXPECT_EQ(res, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS