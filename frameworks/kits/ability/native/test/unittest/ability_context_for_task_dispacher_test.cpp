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
#include <singleton.h>

#include "ability_context.h"
#include "application_context.h"
#include "event_runner.h"
#include "context_deal.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityContextForTaskDispatcherTest : public testing::Test {
public:
    AbilityContextForTaskDispatcherTest()
    {}
    ~AbilityContextForTaskDispatcherTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityContextForTaskDispatcherTest::SetUpTestCase(void)
{}

void AbilityContextForTaskDispatcherTest::TearDownTestCase(void)
{}

void AbilityContextForTaskDispatcherTest::SetUp(void)
{}

void AbilityContextForTaskDispatcherTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_AbilityContext_GetUITaskDispatcher_0100
 * @tc.name: GetUITaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(
    AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetUITaskDispatcher_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetUITaskDispatcher_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(false);
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetRunner(runner);
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    abilitycontext->AttachBaseContext(contextdeal);
    EXPECT_TRUE((nullptr != abilitycontext->GetUITaskDispatcher()));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetUITaskDispatcher_0100 end";
}
/**
 * @tc.number: AaFwk_AbilityContext_GetUITaskDispatcher_0200
 * @tc.name: GetUITaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(
    AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetUITaskDispatcher_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetUITaskDispatcher_0200 start";
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    EXPECT_TRUE((nullptr == abilitycontext->GetUITaskDispatcher()));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetUITaskDispatcher_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetMainTaskDispatcher_0100
 * @tc.name: GetMainTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetMainTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMainTaskDispatcher_0100 start";
    std::shared_ptr<EventRunner> runner = EventRunner::Create(false);
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    contextdeal->SetRunner(runner);
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    abilitycontext->AttachBaseContext(contextdeal);
    EXPECT_TRUE((nullptr != abilitycontext->GetMainTaskDispatcher()));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMainTaskDispatcher_0100 end";
}
/**
 * @tc.number: AaFwk_AbilityContext_GetMainTaskDispatcher_0200
 * @tc.name: GetMainTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetMainTaskDispatcher_0200,
    Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMainTaskDispatcher_0200 start";
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    EXPECT_TRUE((nullptr == abilitycontext->GetMainTaskDispatcher()));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMainTaskDispatcher_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_CreateParallelTaskDispatcher_0100
 * @tc.name: CreateParallelTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_CreateParallelTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateParallelTaskDispatcher_0100 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    contextdeal->SetApplicationContext(appcontext);
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    abilitycontext->AttachBaseContext(contextdeal);
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != abilitycontext->CreateParallelTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateParallelTaskDispatcher_0100 end";
}
/**
 * @tc.number: AaFwk_AbilityContext_CreateParallelTaskDispatcher_0200
 * @tc.name: CreateParallelTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_CreateParallelTaskDispatcher_0200,
    Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateParallelTaskDispatcher_0200 start";
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    std::string name("ParalleTest");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == abilitycontext->CreateParallelTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateParallelTaskDispatcher_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_CreateSerialTaskDispatcher_0100
 * @tc.name: CreateSerialTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_CreateSerialTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateSerialTaskDispatcher_0100 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    contextdeal->SetApplicationContext(appcontext);
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    abilitycontext->AttachBaseContext(contextdeal);
    std::string name("Serial");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != abilitycontext->CreateSerialTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateSerialTaskDispatcher_0100 end";
}
/**
 * @tc.number: AaFwk_AbilityContext_CreateSerialTaskDispatcher_0200
 * @tc.name: CreateSerialTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_CreateSerialTaskDispatcher_0200,
    Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateSerialTaskDispatcher_0200 start";
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    std::string name("Serial");
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == abilitycontext->CreateSerialTaskDispatcher(name, high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_CreateSerialTaskDispatcher_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetGlobalTaskDispatcher_0100
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetGlobalTaskDispatcher_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetGlobalTaskDispatcher_0100 start";
    std::shared_ptr<ContextDeal> contextdeal = std::make_shared<ContextDeal>();
    std::shared_ptr<ApplicationContext> appcontext = std::make_shared<ApplicationContext>();
    contextdeal->SetApplicationContext(appcontext);
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    abilitycontext->AttachBaseContext(contextdeal);
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr != abilitycontext->GetGlobalTaskDispatcher(high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetGlobalTaskDispatcher_0100 end";
}
/**
 * @tc.number: AaFwk_AbilityContext_GetGlobalTaskDispatcher_0200
 * @tc.name: GetGlobalTaskDispatcher
 * @tc.desc: Test the attachbasecontext call to verify that the return value of gethapmoduleinfo is correct.
 */
HWTEST_F(AbilityContextForTaskDispatcherTest, AaFwk_AbilityContext_GetGlobalTaskDispatcher_0200,
    Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetGlobalTaskDispatcher_0200 start";
    std::shared_ptr<AbilityContext> abilitycontext = std::make_shared<AbilityContext>();
    TaskPriority high = TaskPriority::HIGH;
    EXPECT_TRUE((nullptr == abilitycontext->GetGlobalTaskDispatcher(high)));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetGlobalTaskDispatcher_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
