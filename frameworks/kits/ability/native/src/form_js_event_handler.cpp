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

#include "form_constants.h"
#include "form_host_client.h"
#include "form_js_event_handler.h"
#include "form_mgr.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AppExecFwk {
FormJsEventHandler::FormJsEventHandler(
    const std::shared_ptr<EventRunner> &runner, const std::shared_ptr<Ability> &ability, const FormJsInfo &formJsInfo)
    : EventHandler(runner), formJsInfo_(formJsInfo), ability_(ability)
{
    HILOG_INFO("%{public}s called.", __func__);
}

/**
 * Process the event. Developers should override this method.
 *
 * @param event The event should be processed.
 */
void FormJsEventHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!event) {
        HILOG_ERROR("%{public}s, param illegal, event is nullptr", __func__);
        return;
    }
    HILOG_DEBUG("%{public}s, inner event id obtained: %u.", __func__, event->GetInnerEventId());

    switch (event->GetInnerEventId()) {
        case FORM_ROUTE_EVENT: {
            auto object = event->GetUniqueObject<Want>();
            Want want = *object;
            ProcessRouterEvent(want);
            break;
        }
        case FORM_MESSAGE_EVENT: {
            auto object = event->GetUniqueObject<Want>();
            Want want = *object;
            ProcessMessageEvent(want);
            break;
        }
        default: {
            HILOG_WARN("unsupported event.");
            break;
        }
    }
}

/**
 * @brief Process js router event.
 * @param want Indicates the event to be processed.
 */
void FormJsEventHandler::ProcessRouterEvent(Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsSystemApp()) {
        HILOG_WARN("%{public}s, not system application, cannot mixture package router", __func__);
        return;
    }
    if (!want.HasParameter(Constants::PARAM_FORM_ABILITY_NAME_KEY)) {
        HILOG_ERROR("%{public}s, param illegal, abilityName is not exist", __func__);
        return;
    }

    std::string abilityName = want.GetStringParam(Constants::PARAM_FORM_ABILITY_NAME_KEY);
    want.SetElementName(formJsInfo_.bundleName, abilityName);
    want.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formJsInfo_.formId));
    ability_->StartAbility(want);
}
/**
 * @brief Process js message event.
 * @param want Indicates the event to be processed.
 */
void FormJsEventHandler::ProcessMessageEvent(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!want.HasParameter(Constants::PARAM_FORM_IDENTITY_KEY)) {
        HILOG_ERROR("%{public}s, formid is not exist", __func__);
        return;
    }
    std::string strFormId = want.GetStringParam(Constants::PARAM_FORM_IDENTITY_KEY);
    int64_t formId = std::stoll(strFormId);
    if (formId <= 0) {
        HILOG_ERROR("%{public}s error, The passed formid is invalid. Its value must be larger than 0.", __func__);
        return;
    }

    if (!want.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        HILOG_ERROR("%{public}s, message info is not exist", __func__);
        return;
    }

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        HILOG_ERROR("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return;
    }

    // requestForm request to fms
    int resultCode = FormMgr::GetInstance().MessageEvent(formId, want, FormHostClient::GetInstance());
    if (resultCode != ERR_OK) {
        HILOG_ERROR(
            "%{public}s error, failed to notify the form service that the form user's lifecycle is updated, error "
            "code is %{public}d.",
            __func__,
            resultCode);
        return;
    }
}

bool FormJsEventHandler::IsSystemApp() const
{
    HILOG_INFO("%{public}s called.", __func__);

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid > Constants::MAX_SYSTEM_APP_UID) {
        HILOG_WARN("%{public}s warn, callingUid is %{public}d, which is larger than %{public}d.",
            __func__,
            callingUid,
            Constants::MAX_SYSTEM_APP_UID);
        return false;
    } else {
        HILOG_DEBUG("%{public}s, callingUid = %{public}d.", __func__, callingUid);
        return true;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
