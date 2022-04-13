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

#ifndef FOUNDATION_APPEXECFWK_OHOS_MISSION_LISTENER_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_MISSION_LISTENER_PROXY_H

#include <string>
#include "iremote_proxy.h"
#include "mission_listener_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for mission listener.
 */
class MissionListenerProxy : public IRemoteProxy<IMissionListener> {
public:
    explicit MissionListenerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IMissionListener>(impl)
    {}

    /**
     * @brief Sends OnMissionCreated request.
     *
     * @param missionId Indicators id of mission.
     */
    virtual void OnMissionCreated(int32_t missionId) override;

    /**
     * @brief Sends OnMissionDestroyed request.
     *
     * @param missionId Indicators id of mission.
     */
    virtual void OnMissionDestroyed(int32_t missionId) override;

    /**
     * @brief Sends OnMissionSnapshotChanged request.
     *
     * @param missionId Indicators id of mission.
     */
    virtual void OnMissionSnapshotChanged(int32_t missionId) override;

    /**
     * @brief Sends OnMissionMovedToFront request.
     *
     * @param missionId Indicators id of mission.
     */
    virtual void OnMissionMovedToFront(int32_t missionId) override;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called when a mission has changed it's icon.
     *
     * @param missionId, mission Id.
     * @param icon, mission icon.
     */
    virtual void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;
#endif

private:
    void SendRequestCommon(int32_t missionId, IMissionListener::MissionListenerCmd cmd);

private:
    static inline BrokerDelegator<MissionListenerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_MISSION_LISTENER_PROXY_H
