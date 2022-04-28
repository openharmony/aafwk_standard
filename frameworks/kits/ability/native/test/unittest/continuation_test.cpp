/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability.h"
#include "ability_impl.h"
#include "abs_shared_result_set.h"
#include "hilog_wrapper.h"
#include "context_deal.h"
#include "continuation_manager.h"
#include "continuation_handler.h"
#include "data_ability_predicates.h"
#include "mock_ability_impl.h"
#include "mock_ability_lifecycle_callbacks.h"
#include "mock_ability_token.h"
#include "mock_continuation_ability.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ContinuationTest : public testing::Test {
public:
    ContinuationTest() : continuationManager_(nullptr), ability_(nullptr), abilityInfo_(nullptr),
        continueToken_(nullptr)
    {}
    ~ContinuationTest()
    {}
    std::shared_ptr<ContinuationManager> continuationManager_;
    std::shared_ptr<Ability> ability_;
    std::shared_ptr<AbilityInfo> abilityInfo_;
    sptr<IRemoteObject> continueToken_;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContinuationTest::SetUpTestCase(void)
{}

void ContinuationTest::TearDownTestCase(void)
{}

void ContinuationTest::SetUp(void)
{
    continuationManager_ = std::make_shared<ContinuationManager>();
    continueToken_ = sptr<IRemoteObject>(new (std::nothrow)MockAbilityToken());
    abilityInfo_ = std::make_shared<AbilityInfo>();
    abilityInfo_->name = "ability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo_->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow)AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);
    ability_ = std::make_shared<MockContinuationAbility>();
    ability_->Init(abilityInfo_, application, handler, continueToken_);
}

void ContinuationTest::TearDown(void)
{}

/*
 * @tc.name: continue_manager_init_001
 * @tc.desc: init ContinuationManager with illegal ability
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_001 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    std::shared_ptr<Ability> ability = nullptr;
    bool result = continuationManager_->Init(ability, continueToken_, abilityInfo_, continuationHandler);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_001 end";
}

/*
 * @tc.name: continue_manager_init_002
 * @tc.desc: init ContinuationManager with illegal continueToken
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_init_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_init_002 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    sptr<IRemoteObject> continueToken = nullptr;
    bool result = continuationManager_->Init(ability_, continueToken, abilityInfo_, continuationHandler);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_manager_init_002 end";
}

/*
 * @tc.name: continue_manager_start_continuation_001
 * @tc.desc: call StartContinuation
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_start_continuation_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_start_continuation_001 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    bool startResult = continuationManager_->StartContinuation();
    EXPECT_TRUE(startResult);
    GTEST_LOG_(INFO) << "continue_manager_start_continuation_001 end";
}

/*
 * @tc.name: continue_manager_save_data_001
 * @tc.desc: call SaveData
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_manager_save_data_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_manager_save_data_001 start";

    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability_;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    continuationManager_->Init(ability_, continueToken_, abilityInfo_, continuationHandler);
    WantParams wantParams;
    bool saveDataResult = continuationManager_->SaveData(wantParams);
    EXPECT_TRUE(saveDataResult);
    GTEST_LOG_(INFO) << "continue_manager_save_data_001 end";
}

/*
 * @tc.name: continue_handler_start_continue_001
 * @tc.desc: call HandleStartContinuation with illegal token
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_handler_start_continue_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_start_continue_001 start";

    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    bool result = continuationHandler->HandleStartContinuation(nullptr, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_start_continue_001 end";
}

/*
 * @tc.name: continue_handler_start_continue_002
 * @tc.desc: call HandleStartContinuation without call SetAbilityInfo
 * @tc.type: FUNC
 * @tc.require: AR000GI8IP
 */
HWTEST_F(ContinuationTest, continue_handler_start_continue_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "continue_handler_start_continue_002 start";

    std::shared_ptr<Ability> ability = std::make_shared<MockContinuationAbility>();
    sptr<IRemoteObject> continueToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
    std::weak_ptr<Ability> abilityTmp = ability;
    std::shared_ptr<ContinuationHandler> continuationHandler =
        std::make_shared<ContinuationHandler>(continuationManager, abilityTmp);
    bool result = continuationHandler->HandleStartContinuation(continueToken, "mock_deviceId");
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "continue_handler_start_continue_002 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
