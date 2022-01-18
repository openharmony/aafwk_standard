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

#define private public
#include "ability.h"
#undef private
#include "ability_info.h"
#include "ability_handler.h"
#include "ability_local_record.h"
#include "ability_start_setting.h"
#include "context_deal.h"
#include "form_mgr.h"
#include "mock_form_mgr_service.h"
#include "mock_page_ability.h"
#include "mock_bundle_manager_form.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using OHOS::Parcel;

class FormAbilityTest : public testing::Test {
public:
    FormAbilityTest() : ability_(nullptr)
    {}
    ~FormAbilityTest()
    {}
    std::shared_ptr<Ability> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    sptr<BundleMgrService> mockBundleMgr_;
};

void FormAbilityTest::SetUpTestCase(void)
{}

void FormAbilityTest::TearDownTestCase(void)
{}

void FormAbilityTest::SetUp(void)
{
    ability_ = std::make_shared<Ability>();
    FormMgr::GetInstance().SetFormMgrService(sptr<MockFormMgrService>(new (std::nothrow) MockFormMgrService()));
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);

    mockBundleMgr_ = new (std::nothrow) BundleMgrService();
    EXPECT_TRUE(mockBundleMgr_ != nullptr);
    ability_->SetBundleManager(mockBundleMgr_);
}

void FormAbilityTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0100
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally when want is nothing.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    Want want;
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    EXPECT_EQ(false, ability_->AcquireForm(0, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0200
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "Form0200";
    std::string moduleName = "ModuleName0200";
    std::string deviceId = "DeviceId0200";
    std::string bundleName = "Bundle0200";
    std::string abilityName = "Ability0200";
    bool tempFormFlg = false;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    EXPECT_EQ(true, ability_->AcquireForm(0, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0300
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally when the same id is requested twice in succession.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "Form0300";
    std::string moduleName = "ModuleName0300";
    std::string deviceId = "DeviceId0300";
    std::string bundleName = "Bundle0300";
    std::string abilityName = "Ability0300";
    bool tempFormFlg = false;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    // first request
    ability_->AcquireForm(300, want, callback);
    // second request by same want
    EXPECT_EQ(false, ability_->AcquireForm(300, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0400
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally when form id is negative in want
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0400 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "Form0400";
    std::string moduleName = "ModuleName0400";
    std::string deviceId = "DeviceId0400";
    std::string bundleName = "Bundle0400";
    std::string abilityName = "Ability0400";
    bool tempFormFlg = false;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    EXPECT_EQ(false, ability_->AcquireForm(-150, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0400 end";
}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0500
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally when select form id when acquire temporary form
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0500 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "Form0500";
    std::string moduleName = "ModuleName0500";
    std::string deviceId = "DeviceId0500";
    std::string bundleName = "Bundle0500";
    std::string abilityName = "Ability0500";
    bool tempFormFlg = true;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    EXPECT_EQ(false, ability_->AcquireForm(60, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0500 end";
}

/**
 * @tc.number: AaFwk_Ability_AcquireForm_0600
 * @tc.name: AcquireForm
 * @tc.desc: Test whether acquireForm is called normally when dimension is zero or negative in want
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_AcquireForm_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0600 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "Form0600";
    std::string moduleName = "ModuleName0600";
    std::string deviceId = "DeviceId0600";
    std::string bundleName = "Bundle0600";
    std::string abilityName = "Ability0600";
    bool tempFormFlg = false;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg)
        .SetParam(Constants::PARAM_FORM_DIMENSION_KEY, -1);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    EXPECT_EQ(false, ability_->AcquireForm(0, want, callback));

    GTEST_LOG_(INFO) << "AaFwk_Ability_AcquireForm_0600 end";
}

/**
 * @tc.number: AaFwk_Ability_UpdateForm_0100
 * @tc.name: UpdateForm
 * @tc.desc: Test whether updateForm is called normally when jsonFormProviderData is empty.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_UpdateForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 100;
    FormProviderData formProviderData;

    EXPECT_EQ(false, ability_->UpdateForm(formId, formProviderData));

    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_UpdateForm_0200
 * @tc.name: UpdateForm
 * @tc.desc: Test whether updateForm is called normally when recoverStatus is recovering.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_UpdateForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 200L;
    FormProviderData formProviderData = FormProviderData(std::string("{\"city\": \"beijing200\"}"));
    FormMgr::SetRecoverStatus(Constants::IN_RECOVERING);

    EXPECT_EQ(false, ability_->UpdateForm(formId, formProviderData));

    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_UpdateForm_0300
 * @tc.name: UpdateForm
 * @tc.desc: Test whether updateForm is called normally when fms is error.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_UpdateForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 300L;
    FormProviderData formProviderData = FormProviderData(std::string("{\"city\": \"beijing300\"}"));
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);

    EXPECT_EQ(false, ability_->UpdateForm(formId, formProviderData));

    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_UpdateForm_0400
 * @tc.name: UpdateForm
 * @tc.desc: Test whether updateForm is called normally when fms is ok.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_UpdateForm_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0400 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 400L;
    FormProviderData formProviderData = FormProviderData(std::string("{\"city\": \"beijing400\"}"));

    EXPECT_EQ(true, ability_->UpdateForm(formId, formProviderData));

    GTEST_LOG_(INFO) << "AaFwk_Ability_UpdateForm_0400 end";
}
/**
 * @tc.number: AaFwk_Ability_DeleteForm_0100
 * @tc.name: DeleteForm
 * @tc.desc: Test whether DeleteForm is called normally
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DeleteForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0100 start";

    std::string formName1 = "Form0101";
    std::string moduleName1 = "ModuleName0101";
    std::string deviceId1 = "DeviceId0101";
    std::string bundleName1 = "Bundle0101";
    std::string abilityName1 = "Ability0101";
    bool tempFormFlg1 = false;
    Want want1;
    want1.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 120L)
        .SetParam(Constants::PARAM_FORM_NAME_KEY, formName1)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName1)
        .SetElementName(deviceId1, bundleName1, abilityName1)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg1);
    std::shared_ptr<MockPageAbility::FormCallback> callback1;

    std::string formName2 = "Form0102";
    std::string moduleName2 = "ModuleName0102";
    std::string deviceId2 = "DeviceId0102";
    std::string bundleName2 = "Bundle0102";
    std::string abilityName2 = "Ability0102";
    bool tempFormFlg2 = false;
    Want want2;
    want2.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 121L)
        .SetParam(Constants::PARAM_FORM_NAME_KEY, formName2)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName2)
        .SetElementName(deviceId2, bundleName2, abilityName2)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg2);
    std::shared_ptr<MockPageAbility::FormCallback> callback_2;

    ability_->appCallbacks_.insert(std::make_pair(120L, callback1));
    ability_->userReqParams_.insert(std::make_pair(120L, want1));
    ability_->appCallbacks_.insert(std::make_pair(121L, callback_2));
    ability_->userReqParams_.insert(std::make_pair(121L, want2));

    int64_t formId = 120L;
    ability_->DeleteForm(formId);


    size_t cnt {1};
    EXPECT_EQ(cnt, ability_->appCallbacks_.size());

    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_DeleteForm_0200
 * @tc.name: DeleteForm
 * @tc.desc: Test whether DeleteForm is called normally when form id is negative
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DeleteForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0200 start";

    int64_t formId = -26;
    EXPECT_EQ(false, ability_->DeleteForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_DeleteForm_0300
 * @tc.name: DeleteForm
 * @tc.desc: Test whether DeleteForm is called normally when form is in recover status
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DeleteForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0300 start";

    int64_t formId = 26;
    FormMgr::SetRecoverStatus(Constants::IN_RECOVERING);

    EXPECT_EQ(false, ability_->DeleteForm(formId));
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);

    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_DeleteForm_0400
 * @tc.name: DeleteForm
 * @tc.desc: Test whether DeleteForm is called normally when form is temp form
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DeleteForm_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0400 start";

    std::string formName1 = "Form0401";
    std::string moduleName1 = "ModuleName0401";
    std::string deviceId1 = "DeviceId0401";
    std::string bundleName1 = "Bundle0401";
    std::string abilityName1 = "Ability0401";
    bool tempFormFlg1 = true;
    Want want1;
    want1.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 120L)
        .SetParam(Constants::PARAM_FORM_NAME_KEY, formName1)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName1)
        .SetElementName(deviceId1, bundleName1, abilityName1)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg1);
    std::shared_ptr<MockPageAbility::FormCallback> callback1;

    std::string formName2 = "Form0402";
    std::string moduleName2 = "ModuleName0402";
    std::string deviceId2 = "DeviceId0402";
    std::string bundleName2 = "Bundle0402";
    std::string abilityName2 = "Ability0402";
    bool tempFormFlg2 = true;
    Want want2;
    want2.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, 121L)
        .SetParam(Constants::PARAM_FORM_NAME_KEY, formName2)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName2)
        .SetElementName(deviceId2, bundleName2, abilityName2)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg2);
    std::shared_ptr<MockPageAbility::FormCallback> callback2;

    ability_->appCallbacks_.insert(std::make_pair(120L, callback1));
    ability_->userReqParams_.insert(std::make_pair(120L, want1));
    ability_->appCallbacks_.insert(std::make_pair(121L, callback2));
    ability_->userReqParams_.insert(std::make_pair(121L, want2));
    ability_->lostedByReconnectTempForms_.push_back(120L);
    ability_->lostedByReconnectTempForms_.push_back(121L);

    int64_t formId = 121L;
    ability_->DeleteForm(formId);

    size_t cnt {1};
    EXPECT_EQ(cnt, ability_->lostedByReconnectTempForms_.size());

    ability_->userReqParams_.clear();

    GTEST_LOG_(INFO) << "AaFwk_Ability_DeleteForm_0400 end";
}

/**
 * @tc.number: AaFwk_Ability_CastTempForm_0100
 * @tc.name: CastTempForm
 * @tc.desc: Test whether CastTempForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_CastTempForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 170L;

    EXPECT_EQ(true, ability_->CastTempForm(formId));
    EXPECT_EQ(false, ability_->userReqParams_[formId].GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, true));

    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_CastTempForm_0200
 * @tc.name: CastTempForm
 * @tc.desc: Test whether CastTempForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_CastTempForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 175L;

    EXPECT_EQ(false, ability_->CastTempForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_CastTempForm_0300
 * @tc.name: CastTempForm
 * @tc.desc: Test whether CastTempForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_CastTempForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = -175L;

    EXPECT_EQ(false, ability_->CastTempForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_CastTempForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_EnableUpdateForm_0100
 * @tc.name: EnableUpdateForm
 * @tc.desc: Test whether enableUpdateForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_EnableUpdateForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_EnableUpdateForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    std::vector<int64_t> formIds;
    EXPECT_EQ(false, ability_->EnableUpdateForm(formIds));

    GTEST_LOG_(INFO) << "AaFwk_Ability_EnableUpdateForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_EnableUpdateForm_0200
 * @tc.name: EnableUpdateForm
 * @tc.desc: Test whether enableUpdateForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_EnableUpdateForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_EnableUpdateForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    formIds.push_back(200);
    EXPECT_EQ(true, ability_->EnableUpdateForm(formIds));

    GTEST_LOG_(INFO) << "AaFwk_Ability_EnableUpdateForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_DisableUpdateForm_0100
 * @tc.name: DisableUpdateForm
 * @tc.desc: Test whether disableUpdateForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DisableUpdateForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    std::vector<int64_t> formIds;
    EXPECT_EQ(false, ability_->DisableUpdateForm(formIds));

    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_DisableUpdateForm_0200
 * @tc.name: DisableUpdateForm
 * @tc.desc: Test whether disableUpdateForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DisableUpdateForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    std::vector<int64_t> formIds;
    formIds.push_back(200);
    EXPECT_EQ(true, ability_->DisableUpdateForm(formIds));

    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_DisableUpdateForm_0300
 * @tc.name: EnableUpdateForm
 * @tc.desc: Test whether enableUpdateForm is called normally.(RecoverStatus is IN_RECOVERING)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_DisableUpdateForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    formIds.push_back(300);

    // SetRecoverStatus IN_RECOVERING
    volatile int recoverStatus = Constants::IN_RECOVERING;
    FormMgr::GetInstance().SetRecoverStatus(recoverStatus);

    EXPECT_EQ(false, ability_->DisableUpdateForm(formIds));

    // SetRecoverStatus NOT_IN_RECOVERY
    recoverStatus = Constants::NOT_IN_RECOVERY;
    FormMgr::GetInstance().SetRecoverStatus(recoverStatus);

    GTEST_LOG_(INFO) << "AaFwk_Ability_DisableUpdateForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_SetFormNextRefreshTime_0100
 * @tc.name: SetFormNextRefreshTime
 * @tc.desc: Test whether SetFormNextRefreshTime is called normally.(next time litte than 5 mins)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_SetFormNextRefreshTime_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0100 start";
    int64_t formId = 100;
    int64_t nextTime = Constants::MIN_NEXT_TIME - 1;
    EXPECT_EQ(false, ability_->SetFormNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_SetFormNextRefreshTime_0200
 * @tc.name: SetFormNextRefreshTime
 * @tc.desc: Test whether SetFormNextRefreshTime is called normally.(RecoverStatus is IN_RECOVERING)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_SetFormNextRefreshTime_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0200 start";
    int64_t formId = 200;
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // SetRecoverStatus IN_RECOVERING
    volatile int recoverStatus = Constants::IN_RECOVERING;
    FormMgr::GetInstance().SetRecoverStatus(recoverStatus);

    EXPECT_EQ(false, ability_->SetFormNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_SetFormNextRefreshTime_0300
 * @tc.name: SetFormNextRefreshTime
 * @tc.desc: Test whether SetFormNextRefreshTime is called normally.(SetNextRefreshTime reply false)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_SetFormNextRefreshTime_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0300 start";
    int64_t invalidFormId = 0;
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // SetRecoverStatus NOT_IN_RECOVERY
    int recoverStatus = Constants::NOT_IN_RECOVERY;
    FormMgr::GetInstance().SetRecoverStatus(recoverStatus);

    EXPECT_EQ(false, ability_->SetFormNextRefreshTime(invalidFormId, nextTime));
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_SetFormNextRefreshTime_0400
 * @tc.name: SetFormNextRefreshTime
 * @tc.desc: Test whether SetFormNextRefreshTime is called normally.(SetNextRefreshTime reply true)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_SetFormNextRefreshTime_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0400 start";
    int64_t formId = 400;
    int64_t nextTime = Constants::MIN_NEXT_TIME;

    // SetRecoverStatus NOT_IN_RECOVERY
    int recoverStatus = Constants::NOT_IN_RECOVERY;
    FormMgr::GetInstance().SetRecoverStatus(recoverStatus);

    EXPECT_EQ(true, ability_->SetFormNextRefreshTime(formId, nextTime));
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetFormNextRefreshTime_0400 end";
}

/**
 * @tc.number: AaFwk_Ability_RequestForm_0100
 * @tc.name: RequestForm
 * @tc.desc: Normal case: test whether RequestForm is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_RequestForm_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 100L;

    EXPECT_EQ(true, ability_->RequestForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_RequestForm_0200
 * @tc.name: RequestForm
 * @tc.desc: Abnormal case: test whether return fasle when formId is 0.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_RequestForm_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    int64_t formId = 0;

    EXPECT_EQ(false, ability_->RequestForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_RequestForm_0300
 * @tc.name: RequestForm
 * @tc.desc: Abnormal case: test whether return fasle when RecoverStatus is IN_RECOVERING.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_RequestForm_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    FormMgr::SetRecoverStatus(Constants::IN_RECOVERING);
    int64_t formId = 300L;

    EXPECT_EQ(false, ability_->RequestForm(formId));

    GTEST_LOG_(INFO) << "AaFwk_Ability_RequestForm_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyInvisibleForms_0100
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Normal case: test whether NotifyInvisibleForms is called normally.(formId is illegal)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyInvisibleForms_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0100 start";
    std::vector<int64_t> formIds;
    ability_->NotifyInvisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyInvisibleForms_0200
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Normal case: test whether NotifyInvisibleForms is called normally.(RecoverStatus is IN_RECOVERING)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyInvisibleForms_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0200 start";
    std::vector<int64_t> formIds;
    formIds.push_back(1);
    FormMgr::SetRecoverStatus(Constants::IN_RECOVERING);
    ability_->NotifyInvisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyInvisibleForms_0300
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Normal case: test whether NotifyInvisibleForms is called normally.(FormMgr reply 0)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyInvisibleForms_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0300 start";
    std::vector<int64_t> formIds;
    formIds.push_back(1);
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);
    ability_->NotifyInvisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyInvisibleForms_0400
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Normal case: test whether NotifyInvisibleForms is called normally.(FormMgr reply 1)
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyInvisibleForms_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0400 start";
    std::vector<int64_t> formIds;
    formIds.push_back(1);
    formIds.push_back(2);
    FormMgr::SetRecoverStatus(Constants::NOT_IN_RECOVERY);
    ability_->NotifyInvisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyInvisibleForms_0400 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyVisibleForms_0100
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Normal case: test whether NotifyVisibleForms is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyVisibleForms_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    formIds.push_back(100);
    formIds.push_back(101);
    ability_->NotifyVisibleForms(formIds);

    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyVisibleForms_0200
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Abnormal case: test whether NotifyVisibleForms is called with
 *           specified error log when the size of formIds is empty.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyVisibleForms_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    ability_->NotifyVisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyVisibleForms_0300
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Normal case: test whether NotifyVisibleForms is called with
 *           specified error log when the size of formIds is larger than 32.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyVisibleForms_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    for (int64_t formId = 300L; formId < 333L; formId++) {
        formIds.push_back(formId);
    }
    ability_->NotifyVisibleForms(formIds);
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_NotifyVisibleForms_0400
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Normal case: test whether NotifyVisibleForms is called with
 *           specified error log when the recover status is in recovering.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_NotifyVisibleForms_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0400 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::vector<int64_t> formIds;
    formIds.push_back(100);
    formIds.push_back(101);
    FormMgr::SetRecoverStatus(Constants::IN_RECOVERING);
    ability_->NotifyVisibleForms(formIds);

    GTEST_LOG_(INFO) << "AaFwk_Ability_NotifyVisibleForms_0400 end";
}

/**
 * @tc.number: AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0100
 * @tc.name: OnDeathReceived
 * @tc.desc: Normal case: test whether OnDeathReceived is called normally with a form.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName = "formName0100";
    std::string moduleName = "moduleName0100";
    std::string deviceId = "deviceId0100";
    std::string bundleName = "bundleName0100";
    std::string abilityName = "abilityName0100";
    bool tempFormFlg = false;
    Want want;
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName)
        .SetElementName(deviceId, bundleName, abilityName)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg);
    std::shared_ptr<MockPageAbility::FormCallback> callback;

    FormMgr::GetInstance().RegisterDeathCallback(ability_);

    int64_t formId = 100L;
    ability_->userReqParams_.emplace(formId, want);
    ability_->appCallbacks_.emplace(formId, callback);

    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.empty());
    ability_->OnDeathReceived();
    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.empty());
    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0200
 * @tc.name: OnDeathReceived
 * @tc.desc: Normal case: test whether OnDeathReceived is called normally with a form and a temp form.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName1 = "formName0200";
    std::string moduleName1 = "moduleName0200";
    std::string deviceId1 = "deviceId0200";
    std::string bundleName1 = "bundleName0200";
    std::string abilityName1 = "abilityName0200";
    bool tempFormFlg1 = false;
    Want want1;
    want1.SetParam(Constants::PARAM_FORM_NAME_KEY, formName1)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName1)
        .SetElementName(deviceId1, bundleName1, abilityName1)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg1);
    std::shared_ptr<MockPageAbility::FormCallback> callback1;

    std::string formName2 = "formName0201";
    std::string moduleName2 = "moduleName0201";
    std::string deviceId2 = "deviceId0201";
    std::string bundleName2 = "bundleName0201";
    std::string abilityName2 = "abilityName0201";
    bool tempFormFlg2 = true;
    Want want2;
    want2.SetParam(Constants::PARAM_FORM_NAME_KEY, formName2)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName2)
        .SetElementName(deviceId2, bundleName2, abilityName2)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg2);
    std::shared_ptr<MockPageAbility::FormCallback> callback2;

    FormMgr::GetInstance().RegisterDeathCallback(ability_);

    int64_t formId1 = 200L;
    int64_t formId2 = 201L;
    ability_->userReqParams_.emplace(formId1, want1);
    ability_->userReqParams_.emplace(formId2, want2);
    ability_->appCallbacks_.emplace(formId1, callback1);
    ability_->appCallbacks_.emplace(formId2, callback2);

    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.empty());
    ability_->OnDeathReceived();
    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.size() == 1);
    EXPECT_EQ(formId2, ability_->lostedByReconnectTempForms_[0]);
    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0300
 * @tc.name: OnDeathReceived
 * @tc.desc: Normal case: test whether OnDeathReceived is called normally with two temp form.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    sptr<AbilityThread> abilityThread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner, abilityThread);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    std::string formName1 = "formName0300";
    std::string moduleName1 = "moduleName0300";
    std::string deviceId1 = "deviceId0300";
    std::string bundleName1 = "bundleName0300";
    std::string abilityName1 = "abilityName0300";
    bool tempFormFlg1 = true;
    Want want1;
    want1.SetParam(Constants::PARAM_FORM_NAME_KEY, formName1)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName1)
        .SetElementName(deviceId1, bundleName1, abilityName1)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg1);
    std::shared_ptr<MockPageAbility::FormCallback> callback1;

    std::string formName2 = "formName0301";
    std::string moduleName2 = "moduleName0301";
    std::string deviceId2 = "deviceId0301";
    std::string bundleName2 = "bundleName0301";
    std::string abilityName2 = "abilityName0301";
    bool tempFormFlg2 = true;
    Want want2;
    want2.SetParam(Constants::PARAM_FORM_NAME_KEY, formName2)
        .SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName2)
        .SetElementName(deviceId2, bundleName2, abilityName2)
        .SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, tempFormFlg2);
    std::shared_ptr<MockPageAbility::FormCallback> callback2;

    FormMgr::GetInstance().RegisterDeathCallback(ability_);

    int64_t formId1 = 200L;
    int64_t formId2 = 201L;
    ability_->userReqParams_.emplace(formId1, want1);
    ability_->userReqParams_.emplace(formId2, want2);
    ability_->appCallbacks_.emplace(formId1, callback1);
    ability_->appCallbacks_.emplace(formId2, callback2);

    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.empty());
    ability_->OnDeathReceived();
    EXPECT_EQ(true, ability_->lostedByReconnectTempForms_.size() == 2);
    EXPECT_EQ(formId1, ability_->lostedByReconnectTempForms_[0]);
    EXPECT_EQ(formId2, ability_->lostedByReconnectTempForms_[1]);

    GTEST_LOG_(INFO) << "AaFwk_Ability_FmsDeathCallback_OnDeathReceived_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_CheckFMSReady_0100
 * @tc.name: CheckFMSReady
 * @tc.desc: Test whether CheckFMSReady is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_CheckFMSReady_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_CheckFMSReady_0100 start";

    std::string bundleName = "Bundle";
    std::vector<FormInfo> formInfos;
    formInfos.clear();
    EXPECT_EQ(true, ability_->CheckFMSReady());

    GTEST_LOG_(INFO) << "AaFwk_Ability_CheckFMSReady_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetAllFormsInfo_0100
 * @tc.name: GetAllFormsInfo
 * @tc.desc: Test whether GetAllFormsInfo is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_GetAllFormsInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAllFormsInfo_0100 start";

    std::string bundleName = "Bundle";
    std::vector<FormInfo> formInfos;
    formInfos.clear();
    EXPECT_EQ(true, ability_->GetAllFormsInfo(formInfos));
    size_t dataSize {1};
    EXPECT_EQ(dataSize, formInfos.size());

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAllFormsInfo_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetFormsInfoByApp_0100
 * @tc.name: GetFormsInfoByApp
 * @tc.desc: Test whether GetFormsInfoByApp is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_GetFormsInfoByApp_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetFormsInfoByApp_0100 start";

    std::string bundleName = "Bundle";
    std::vector<FormInfo> formInfos;
    formInfos.clear();
    EXPECT_EQ(true, ability_->GetFormsInfoByApp(bundleName, formInfos));
    size_t dataSize {1};
    EXPECT_EQ(dataSize, formInfos.size());
    bundleName = "";
    EXPECT_EQ(false, ability_->GetFormsInfoByApp(bundleName, formInfos));

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetFormsInfoByApp_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetFormsInfoByModule_0100
 * @tc.name: GetFormsInfoByModule
 * @tc.desc: Test whether GetFormsInfoByModule is called normally.
 */
HWTEST_F(FormAbilityTest, AaFwk_Ability_GetFormsInfoByModule_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetFormsInfoByModule_0100 start";

    std::string bundleName = "Bundle";
    std::string moduleName = "Module";
    std::vector<FormInfo> formInfos;
    formInfos.clear();
    EXPECT_EQ(true, ability_->GetFormsInfoByModule(bundleName, moduleName, formInfos));
    size_t dataSize {1};
    EXPECT_EQ(dataSize, formInfos.size());
    bundleName = "";
    EXPECT_EQ(false, ability_->GetFormsInfoByModule(bundleName, moduleName, formInfos));

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetFormsInfoByModule_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
