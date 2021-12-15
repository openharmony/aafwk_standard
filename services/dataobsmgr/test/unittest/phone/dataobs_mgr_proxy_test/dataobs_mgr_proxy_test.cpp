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
using ::testing::_;

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
 * Feature: DataObsManagerStub
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 start";
    const int testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    const int retVal = dataObsManagerProxy->RegisterObserver(*uri, dataObserver);

    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerStub
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub UnregisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 start";
    const int testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    const int retVal = dataObsManagerProxy->UnregisterObserver(*uri, dataObserver);

    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerStub
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsManagerStub NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerStub NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 start";
    const int testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);

    const int retVal = dataObsManagerProxy->NotifyChange(*uri);

    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS