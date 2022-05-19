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
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_mgr.h"
#include "form_mgr_errors.h"
#include "form_mgr_service.h"
#include "form_mgr_adapter.h"
#undef private
#include "hilog_wrapper.h"
#include "mock_bundle_manager.h"
#include "mock_form_host_client.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string NON_SYSTEM_APP_BUNDLE_NAME = "com.form.host.app600";
const std::string FORM_PROVIDER_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

class FmsFormMgrNotifyVisibleFormsTest : public testing::Test {
public:
    FmsFormMgrNotifyVisibleFormsTest() : formMgrService_(nullptr)
    {}
    ~FmsFormMgrNotifyVisibleFormsTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    sptr<MockFormHostClient> token_;
    std::shared_ptr<FormMgrService> formMgrService_;
};
void FmsFormMgrNotifyVisibleFormsTest::SetUpTestCase(void)
{
    FormBmsHelper::GetInstance().SetBundleManager(new BundleMgrService());
}

void FmsFormMgrNotifyVisibleFormsTest::TearDownTestCase(void)
{}

void FmsFormMgrNotifyVisibleFormsTest::SetUp(void)
{
    formMgrService_ = DelayedSingleton<FormMgrService>::GetInstance();
    formMgrService_->OnStart();
    token_ = new (std::nothrow) MockFormHostClient();

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_PROVIDER_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
}

void FmsFormMgrNotifyVisibleFormsTest::TearDown(void)
{}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_002
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return code is ERR_APPEXECFWK_FORM_INVALID_PARAM.
 * @tc.info: The callerToken is nullptr.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_002 start";

    // set the callerToken to nullptr.
    token_ = nullptr;

    // create formIds
    int64_t formId1 = 301;
    int64_t formId2 = 302;
    std::vector<int64_t> formIds;
    formIds.push_back(formId1);
    formIds.push_back(formId2);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId1);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);

    FormItemInfo formiteminfo2;
    formiteminfo2.SetFormId(formId2);
    formiteminfo2.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo2.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo2.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo2, 0);

    EXPECT_EQ(ERR_APPEXECFWK_FORM_INVALID_PARAM, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_,
    Constants::FORM_VISIBLE));

    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_002 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_003
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: Create two formId and push formIds, but only create one form record.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_003 start";

    // create formIds
    int64_t formId1 = 401;
    int64_t formId2 = 402;
    std::vector<int64_t> formIds;
    formIds.push_back(formId1);
    formIds.push_back(formId2);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId1);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(true);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId1, true);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId1, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));

    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_003 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_004
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: Create two formId and push formIds, but only create one form host record.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_004 start";

    // create formIds
    int64_t formId1 = 501;
    int64_t formId2 = 502;
    std::vector<int64_t> formIds;
    formIds.push_back(formId1);
    formIds.push_back(formId2);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId1);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(true);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId1, true);

    FormItemInfo formiteminfo2;
    formiteminfo2.SetFormId(formId2);
    formiteminfo2.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo2.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo2.SetFormVisibleNotify(true);
    formiteminfo2.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo2, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId2, true);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId1, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));

    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_004 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_005
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: Create two formId and push formIds, two form records and two form host records.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_005 start";

    // create formIds
    int64_t formId1 = 601;
    int64_t formId2 = 602;
    std::vector<int64_t> formIds;
    formIds.push_back(formId1);
    formIds.push_back(formId2);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId1);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(true);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId1, true);

    FormItemInfo formiteminfo2;
    formiteminfo2.SetFormId(formId2);
    formiteminfo2.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo2.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo2.SetFormVisibleNotify(true);
    formiteminfo2.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo2, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId2, true);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId1, 0);
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo2, token_, formId2, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_005 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_006
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: The NeedRefresh is false.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_006 start";

    // create formIds
    int64_t formId = 700;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(true);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId, false);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_006 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_007
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: The FormVisibleNotify is false.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_007 start";

    // create formIds
    int64_t formId = 800;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId);
    formiteminfo1.SetProviderBundleName(FORM_PROVIDER_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(false);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId, true);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId, 0);

    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_007 end";
}

/**
 * @tc.number: FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_008
 * @tc.name: NotifyVisibleForms
 * @tc.desc: Verify that the return value is ERR_OK.
 * @tc.info: The form provider is not a system app.
 */
HWTEST_F(FmsFormMgrNotifyVisibleFormsTest, FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_008 start";

    // create formIds
    int64_t formId = 900;
    std::vector<int64_t> formIds;
    formIds.push_back(formId);

    // create formRecords
    FormItemInfo formiteminfo1;
    formiteminfo1.SetFormId(formId);
    formiteminfo1.SetProviderBundleName(NON_SYSTEM_APP_BUNDLE_NAME);
    formiteminfo1.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    formiteminfo1.SetFormVisibleNotify(true);
    formiteminfo1.SetTemporaryFlag(false);
    FormDataMgr::GetInstance().AllotFormRecord(formiteminfo1, 0);
    FormDataMgr::GetInstance().SetNeedRefresh(formId, true);

    // create formHostRecord
    FormDataMgr::GetInstance().AllotFormHostRecord(formiteminfo1, token_, formId, 0);

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_PROVIDER_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
    EXPECT_EQ(ERR_OK, FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, token_, Constants::FORM_VISIBLE));
    GTEST_LOG_(INFO) << "FmsFormMgrNotifyVisibleFormsTest_NotifyVisibleForms_008 end";
}
}
