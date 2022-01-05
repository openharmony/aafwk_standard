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

#include "resume_mission_container.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
ResumeMissionContainer::ResumeMissionContainer(const std::shared_ptr<AbilityEventHandler> &handler) : handler_(handler)
{}

void ResumeMissionContainer::Save(const std::shared_ptr<MissionRecord> &mission)
{
    HILOG_INFO("Save.");
    CHECK_POINTER(mission);

    auto missionId = mission->GetMissionRecordId();
    auto finder = missionMaps_.find(missionId);
    std::shared_ptr<MissionRecord> backup;
    if (finder == missionMaps_.end()) {
        HILOG_INFO("Backup");
        backup = std::make_shared<MissionRecord>(mission);
        missionMaps_.emplace(missionId, backup);
    }

    auto missionSaveTime = DelayedSingleton<AbilityManagerService>::GetInstance()->GetMissionSaveTime();
    HILOG_INFO("mission save time : %{public}d", missionSaveTime);
    if (backup) {
        HILOG_INFO("start save time...");
        handler_->PostTask(
            [this, missionId]() { Remove(missionId); }, taskName_ + std::to_string(missionId), missionSaveTime);
    }
}

void ResumeMissionContainer::Resume(const std::shared_ptr<MissionRecord> &mission)
{
    HILOG_INFO("Resume.");
    CHECK_POINTER(mission);

    auto backup = missionMaps_.find(mission->GetMissionRecordId());
    mission->Resume(backup->second);
    missionMaps_.erase(mission->GetMissionRecordId());
}

void ResumeMissionContainer::Remove(int missionId)
{
    HILOG_INFO("Remove.");
    missionMaps_.erase(missionId);
    handler_->RemoveTask(taskName_ + std::to_string(missionId));
}

bool ResumeMissionContainer::IsResume(int missionId)
{
    return (missionMaps_.find(missionId) != missionMaps_.end());
}
}  // namespace AAFwk
}  // namespace OHOS