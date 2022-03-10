
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

#include "form_refresh_connection.h"

#include <cinttypes>

#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_supply_callback.h"
#include "form_task_mgr.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
FormRefreshConnection::FormRefreshConnection(const int64_t formId, const Want& want,
    const std::string &bundleName, const std::string &abilityName)
    :formId_(formId),
    want_(want)
{
    SetProviderKey(bundleName, abilityName);
}
/**
 * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
 *
 * @param element Service ability's ElementName.
 * @param remoteObject The session proxy of service ability.
 * @param resultCode ERR_OK on success, others on failure.
 * @return none.
 */
void FormRefreshConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (resultCode != ERR_OK) {
        HILOG_ERROR("%{public}s, abilityName:%{public}s, formId:%{public}" PRId64 ", resultCode:%{public}d",
            __func__, element.GetAbilityName().c_str(), formId_, resultCode);
        return;
    }
    FormSupplyCallback::GetInstance()->AddConnection(this);

    if (want_.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        std::string message = want_.GetStringParam(Constants::PARAM_MESSAGE_KEY);
        Want msgWant = Want(want_);
        msgWant.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostFormEventTask(formId_, message, msgWant, remoteObject);
    } else if (want_.HasParameter(Constants::RECREATE_FORM_KEY)) {
        Want cloneWant = Want(want_);
        cloneWant.RemoveParam(Constants::RECREATE_FORM_KEY);
        cloneWant.SetParam(Constants::ACQUIRE_TYPE, Constants::ACQUIRE_TYPE_RECREATE_FORM);
        cloneWant.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostAcquireTask(formId_, cloneWant, remoteObject);
    } else {
        Want want;
        want.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
        FormTaskMgr::GetInstance().PostRefreshTask(formId_, want, remoteObject);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS