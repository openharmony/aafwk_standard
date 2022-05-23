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
#include <thread>
#include <string>

#include "accesstoken_kit.h"
#include "appexecfwk_errors.h"
#define private public
#include "form_bms_helper.h"
#include "form_mgr_errors.h"
#include "form_mgr_service.h"
#include "form_mgr_adapter.h"
#include "form_data_mgr.h"
#include "form_mgr.h"
#undef private
#include "form_constants.h"
#include "hilog_wrapper.h"
#include "mock_form_host_client.h"
#include "mock_bundle_manager.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string FORM_HOST_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormMgrNotifyInvisibleFormsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    sptr<OHOS::AppExecFwk::MockFormHostClient> token_;
    std::shared_ptr<FormMgrService> formMgrService_;
};
void FmsFormMgrNotifyInvisibleFormsTest::SetUpTestCase(void)
{
    FormBmsHelper::GetInstance().SetBundleManager(new BundleMgrService());
}

void FmsFormMgrNotifyInvisibleFormsTest::TearDownTestCase(void)
{}

void FmsFormMgrNotifyInvisibleFormsTest::SetUp(void)
{
    formMgrService_ = DelayedSingleton<FormMgrService>::GetInstance();
    formMgrService_->OnStart();

    // token
    token_ = new (std::nothrow) OHOS::AppExecFwk::MockFormHostClient();

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_HOST_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
}

void FmsFormMgrNotifyInvisibleFormsTest::TearDown(void)
{
    formMgrService_->OnStop();
    token_ = nullptr;
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_APPEXECFWK_FORM_INVALID_PARAM.
 * @tc.info: callerToken is nullptr.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002 start";

    int64_t formId = 2;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.temporaryFlag_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    // clear callerToken
    token_ = nullptr;

    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds,
        token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_002 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: form record is not found.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003 start";

    int64_t formId = 3;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_003 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is not found.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004 start";

    int64_t formId = 4;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.temporaryFlag_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_004 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is found, but formVisibleNotify is false.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005 start";

    int64_t formId = 5;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = false;
    iteminfo.temporaryFlag_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_005 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: host form record is found, formVisibleNotify is true, it is a SystemApp.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006 start";

    int64_t formId = 6;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = FORM_HOST_BUNDLE_NAME;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = true;
    iteminfo.temporaryFlag_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_006 end";
}

/**
 * @tc.number: FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007
 * @tc.name: NotifyInvisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: it is not a SystemApp.
 */
HWTEST_F(FmsFormMgrNotifyInvisibleFormsTest, FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007,
TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007 start";

    int64_t formId = 7;
    std::string mockBundleName = "com.form.host.app600";
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // creat formRecords_
    FormItemInfo iteminfo;
    iteminfo.formId_ = formId;
    iteminfo.providerBundleName_ = mockBundleName;
    iteminfo.abilityName_ = FORM_PROVIDER_ABILITY_NAME;
    iteminfo.formVisibleNotify_ = true;
    iteminfo.temporaryFlag_ = true;
    FormDataMgr::GetInstance().AllotFormRecord(iteminfo, 0);

    // creat clientRecords_
    FormDataMgr::GetInstance().AllotFormHostRecord(iteminfo, token_, formId, 0);

    // Permission install (mockBundleName)
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, mockBundleName, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_INVISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyInvisibleFormsTest_NotifyInvisibleForms_007 end";
}
}
