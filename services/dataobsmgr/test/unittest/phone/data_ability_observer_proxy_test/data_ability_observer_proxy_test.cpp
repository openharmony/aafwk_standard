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
#define private public
#include "mock_data_obs_manager_onchange_callback.h"
#include "data_ability_observer_proxy.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AAFwk {
class DataAbilityObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataAbilityObserverProxy> proxy_ = nullptr;
};

void DataAbilityObserverProxyTest::SetUpTestCase(void)
{}
void DataAbilityObserverProxyTest::TearDownTestCase(void)
{}
void DataAbilityObserverProxyTest::SetUp()
{}
void DataAbilityObserverProxyTest::TearDown()
{}

/*
 * Feature: DataAbilityObserverProxy.
 * Function: DataObsManagerProxy::OnChange is called.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataAbilityObserverProxyTest, DataAbilityObserverProxy_OnChangeInner_001, TestSize.Level1)
{
    // 1.stub define
    sptr<MockDataObsManagerOnChangeCallBack> mockDataAbilityObserverStub(new MockDataObsManagerOnChangeCallBack());

    // 2.obsver1 define
    sptr<DataAbilityObserverProxy> proxy(new DataAbilityObserverProxy(mockDataAbilityObserverStub));

    EXPECT_CALL(*mockDataAbilityObserverStub, OnChange()).Times(1);

    if (proxy != nullptr) {
        proxy->OnChange();
    }
}
}  // namespace AAFwk
}  // namespace OHOS