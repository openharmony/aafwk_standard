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

#ifndef OHOS_AAFWK_MISSION_INFO_MGR_H
#define OHOS_AAFWK_MISSION_INFO_MGR_H

#include <list>
#include <mutex>
#include <string>

#include "inner_mission_info.h"
#include "mission_listener_controller.h"
#include "mission_snapshot.h"
#include "snapshot.h"
#include "task_data_persistence_mgr.h"

namespace OHOS {
namespace AAFwk {
const int MIN_MISSION_ID = 1;
const int MAX_MISSION_ID = INT_MAX;

class MissionInfoMgr : public std::enable_shared_from_this<MissionInfoMgr> {
    DECLARE_DELAYED_SINGLETON(MissionInfoMgr)
public:
    /**
     * @brief generate mission id of mission info object.
     * @param misisonId Indicates the missionInfo object of user to operate.
     * @return Returns true if the missionId is successfully generated; returns false otherwise.
     */
    bool GenerateMissionId(int32_t &misisonId);

    /**
     * @brief initialization of mission info manager.
     * @param userId Indicates the missionInfo object of user to operate.
     * @return Returns true if init successfully, returns false otherwise.
     */
    bool Init(int userId);

    /**
     * @brief Add the mission info.
     * @param missionInfo Indicates the missionInfo object to be Added.
     * @return Returns true if the data is successfully saved; returns false otherwise.
     */
    bool AddMissionInfo(const InnerMissionInfo &missionInfo);

    /**
     * @brief Update the mission info.
     * @param missionInfo Indicates the missionInfo object to be updated.
     * @return Returns true if the data is successfully saved; returns false otherwise.
     */
    bool UpdateMissionInfo(const InnerMissionInfo &missionInfo);

    /**
     * @brief Delete the mission info corresponding to the mission Id.
     * @param missionId Indicates this mission id.
     * @return Returns true if the data is successfully deleted; returns false otherwise.
     */
    bool DeleteMissionInfo(int missionId);

    /**
     * @brief Get all mission infos,sorted by time stamp.
     * @param numMax max num of missions.
     * @return ERR_OK if get success.
     */
    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos);

    /**
     * @brief Get mission info by mission id.
     * @param missionId indicates this mission id.
     * @param missionInfo indicates the missionInfo object related to the missionId.
     * @return ERR_OK if get success; return else otherwise.
     */
    int GetMissionInfoById(int32_t missionId, MissionInfo &missionInfo);

    /**
     * @brief Get inner mission info by mission id.
     * @param missionId indicates this mission id.
     * @param innerMissionInfo indicates the inner missionInfo object related to the missionId.
     * @return ERR_OK if get success; return else otherwise.
     */
    int GetInnerMissionInfoById(int32_t missionId, InnerMissionInfo &innerMissionInfo);

    /**
     * @brief Try find reused mission info.
     *
     * @param missionName name of mission.
     * @param info found mission info.
     * @return true if success.
     */
    bool FindReusedSingletonMission(const std::string &missionName, InnerMissionInfo &info);

    /**
     * @brief Update mission timestamp.
     *
     * @param missionId indicates this mission id.
     * @param timestamp indicates this mission timestamp.
     */
    void UpdateMissionTimeStamp(int32_t missionId, const std::string& timestamp);

    /**
     * @brief Delete all the mission info.
     *
     * @param listenerController The mission listener controller.
     */
    bool DeleteAllMissionInfos(const std::shared_ptr<MissionListenerController> &listenerController);

    /**
     * @brief Update mission label.
     *
     * @param missionId indicates this mission id.
     * @param label indicates this mission label.
     * @return 0 if success.
     */
    int UpdateMissionLabel(int32_t missionId, const std::string& label);

    /**
     * @brief dump mission info
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string> &info);

    /**
     * @brief update mission snapshot
     * @param missionId mission id
     * @param abilityToken abilityToken to get current mission snapshot
     * @param missionSnapshot result of snapshot
     * @return return true if update mission snapshot success, else false
     */
    bool UpdateMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
        MissionSnapshot& missionSnapshot) const;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Get the Snapshot object
     * @param missionId Indicates this mission id.
     * @return Returns PixelMap of snapshot.
     */
    sptr<Media::PixelMap> GetSnapshot(int32_t missionId) const;
#endif

    /**
     * @brief get the mission snapshot object
     * @param missionId mission id
     * @param abilityToken abilityToken to get current mission snapshot
     * @param missionSnapshot result of snapshot
     * @param force force get snapshot from window manager service.
     * @return true return true if get mission snapshot success, else false
     */
    bool GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
        MissionSnapshot& missionSnapshot, bool force = false) const;

    /**
     * @brief register snapshotHandler
     * @param handler the snapshotHandler
     */
    void RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler);

    void HandleUnInstallApp(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions);
private:
    /**
     * @brief Boot query mission info.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    bool LoadAllMissionInfo();

    void GetMatchedMission(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions);

private:
    int32_t currentMisionId_ = MIN_MISSION_ID;
    std::unordered_map<int32_t, bool> missionIdMap_; // key:distributed misisonid, vaule: has been saved
    std::list<InnerMissionInfo> missionInfoList_;
    std::shared_ptr<TaskDataPersistenceMgr> taskDataPersistenceMgr_;
    sptr<ISnapshotHandler> snapshotHandler_;
    mutable std::recursive_mutex mutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MISSION_INFO_MGR_H