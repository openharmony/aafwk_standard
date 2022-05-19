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
#include "form_ability_c.h"
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
    FORM_EVENT_REQ_DELETE_FORM_COMMON, FORM_EVENT_REQ_ACQUIRE_FORM_3100, FORM_EVENT_REQ_ACQUIRE_FORM_3200,
    FORM_EVENT_REQ_ACQUIRE_FORM_BATCH, FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH,
};
void FormAbilityC::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_, std::to_string(formJsInfo.formId));
}
void FormAbilityC::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
    FormTestUtils::PublishEvent(this->caseName_, this->code_ + 1, formJsInfo.formData);
}

void FormAbilityC::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    HILOG_INFO("%{public}s called", __func__);
}
void FormAbilityC::FMS_deleteFormCommon(std::string strFormId)
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

void FormAbilityC::FMS_acquireForm_3100(std::string data)
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

void FormAbilityC::FMS_acquireForm_3200(std::string data)
{
    HILOG_INFO("%{public}s called", __func__);
    std::shared_ptr<AcquireFormCallback> callback =
        std::make_shared<AcquireFormCallback>(FORM_EVENT_RECV_ACQUIRE_FORM_3200, EVENT_CODE_3200);
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_3200, EVENT_CODE_3200, "false");
    }
}
void FormAbilityC::FMS_acquireFormBatch(std::string data)
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

void FormAbilityC::FMS_acquireFormTempBatch(std::string data)
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
FormAbilityC::~FormAbilityC()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}
void FormAbilityC::OnStart(const Want &want)
{
    HILOG_INFO("FormAbilityC::onStart");
    Ability::OnStart(want);
}
void FormAbilityC::OnActive()
{
    HILOG_INFO("FormAbilityC::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityC::OnStop()
{
    HILOG_INFO("FormAbilityC::OnStop");

    Ability::OnStop();
}
void FormAbilityC::OnInactive()
{
    HILOG_INFO("FormAbilityC::OnInactive");

    Ability::OnInactive();
}
void FormAbilityC::OnBackground()
{
    HILOG_INFO("FormAbilityC::OnBackground");

    Ability::OnBackground();
}
void FormAbilityC::SubscribeEvent()
{
    HILOG_INFO("FormAbilityC::SubscribeEvent");
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
void FormAbilityC::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("FormAbilityC::Init");
    Ability::Init(abilityInfo, application, handler, token);
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_COMMON] = &FormAbilityC::FMS_deleteFormCommon;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_BATCH] = &FormAbilityC::FMS_acquireFormBatch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_TEMP_FORM_BATCH] = &FormAbilityC::FMS_acquireFormTempBatch;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_3100] = &FormAbilityC::FMS_acquireForm_3100;
    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_3200] = &FormAbilityC::FMS_acquireForm_3200;

    SubscribeEvent();
}

void FormAbilityC::handleEvent(std::string action, std::string data)
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

void FormAbilityC::Clear()
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

REGISTER_AA(FormAbilityC)
}  // namespace AppExecFwk
}  // namespace OHOS