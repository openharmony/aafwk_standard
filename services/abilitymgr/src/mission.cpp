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

#include "mission.h"

namespace OHOS {
namespace AAFwk {
Mission::Mission(int32_t id, const std::shared_ptr<AbilityRecord> abilityRecord, const std::string &missionName)
    : missionId_(id), abilityRecord_(abilityRecord), missionName_(missionName)
{
}

Mission::Mission(const std::shared_ptr<Mission> &mission)
{
    if (!mission) {
        return;
    }

    missionId_ = mission->missionId_;
    abilityRecord_ = mission->abilityRecord_;
    missionName_ = mission->missionName_;
    ownerMissionList_ = mission->ownerMissionList_;
}

Mission::~Mission()
{}

std::shared_ptr<AbilityRecord> Mission::GetAbilityRecord() const
{
    return abilityRecord_;
}

int32_t Mission::GetMissionId() const
{
    return missionId_;
}

bool Mission::IsSingletonAbility() const
{
    if (abilityRecord_) {
        return abilityRecord_->GetAbilityInfo().launchMode == AppExecFwk::LaunchMode::SINGLETON;
    }

    return false;
}

std::shared_ptr<MissionList> Mission::GetMissionList()
{
    return ownerMissionList_.lock();
}

std::string Mission::GetMissionName() const
{
    return missionName_;
}

void Mission::SetMissionList(const std::shared_ptr<MissionList> &missionList)
{
    ownerMissionList_ = missionList;
    if (abilityRecord_) {
        abilityRecord_->SetMissionList(missionList);
    }
}

void Mission::SetLockedState(bool lockedState)
{
    lockedState_ = lockedState;
}

bool Mission::IsLockedState() const
{
    return lockedState_;
}

void Mission::SetMovingState(bool movingState)
{
    isMovingToFront_ = movingState;
}

bool Mission::IsMovingState() const
{
    return isMovingToFront_;
}

void Mission::SetANRState()
{
    isANRState_ = true;
}

bool Mission::IsANRState() const
{
    return isANRState_;
}

void Mission::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "    Mission ID #" + std::to_string(missionId_);
    dumpInfo += "  mission name #[" + missionName_ + "]" + "  lockedState #" + std::to_string(lockedState_)
        + "  ANR State #" + std::to_string(isANRState_);
    info.push_back(dumpInfo);
    if (abilityRecord_) {
        abilityRecord_->Dump(info);
    }
}

}  // namespace AAFwk
}  // namespace OHOS
