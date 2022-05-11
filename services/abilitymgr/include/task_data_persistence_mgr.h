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

#ifndef FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_TASK_DATA_PERSISTENCE_MGR_H
#define FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_TASK_DATA_PERSISTENCE_MGR_H

#include <memory>
#include <unordered_map>

#include "singleton.h"
#include "ability_event_handler.h"
#include "mission_data_storage.h"

namespace OHOS {
namespace AAFwk {
const std::string THREAD_NAME = "TaskDataStorage";
const std::string SAVE_MISSION_INFO = "SaveMissionInfo";
const std::string DELETE_MISSION_INFO = "DeleteMissionInfo";
const std::string SAVE_MISSION_SNAPSHOT = "SaveMissionSnapshot";
const std::string GET_MISSION_SNAPSHOT = "GetMissionSnapshot";

class TaskDataPersistenceMgr : public std::enable_shared_from_this<TaskDataPersistenceMgr> {
    DECLARE_DELAYED_SINGLETON(TaskDataPersistenceMgr)
public:
    /**
     * @brief initialization of task data persistence manager.
     * @param user id Indicates the missionInfo object of user to operate.
     * @return Returns true if init successfully, returns false otherwise.
     */
    bool Init(int userId);

    /**
     * @brief Boot query persistent storage.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList);

    /**
     * @brief Save the mission data.
     * @param missionInfo Indicates the missionInfo object to be save.
     * @return Returns true if the data is successfully saved; returns false otherwise.
     */
    bool SaveMissionInfo(const InnerMissionInfo &missionInfo);

    /**
     * @brief Delete the mission data corresponding to the mission Id.
     * @param missionId Indicates this mission id.
     * @return Returns true if the data is successfully deleted; returns false otherwise.
     */
    bool DeleteMissionInfo(int missionId);

    /**
     * @brief Remove user directory.
     * @param userId Indicates this user id.
     * @return Returns true if the directory is successfully removed; returns false otherwise.
     */
    bool RemoveUserDir(int32_t userId);

    /**
     * @brief save mission snapshot
     * @param missionId id of mission
     * @param snapshot result of snapshot
     * @return return true if update mission snapshot success, else false
     */
    bool SaveMissionSnapshot(int missionId, const MissionSnapshot& snapshot);

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Get the Snapshot object
     * @param missionId Indicates this mission id.
     * @return Returns PixelMap of snapshot.
     */
    sptr<Media::PixelMap> GetSnapshot(int missionId) const;
#endif

    /**
     * @brief Get the mission snapshot object
     * @param missionId id of mission
     * @param missionSnapshot
     * @return return true if update mission snapshot success, else false
     */
    bool GetMissionSnapshot(int missionId, MissionSnapshot& missionSnapshot);

private:
    std::unordered_map<int, std::shared_ptr<MissionDataStorage>> missionDataStorageMgr_;
    std::shared_ptr<MissionDataStorage> currentMissionDataStorage_;
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    int32_t currentUserId_ = -1;
    std::mutex mutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_TASK_DATA_PERSISTENCE_MGR_H