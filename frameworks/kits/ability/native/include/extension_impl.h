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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_IMPL_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_IMPL_H

#include "extension.h"
#include "lifecycle_state_info.h"

namespace OHOS {
class IRemoteObject;
namespace AAFwk {
class Want;
}
namespace AppExecFwk {
struct AbilityInfo;
class OHOSApplication;
class AbilityHandler;
class AbilityLocalRecord;
}
namespace AbilityRuntime {
/**
 * @brief Responsible for managing and scheduling the life cycle of extension.
 */
class ExtensionImpl : public std::enable_shared_from_this<ExtensionImpl> {
public:
    ExtensionImpl() = default;
    virtual ~ExtensionImpl() = default;

    /**
     * @brief Init the object.
     *
     * @param application the application info.
     * @param record the extension record.
     * @param extension the extension object.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    void Init(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        std::shared_ptr<Extension> &extension,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token);

    /**
     * @brief Connect the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to connect to.
     * @param targetState The terget state.
     *
     */
    virtual void HandleExtensionTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState);

    /**
     * @brief scheduling update configuration of extension.
     *
     * @param config Configuration
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * @brief Connect the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to connect to.
     *
     */
    sptr<IRemoteObject> ConnectExtension(const Want &want);

    /**
     * @brief Disconnects the connected object.
     *
     * @param want The Want object to disconnect to.
     */
    void DisconnectExtension(const Want &want);

    /**
     * @brief Command the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service Extension has been started. The startId is incremented
     * by 1 every time the Extension is started. For example, if the Extension has been started for six times, the value
     * of startId is 6.
     */
    void CommandExtension(const Want &want, bool restart, int startId);
protected:
    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     * @param want  The Want object to switch the life cycle.
     */
    void Start(const Want &want);

    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     */
    void Stop();

    int lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<IRemoteObject> token_;
    std::shared_ptr<Extension> extension_;
};
}
}
#endif  // FOUNDATION_ABILITYRUNTIME_OHOS_EXTENSION_IMPL_H