
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

#include "form_acquire_state_connection.h"

#include "appexecfwk_errors.h"
#include "form_constants.h"
#include "form_supply_callback.h"
#include "form_task_mgr.h"
#include "form_util.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
FormAcquireStateConnection::FormAcquireStateConnection(const std::string &bundleName, const std::string &abilityName,
                                                       const Want &want, std::string &provider)
    : want_(want), provider_(provider)
{
    SetProviderKey(bundleName, abilityName);
}

/**
 * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
 * @param element service ability's ElementName.
 * @param remoteObject the session proxy of service ability.
 * @param resultCode ERR_OK on success, others on failure.
 */
void FormAcquireStateConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (resultCode != ERR_OK) {
        HILOG_ERROR("%{public}s, abilityName:%{public}s, resultCode:%{public}d",
            __func__, element.GetAbilityName().c_str(), resultCode);
        return;
    }
    FormSupplyCallback::GetInstance()->AddConnection(this);
    Want want;
    want.SetParam(Constants::FORM_CONNECT_ID, this->GetConnectId());
    FormTaskMgr::GetInstance().PostAcquireStateTask(want_, provider_, want, remoteObject);
}
}  // namespace AppExecFwk
}  // namespace OHOS