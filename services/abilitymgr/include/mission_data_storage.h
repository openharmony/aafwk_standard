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

#ifndef FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_MISSION_DATA_STORAGE_H
#define FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_MISSION_DATA_STORAGE_H

#include <list>
#include <mutex>
#include <queue>

#include "event_handler.h"
#include "inner_mission_info.h"
#include "mission_snapshot.h"

namespace OHOS {
namespace AAFwk {
const std::string TASK_DATA_FILE_BASE_PATH = "/data/service/el1/public/AbilityManagerService";
const std::string MISSION_DATA_FILE_PATH = "MissionInfo";
const std::string MISSION_JSON_FILE_PREFIX = "mission";
const std::string JSON_FILE_SUFFIX = ".json";
const std::string PNG_FILE_SUFFIX = ".png";

class MissionDataStorage : public std::enable_shared_from_this<MissionDataStorage> {
public:
    MissionDataStorage() = default;
    MissionDataStorage(int userId);
    virtual ~MissionDataStorage();

    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);

    /**
     * @brief GeT all mission info.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList);

    /**
     * @brief Save the mission data.
     * @param missionInfo Indicates the missionInfo object to be save.
     */
    void SaveMissionInfo(const InnerMissionInfo &missionInfo);

    /**
     * @brief Delete the bundle data corresponding to the mission Id.
     * @param missionId Indicates this mission id.
     */
    void DeleteMissionInfo(int missionId);

    /**
     * @brief Save mission snapshot
     * @param missionId Indicates this mission id.
     * @param missionSnapshot the mission snapshot to save
     */
    void SaveMissionSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot);

    /**
     * @brief Delete mission snapshot
     * @param missionId Indicates this mission id.
     */
    void DeleteMissionSnapshot(int32_t missionId);

    /**
     * @brief Get the Mission Snapshot object
     * @param missionId
     * @param missionSnapshot
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool GetMissionSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot);

#ifdef SUPPORT_GRAPHICS
public:
    /**
     * @brief Get the Snapshot object
     * @param missionId Indicates this mission id.
     * @return Returns PixelMap of snapshot.
     */
    sptr<Media::PixelMap> GetSnapshot(int missionId) const;
    
    std::unique_ptr<Media::PixelMap> GetPixelMap(int missionId) const;

private:
    std::map<int32_t, std::shared_ptr<Media::PixelMap>> cachedPixelMap_;
#endif

private:
    std::string GetMissionDataDirPath() const;

    std::string GetMissionDataFilePath(int missionId);

    std::string GetMissionSnapshotPath(int32_t missionId) const;

    bool CheckFileNameValid(const std::string &fileName);

    bool WriteToPng(const char* fileName, uint32_t width, uint32_t height, const uint8_t* data);

    bool GetCachedSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot);

    bool SaveCachedSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot);

    bool DeleteCachedSnapshot(int32_t missionId);

    void SaveSnapshotFile(int32_t missionId, const MissionSnapshot& missionSnapshot);

    int userId_ = 0;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex cachedPixelMapMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_SERVICES_ABILITYMGR_INCLUDE_MISSION_DATA_STORAGE_H
