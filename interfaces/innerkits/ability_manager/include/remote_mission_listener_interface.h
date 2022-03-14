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

#ifndef OHOS_AAFWK_REMOTE_MISSION_LISTENER_INTERFACE_H
#define OHOS_AAFWK_REMOTE_MISSION_LISTENER_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IRemoteMissionListener
 * IRemoteMissionListener is used to notify caller that remote device mission is changed.
 */
class IRemoteMissionListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.RemoteMissionListener");

    /**
     * @brief When the remote device mission changed, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     */
    virtual void NotifyMissionsChanged(const std::string& deviceId) = 0;

    /**
     * @brief When the snapshot of a mission changed, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     * @param missionId, mission Id.
     */
    virtual void NotifySnapshot(const std::string& deviceId, int32_t missionId) = 0;

    /**
     * @brief When the remote device disconnect, AbilityMs notify the listener.
     *
     * @param deviceId, remote device Id.
     * @param state, device state.
     */
    virtual void NotifyNetDisconnect(const std::string& deviceId, int32_t state) = 0;

    enum {
        // ipc id for NotifyMissionsChanged
        NOTIFY_MISSION_CHANGED = 0,

        // ipc id for NotifySnapshot
        NOTIFY_SNAPSHOT = 1,

        // ipc id for NotifyNetDisconnect
        NOTIFY_NET_DISCONNECT = 2
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_REMOTE_MISSION_LISTENER_INTERFACE_H
