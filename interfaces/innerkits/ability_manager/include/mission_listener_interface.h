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

#ifndef OHOS_AAFWK_MISSION_LISTENER_INTERFACE_H
#define OHOS_AAFWK_MISSION_LISTENER_INTERFACE_H

#include "iremote_broker.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map.h"
#endif

namespace OHOS {
namespace AAFwk {
/**
 * @class IMissionListener
 * IMissionListener is used to notify caller ability that connect or disconnect is complete.
 */
class IMissionListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.MissionListener");

    /**
     * @brief When a mission is created, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionCreated(int32_t missionId) = 0;

    /**
     * @brief When a mission is destroyed, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionDestroyed(int32_t missionId) = 0;

    /**
     * @brief When the snapshot of a mission changes, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionSnapshotChanged(int32_t missionId) = 0;

    /**
     * @brief When a mission is moved to front, AbilityMs notifies the listener of the mission id
     *
     * @param missionId, mission Id.
     */
    virtual void OnMissionMovedToFront(int32_t missionId) = 0;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called when a mission has changed it's icon.
     *
     * @param missionId, mission Id.
     * @param icon, mission icon.
     */
    virtual void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;
#endif

    enum MissionListenerCmd {
        // ipc id for OnMissionCreated
        ON_MISSION_CREATED = 0,

        // ipc id for OnMissionDestroyed
        ON_MISSION_DESTROYED,

        // ipc id for OnMissionSnapshotChanged
        ON_MISSION_SNAPSHOT_CHANGED,

        // ipc id for OnMissionMovedToFront
        ON_MISSION_MOVED_TO_FRONT,

        // ipc id for OnMissionIconUpdated
        ON_MISSION_ICON_UPDATED,

        // maximum of enum
        MISSION_LINSTENER_CMD_MAX
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MISSION_LISTENER_INTERFACE_H
