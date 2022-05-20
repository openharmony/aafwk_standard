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

#include "page_ability_impl.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Handle ability transaction start, sourceState:%{public}d, targetState:%{public}d, "
             "isNewWant:%{public}d, sceneFlag:%{public}d.",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant,
        targetState.sceneFlag);
    if (ability_ == nullptr) {
        HILOG_ERROR("Handle ability transaction error, ability_ is null.");
        return;
    }
    ability_->sceneFlag_ = targetState.sceneFlag;
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequsetFocus(want);
            AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        }
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND || lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        if (targetState.state == AAFwk::ABILITY_STATE_ACTIVE || targetState.state == AAFwk::ABILITY_STATE_INACTIVE) {
            HILOG_ERROR("Invalid state.");
            return;
        }
    }

    SetLifeCycleStateInfo(targetState);

    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
        ability_->SetStartAbilitySetting(targetState.setting);
        Start(want);
        CheckAndRestore();
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE &&
        targetState.state != AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        Inactive();
    }

    if (targetState.state == AAFwk::ABILITY_STATE_BACKGROUND_NEW ||
        targetState.state == AAFwk::ABILITY_STATE_BACKGROUND) {
        CheckAndSave();
    }

    if (AbilityTransaction(want, targetState)) {
        HILOG_INFO("Handle ability transaction done, notify ability manager service.");
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
    }
    HILOG_INFO("PageAbilityImpl::HandleAbilityTransaction end");
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
    HILOG_INFO("PageAbilityImpl::AbilityTransaction begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) {
                Background();
            }
            Stop();
            break;
        }
        case AAFwk::ABILITY_STATE_INACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) {
                Inactive();
            }
            ret = false;
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (targetState.isNewWant) {
                NewWant(want);
            }
            SerUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                         targetState.caller.abilityName);

            if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND_NEW ||
                lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND) {
                Foreground(want);
            } else {
                if (ability_) {
                    ability_->RequsetFocus(want);
                }
            }
            break;
        }
        case AAFwk::ABILITY_STATE_ACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_BACKGROUND) {
                Foreground(want);
            }
            Active();
            ret = false;
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
            HILOG_ERROR("PageAbilityImpl::HandleAbilityTransaction state error");
            break;
        }
    }
    HILOG_INFO("PageAbilityImpl::AbilityTransaction end: retVal = %{public}d", (int)ret);
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
    HILOG_INFO("PageAbilityImpl::DoKeyDown begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("PageAbilityImpl::DoKeyDown ability_ == nullptr");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    HILOG_INFO("PageAbilityImpl::DoKeyDown called %{public}s And Focus is %{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyDown(keyEvent);
    HILOG_INFO("PageAbilityImpl::DoKeyDown end");
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
    HILOG_INFO("PageAbilityImpl::DoKeyUp begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("PageAbilityImpl::DoKeyUp ability_ == nullptr");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    HILOG_INFO("PageAbilityImpl::DoKeyUp called %{public}s And Focus is %{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnKeyUp(keyEvent);
    HILOG_INFO("PageAbilityImpl::DoKeyUp end");
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
    HILOG_INFO("PageAbilityImpl::DoPointerEvent begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("PageAbilityImpl::DoPointerEvent ability_ == nullptr");
        return;
    }
    auto abilityInfo = ability_->GetAbilityInfo();
    HILOG_INFO("PageAbilityImpl::DoPointerEvent called %{public}s And Focus is %{public}s",
        abilityInfo->name.c_str(),
        ability_->HasWindowFocus() ? "true" : "false");

    ability_->OnPointerEvent(pointerEvent);
    HILOG_INFO("PageAbilityImpl::DoPointerEvent end");
}
}  // namespace AppExecFwk
}  // namespace OHOS