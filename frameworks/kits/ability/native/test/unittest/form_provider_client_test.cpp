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

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_mgr_errors.h"
#include "form_provider_client.h"
#include "mock_form_supply_callback.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string FORM_MANAGER_SERVICE_BUNDLE_NAME = "com.form.fms.app";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";
const std::string FORM_SUPPLY_INFO = "com.form.supply.info.test";

class FormProviderClientTest : public testing::Test {
public:
    FormProviderClientTest()
    {}
    ~FormProviderClientTest()
    {}
    sptr<FormProviderClient> instance_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FormProviderClientTest::SetUpTestCase(void)
{}

void FormProviderClientTest::TearDownTestCase(void)
{}

void FormProviderClientTest::SetUp(void)
{
    instance_ = new FormProviderClient();

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_MANAGER_SERVICE_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
}

void FormProviderClientTest::TearDown(void)
{
    instance_ = nullptr;
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0100
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);
    int64_t formId = 1001L;
    instance_->AcquireProviderFormInfo(formId, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0200
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0200 start";

    instance_->SetOwner(nullptr);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 200)
        .SetParam(Constants::FORM_CONNECT_ID, 200L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 200L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 1002L;
    instance_->AcquireProviderFormInfo(formId, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0200 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0300
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::ACQUIRE_TYPE, 300)
        .SetParam(Constants::FORM_CONNECT_ID, 300L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 300L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 1003L;
    instance_->AcquireProviderFormInfo(formId, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0400
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0400 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);
    instance_->ClearOwner(ability);

    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 400)
        .SetParam(Constants::FORM_CONNECT_ID, 400L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 400L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 1004L;
    instance_->AcquireProviderFormInfo(formId, want, callerToken);

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0400 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_AcquireProviderFormInfo_0500
 * @tc.name: AcquireProviderFormInfo
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_AcquireProviderFormInfo_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0500 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 500)
        .SetParam(Constants::FORM_CONNECT_ID, 500L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 500L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    const sptr<IRemoteObject> callerToken = nullptr;
    int64_t formId = 1005L;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->AcquireProviderFormInfo(formId, want, callerToken));
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_AcquireProviderFormInfo_0500 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_EventNotify_0100
 * @tc.name: EventNotify
 * @tc.desc: Verify that the return value of EventNotify is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_EventNotify_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0100 start";

    // Ability
    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    // want
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    // formEvents
    int64_t formId = 100;
    std::vector<int64_t> formEvents;
    formEvents.push_back(formId);

    int32_t formVisibleType = Constants::FORM_INVISIBLE;

    EXPECT_EQ(ERR_OK, instance_->EventNotify(formEvents, formVisibleType, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_EventNotify_0200
 * @tc.name: EventNotify
 * @tc.desc: Verify that the return value of EventNotify is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_EventNotify_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0200 start";

    // Ability is nullptr
    instance_->SetOwner(nullptr);

    // callerToken
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    // want
    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 100)
        .SetParam(Constants::FORM_CONNECT_ID, 100L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 100L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    // formEvents
    int64_t formId = 100;
    std::vector<int64_t> formEvents;
    formEvents.push_back(formId);

    int32_t formVisibleType = Constants::FORM_INVISIBLE;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY,
    instance_->EventNotify(formEvents, formVisibleType, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_EventNotify_0200 end";
}


/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 101)
        .SetParam(Constants::FORM_CONNECT_ID, 101L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 101L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 720L;
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 102)
        .SetParam(Constants::FORM_CONNECT_ID, 102L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 102L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 722L;
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0200 end";
}


/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormCastTempForm_0100
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormCastTempForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::ACQUIRE_TYPE, 103)
        .SetParam(Constants::FORM_CONNECT_ID, 103L)
        .SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 103L)
        .SetParam(Constants::FORM_SUPPLY_INFO, FORM_SUPPLY_INFO);

    int64_t formId = 723L;
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->NotifyFormCastTempForm(formId, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormCastTempForm_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0100
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 104L);

    int64_t formId = 724L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0100 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0200
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 105L);
    int64_t formId = 725L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0200 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_FireFormEvent_0300
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_FireFormEvent_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 106L);

    int64_t formId = 726L;
    std::string message = "event message";
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->FireFormEvent(formId, message, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_FireFormEvent_0300 end";
}

/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0100
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0100 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 107L);

    std::vector<int64_t> formIds = {727L, 728L, 729L};
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_OK, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0100 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0200
 * @tc.name: FireFormEvent
 * @tc.desc: Verify that the return value of FireFormEvent is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0200 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(nullptr);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 108L);
    std::vector<int64_t> formIds = {730L, 731L, 732L};
    const sptr<IRemoteObject> callerToken = MockFormSupplyCallback::GetInstance();

    EXPECT_EQ(ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0200 end";
}
/**
 * @tc.number: AaFwk_FormProviderClient_NotifyFormsDelete_0300
 * @tc.name: NotifyFormCastTempForm
 * @tc.desc: Verify that the return value of AddForm is correct.
 */
HWTEST_F(FormProviderClientTest, AaFwk_FormProviderClient_NotifyFormsDelete_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0300 start";

    const std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    instance_->SetOwner(ability);

    Want want;
    want.SetParam(Constants::PARAM_FORM_MANAGER_SERVICE_BUNDLENAME_KEY, FORM_MANAGER_SERVICE_BUNDLE_NAME)
        .SetParam(Constants::FORM_CONNECT_ID, 109L);

    std::vector<int64_t> formIds = {730L, 731L, 732L};
    const sptr<IRemoteObject> callerToken = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, instance_->NotifyFormsDelete(formIds, want, callerToken));

    GTEST_LOG_(INFO) << "AaFwk_FormProviderClient_NotifyFormsDelete_0300 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
