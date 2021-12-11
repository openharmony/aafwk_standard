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

#include "mock_data_ability_observer_stub.h"

#include "dataobs_mgr_service.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
class DataObsMgrServiceTest : public testing::Test {
public:
    DataObsMgrServiceTest() = default;
    virtual ~DataObsMgrServiceTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsMgrServiceTest::SetUpTestCase(void)
{}
void DataObsMgrServiceTest::TearDownTestCase(void)
{}
void DataObsMgrServiceTest::SetUp()
{}
void DataObsMgrServiceTest::TearDown()
{}

/*
 * Feature: DataObsMgrService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService QueryServiceState
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService could query service state.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_QueryServiceState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: GetEventHandler
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService GetEventHandler
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService could get eventHandler.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_GetEventHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_GetEventHandler_0100 start";
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    const std::shared_ptr<EventHandler> retVal = dataObsMgrServer->GetEventHandler();

    EXPECT_EQ(nullptr, retVal.get());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_GetEventHandler_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: OnStart
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService OnStart
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService OnStart is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStart_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_RUNNING;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStart();
    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());
    EXPECT_NE(nullptr, dataObsMgrServer->GetEventHandler().get());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStart_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_CALL(*dataobsAbility, OnChange()).Times(1).WillOnce(testing::Return());
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: OnStop
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService OnStop
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService OnStop is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_OnStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());
    EXPECT_EQ(nullptr, dataObsMgrServer->GetEventHandler().get());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS