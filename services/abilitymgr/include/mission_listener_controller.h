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

#ifndef OHOS_AAFWK_MISSION_LISTENER_CONTROLLER_H
#define OHOS_AAFWK_MISSION_LISTENER_CONTROLLER_H

#include <mutex>
#include <vector>

#include "event_handler.h"
#include "mission_listener_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class MissionListenerController
 * MissionListenerController manage mission listeners.
 */
class MissionListenerController : public std::enable_shared_from_this<MissionListenerController> {
public:
    MissionListenerController();
    ~MissionListenerController();

    /**
     * init mission listener controller.
     *
     */
    void Init();

    /**
     * add mission listener callback.
     *
     * @param listener the listener callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddMissionListener(const sptr<IMissionListener> &listener);

    /**
     * delete a mission listener callback.
     *
     * @param listener the listener callback.
     */
    void DelMissionListener(const sptr<IMissionListener> &listener);

    /**
     * notify listeners that mission was created.
     *
     * @param missionId target mission id.
     */
    void NotifyMissionCreated(int32_t missionId);

    /**
     * notify listeners that mission was created.
     *
     * @param missionId target mission id.
     */
    void NotifyMissionDestroyed(int32_t missionId);

    /**
     * notify listeners that mission was created.
     *
     * @param missionId target mission id.
     */
    void NotifyMissionSnapshotChanged(int32_t missionId);

    /**
     * notify listeners that mission was created.
     *
     * @param missionId target mission id.
     */
    void NotifyMissionMovedToFront(int32_t missionId);

    void HandleUnInstallApp(const std::list<int32_t> &missions);

private:
    void NotifyListeners(int32_t missionId, IMissionListener::MissionListenerCmd cmd);
    void OnListenerDied(const wptr<IRemoteObject> &remote);

    class ListenerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using ListenerDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
        explicit ListenerDeathRecipient(ListenerDiedHandler handler);
        ~ListenerDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    private:
        ListenerDiedHandler diedHandler_;
    };

private:
    std::recursive_mutex listenerLock_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::vector<sptr<IMissionListener>> missionListeners_;
    sptr<IRemoteObject::DeathRecipient> listenerDeathRecipient_;

    using ListenerFun = void (IMissionListener::*)(int);
    std::unordered_map<uint32_t, ListenerFun> listenerFunMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MISSION_LISTENER_CONTROLLER_H
