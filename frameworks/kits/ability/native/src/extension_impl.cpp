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

#include "extension_impl.h"

#include "ability_manager_client.h"
#include "ability_local_record.h"
#include "hitrace_meter.h"
#include "extension_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void ExtensionImpl::Init(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    std::shared_ptr<Extension> &extension,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("ExtensionImpl Init begin.");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        extension == nullptr) {
        HILOG_ERROR("ExtensionImpl::init failed, some object is nullptr");
        return;
    }

    token_ = record->GetToken();
    extension_ = extension;
    extension_->Init(record, application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    HILOG_INFO("ExtensionImpl Init end.");
}

/**
 * @brief Handling the life cycle switching of Extension.
 *
 * @param want Indicates the structure containing information about the extension.
 * @param targetState The life cycle state to switch to.
 *
 */
void ExtensionImpl::HandleExtensionTransaction(const Want &want,
    const AAFwk::LifeCycleStateInfo &targetState)
{
    HILOG_INFO("ExtensionImpl::HandleExtensionTransaction begin sourceState:%{public}d; targetState: %{public}d; "
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
                Stop();
            }
            break;
        }
        case AAFwk::ABILITY_STATE_INACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
                Start(want);
            }
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("ExtensionImpl::HandleAbilityTransaction state is error");
            break;
        }
    }

    if (ret) {
        HILOG_INFO("ExtensionImpl::HandleAbilityTransaction before AbilityManagerClient->AbilityTransitionDone");
        AAFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, restoreData);
        HILOG_INFO("ExtensionImpl::HandleAbilityTransaction after AbilityManagerClient->AbilityTransitionDone");
    }
    HILOG_INFO("ExtensionImpl::HandleAbilityTransaction end");
}

void ExtensionImpl::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_INFO("%{public}s begin.", __func__);

    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::ScheduleUpdateConfiguration extension_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        extension_->OnConfigurationUpdated(config);
    }

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want  The Want object to switch the life cycle.
 */
void ExtensionImpl::Start(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Start extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl::Start");
    extension_->OnStart(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void ExtensionImpl::Stop()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Stop extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl::Stop");
    extension_->OnStop();
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Connect the extension. and Calling information back to Extension.
 *
 * @param want The Want object to connect to.
 *
 */
sptr<IRemoteObject> ExtensionImpl::ConnectExtension(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::ConnectAbility extension_ is nullptr");
        return nullptr;
    }

    HILOG_INFO("ExtensionImpl:: ConnectAbility");
    sptr<IRemoteObject> object = extension_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    HILOG_INFO("%{public}s end.", __func__);

    return object;
}

/**
 * @brief Disconnects the connected object.
 *
 * @param want The Want object to disconnect to.
 */
void ExtensionImpl::DisconnectExtension(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::DisconnectAbility extension_ is nullptr");
        return;
    }

    extension_->OnDisconnect(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Command the Extension. and Calling information back to Extension.
 *
 * @param want The Want object to command to.
 *
 * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 *
 * @param startId Indicates the number of times the Service Extension has been started. The startId is incremented by 1
 * every time the Extension is started. For example, if the Extension has been started for six times,
 * the value of startId is 6.
 */
void ExtensionImpl::CommandExtension(const Want &want, bool restart, int startId)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::CommandAbility extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl:: CommandAbility");
    extension_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    HILOG_INFO("%{public}s end.", __func__);
}
}
}
