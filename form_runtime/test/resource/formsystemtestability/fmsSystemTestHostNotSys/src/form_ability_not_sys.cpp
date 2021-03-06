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
#include "form_ability_not_sys.h"
#include "hilog_wrapper.h"
#include "form_st_common_info.h"
#include "form_test_utils.h"

namespace {
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;
}

namespace OHOS {
namespace AppExecFwk {
void FormAbilityNotSys::AcquireFormCallback::OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
}
void FormAbilityNotSys::AcquireFormCallback::OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const
{
    HILOG_INFO("%{public}s called", __func__);
}
void FormAbilityNotSys::AcquireFormCallback::OnFormUninstall(const int64_t formId) const
{
    HILOG_INFO("%{public}s called", __func__);
}
void FormAbilityNotSys::FMS_acquireForm_0100()
{
    std::shared_ptr<AcquireFormCallback> callback = std::make_shared<AcquireFormCallback>("FMS_acquireForm_0100");
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
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_ACQUIRE_FORM_0100, EVENT_CODE_100, "false");
    }
}

void FormAbilityNotSys::FMS_deleteForm_0100()
{
    int64_t formId = 1;
    bool bResult = DeleteForm(formId);
    if (bResult) {
        HILOG_INFO("[FMS_deleteForm_0100] end");
    } else {
        HILOG_ERROR("[FMS_deleteForm_0100] error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_DELETE_FORM_0100, EVENT_CODE_100, "false");
    }
}

void FormAbilityNotSys::FMS_releaseForm_0100()
{
    int64_t formId = 1;
    bool isReleaseCache = true;
    bool bResult = ReleaseForm(formId, isReleaseCache);
    if (bResult) {
        HILOG_INFO("[FMS_releaseForm_0100] end");
    } else {
        HILOG_ERROR("[FMS_releaseForm_0100] error");
        FormTestUtils::PublishEvent(FORM_EVENT_RECV_RELEASE_FORM_0100, EVENT_CODE_100, "false");
    }
}

void FormAbilityNotSys::OnStart(const Want &want)
{
    HILOG_INFO("FormAbilityNotSys::onStart");
    Ability::OnStart(want);

    std::vector<std::string> eventList = {
        FORM_EVENT_REQ_ACQUIRE_FORM_0100,
        FORM_EVENT_REQ_DELETE_FORM_0100,
        FORM_EVENT_REQ_RELEASE_FORM_0100,
    };
    SubscribeEvent(eventList);
}
void FormAbilityNotSys::OnActive()
{
    HILOG_INFO("FormAbilityNotSys::OnActive");
    Ability::OnActive();
    std::string eventData = GetAbilityName() + FORM_ABILITY_STATE_ONACTIVE;
    FormTestUtils::PublishEvent(FORM_EVENT_ABILITY_ONACTIVED, 0, eventData);
}

void FormAbilityNotSys::OnStop()
{
    HILOG_INFO("FormAbilityNotSys::OnStop");

    Ability::OnStop();
}
void FormAbilityNotSys::OnInactive()
{
    HILOG_INFO("FormAbilityNotSys::OnInactive");

    Ability::OnInactive();
}
void FormAbilityNotSys::OnBackground()
{
    HILOG_INFO("FormAbilityNotSys::OnBackground");

    Ability::OnBackground();
}
void FormAbilityNotSys::SubscribeEvent(const std::vector<std::string> &eventList)
{
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
void FormAbilityNotSys::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("FormAbilityNotSys::Init");
    Ability::Init(abilityInfo, application, handler, token);

    memberFuncMap_[FORM_EVENT_REQ_ACQUIRE_FORM_0100] = &FormAbilityNotSys::FMS_acquireForm_0100;
    memberFuncMap_[FORM_EVENT_REQ_DELETE_FORM_0100] = &FormAbilityNotSys::FMS_deleteForm_0100;
    memberFuncMap_[FORM_EVENT_REQ_RELEASE_FORM_0100] = &FormAbilityNotSys::FMS_releaseForm_0100;
}

void FormAbilityNotSys::handleEvent(std::string action)
{
    auto itFunc = memberFuncMap_.find(action);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)();
        }
    }
}

void FormAbilityNotSys::Clear()
{
}

void FormEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("FormEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("FormEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    ability_->handleEvent(eventName);
}

void FormEventSubscriber::KitTerminateAbility()
{
    if (ability_ != nullptr) {
        ability_->TerminateAbility();
    }
}

REGISTER_AA(FormAbilityNotSys)
}  // namespace AppExecFwk
}  // namespace OHOS