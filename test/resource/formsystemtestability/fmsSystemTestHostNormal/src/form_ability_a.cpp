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
#include "form_ability_a.h"
#include "hilog_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"
#include "system_test_form_util.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
std::vector<std::string> eventList = {
    FORM_EVENT_REQ_DELETE_FORM_COMMON, FORM_EVENT_REQ_ACQUIRE_FORM_TEMP,
    FORM_EVENT_REQ_ACQUIRE_FORM_0300, FORM_EVENT_REQ_ACQUIRE_FORM_0400, FORM_EVENT_REQ_ACQUIRE_FORM_0500,
    FORM_EVENT_REQ_ACQUIRE_FORM_0600, FORM_EVENT_REQ_ACQUIRE_FORM_0700, FORM_EVENT_REQ_ACQUIRE_FORM_1000,
    FORM_EVENT_REQ_ACQUIRE_FORM_1100, FORM_EVENT_REQ_ACQUIRE_FORM_1200, FORM_EVENT_REQ_ACQUIRE_FORM_1500_1,
    FORM_EVENT_REQ_ACQUIRE_FORM_1600, FORM_EVENT_REQ_ACQUIRE_FORM_1600_1, FORM_EVENT_REQ_ACQUIRE_FORM_1800,
    FORM_EVENT_REQ_ACQUIRE_FORM_1800_1, FORM_EVENT_REQ_ACQUIRE_FORM_1900, FORM_EVENT_REQ_ACQUIRE_FORM_2400,
    FORM_EVENT_REQ_ACQUIRE_FORM_2100, FORM_EVENT_REQ_ACQUIRE_FORM_2200, FORM_EVENT_REQ_ACQUIRE_FORM_2300,
    FORM_EVENT_REQ_ACQUIRE_FORM_2500, FORM_EVENT_REQ_ACQUIRE_FORM_2600, FORM_EVENT_REQ_ACQUIRE_FORM_2600_1,
    FORM_EVENT_REQ_ACQUIRE_FORM_2700, FORM_EVENT_REQ_ACQUIRE_FORM_2800, FORM_EVENT_REQ_ACQUIRE_FORM_2900,
    FORM_EVENT_REQ_ACQUIRE_FORM_2900_1, FORM_EVENT_REQ_ACQUIRE_FORM_3000, FORM_EVENT_REQ_ACQUIRE_FORM_3100,
    FORM_EVENT_REQ_ACQUIRE_FORM_BATCH, FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH, FORM_EVENT_REQ_CLEAR_FORM_BATCH,
    FORM_EVENT_REQ_ACQUIRE_FORM_BATCH_B,
};
void FormAbilityA::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_, std::to_string(formJsInfo.formId));
}

void FormAbilityA::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_ + 1, formJsInfo.formData);

    if (this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1200
        || this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1600
        || this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1600_1
        || this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1800
        || this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1800_1
        || this->caseName_ == FORM_EVENT_RECV_ACQUIRE_FORM_1900) {
        std::string strFormId = std::to_string(formJsInfo.formId);
        HILOG_INFO("%{public}s, delete form, formId: %{public}s", __func__, strFormId.c_str());
        ability_->FMS_deleteFormCommon(strFormId);
    }
}

void FormAbilityA::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    HILOG_INFO("%{public}s called", __func__);
}

void FormAbilityA::FMS_deleteFormCommon(std::string strFormId)
{
    HILOG_INFO("%{public}s called, formId: %{public}s", __func__, strFormId.c_str());
    if (strFormId.empty()) {
        HILOG_ERROR("DeleteForm error, formId is 0");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "false");
        return;
    }
    int64_t formId = std::stoll(strFormId);
    sleep(1);
    bool bResult = DeleteForm(formId);
    sleep(1);
    if (bResult) {
        HILOG_INFO("DeleteForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "true");
    } else {
        HILOG_ERROR("DeleteForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_COMMON, EVENT_CODE_999, "false");
    }
}

void FormAbilityA::FMS_acquireForm_0300(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_0300, EVENT_CODE_300);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(-1, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0300, EVENT_CODE_300, "false");
    }
}

void FormAbilityA::FMS_acquireForm_0400(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_0400, EVENT_CODE_400);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, "com.form.bundlename99", FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0400, EVENT_CODE_400, "false");
    }
}

void FormAbilityA::FMS_acquireForm_0500(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_0500, EVENT_CODE_500);
    // Set Want info begin
    std::string moduleName = "moduleName99";
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, moduleName);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0500, EVENT_CODE_500, "false");
    }
}

void FormAbilityA::FMS_acquireForm_0600(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_0600, EVENT_CODE_600);
    // Set Want info begin
    std::string formName = "formName999";
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, formName);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0600, EVENT_CODE_600, "false");
    }
}

void FormAbilityA::FMS_acquireForm_0700(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_0700, EVENT_CODE_700);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, "abilityName9");
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0700, EVENT_CODE_700, "false");
    }
}

void FormAbilityA::FMS_acquireForm_1000(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1000, EVENT_CODE_1000);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, 0);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1000, EVENT_CODE_1000, "false");
    }
}

void FormAbilityA::FMS_acquireForm_1100(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1100);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1100, EVENT_CODE_1100, "");
    }
}

void FormAbilityA::FMS_acquireForm_1200(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1200);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME2);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME2);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME2, FORM_PROVIDER_ABILITY_NAME2);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1200, EVENT_CODE_1200, "");
    }
}

void FormAbilityA::FMS_acquireForm_1500_1(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1510);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1500_1, EVENT_CODE_1510, "");
    }
}

void FormAbilityA::FMS_acquireForm_1600(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1600);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1600, EVENT_CODE_1600, "");
    }
}

void FormAbilityA::FMS_acquireForm_1600_1(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1610);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1600_1, EVENT_CODE_1610, "");
    }
}

void FormAbilityA::FMS_acquireForm_1800(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1800);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1800, EVENT_CODE_1800, "");
    }
}

void FormAbilityA::FMS_acquireForm_1800_1(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1810);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1800_1, EVENT_CODE_1810, "");
    }
}

void FormAbilityA::FMS_acquireForm_1900(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1900);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_1900, EVENT_CODE_1900, "");
    }
}

void FormAbilityA::FMS_acquireForm_2100(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2100, EVENT_CODE_2100);
    callback->ability_ = this;
    // Set Want info begin
    int64_t formId = std::stoll(data);
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(formId, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2100, EVENT_CODE_2100, "false");
    }
}

void FormAbilityA::FMS_acquireForm_2200(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2200, EVENT_CODE_2200);

    // Set Want info end
    bool bResult = CastTempForm(-1);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2200, EVENT_CODE_2200, "false");
    }
}

void FormAbilityA::FMS_acquireForm_2300(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2300, EVENT_CODE_2300);

    // Set Want info end
    bool bResult = CastTempForm(0);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2300, EVENT_CODE_2300, "false");
    }
}

void FormAbilityA::FMS_acquireForm_2400(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2400);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2400, EVENT_CODE_2400, "");
    }
}

void FormAbilityA::FMS_acquireForm_2500(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2500, EVENT_CODE_2500);

    // Set Want info end
    bool bResult = CastTempForm(EVENT_CODE_1234);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2500, EVENT_CODE_2500, "false");
    }
}

void FormAbilityA::FMS_acquireForm_2600_1(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2610);

    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2600_1, EVENT_CODE_2610, "");
    }
}

void FormAbilityA::FMS_acquireForm_2600(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2600, EVENT_CODE_2600);
    callback->ability_ = this;
    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = CastTempForm(formId);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2600, EVENT_CODE_2600, "true");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2600, EVENT_CODE_2600, "false");
    }

    FMS_deleteFormCommon(data);
}

void FormAbilityA::FMS_acquireForm_2700(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2700, EVENT_CODE_2700);

    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = CastTempForm(formId);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2700, EVENT_CODE_2700, "false");
    }

    FMS_deleteFormCommon(data);
}

void FormAbilityA::FMS_acquireForm_2800(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2800, EVENT_CODE_2800);

    // Set Want info end
    int64_t formId = std::stoll(data);
    bool bResult = CastTempForm(formId);
    if (bResult) {
        HILOG_INFO("CastTempForm end");
    } else {
        HILOG_ERROR("CastTempForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2800, EVENT_CODE_2800, "false");
    }

    FMS_deleteFormCommon(data);
}

void FormAbilityA::FMS_acquireForm_2900(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2900);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2900, EVENT_CODE_2900, "");
    }
}

void FormAbilityA::FMS_acquireForm_2900_1(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_2900_1, EVENT_CODE_2910);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME2);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME2);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME2, FORM_PROVIDER_ABILITY_NAME2);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_2900_1, EVENT_CODE_2910, "");
    }
}

void FormAbilityA::FMS_acquireForm_3000(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_3000, EVENT_CODE_3000);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_3000, EVENT_CODE_3000, "false");
    }
}

void FormAbilityA::FMS_acquireForm_3100(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100);
    callback->ability_ = this;
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);
    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_3100, EVENT_CODE_3100, "");
    }
}

void FormAbilityA::FMS_acquireForm_3300(std::string data)
{
}

void FormAbilityA::FMS_acquireForm_3400(std::string data)
{
}

void FormAbilityA::FMS_acquireForm_3500(std::string data)
{
}

void FormAbilityA::FMS_acquireForm_tempForm(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP);

    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    // Set Want info end
    bool bResult = AcquireForm(0, want, callback);
    if (bResult) {
        HILOG_INFO("AcquireForm end");
    } else {
        HILOG_ERROR("AcquireForm error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_TEMP, EVENT_CODE_TEMP, "");
    }
}

void FormAbilityA::FMS_acquireFormBatch(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    HILOG_INFO("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        HILOG_INFO("Batch add form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "true");
    } else {
        HILOG_ERROR("Batch add form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH, EVENT_CODE_BATCH, "false");
    }
}

void FormAbilityA::FMS_acquireFormBatchB(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME2);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME2);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME2, FORM_PROVIDER_ABILITY_NAME2);

    int formCount = std::stoi(data);
    HILOG_INFO("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        HILOG_INFO("Batch add form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B, EVENT_CODE_BATCH_B, "true");
    } else {
        HILOG_ERROR("Batch add form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_BATCH_B, EVENT_CODE_BATCH_B, "false");
    }
}

void FormAbilityA::FMS_acquireFormTempBatch(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    // Set Want info begin
    Want want;
    want.SetParam(Constants::PARAM_FORM_DIMENSION_KEY, FORM_DIMENSION_1);
    want.SetParam(Constants::PARAM_FORM_NAME_KEY, PARAM_FORM_NAME1);
    want.SetParam(Constants::PARAM_MODULE_NAME_KEY, PARAM_PROVIDER_MODULE_NAME1);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_FALSE);
    want.SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, FORM_TEMP_FORM_FLAG_TRUE);
    want.SetElementName(FORM_TEST_DEVICEID, FORM_PROVIDER_BUNDLE_NAME1, FORM_PROVIDER_ABILITY_NAME1);

    int formCount = std::stoi(data);
    HILOG_INFO("%{public}s, formCount: %{public}d", __func__, formCount);
    want.SetParam(Constants::PARAM_FORM_ADD_COUNT, formCount);
    // Set Want info end
    int result = STtools::SystemTestFormUtil::BatchAddFormRecords(want);
    if (result == ERR_OK) {
        HILOG_INFO("Batch add temp form end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "true");
    } else {
        HILOG_ERROR("Batch add temp form error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_TEMP_FORM_BATCH, EVENT_CODE_TEMP_BATCH, "false");
    }
}

void FormAbilityA::FMS_deleteFormBatch(std::string strFormId)
{
    HILOG_INFO("%{public}s called", __func__);
    int result = STtools::SystemTestFormUtil::ClearFormRecords();
    if (result == ERR_OK) {
        HILOG_INFO("Clear form records end");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH, "true");
    } else {
        HILOG_ERROR("Clear form records error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_CLEAR_FORM_BATCH, EVENT_CODE_CLEAR_BATCH, "false");
    }
}

FormAbilityA::~FormAbilityA()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FormAbilityA::OnStart(const Want &want)
{
    HILOG_INFO("FormAbilityA::onStart");
    Ability::OnStart(want);
}

void FormAbilityA::OnActive()
{
    HILOG_INFO("FormAbilityA::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityA::OnStop()
{
    HILOG_INFO("FormAbilityA::OnStop");

    Ability::OnStop();
}

void FormAbilityA::OnInactive()
{
    HILOG_INFO("FormAbilityA::OnInactive");

    Ability::OnInactive();
}

void FormAbilityA::OnBackground()
{
    HILOG_INFO("FormAbilityA::OnBackground");

    Ability::OnBackground();
}

void FormAbilityA::SubscribeEvent()
{
    HILOG_INFO("FormAbilityA::SubscribeEvent");
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FormEventSubscriber>(subscribeInfo);
    subscriber_->ability_ = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

// KitTest End
void FormAbilityA::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("FormAbilityA::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0300] = &FormAbilityA::FMS_acquireForm_0300;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0400] = &FormAbilityA::FMS_acquireForm_0400;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0500] = &FormAbilityA::FMS_acquireForm_0500;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0600] = &FormAbilityA::FMS_acquireForm_0600;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0700] = &FormAbilityA::FMS_acquireForm_0700;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1000] = &FormAbilityA::FMS_acquireForm_1000;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1100] = &FormAbilityA::FMS_acquireForm_1100;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1200] = &FormAbilityA::FMS_acquireForm_1200;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1500_1] = &FormAbilityA::FMS_acquireForm_1500_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1600] = &FormAbilityA::FMS_acquireForm_1600;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1600_1] = &FormAbilityA::FMS_acquireForm_1600_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1800] = &FormAbilityA::FMS_acquireForm_1800;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1800_1] = &FormAbilityA::FMS_acquireForm_1800_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_1900] = &FormAbilityA::FMS_acquireForm_1900;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2100] = &FormAbilityA::FMS_acquireForm_2100;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2200] = &FormAbilityA::FMS_acquireForm_2200;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2300] = &FormAbilityA::FMS_acquireForm_2300;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2400] = &FormAbilityA::FMS_acquireForm_2400;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2500] = &FormAbilityA::FMS_acquireForm_2500;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2600] = &FormAbilityA::FMS_acquireForm_2600;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2600_1] = &FormAbilityA::FMS_acquireForm_2600_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2700] = &FormAbilityA::FMS_acquireForm_2700;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2800] = &FormAbilityA::FMS_acquireForm_2800;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2900] = &FormAbilityA::FMS_acquireForm_2900;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_2900_1] = &FormAbilityA::FMS_acquireForm_2900_1;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_3000] = &FormAbilityA::FMS_acquireForm_3000;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_3100] = &FormAbilityA::FMS_acquireForm_3100;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_COMMON] = &FormAbilityA::FMS_deleteFormCommon;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_TEMP] = &FormAbilityA::FMS_acquireForm_tempForm;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_BATCH] = &FormAbilityA::FMS_acquireFormBatch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_BATCH_B] = &FormAbilityA::FMS_acquireFormBatchB;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH] = &FormAbilityA::FMS_acquireFormTempBatch;
    memberFuncMap_[FORM_EVENT_REQ_CLEAR_FORM_BATCH] = &FormAbilityA::FMS_deleteFormBatch;
    SubscribeEvent();
}

void FormAbilityA::handleEvent(std::string action, std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    if (calledFuncMap_.find(action) != calledFuncMap_.end()) {
        return;
    }
    calledFuncMap_.emplace(action, 0);
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data);
        }
    }
}

void FormAbilityA::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("FormEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("KitTestEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("FormEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName, data.GetData());

    CommonEventManager::UnSubscribeCommonEvent(ability_->subscriber_);
}

void FormEventSubscriber::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityA)
}  // namespace AppExecFwk
}  // namespace OHOS