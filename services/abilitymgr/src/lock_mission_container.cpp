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

#include "lock_mission_container.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool LockMissionContainer::IsLockedMissionState() const
{
    return lockState_ != LockMissionState::LOCK_MISSION_STATE_NONE;
}

int LockMissionContainer::GetLockedMissionState() const
{
    return lockState_;
}

bool LockMissionContainer::SetLockedMission(
    const std::shared_ptr<MissionRecord> &mission, int lockerUid, bool isSystemApp)
{
    auto lockMission = lockMission_.lock();
    if (lockMission) {
        HILOG_WARN("It already exists lock mission , deny lock request. locked mission id :%{public}d, name : "
                   "%{public}s ,request mission id :%{public}d, name : %{public}s",
            lockMission->GetMissionRecordId(),
            lockMission->GetName().c_str(),
            mission->GetMissionRecordId(),
            mission->GetName().c_str());
        return false;
    }

    HILOG_INFO("lock mission ID: %{public}d , name : %{public}s lockerUid : %{public}d",
        mission->GetMissionRecordId(),
        mission->GetName().c_str(),
        lockerUid);
    lockMission_ = mission;
    lockerUid_ = lockerUid;
    lockState_ =
        isSystemApp ? LockMissionState::LOCK_MISSION_STATE_PINNED : LockMissionState::LOCK_MISSION_STATE_LOCKED;
    return true;
}

bool LockMissionContainer::ReleaseLockedMission(
    const std::shared_ptr<MissionRecord> &mission, int callerUid, bool isForce)
{
    auto lockMission = lockMission_.lock();
    if (lockMission == nullptr) {
        HILOG_WARN("There is no locked mission. release locked mission fail.");
        return false;
    }

    if (isForce && lockMission == mission) {
        HILOG_INFO("release lock mission ID: %{public}d , name : %{public}s lockerUid : %{public}d",
            lockMission->GetMissionRecordId(),
            lockMission->GetName().c_str(),
            lockerUid_);
        Clear();
        return true;
    }

    if (callerUid != lockerUid_) {
        HILOG_WARN("caller uid not equal to locker uid. release locked mission fail.");
        return false;
    }

    if (mission != lockMission) {
        HILOG_WARN("unlock mission not equal to locked mission. release locked mission fail.");
        return false;
    }

    HILOG_INFO("Normal release lock mission ID: %{public}d , name : %{public}s lockerUid : %{public}d",
        lockMission->GetMissionRecordId(),
        lockMission->GetName().c_str(),
        lockerUid_);

    Clear();
    return true;
}

bool LockMissionContainer::IsSameLockedMission(std::string bundleName) const
{
    auto lockMission = lockMission_.lock();
    if (lockMission == nullptr) {
        HILOG_WARN("There is no locked mission. release locked mission fail.");
        return false;
    }
    return lockMission->GetName() == bundleName;
}

void LockMissionContainer::Clear()
{
    lockMission_.reset();
    lockerUid_ = -1;
    lockState_ = LockMissionState::LOCK_MISSION_STATE_NONE;
}

std::shared_ptr<MissionRecord> LockMissionContainer::GetLockMission()
{
    return lockMission_.lock();
}
}  // namespace AAFwk
}  // namespace OHOS