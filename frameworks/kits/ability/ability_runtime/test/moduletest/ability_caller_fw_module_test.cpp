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
#include "ability_thread.h"
#define private public
#define protected public
#include "ability_context_impl.h"
#include "caller_callback.h"
#undef private
#undef protected
#include "ability_context.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS;
using namespace AAFwk;

namespace {
const std::string ACE_SERVICE_ABILITY_NAME = "AceServiceAbility";
}
class AbilityCallerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static constexpr int TEST_WAIT_TIME = 500 * 1000;  // 500 ms
public:
    std::unique_ptr<AbilityContextImpl> context_ = nullptr;
};

void AbilityCallerTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);

    auto task = []()->Ability* { return new (std::nothrow) Ability; };

    AbilityLoader::GetInstance().RegisterAbility(ACE_SERVICE_ABILITY_NAME, task);
}

void AbilityCallerTest::TearDownTestCase(void)
{}

void AbilityCallerTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContextImpl>();
}

void AbilityCallerTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_StartAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: Ability caller to process StartAbility, and the result is success.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_StartAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbilityNameA";
    abilityInfo->bundleName = "DemoBundleNameA";
    abilityInfo->deviceId = "DemoDeviceIdA";
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
                        std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    EXPECT_FALSE(callback->IsCallBack());

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_TRUE(ret == 0);

    AppExecFwk::ElementName element("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");
    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    context_->localCallContainer_->OnAbilityConnectDone(element, callRemoteObject, ERR_OK);
    EXPECT_TRUE(callback->IsCallBack());
}

/**
 * @tc.number: AaFwk_Ability_StartAbility_0200
 * @tc.name: AbilityFwk
 * @tc.desc: Ability caller to process StartAbility, and the result is fail because call back is nullptr.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_StartAbility_0200, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    ErrCode ret = context_->StartAbility(want, nullptr);
    EXPECT_FALSE(ret == 0);
}

/**
 * @tc.number: AaFwk_Ability_StartAbility_0300
 * @tc.name: AbilityFwk
 * @tc.desc: Ability caller to process StartAbility, and the result is fail because the element of want is empty.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_StartAbility_0300, Function | MediumTest | Level1)
{
    Want want;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    EXPECT_FALSE(callback->IsCallBack());

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_FALSE(ret == 0);
}

/**
 * @tc.number: AaFwk_Ability_ReleaseAbility_0100
 * @tc.name: AbilityFwk
 * @tc.desc: Ability Caller to process ReleaseAbility, and the result is success.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_ReleaseAbility_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbilityNameA";
    abilityInfo->bundleName = "DemoBundleNameA";
    abilityInfo->deviceId = "DemoDeviceIdA";
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
                        std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_TRUE(ret == 0);

    ret = context_->ReleaseAbility(callback);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.number: AaFwk_Ability_ReleaseAbility_0200
 * @tc.name: AbilityFwk
 * @tc.desc: Ability Caller to process ReleaseAbility, and the result is fail because has no caller record.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_ReleaseAbility_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    ErrCode ret = context_->ReleaseAbility(callback);
    EXPECT_FALSE(ret == 0);
}

/**
 * @tc.number: AaFwk_Ability_OnCallStubDied_0100
 * @tc.name: AbilityFwk
 * @tc.desc: Ability Caller to process OnCallStubDied, and the result is success.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_OnCallStubDied_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityThread());
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    abilityInfo->name = "DemoAbilityNameA";
    abilityInfo->bundleName = "DemoBundleNameA";
    abilityInfo->deviceId = "DemoDeviceIdA";
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
                        std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    AbilityThread::AbilityThreadMain(application, abilityRecord, nullptr);

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    bool isSetOnReleaseCalled = false;
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});
    callback->SetOnRelease([&isSetOnReleaseCalled](const std::string &result) mutable {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        EXPECT_TRUE(result == "died");
        isSetOnReleaseCalled = true;
    });

    ErrCode ret = context_->StartAbility(want, callback);
    EXPECT_TRUE(ret == 0);

    std::shared_ptr<LocalCallRecord> localCallRecord;
    AppExecFwk::ElementName elementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");
    context_->localCallContainer_->GetCallLocalreocrd(elementName, localCallRecord);

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    localCallRecord->OnCallStubDied(callRemoteObject);
    EXPECT_TRUE(isSetOnReleaseCalled);
}

/**
 * @tc.number: AaFwk_Ability_OnCallStubDied_0200
 * @tc.name: AbilityFwk
 * @tc.desc: Ability Caller to process OnCallStubDied, and the result is fail because no caller.
 */
HWTEST_F(AbilityCallerTest, AaFwk_Ability_OnCallStubDied_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    bool isSetOnReleaseCalled = false;
    callback->SetOnRelease([&isSetOnReleaseCalled](const std::string &result) mutable {
        GTEST_LOG_(ERROR) << "OnRelease-----------" << result;
        isSetOnReleaseCalled = true;
    });

    AppExecFwk::ElementName elementName ("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    LocalCallRecord localCallRecord(elementName);

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    localCallRecord.OnCallStubDied(callRemoteObject);
    EXPECT_FALSE(isSetOnReleaseCalled);
}
}  // namespace AppExecFwk
}  // namespace OHOS