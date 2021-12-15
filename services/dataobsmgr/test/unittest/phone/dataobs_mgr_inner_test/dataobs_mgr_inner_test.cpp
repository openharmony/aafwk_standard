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
#include <algorithm>
#include <functional>
#include "uri.h"
#define private public
#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_inner.h"
#include "mock_data_ability_observer_stub.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;

using Uri = OHOS::Uri;
using ObsListType = OHOS::AAFwk::DataObsMgrInner::ObsListType;
using ObsRecipientMapType = OHOS::AAFwk::DataObsMgrInner::ObsRecipientMapType;

namespace OHOS {
namespace AAFwk {
class DataObsMgrInnerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_ = nullptr;
};
void DataObsMgrInnerTest::SetUpTestCase(void)
{}
void DataObsMgrInnerTest::TearDownTestCase(void)
{}
void DataObsMgrInnerTest::SetUp()
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
}
void DataObsMgrInnerTest::TearDown()
{}

/*
 * Feature: DataObsMgrInner
 * Function: Register and unregister function test
 * SubFunction: HandleRegisterObserver/HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleRegisterObserver_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }

    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);

    EXPECT_EQ(dataObsMgrInner_->ObsExistInMap(callback), true);
    dataObsMgrInner_->HandleUnregisterObserver(uri, callback);
    EXPECT_EQ(dataObsMgrInner_->ObsExistInMap(callback), false);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register and unregister function test
 * SubFunction: OnChange
 * FunctionPoints: When the data changes, call the OnChange function of the registered dataabilityobserver
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleNotifyChange_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    EXPECT_CALL(*mockDataAbilityObserverStub, OnChange()).Times(1);

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);
    dataObsMgrInner_->HandleNotifyChange(uri);
}

/*
 * Feature: DataObsMgrInner
 * Function: GetObsListFromMap/RemoveObsFromMap/ObsExistInMap function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_GetRemoveObsListFromMap_ObsExistInMap_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);

    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub2(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback2(
        new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub2));

    dataObsMgrInner_->HandleRegisterObserver(uri, callback2);
    ObsListType obslist;
    dataObsMgrInner_->GetObsListFromMap(uri, obslist);
    EXPECT_EQ((std::size_t)2, obslist.size());
    EXPECT_EQ(true, dataObsMgrInner_->ObsExistInMap(callback));
    EXPECT_EQ(true, dataObsMgrInner_->ObsExistInMap(callback2));

    dataObsMgrInner_->RemoveObsFromMap(callback);
    EXPECT_EQ(false, dataObsMgrInner_->ObsExistInMap(callback));
    obslist.clear();
    dataObsMgrInner_->GetObsListFromMap(uri, obslist);
    EXPECT_EQ((std::size_t)1, obslist.size());
    EXPECT_EQ(false, dataObsMgrInner_->ObsExistInMap(callback));

    dataObsMgrInner_->RemoveObsFromMap(callback2);
    EXPECT_EQ(false, dataObsMgrInner_->ObsExistInMap(callback2));
    obslist.clear();
    dataObsMgrInner_->GetObsListFromMap(uri, obslist);
    EXPECT_EQ((std::size_t)0, obslist.size());
    EXPECT_EQ(false, dataObsMgrInner_->ObsExistInMap(callback2));
}

/*
 * Feature: DataObsMgrInner
 * Function: AddObsDeathRecipient/RemoveObsDeathRecipient function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_AddRemove_ObsDeathRecipient_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }

    sptr<IRemoteObject> observer(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner_->AddObsDeathRecipient(callback);

    ObsRecipientMapType::const_iterator it;
    it = dataObsMgrInner_->recipientMap_.find(observer);
    EXPECT_EQ(true, it != dataObsMgrInner_->recipientMap_.end());

    dataObsMgrInner_->RemoveObsDeathRecipient(callback);
    it = dataObsMgrInner_->recipientMap_.find(observer);
    EXPECT_EQ(false, it != dataObsMgrInner_->recipientMap_.end());
}
}  // namespace AAFwk
}  // namespace OHOS