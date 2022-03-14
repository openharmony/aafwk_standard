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
#define private public
#include "ability_context_impl.h"
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
class AbilityContextImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<AbilityContextImpl> context_ = nullptr;
};

void AbilityContextImplTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void AbilityContextImplTest::TearDownTestCase(void)
{}

void AbilityContextImplTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContextImpl>();
}

void AbilityContextImplTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0100, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is not null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0200, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    context_->localCallContainer_ = new (std::nothrow)LocalCallContainer();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseAbility, and the result is success.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseAbility_0100beagin==============.";

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    context_->localCallContainer_ = new (std::nothrow) LocalCallContainer();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    std::string uri = elementName.GetURI();
    context_->localCallContainer_->callProxyRecords_.emplace(uri, localCallRecord);

    ErrCode ret = context_->ReleaseAbility(callback);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseAbility_0100end==============.";
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseAbility, and the result is fail because localCallContainer is null.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseAbility_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    ErrCode ret = context_->ReleaseAbility(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}
}  // namespace AppExecFwk
}  // namespace OHOS