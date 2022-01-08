/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_REMOTE_MISSION_LISTENER_PROXY_H
#define OHOS_AAFWK_REMOTE_MISSION_LISTENER_PROXY_H

#include "iremote_proxy.h"
#include "remote_mission_listener_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for remote mission listener proxy.
 */
class RemoteMissionListenerProxy : public IRemoteProxy<IRemoteMissionListener> {
public:
    explicit RemoteMissionListenerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IRemoteMissionListener>(impl)
    {}

    /**
     * @brief When the remote device mission changed, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     */
    virtual void NotifyMissionsChanged(const std::string& deviceId) override;

    /**
     * @brief When the snapshot of a mission changed, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     * @param missionId, mission Id.
     */
    virtual void NotifySnapshot(const std::string& deviceId, int32_t missionId) override;

    /**
     * @brief When the remote device disconnect, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     * @param state, device state.
     */
    virtual void NotifyNetDisconnect(const std::string& deviceId, int32_t state) override;

private:
    static inline BrokerDelegator<RemoteMissionListenerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_REMOTE_MISSION_LISTENER_PROXY_H
