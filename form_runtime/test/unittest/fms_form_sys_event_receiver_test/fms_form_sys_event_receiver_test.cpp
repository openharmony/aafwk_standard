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

#include "accesstoken_kit.h"
#include "common_event_manager.h"
#include "common_event_data.h"
#include "common_event_support.h"
#include "form_ams_helper.h"
#include "form_constants.h"
#define private public
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_host_interface.h"
#include "form_mgr.h"
#undef private
#include "form_mgr_errors.h"
#include "form_mgr_service.h"
#include "form_refresh_limiter.h"
#include "form_sys_event_receiver.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "form_bms_helper.h"
#include "iservice_registry.h"

#include "mock_ability_manager.h"
#include "mock_bundle_manager.h"
#include "mock_form_host_client.h"
#include "running_process_info.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Security;

namespace {
const std::string PERMISSION_NAME_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string PARAM_PROVIDER_PACKAGE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_BUNDLE_NAME = "com.form.provider.service";
const std::string FORM_PROVIDER_BUNDLE_NAME_1 = "com.form.provider.service1";
const std::string PARAM_PROVIDER_MODULE_NAME = "com.form.provider.app.test.abiliy";
const std::string FORM_PROVIDER_ABILITY_NAME = "com.form.provider.app.test.abiliy";
const std::string PARAM_FORM_NAME = "com.form.name.test";

const std::string FORM_JS_COMPOMENT_NAME = "jsComponentName";
const std::string FORM_PROVIDER_MODULE_SOURCE_DIR = "";

const std::string FORM_HOST_BUNDLE_NAME = "com.form.host.app";

const int32_t PARAM_FORM_DIMENSION_VALUE = 1;

const std::string KEY_UID = "uid";
const std::string KEY_USER_ID = "userId";
const std::string KEY_BUNDLE_NAME = "bundleName";
const std::string DEVICE_ID = "ohos-phone1";
const std::string DEF_LABEL1 = "PermissionFormRequireGrant";

template<typename F>
static void WaitUntilTaskCalled(const F &f, const std::shared_ptr<EventHandler> &handler, std::atomic<bool> &taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<EventHandler> &handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}

class FmsFormSysEventReceiverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void CreateEventData(std::string bundle, int64_t formId,
        int callingUid, std::string actionType, EventFwk::CommonEventData &eventData);
    void CreateEventData(std::string bundle, int64_t formId,
        int callingUid, int32_t userId, std::string actionType, EventFwk::CommonEventData &eventData);
    void CreateFormRecordAndFormInfo(std::string bundle, int64_t formId, int callingUid);
    void ClearFormRecord(int64_t formId);

protected:
    sptr<MockFormHostClient> token_;
    std::shared_ptr<FormMgrService> formyMgrServ_ = DelayedSingleton<FormMgrService>::GetInstance();
};

void FmsFormSysEventReceiverTest::SetUpTestCase()
{
    FormBmsHelper::GetInstance().SetBundleManager(new BundleMgrService());
    FormAmsHelper::GetInstance().SetAbilityManager(new MockAbilityMgrService());
}

void FmsFormSysEventReceiverTest::TearDownTestCase()
{}

void FmsFormSysEventReceiverTest::SetUp()
{
    formyMgrServ_->OnStart();
    token_ = new (std::nothrow) MockFormHostClient();

    // Permission install
    int userId = 0;
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, FORM_PROVIDER_BUNDLE_NAME, 0);
    auto flag = OHOS::Security::AccessToken::PERMISSION_USER_FIXED;
    AccessToken::AccessTokenKit::GrantPermission(tokenId, PERMISSION_NAME_REQUIRE_FORM, flag);
}

void FmsFormSysEventReceiverTest::TearDown()
{}

void FmsFormSysEventReceiverTest::CreateEventData(std::string bundle, int64_t formId,
    int callingUid, std::string actionType, EventFwk::CommonEventData &eventData)
{
    Want want;
    want.SetAction(actionType);
    want.SetBundle(bundle);
    want.SetParam(KEY_UID, callingUid);
    eventData.SetWant(want);
}
void FmsFormSysEventReceiverTest::CreateEventData(std::string bundle, int64_t formId,
    int callingUid, int32_t userId, std::string actionType, EventFwk::CommonEventData &eventData)
{
    Want want;
    want.SetAction(actionType);
    want.SetBundle(bundle);
    want.SetParam(KEY_UID, callingUid);
    want.SetParam(KEY_USER_ID, userId);
    eventData.SetWant(want);
}
void FmsFormSysEventReceiverTest::CreateFormRecordAndFormInfo(std::string bundle, int64_t formId, int callingUid)
{
    FormItemInfo record;
    record.SetFormId(formId);
    record.SetProviderBundleName(bundle);
    record.SetModuleName(PARAM_FORM_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record.SetFormName(PARAM_FORM_NAME);
    record.SetSpecificationId(PARAM_FORM_DIMENSION_VALUE);
    record.SetTemporaryFlag(true);

    FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);

    FormRecord realFormRecord;
    FormDataMgr::GetInstance().GetFormRecord(formId, realFormRecord);

    FormDBInfo formDBInfo(formId, realFormRecord);
    FormDbCache::GetInstance().SaveFormInfo(formDBInfo);

    FormDataMgr::GetInstance().AllotFormHostRecord(record, token_, formId, callingUid);
}

void FmsFormSysEventReceiverTest::ClearFormRecord(int64_t formId)
{
    FormDataMgr::GetInstance().DeleteFormRecord(formId);
    FormDbCache::GetInstance().DeleteFormInfo(formId);
    FormDataMgr::GetInstance().DeleteHostRecord(token_, formId);
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleProviderRemoved works.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_001 start";
    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcff00000000;
    int callingUid {0};

    FormItemInfo record;
    record.SetFormId(formId);
    record.SetProviderBundleName(bundle);
    record.SetModuleName(PARAM_PROVIDER_MODULE_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record.SetFormName(PARAM_FORM_NAME);
    record.SetSpecificationId(PARAM_FORM_DIMENSION_VALUE);
    record.SetTemporaryFlag(false);
    Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    want.SetBundle(bundle);
    FormRecord realFormRecord = FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    // Set database info
    FormDBInfo formDBInfo(formId, realFormRecord);
    std::vector<FormDBInfo> allFormInfo;
    FormDbCache::GetInstance().SaveFormInfo(formDBInfo);
    // Set form host record
    FormItemInfo info;
    FormDataMgr::GetInstance().AllotFormHostRecord(info, token_, formId, callingUid);
    EventFwk::CommonEventData eventData;
    eventData.SetWant(want);
    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);
    FormDbCache::GetInstance().GetAllFormInfo(allFormInfo);
    FormDBInfo tempFormDBInfo;
    EXPECT_EQ(ERR_APPEXECFWK_FORM_NOT_EXIST_ID, FormDbCache::GetInstance().GetDBRecord(formId, tempFormDBInfo));
    FormDataMgr::GetInstance().DeleteFormRecord(formId);
    FormDbCache::GetInstance().DeleteFormInfo(formId);
    FormDataMgr::GetInstance().DeleteHostRecord(token_, formId);
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_001 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleBundleDataCleared works.
 * [COMMON_EVENT_PACKAGE_DATA_CLEARED] want's uid is 0.  formrecord's uid is 15.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_002 start";
    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcff00000000;
    int callingUid {15};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED;
    EventFwk::CommonEventData eventData;
    int callingUidForWant = 0;
    CreateEventData(bundle, formId, callingUidForWant, actionType, eventData);
    CreateFormRecordAndFormInfo(bundle, formId, callingUid);

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    FormRecord tempFormRecord;
    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    ClearFormRecord(formId);
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_002 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleBundleDataCleared works.
 * [COMMON_EVENT_PACKAGE_DATA_CLEARED] want's uid and formrecord's and hostrecord's uid is 15.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_003 start";

    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    std::string bundle1 = FORM_PROVIDER_BUNDLE_NAME_1;
    int64_t formId = 0x0ffabcdf00000000;
    int callingUid {15};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED;
    EventFwk::CommonEventData eventData;
    CreateEventData(bundle1, formId, callingUid, actionType, eventData);
    CreateFormRecordAndFormInfo(bundle, formId, callingUid);

    FormRecord tempFormRecord;
    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    ClearFormRecord(formId);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_003 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleBundleDataCleared works.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_004 start";
    EventFwk::CommonEventData eventData;
    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int callingUid {15};
    Want want;
    FormRecord tempFormRecord;
    eventData.SetWant(want);
    want.SetBundle(bundle);
    want.SetParam(KEY_UID, callingUid);
    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_004 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleBundleDataCleared works.
 * invalid action.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_005 start";

    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcdf00000000;
    int callingUid {15};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED + "ERROR";
    EventFwk::CommonEventData eventData;
    CreateEventData(bundle, formId, callingUid, actionType, eventData);

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_005 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if  HandleBundleDataCleared works.
 * [COMMON_EVENT_PACKAGE_DATA_CLEARED] There is 2 callingUids.
 */
HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_006 start";

    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcdf00000000;
    int callingUid {15};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED;
    EventFwk::CommonEventData eventData;
    CreateEventData(bundle, formId, callingUid, actionType, eventData);

    // CreateFormRecordAndFormInfo
    FormItemInfo record;
    record.SetFormId(formId);
    record.SetProviderBundleName(bundle);
    record.SetModuleName(PARAM_FORM_NAME);
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME);
    record.SetFormName(PARAM_FORM_NAME);
    record.SetSpecificationId(PARAM_FORM_DIMENSION_VALUE);
    record.SetTemporaryFlag(true);
    FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    // AddFormUserUid
    int new_callingUid = 150;
    FormDataMgr::GetInstance().AddFormUserUid(formId, new_callingUid);
    FormRecord realFormRecord;
    FormDataMgr::GetInstance().GetFormRecord(formId, realFormRecord);
    FormDBInfo formDBInfo(formId, realFormRecord);
    FormDbCache::GetInstance().SaveFormInfo(formDBInfo);
    FormDataMgr::GetInstance().AllotFormHostRecord(record, token_, formId, callingUid);

    FormRecord tempFormRecord;
    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    ClearFormRecord(formId);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_006 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if HandleProviderUpdated works.
 * [COMMON_EVENT_ABILITY_UPDATED] ProviderFormUpdated return false. delete formrecord.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_007 start";

    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcdf00000000;
    int callingUid {0};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    EventFwk::CommonEventData eventData;
    CreateEventData(bundle, formId, callingUid, actionType, eventData);
    CreateFormRecordAndFormInfo(bundle, formId, callingUid);

    FormRecord tempFormRecord;
    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    ClearFormRecord(formId);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_007 end";
}

/*
 * Feature: FormMgrService
 * Function: FormMgr
 * SubFunction: OnReceiveEvent Functions
 * FunctionPoints: FormMgr OnReceiveEvent interface
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify if HandleProviderUpdated works.
 * [COMMON_EVENT_ABILITY_UPDATED] ProviderFormUpdated return true. refresh form.
 */

HWTEST_F(FmsFormSysEventReceiverTest, OnReceiveEvent_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_008 start";

    std::string bundle = FORM_PROVIDER_BUNDLE_NAME;
    int64_t formId = 0x0ffabcdf00000000;
    int callingUid {15};
    std::string actionType = EventFwk::CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    EventFwk::CommonEventData eventData;
    CreateEventData(bundle, formId, callingUid, actionType, eventData);

    // CreateFormRecordAndFormInfo
    FormItemInfo record;
    record.SetFormId(formId);
    record.SetProviderBundleName(bundle);
    record.SetModuleName(PARAM_PROVIDER_MODULE_NAME); // model name
    record.SetAbilityName(FORM_PROVIDER_ABILITY_NAME); // ability name
    record.SetFormName(PARAM_FORM_NAME); // form name
    record.SetSpecificationId(PARAM_FORM_DIMENSION_VALUE);
    record.SetTemporaryFlag(true);

    FormDataMgr::GetInstance().AllotFormRecord(record, callingUid);
    FormRecord realFormRecord;
    FormDataMgr::GetInstance().GetFormRecord(formId, realFormRecord);
    FormDBInfo formDBInfo(formId, realFormRecord);
    FormDbCache::GetInstance().SaveFormInfo(formDBInfo);
    FormDataMgr::GetInstance().AllotFormHostRecord(record, token_, formId, callingUid);

    FormRecord tempFormRecord;
    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    FormSysEventReceiver testCase;
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    testCase.SetEventHandler(handler);
    testCase.OnReceiveEvent(eventData);
    WaitUntilTaskDone(handler);

    ASSERT_TRUE(FormDataMgr::GetInstance().GetFormRecord(formId, tempFormRecord));

    ClearFormRecord(formId);

    GTEST_LOG_(INFO) << "fms_form_sys_event_receiver_test_008 end";
}
}