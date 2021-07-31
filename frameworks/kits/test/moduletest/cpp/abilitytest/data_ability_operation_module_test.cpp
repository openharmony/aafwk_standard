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

#include <climits>
#include <gtest/gtest.h>
#include "ability_thread.h"
#include "ability_local_record.h"
#include "ability_loader.h"
#include "ability_impl_factory.h"
#include "data_ability_helper.h"
#include "context_deal.h"
#include "ohos_application.h"
#include "sys_mgr_client.h"
#include "ability_manager_interface.h"
#include "ability_manager_client.h"
#include "system_ability_definition.h"
#include "demo_ability_test.h"
#include "mock_bundle_manager.h"
#include "mock_ability_manager_service.h"
#include "data_ability_operation.h"
#include "data_ability_operation_builder.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace AAFwk;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AppExecFwk;
/*
 * Parameters:
 * Action
 * Entity
 * Flag
 * ElementName
 */
// const std::string ABILITY_NAME("DemoAbility");
class DataAbilityOperationModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject_;
    // static constexpr int TEST_WAIT_TIME = 500 * 1000;  // 500 ms
    // static const int RESULT_CODE = 1992;
};

void DataAbilityOperationModuleTest::SetUpTestCase(void)
{}

void DataAbilityOperationModuleTest::TearDownTestCase(void)
{}

void DataAbilityOperationModuleTest::SetUp(void)
{
    abilityObject_ = new MockAbilityManagerService();
    OHOS::sptr<OHOS::IRemoteObject> bundleObject = new BundleMgrService();
    auto sysMgr = OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject_);
    sysMgr->RegisterSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void DataAbilityOperationModuleTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_DataAbilityOperation_InsertBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_InsertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_InsertBuilder_0100";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    EXPECT_NE(abilityToken, nullptr);
    if (abilityToken != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->type = AppExecFwk::AbilityType::DATA;
        abilityInfo->name = "DemoAbility";
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

        AbilityThread::AbilityThreadMain(application, abilityRecord);

        std::shared_ptr<DemoAbility> context = std::make_shared<DemoAbility>();
        std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
        std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(context, uri, true);

        EXPECT_NE(helper, nullptr);
        std::shared_ptr<ValuesBucket> values = std::make_shared<ValuesBucket>();
        std::shared_ptr<DataAbilityOperation> operation =
            DataAbilityOperation::NewInsertBuilder(uri)->WithValuesBucket(values)->Build();
        EXPECT_NE(operation, nullptr);

        bool isInsertOperation = operation->IsInsertOperation();
        EXPECT_TRUE(isInsertOperation);

        std::shared_ptr<Uri> operationUri = operation->GetUri();
        EXPECT_EQ(operationUri->ToString(), "dataability://com.example.myapplication5.DataAbilityTest");

        int type = operation->GetType();
        EXPECT_EQ(type, DataAbilityOperation::TYPE_INSERT);
    }
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_InsertBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_UpdateBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_UpdateBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_UpdateBuilder_0100";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    EXPECT_NE(abilityToken, nullptr);
    if (abilityToken != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->type = AppExecFwk::AbilityType::DATA;
        abilityInfo->name = "DemoAbility";
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

        AbilityThread::AbilityThreadMain(application, abilityRecord);

        std::shared_ptr<DemoAbility> context = std::make_shared<DemoAbility>();
        std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
        std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(context, uri, true);

        EXPECT_NE(helper, nullptr);
        std::shared_ptr<ValuesBucket> values = std::make_shared<ValuesBucket>();
        std::shared_ptr<DataAbilityPredicates> predicates = std::make_shared<DataAbilityPredicates>();
        std::shared_ptr<DataAbilityOperation> operation = DataAbilityOperation::NewUpdateBuilder(uri)
                                                              ->WithValuesBucket(values)
                                                              ->WithPredicatesBackReference(0, 0)
                                                              ->WithPredicates(predicates)
                                                              ->WithInterruptionAllowed(true)
                                                              ->Build();
        EXPECT_NE(operation, nullptr);

        bool isUpdateOperation = operation->IsUpdateOperation();
        EXPECT_TRUE(isUpdateOperation);

        bool isInterruptionAllowed = operation->IsInterruptionAllowed();
        EXPECT_TRUE(isInterruptionAllowed);

        std::shared_ptr<ValuesBucket> valuesBucket = operation->GetValuesBucket();
        EXPECT_NE(valuesBucket, nullptr);
    }
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_UpdateBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_AssertBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_AssertBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_AssertBuilder_0100";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    EXPECT_NE(abilityToken, nullptr);
    if (abilityToken != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->type = AppExecFwk::AbilityType::DATA;
        abilityInfo->name = "DemoAbility";
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

        AbilityThread::AbilityThreadMain(application, abilityRecord);

        std::shared_ptr<DemoAbility> context = std::make_shared<DemoAbility>();
        std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
        std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(context, uri, true);

        EXPECT_NE(helper, nullptr);

        std::shared_ptr<DataAbilityOperation> operation =
            DataAbilityOperation::NewAssertBuilder(uri)->WithExpectedCount(2)->Build();
        EXPECT_NE(operation, nullptr);

        bool isAssertOperation = operation->IsAssertOperation();
        EXPECT_TRUE(isAssertOperation);

        int count = operation->GetExpectedCount();
        EXPECT_EQ(count, 2);
    }
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_AssertBuilder_0100";
}

/**
 * @tc.number: AaFwk_DataAbilityOperation_DeleteBuilder_0100
 * @tc.name: DataAbilityOperation
 * @tc.desc: Get the file types in the dataability.

 */
HWTEST_F(DataAbilityOperationModuleTest, AaFwk_DataAbilityOperation_DeleteBuilder_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_DeleteBuilder_0100";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    EXPECT_NE(abilityToken, nullptr);
    if (abilityToken != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->type = AppExecFwk::AbilityType::DATA;
        abilityInfo->name = "DemoAbility";
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

        AbilityThread::AbilityThreadMain(application, abilityRecord);

        std::shared_ptr<DemoAbility> context = std::make_shared<DemoAbility>();
        std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability://com.example.myapplication5.DataAbilityTest");
        std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(context, uri, true);
        EXPECT_NE(helper, nullptr);
        std::shared_ptr<DataAbilityPredicates> predicates = std::make_shared<DataAbilityPredicates>();
        EXPECT_NE(predicates, nullptr);
        std::shared_ptr<DataAbilityOperationBuilder> builder = DataAbilityOperation::NewDeleteBuilder(uri);
        EXPECT_NE(builder, nullptr);
        std::shared_ptr<DataAbilityOperation> operation = builder->WithPredicates(predicates)->Build();
        EXPECT_NE(operation, nullptr);
        bool isDeleteOperation = operation->IsDeleteOperation();
        EXPECT_TRUE(isDeleteOperation);
    }
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityOperation_DeleteBuilder_0100";
}

}  // namespace AppExecFwk
}  // namespace OHOS