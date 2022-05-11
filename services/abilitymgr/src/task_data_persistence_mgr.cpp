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

#include "task_data_persistence_mgr.h"
#include "ability_util.h"
#include "file_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
TaskDataPersistenceMgr::TaskDataPersistenceMgr()
{
    HILOG_INFO("TaskDataPersistenceMgr instance is created");
}

TaskDataPersistenceMgr::~TaskDataPersistenceMgr()
{
    eventLoop_.reset();
    handler_.reset();
    HILOG_INFO("TaskDataPersistenceMgr instance is destroyed");
}

bool TaskDataPersistenceMgr::Init(int userId)
{
    if (!eventLoop_) {
        eventLoop_ = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHECK_POINTER_RETURN_BOOL(eventLoop_);
    }

    if (!handler_) {
        handler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
        CHECK_POINTER_RETURN_BOOL(handler_);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (missionDataStorageMgr_.find(userId) == missionDataStorageMgr_.end()) {
        currentMissionDataStorage_ = std::make_shared<MissionDataStorage>(userId);
        missionDataStorageMgr_.insert(std::make_pair(userId, currentMissionDataStorage_));
    } else {
        currentMissionDataStorage_ = missionDataStorageMgr_[userId];
    }
    currentUserId_ = userId;

    CHECK_POINTER_RETURN_BOOL(currentMissionDataStorage_);
    HILOG_INFO("Init success.");
    return true;
}

bool TaskDataPersistenceMgr::LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!currentMissionDataStorage_) {
        HILOG_ERROR("currentMissionDataStorage_ is nullptr");
        return false;
    }

    return currentMissionDataStorage_->LoadAllMissionInfo(missionInfoList);
}

bool TaskDataPersistenceMgr::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        HILOG_ERROR("handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> SaveMissionInfoFunc = [weakPtr, missionInfo]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->SaveMissionInfo(missionInfo);
        }
    };
    return handler_->PostTask(SaveMissionInfoFunc, SAVE_MISSION_INFO);
}

bool TaskDataPersistenceMgr::DeleteMissionInfo(int missionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        HILOG_ERROR("handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> DeleteMissionInfoFunc = [weakPtr, missionId]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->DeleteMissionInfo(missionId);
        }
    };
    return handler_->PostTask(DeleteMissionInfoFunc, DELETE_MISSION_INFO);
}

bool TaskDataPersistenceMgr::RemoveUserDir(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentUserId_ == userId) {
        HILOG_ERROR("can not removed current user dir");
        return false;
    }
    std::string userDir = TASK_DATA_FILE_BASE_PATH + "/" + std::to_string(userId);
    bool ret = OHOS::HiviewDFX::FileUtil::ForceRemoveDirectory(userDir);
    if (!ret) {
        HILOG_ERROR("remove user dir %{public}s failed.", userDir.c_str());
        return false;
    }
    return true;
}

bool TaskDataPersistenceMgr::SaveMissionSnapshot(int missionId, const MissionSnapshot& snapshot)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_ || !currentMissionDataStorage_) {
        HILOG_ERROR("snapshot: handler_ or currentMissionDataStorage_ is nullptr");
        return false;
    }

    std::weak_ptr<MissionDataStorage> weakPtr(currentMissionDataStorage_);
    std::function<void()> SaveMissionSnapshotFunc = [weakPtr, missionId, snapshot]() {
        auto missionDataStorage = weakPtr.lock();
        if (missionDataStorage) {
            missionDataStorage->SaveMissionSnapshot(missionId, snapshot);
        }
    };
    return handler_->PostTask(SaveMissionSnapshotFunc, SAVE_MISSION_SNAPSHOT);
}

#ifdef SUPPORT_GRAPHICS
sptr<Media::PixelMap> TaskDataPersistenceMgr::GetSnapshot(int missionId) const
{
    if (!currentMissionDataStorage_) {
        HILOG_ERROR("snapshot: currentMissionDataStorage_ is nullptr");
        return nullptr;
    }
    return currentMissionDataStorage_->GetSnapshot(missionId);
}
#endif

bool TaskDataPersistenceMgr::GetMissionSnapshot(int missionId, MissionSnapshot& snapshot)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!currentMissionDataStorage_) {
        HILOG_ERROR("snapshot: currentMissionDataStorage_ is nullptr");
        return false;
    }
    return currentMissionDataStorage_->GetMissionSnapshot(missionId, snapshot);
}
}  // namespace AAFwk
}  // namespace OHOS
