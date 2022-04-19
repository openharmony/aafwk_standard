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

#include "service_ability_impl.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
/**
 * @brief Handling the life cycle switching of PageAbility.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 */
void ServiceAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    HILOG_INFO("ServiceAbilityImpl::HandleAbilityTransaction begin sourceState:%{public}d; targetState: %{public}d; "
             "isNewWant: %{public}d",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant);
    if (lifecycleState_ == targetState.state) {
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }

    bool ret = true;

    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) {
#ifdef SUPPORT_GRAPHICS
                Background();
#endif
                Stop();
            }
            break;
        }
        case AAFwk::ABILITY_STATE_INACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
                SerUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                             targetState.caller.abilityName);
                Start(want);
            }
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("ServiceAbilityImpl::HandleAbilityTransaction state is error");
            break;
        }
    }

    if (ret) {
        HILOG_INFO("Handle service transaction done, notify ability manager service.");
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
