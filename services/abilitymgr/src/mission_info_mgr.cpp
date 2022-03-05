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

#include "mission_info_mgr.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
MissionInfoMgr::MissionInfoMgr()
{
    HILOG_INFO("MissionInfoMgr instance is created");
}

MissionInfoMgr::~MissionInfoMgr()
{
    HILOG_INFO("MissionInfoMgr instance is destroyed");
}

bool MissionInfoMgr::GenerateMissionId(int32_t &misisonId)
{
    if (currentMisionId_ == MAX_MISSION_ID) {
        currentMisionId_ = MIN_MISSION_ID;
    }

    for (int32_t index = currentMisionId_; index < MAX_MISSION_ID; index++) {
        if (missionIdMap_.find(index) == missionIdMap_.end()) {
            misisonId = index;
            missionIdMap_[misisonId] = false;
            currentMisionId_ = missionId + 1;
            return true;
        }
    }

    HILOG_ERROR("cannot generate mission id");
    return false;
}

bool MissionInfoMgr::Init(int userId)
{
    if (!taskDataPersistenceMgr_) {
        taskDataPersistenceMgr_ = DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
        if (!taskDataPersistenceMgr_) {
            HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
            return false;
        }
    }

    if (!taskDataPersistenceMgr_->Init(userId)) {
        return false;
    }

    missionInfoList_.clear();
    missionIdMap_.clear();
    if (!LoadAllMissionInfo()) {
        return false;
    }

    return true;
}

bool MissionInfoMgr::AddMissionInfo(const InnerMissionInfo &missionInfo)
{
    auto id = missionInfo.missionInfo.id;
    if (missionIdMap_.find(id) != missionIdMap_.end() && missionIdMap_[id]) {
        HILOG_ERROR("add mission info failed, missionId %{public}d already exists", id);
        return false;
    }

    auto listIter = missionInfoList_.begin();
    for (; listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.time < missionInfo.missionInfo.time) {
            break;  // first listIter->time < missionInfo.time
        }
    }

    if (!taskDataPersistenceMgr_->SaveMissionInfo(missionInfo)) {
        HILOG_ERROR("save mission info failed");
        return false;
    }

    missionInfoList_.insert(listIter, missionInfo);
    missionIdMap_[id] = true;
    return true;
}

bool MissionInfoMgr::UpdateMissionInfo(const InnerMissionInfo &missionInfo)
{
    auto id = missionInfo.missionInfo.id;
    if (missionIdMap_.find(id) == missionIdMap_.end()
        || (missionIdMap_.find(id) != missionIdMap_.end() && !missionIdMap_[id])) {
        HILOG_ERROR("update mission info failed, missionId %{public}d not exists", id);
        return false;
    }

    auto listIter = missionInfoList_.begin();
    for (; listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.id == id) {
            break;
        }
    }

    if (listIter == missionInfoList_.end()) {
        HILOG_ERROR("update mission info failed, missionId %{public}d not exists", id);
        return false;
    }

    if (missionInfo.missionInfo.time == listIter->missionInfo.time) {
        // time not changes, no need sort again
        *listIter = missionInfo;
        return true;
    }

    missionInfoList_.erase(listIter);
    missionIdMap_.erase(id);
    return AddMissionInfo(missionInfo);
}

bool MissionInfoMgr::DeleteMissionInfo(int missionId)
{
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_WARN("missionId %{public}d not exists, no need delete", missionId);
        return true;
    }

    if (!missionIdMap_[missionId]) {
        HILOG_WARN("missionId %{public}d distriubted but not saved, no need delete", missionId);
        missionIdMap_.erase(missionId);
        return true;
    }

    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    if (!taskDataPersistenceMgr_->DeleteMissionInfo(missionId)) {
        HILOG_ERROR("delete mission info failed");
        return false;
    }

    for (auto listIter = missionInfoList_.begin(); listIter != missionInfoList_.end(); listIter++) {
        if (listIter->missionInfo.id == missionId) {
            missionInfoList_.erase(listIter);
            break;
        }
    }

    missionIdMap_.erase(missionId);
    return true;
}

bool MissionInfoMgr::DeleteAllMissionInfos(const std::shared_ptr<MissionListenerController> &listenerController)
{
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    for (auto listIter = missionInfoList_.begin(); listIter != missionInfoList_.end();) {
        if (!(listIter->missionInfo.lockedState)) {
            missionIdMap_.erase(listIter->missionInfo.id);
            taskDataPersistenceMgr_->DeleteMissionInfo(listIter->missionInfo.id);
            if (listenerController) {
                listenerController->NotifyMissionDestroyed(listIter->missionInfo.id);
            }
            missionInfoList_.erase(listIter++);
        } else {
            ++listIter;
        }
    }
    return true;
}

static bool DoesNotShowInTheMissionList(int32_t startMethod)
{
    switch (static_cast<StartMethod>(startMethod)) {
        case StartMethod::START_CALL:
            return true;
        default:
            return false;
    }
}

int MissionInfoMgr::GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("GetMissionInfos, numMax:%{public}d", numMax);
    if (numMax < 0) {
        return -1;
    }

    for (auto &mission : missionInfoList_) {
        if (static_cast<int>(missionInfos.size()) >= numMax) {
            break;
        }

        if (DoesNotShowInTheMissionList(mission.startMethod)) {
            HILOG_INFO("MissionId[%{public}d] don't show in mission list", mission.missionInfo.id);
            continue;
        }
        MissionInfo info = mission.missionInfo;
        missionInfos.emplace_back(info);
    }

    return 0;
}

int MissionInfoMgr::GetMissionInfoById(int32_t missionId, MissionInfo &missionInfo)
{
    HILOG_INFO("GetMissionInfoById, missionId:%{public}d", missionId);
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_ERROR("missionId %{public}d not exists, get mission info failed", missionId);
        return -1;
    }

    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionId](const InnerMissionInfo item) {
            return item.missionInfo.id == missionId;
        }
    );

    if (it == missionInfoList_.end()) {
        HILOG_ERROR("no such mission:%{public}d", missionId);
        return -1;
    }

    if (DoesNotShowInTheMissionList((*it).startMethod)) {
        HILOG_INFO("MissionId[%{public}d] don't show in mission list", (*it).missionInfo.id);
        return -1;
    }

    HILOG_INFO("GetMissionInfoById, find missionId missionId:%{public}d", missionId);
    missionInfo = (*it).missionInfo;
    return 0;
}

int MissionInfoMgr::GetInnerMissionInfoById(int32_t missionId, InnerMissionInfo &innerMissionInfo)
{
    if (missionIdMap_.find(missionId) == missionIdMap_.end()) {
        HILOG_ERROR("missionId %{public}d not exists, get inner mission info failed", missionId);
        return -1;
    }

    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionId](const InnerMissionInfo item) {
            return item.missionInfo.id == missionId;
        }
    );

    if (it == missionInfoList_.end()) {
        HILOG_ERROR("no such mission:%{public}d", missionId);
        return -1;
    }
    innerMissionInfo = *it;
    return 0;
}

bool MissionInfoMgr::FindReusedSingletonMission(const std::string &missionName, InnerMissionInfo &info)
{
    if (missionName.empty()) {
        return false;
    }

    auto it = std::find_if(missionInfoList_.begin(), missionInfoList_.end(),
        [&missionName](const InnerMissionInfo item) {
            return (missionName == item.missionName && item.isSingletonMode);
        }
    );

    if (it == missionInfoList_.end()) {
        HILOG_ERROR("can not find target singleton mission:%{public}s", missionName.c_str());
        return false;
    }
    info = *it;
    return true;
}

void MissionInfoMgr::UpdateMissionTimeStamp(int32_t missionId, const std::string& timestamp)
{
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("UpdateMissionTimeStamp failed, missionId %{public}d not exists", missionId);
        return;
    }

    if (timestamp == it->missionInfo.time) {
        return;
    }
    InnerMissionInfo updateInfo = *it;
    updateInfo.missionInfo.time = timestamp;

    missionInfoList_.erase(it);
    missionIdMap_.erase(missionId);
    (void)AddMissionInfo(updateInfo);
}

int MissionInfoMgr::UpdateMissionLabel(int32_t missionId, const std::string& label)
{
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("task data persist not init.");
        return -1;
    }
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("UpdateMissionLabel failed, missionId %{public}d not exists", missionId);
        return -1;
    }

    InnerMissionInfo updateInfo = *it;
    updateInfo.missionInfo.label = label;

    if (!taskDataPersistenceMgr_->SaveMissionInfo(updateInfo)) {
        HILOG_ERROR("save mission info failed.");
        return -1;
    }
    return 0;
}

bool MissionInfoMgr::LoadAllMissionInfo()
{
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("taskDataPersistenceMgr_ is nullptr");
        return false;
    }

    if (!taskDataPersistenceMgr_->LoadAllMissionInfo(missionInfoList_)) {
        HILOG_ERROR("load mission info failed");
        return false;
    }

    // sort by time
    auto cmpFunc = [] (const InnerMissionInfo &infoBase, const InnerMissionInfo &infoCmp) {
        return infoBase.missionInfo.time > infoCmp.missionInfo.time;
    };
    missionInfoList_.sort(cmpFunc);

    for (const auto &info : missionInfoList_) {
        missionIdMap_[info.missionInfo.id] = true;
    }
    return true;
}

void MissionInfoMgr::HandleUnInstallApp(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions)
{
    HILOG_INFO("HandleUnInstallApp, bundleName:%{public}s, uid:%{public}d", bundleName.c_str(), missionId);
    GetMatchedMission(bundleName, uid, missions);
    if (missions.empty()) {
        return;
    }

    for (auto missionId : missions) {
        DeleteMissionInfo(missionId);
    }
}

void MissionInfoMgr::GetMatchedMission(const std::string &bundleName, int32_t uid, std::list<int32_t> &missions)
{
    for (const auto& innerMissionInfo : missionInfoList_) {
        if (innerMissionInfo.bundleName == bundleName && innerMissionInfo.uid == uid) {
            missions.push_back(innerMissionInfo.missionInfo.id);
        }
    }
}

void MissionInfoMgr::Dump(std::vector<std::string> &info)
{
    for (const auto& innerMissionInfo : missionInfoList_) {
        innerMissionInfo.Dump(info);
    }
}

void MissionInfoMgr::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    snapshotHandler_ = handler;
}

bool MissionInfoMgr::UpdateMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot) const
{
    HILOG_ERROR("snapshot: UpdateMissionSnapshot, missionId:%{public}d", missionId);
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("snapshot: get mission failed, missionId %{public}d not exists", missionId);
        return false;
    }
    if (!snapshotHandler_) {
        HILOG_ERROR("snapshot: snapshotHandler_ is nullptr");
        return false;
    }
    Snapshot snapshot;
    int32_t result = snapshotHandler_->GetSnapshot(abilityToken, snapshot);
    if (result != 0) {
        HILOG_ERROR("snapshot: get WMS snapshot failed, result = %{public}d", result);
        return false;
    }
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("snapshot: taskDataPersistenceMgr_ is nullptr");
        return false;
    }
    missionSnapshot.snapshot = snapshot.GetPixelMap();
    missionSnapshot.topAbility = it->missionInfo.want.GetElement();
    if (!taskDataPersistenceMgr_->SaveMissionSnapshot(missionId, missionSnapshot)) {
        HILOG_ERROR("snapshot: save mission snapshot failed");
        return false;
    }
    HILOG_INFO("snapshot: update mission snapshot success");
    return true;
}

bool MissionInfoMgr::GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot) const
{
    HILOG_ERROR("mission_list_info GetMissionSnapshot, missionId:%{public}d", missionId);
    auto it = find_if(missionInfoList_.begin(), missionInfoList_.end(), [missionId](const InnerMissionInfo &info) {
        return missionId == info.missionInfo.id;
    });
    if (it == missionInfoList_.end()) {
        HILOG_ERROR("snapshot: get mission failed, missionId %{public}d not exists", missionId);
        return false;
    }
    if (!taskDataPersistenceMgr_) {
        HILOG_ERROR("snapshot: taskDataPersistenceMgr_ is nullptr");
        return false;
    }
    if (taskDataPersistenceMgr_->GetMissionSnapshot(missionId, missionSnapshot)) {
        missionSnapshot.topAbility = it->missionInfo.want.GetElement();
        HILOG_ERROR("mission_list_info GetMissionSnapshot, find snapshot OK, missionId:%{public}d", missionId);
        return true;
    }
    HILOG_INFO("snapshot: storage mission snapshot not exists, create new snapshot");
    return UpdateMissionSnapshot(missionId, abilityToken, missionSnapshot);
}
}  // namespace AAFwk
}  // namespace OHOS
