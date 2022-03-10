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

#include "ability.h"
#include "ability_impl.h"
#include "ability_state.h"
#include "context_deal.h"
#include "hilog_wrapper.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "page_ability_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityImplActiveTest : public testing::Test {
public:
    AbilityImplActiveTest()
    {}
    ~AbilityImplActiveTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockAbilityimpl : public AbilityImpl {
public:
    MockAbilityimpl() {};
    virtual ~MockAbilityimpl() {};

    void Inactive()
    {
        AbilityImpl::Inactive();
    }
    void Active()
    {
        AbilityImpl::Active();
    }
};

class MockAbilityClass : public Ability {
public:
    MockAbilityClass() : topFocus_(false), topFocusChange_(false), windowFocus_(false), windowFocusChange_(false) {};
    virtual ~MockAbilityClass() {};

    virtual void OnWindowFocusChanged(bool hasFocus)
    {
        windowFocus_ = hasFocus;
        windowFocusChange_ = true;
    };
    virtual void OnTopActiveAbilityChanged(bool topActive)
    {
        topFocus_ = topActive;
        topFocusChange_ = true;
    };
    bool GetWindowFocusStatus()
    {
        return windowFocus_;
    };
    bool GetTopFocusStatus()
    {
        return topFocus_;
    };
    bool GetWindowFocusChangeStatus()
    {
        return windowFocusChange_;
    };
    bool GetTopFocusChangeStatus()
    {
        return topFocusChange_;
    };
    virtual void OnActive() {};
    virtual void OnInactive() {};

private:
    bool topFocus_;
    bool topFocusChange_;
    bool windowFocus_;
    bool windowFocusChange_;
};

void AbilityImplActiveTest::SetUpTestCase(void)
{}

void AbilityImplActiveTest::TearDownTestCase(void)
{}

void AbilityImplActiveTest::SetUp(void)
{}

void AbilityImplActiveTest::TearDown(void)
{}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_Active_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::PAGE;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);
    abilityImpl->Inactive();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 test proc";
    abilityImpl->Active();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 test result check";
    EXPECT_TRUE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_TRUE(mockAbility->GetWindowFocusStatus());
    EXPECT_TRUE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_TRUE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_Active_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_002 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_002 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::SERVICE;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);
    abilityImpl->Inactive();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_002 test proc";
    abilityImpl->Active();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_002 test result check";
    EXPECT_FALSE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetWindowFocusStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_Active_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_003 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_003 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::PAGE;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_003 test proc";
    abilityImpl->Active();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_003 test result check";
    EXPECT_FALSE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetWindowFocusStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_Active_003 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_InActive_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_001 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_001 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::PAGE;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);
    abilityImpl->Active();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_001 test proc";
    abilityImpl->Inactive();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_001 test result check";
    EXPECT_TRUE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetWindowFocusStatus());
    EXPECT_TRUE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_001 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_InActive_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_002 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_002 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::DATA;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);
    abilityImpl->Active();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_002 test proc";
    abilityImpl->Inactive();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_002 test result check";
    EXPECT_FALSE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetWindowFocusStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_002 end";
}

/*
 * Feature: AbilityImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active
 * EnvConditions: NA
 * CaseDescription: Validate when normally entering a string
 */
HWTEST_F(AbilityImplActiveTest, AaFwk_AbilityImpl_InActive_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_003 start";
    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_003 pretest";
    std::shared_ptr<MockAbilityimpl> abilityImpl = std::make_shared<MockAbilityimpl>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<MockAbilityClass> mockAbility = std::make_shared<MockAbilityClass>();
    std::shared_ptr<Ability> ability = std::static_pointer_cast<Ability>(mockAbility);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr, nullptr);
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    abilityInfo->type = AbilityType::PAGE;
    contextDeal->SetAbilityInfo(abilityInfo);
    mockAbility->AttachBaseContext(contextDeal);
    abilityImpl->Init(application, record, ability, handler, token, contextDeal);

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_003 test proc";
    abilityImpl->Inactive();

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_003 test result check";
    EXPECT_FALSE(mockAbility->GetWindowFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetWindowFocusStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusChangeStatus());
    EXPECT_FALSE(mockAbility->GetTopFocusStatus());

    GTEST_LOG_(INFO) << "AaFwk_AbilityImpl_InActive_003 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS
