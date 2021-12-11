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
#include <thread>
#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ability_thread.h"
#include "ability.h"
#include "ability_impl.h"

#include "mock_ability_thread_for_ability_on_configuration_update.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class AbilityThreadTest : public testing::Test {
public:
    AbilityThreadTest() : abilitythread_(nullptr)
    {}
    ~AbilityThreadTest()
    {
        abilitythread_ = nullptr;
    }
    AbilityThread *abilitythread_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityThreadTest::SetUpTestCase(void)
{
    AbilityTest::InitInstance();
}

void AbilityThreadTest::TearDownTestCase(void)
{
    AbilityTest::UnInitInstance();
}

void AbilityThreadTest::SetUp(void)
{}

void AbilityThreadTest::TearDown(void)
{}

AbilityThread::AbilityThread()
    : abilityImpl_(nullptr), token_(nullptr), currentAbility_(nullptr), abilityHandler_(nullptr), runner_(nullptr)
{
    abilityImpl_ = std::make_shared<AbilityImpl>();
    abilityHandler_ = std::make_shared<AbilityHandler>(EventRunner::Create("AbilityTest"), this);

    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<Ability> ability = AbilityTest::GetInstance();
    std::shared_ptr<ContextDeal> contextDeal = nullptr;
    abilityImpl_->Init(application, nullptr, ability, abilityHandler_, nullptr, contextDeal);
}

void AbilityImpl::Init(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &record,
    std::shared_ptr<Ability> &ability, std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token,
    std::shared_ptr<ContextDeal> &contextDeal)
{
    ability_ = ability;
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100 start";
    std::shared_ptr<AbilityThread> abilityThread = std::make_shared<AbilityThread>();

    AppExecFwk::Configuration config;
    uint32_t code = OHOS::AAFwk::IAbilityScheduler::SCHEDULE_UPDATE_CONFIGURATION;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityThread::GetDescriptor())) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "---------- data.WriteInterfaceToken retval is false";
        return;
    }

    if (!data.WriteParcelable(&config)) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "---------- data.WriteParcelable retval is false";
        return;
    }

    if (AbilityTest::GetInstance() == nullptr) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "---------- AbilityTest::GetInstance() is nullptr";
        return;
    }

    auto call = [](const Configuration &) { GTEST_LOG_(INFO) << "---------- OnConfigurationUpdated Call Success"; };

    EXPECT_CALL(*AbilityTest::GetInstance(), OnConfigurationUpdated(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(call));

    abilityThread->OnRemoteRequest(code, data, reply, option);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS