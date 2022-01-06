/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "page_ability_impl.h"
#include "app_log_wrapper.h"

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
void PageAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    APP_LOGI("PageAbilityImpl::HandleAbilityTransaction begin sourceState:%{public}d; targetState: %{public}d; "
             "isNewWant: %{public}d",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant);
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        APP_LOGE("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }

    SetLifeCycleStateInfo(targetState);

    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
        ability_->SetStartAbilitySetting(targetState.setting);
        Start(want);
        CheckAndRestore();
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) {
        Inactive();
    }

    if (targetState.state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        CheckAndSave();
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        APP_LOGI("AbilityThread::HandleAbilityTransaction before AbilityManagerClient->AbilityTransitionDone");
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        APP_LOGI("AbilityThread::HandleAbilityTransaction after AbilityManagerClient->AbilityTransitionDone");
    }
    APP_LOGI("PageAbilityImpl::HandleAbilityTransaction end");
}

/**
 * @brief Handling the life cycle switching of PageAbility in switch.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 * @return return true if the lifecycle transaction successfully, otherwise return false.
 *
 */
bool PageAbilityImpl::AbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    APP_LOGI("PageAbilityImpl::AbilityTransaction begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) {
                Background();
            }
            Stop();
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
                Foreground(want);
            }
            if (targetState.isNewWant) {
                NewWant(want);
            }
            SerUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                         targetState.caller.abilityName);
            Active();
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_INACTIVE) {
                Inactive();
            }
            Background();
            break;
        }
        default: {
            ret = false;
            APP_LOGE("PageAbilityImpl::HandleAbilityTransaction state error");
            break;
        }
    }
    APP_LOGI("PageAbilityImpl::AbilityTransaction end: retVal = %{public}d", (int)ret);
    return ret;
}

/**
 * @brief Execution the KeyDown callback of the ability
 * @param keyEvent Indicates the key-down event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void PageAbilityImpl::DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    APP_LOGI("PageAbilityImpl::DoKeyDown begin");
    if (ability_ == nullptr) {
        APP_LOGE("PageAbilityImpl::DoKeyDown ability_ == nullptr");
        return;
    }
    auto abilitInfo = ability_->GetAbilityInfo();
    APP_LOGI("PageAbilityImpl::DoKeyDown called %{public}s And Focus is %{public}s",
        abilitInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyDown(keyEvent);
    APP_LOGI("PageAbilityImpl::DoKeyDown end");
}

/**
 * @brief Execution the KeyUp callback of the ability
 * @param keyEvent Indicates the key-up event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event is
 * not handled and should be passed to other handlers.
 *
 */
void PageAbilityImpl::DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    APP_LOGI("PageAbilityImpl::DoKeyUp begin");
    if (ability_ == nullptr) {
        APP_LOGE("PageAbilityImpl::DoKeyUp ability_ == nullptr");
        return;
    }
    auto abilitInfo = ability_->GetAbilityInfo();
    APP_LOGI("PageAbilityImpl::DoKeyUp called %{public}s And Focus is %{public}s",
        abilitInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyUp(keyEvent);
    APP_LOGI("PageAbilityImpl::DoKeyUp end");
}

/**
 * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
 * does nothing and returns false.
 * @param touchEvent Indicates information about the touch event.
 *
 * @return Returns true if the event is handled; returns false otherwise.
 *
 */
void PageAbilityImpl::DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    APP_LOGI("PageAbilityImpl::DoPointerEvent begin");
    if (ability_ == nullptr) {
        APP_LOGE("PageAbilityImpl::DoPointerEvent ability_ == nullptr");
        return;
    }
    auto abilitInfo = ability_->GetAbilityInfo();
    APP_LOGI("PageAbilityImpl::DoPointerEvent called %{public}s And Focus is %{public}s",
        abilitInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnPointerEvent(pointerEvent);
    APP_LOGI("PageAbilityImpl::DoPointerEvent end");
}
}  // namespace AppExecFwk
}  // namespace OHOS