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

#include "ability.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_start_setting.h"
#include "context_deal.h"
#include "mock_scene_created_ability.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using OHOS::Parcel;

class SceneCreatedTest : public testing::Test {
public:
    SceneCreatedTest() : ability_(nullptr)
    {}
    ~SceneCreatedTest()
    {
        ability_ = nullptr;
    }
    std::shared_ptr<MockSceneCreatedAbility> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SceneCreatedTest::SetUpTestCase(void)
{}

void SceneCreatedTest::TearDownTestCase(void)
{}

void SceneCreatedTest::SetUp(void)
{
    ability_ = std::make_shared<MockSceneCreatedAbility>();
}

void SceneCreatedTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_OnSceneCreated_001
 * @tc.name: OnSceneCreated
 * @tc.desc: Verify that OnSceneCreated was trigger.
 */
HWTEST_F(SceneCreatedTest, AaFwk_Ability_OnSceneCreated_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_001 start";

    // step1. create application/handle/token
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    sptr<IRemoteObject> token = nullptr;
    // step2. init ability instance
    ability_->Init(abilityInfo, application, handler, token);
    // step3. call OnForeground
    Want want;
    ability_->OnForeground(want);
    // step4. check OnSceneCreated called, use mock ability
    EXPECT_EQ(1, ability_->GetTriggerTime());

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_001 end";
}

/**
 * @tc.number: AaFwk_Ability_OnSceneCreated_002
 * @tc.name: OnSceneCreated
 * @tc.desc: Verify that OnSceneCreated trigger correct time.
 */
HWTEST_F(SceneCreatedTest, AaFwk_Ability_OnSceneCreated_002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_002 start";

    // step1. create application/handle/token
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    sptr<IRemoteObject> token = nullptr;
    // step2. init ability instance
    ability_->Init(abilityInfo, application, handler, token);
    // step3. trigger OnForeground
    Want want;
    ability_->OnForeground(want);
    // step4. check OnSceneCreated called, use mock ability
    EXPECT_EQ(1, ability_->GetTriggerTime());
    // step5. trigger OnForeground again
    ability_->OnForeground(want);
    // step6. check OnSceneCreated called time
    EXPECT_EQ(1, ability_->GetTriggerTime());

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_002 end";
}

/**
 * @tc.number: AaFwk_Ability_OnSceneCreated_005
 * @tc.name: OnSceneCreated
 * @tc.desc: Verify that OnSceneCreated create scene success.
 */
HWTEST_F(SceneCreatedTest, AaFwk_Ability_OnSceneCreated_005, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_005 start";

    // step1. create application/handle/token
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    sptr<IRemoteObject> token = nullptr;
    // step2. init ability instance
    ability_->Init(abilityInfo, application, handler, token);
    // step3. call OnForeground
    Want want;
    ability_->OnForeground(want);
    // step4. check scene was created, use mock ability
    EXPECT_NE(ability_->GetScene(), nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_005 end";
}

/**
 * @tc.number: AaFwk_Ability_OnSceneCreated_006
 * @tc.name: OnSceneCreated
 * @tc.desc: Verify that OnSceneCreated create scene once.
 */
HWTEST_F(SceneCreatedTest, AaFwk_Ability_OnSceneCreated_006, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_006 start";

    // step1. create application/handle/token
    std::shared_ptr<Rosen::WindowScene> firstScene = nullptr;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    sptr<IRemoteObject> token = nullptr;
    // step2. init ability instance
    ability_->Init(abilityInfo, application, handler, token);
    // step3. trigger OnForeground
    Want want;
    ability_->OnForeground(want);
    // step4. check scene was created, use mock ability
    firstScene = ability_->GetScene();
    EXPECT_NE(firstScene, nullptr);
    // step5. trigger OnForeground again
    ability_->OnForeground(want);
    // step6. check scene was same value, created once
    EXPECT_EQ(ability_->GetScene(), firstScene);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSceneCreated_006 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
