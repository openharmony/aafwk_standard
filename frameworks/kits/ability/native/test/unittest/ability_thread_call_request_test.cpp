/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <functional>
#define private public
#include "ability_thread.h"
#define protected public
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ability_handler.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
class CurrentAbilityTest : public Ability {
public:
    sptr<IRemoteObject> CallRequest()
    {
        sptr<IRemoteObject> remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()
                ->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        return remoteObject;
    }
};

class AbilityThreadCallRequestTest : public testing::Test {
public:
    AbilityThreadCallRequestTest() : abilitythread_(nullptr)
    {}
    ~AbilityThreadCallRequestTest()
    {
        abilitythread_ = nullptr;
    }
    AbilityThread *abilitythread_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityThreadCallRequestTest::SetUpTestCase(void)
{}

void AbilityThreadCallRequestTest::TearDownTestCase(void)
{}

void AbilityThreadCallRequestTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "AbilityThreadCallRequestTest SetUp";
}

void AbilityThreadCallRequestTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "AbilityThreadCallRequestTest TearDown";
}


/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0100
 * @tc.name: CallRequest
 * @tc.desc: CallRequest success
 */
HWTEST_F(AbilityThreadCallRequestTest, AaFwk_AbilityThread_CallRequest_0100, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->currentAbility_ = std::make_shared<CurrentAbilityTest>();
        EXPECT_NE(abilitythread->currentAbility_, nullptr);
        auto runner = EventRunner::Create(true);
        abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(runner, abilitythread);
        EXPECT_NE(abilitythread->abilityHandler_, nullptr);
        sptr<IRemoteObject> retval = nullptr;
        retval = abilitythread->CallRequest();
        EXPECT_TRUE(retval != nullptr);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0200
 * @tc.name: CallRequest
 * @tc.desc: CallRequest success fail because currentAbility_ is null
 */
HWTEST_F(AbilityThreadCallRequestTest, AaFwk_AbilityThread_CallRequest_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityThread();
    sptr<IRemoteObject> retval = abilitythread->CallRequest();
    EXPECT_TRUE(retval == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0300
 * @tc.name: CallRequest
 * @tc.desc: CallRequest success fail because abilityHandler_ is null
 */
HWTEST_F(AbilityThreadCallRequestTest, AaFwk_AbilityThread_CallRequest_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0300 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->currentAbility_ = std::make_shared<CurrentAbilityTest>();
        EXPECT_NE(abilitythread->currentAbility_, nullptr);
        sptr<IRemoteObject> retval = abilitythread->CallRequest();
        EXPECT_TRUE(retval == nullptr);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0300 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS