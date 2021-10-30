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

#include "mission_record.h"

#include "hilog_wrapper.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
int MissionRecord::nextMissionId_ = 0;

MissionRecord::MissionRecord(const std::string &bundleName) : bundleName_(bundleName)
{
    missionId_ = GetNextMissionId();
    option_.missionId = missionId_;
}

MissionRecord::MissionRecord(const std::shared_ptr<MissionRecord> &mission)
{
    bundleName_ = mission->bundleName_;
    missionId_ = mission->missionId_;
    abilities_.insert(abilities_.begin(), mission->abilities_.begin(), mission->abilities_.end());
    isLauncherCreate_ = mission->isLauncherCreate_;
    preMissionRecord_ = mission->preMissionRecord_;
    parentMissionStack_ = mission->parentMissionStack_;
    missionDescriptionInfo_ = mission->missionDescriptionInfo_;
}

MissionRecord::~MissionRecord()
{}

int MissionRecord::GetNextMissionId()
{
    return nextMissionId_++;
}

int MissionRecord::GetMissionRecordId() const
{
    return missionId_;
}

int MissionRecord::GetAbilityRecordCount() const
{
    return abilities_.size();
}

std::shared_ptr<AbilityRecord> MissionRecord::GetBottomAbilityRecord() const
{
    if (abilities_.empty()) {
        HILOG_ERROR("abilities is empty");
        return nullptr;
    }
    return abilities_.back();
}

std::shared_ptr<AbilityRecord> MissionRecord::GetTopAbilityRecord() const
{
    if (abilities_.empty()) {
        HILOG_ERROR("abilities is empty");
        return nullptr;
    }
    return abilities_.front();
}

std::shared_ptr<AbilityRecord> MissionRecord::GetLastTopAbility() const
{
    if (abilities_.empty() || abilities_.size() == 1) {
        HILOG_WARN("no last top ability.");
        return nullptr;
    }
    auto iter = abilities_.begin();
    return (*(++iter));
}

std::shared_ptr<AbilityRecord> MissionRecord::GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const
{
    auto abilityToFind = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityToFind, nullptr);

    auto isExist = [targetAbility = abilityToFind](const std::shared_ptr<AbilityRecord> &ability) {
        if (ability == nullptr) {
            return false;
        }
        return targetAbility == ability;
    };

    auto iter = std::find_if(abilities_.begin(), abilities_.end(), isExist);
    if (iter != abilities_.end()) {
        return *iter;
    }

    return nullptr;
}

std::shared_ptr<AbilityRecord> MissionRecord::GetAbilityRecordById(const int64_t recordId) const
{
    for (std::shared_ptr<AbilityRecord> ability : abilities_) {
        if (ability != nullptr && ability->GetRecordId() == recordId) {
            return ability;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> MissionRecord::GetAbilityRecordByCaller(
    const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    for (auto &ability : abilities_) {
        auto callerList = ability->GetCallerRecordList();
        for (auto callerAbility : callerList) {
            if (callerAbility->GetCaller() == caller && callerAbility->GetRequestCode() == requestCode) {
                return ability;
            }
        }
    }
    return nullptr;
}

void MissionRecord::AddAbilityRecordToTop(std::shared_ptr<AbilityRecord> ability)
{
    CHECK_POINTER(ability);
    auto isExist = [targetAbility = ability](const std::shared_ptr<AbilityRecord> &ability) {
        if (ability == nullptr) {
            return false;
        }
        return targetAbility == ability;
    };
    auto iter = std::find_if(abilities_.begin(), abilities_.end(), isExist);
    if (iter == abilities_.end()) {
        abilities_.push_front(ability);
        ability->ForceProcessConfigurationChange(GetConfiguration());
    }
}

bool MissionRecord::RemoveAbilityRecord(std::shared_ptr<AbilityRecord> ability)
{
    CHECK_POINTER_RETURN_BOOL(ability);
    for (auto iter = abilities_.begin(); iter != abilities_.end(); iter++) {
        if ((*iter) == ability) {
            abilities_.erase(iter);
            return true;
        }
    }
    HILOG_ERROR("can not find ability");
    return false;
}

bool MissionRecord::RemoveTopAbilityRecord()
{
    if (abilities_.empty()) {
        HILOG_ERROR("abilities is empty");
        return false;
    }
    abilities_.pop_front();
    return true;
}

void MissionRecord::RemoveAll()
{
    abilities_.clear();
}

void MissionRecord::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "    MissionRecord ID #" + std::to_string(missionId_);
    std::shared_ptr<AbilityRecord> bottomAbility = GetBottomAbilityRecord();
    if (bottomAbility) {
        dumpInfo += "  bottom app [" + bottomAbility->GetAbilityInfo().name + "]" + "  winMode #" +
                    std::to_string(option_.winModeKey);
        info.push_back(dumpInfo);
        for (auto abilityRecord : abilities_) {
            abilityRecord->Dump(info);
        }
    }
}

bool MissionRecord::IsSameMissionRecord(const std::string &bundleName) const
{
    if (bundleName.empty() || bundleName_.empty()) {
        return false;
    }
    return (bundleName == bundleName_);
}

void MissionRecord::GetAllAbilityInfo(std::vector<AbilityRecordInfo> &abilityInfos)
{
    for (auto ability : abilities_) {
        AbilityRecordInfo abilityInfo;
        ability->GetAbilityRecordInfo(abilityInfo);
        abilityInfos.emplace_back(abilityInfo);
    }
}

bool MissionRecord::IsTopAbilityRecordByName(const std::string &abilityName)
{
    std::shared_ptr<AbilityRecord> topAbility = GetTopAbilityRecord();
    if (topAbility == nullptr) {
        return false;
    }

    return (topAbility->GetAbilityInfo().name.compare(abilityName) == 0);
}

void MissionRecord::SetIsLauncherCreate()
{
    isLauncherCreate_ = true;
}

bool MissionRecord::IsLauncherCreate() const
{
    return isLauncherCreate_;
}

void MissionRecord::SetPreMissionRecord(const std::shared_ptr<MissionRecord> &record)
{
    preMissionRecord_ = record;
}

std::shared_ptr<MissionRecord> MissionRecord::GetPreMissionRecord() const
{
    return preMissionRecord_.lock();
}

bool MissionRecord::IsExistAbilityRecord(int32_t id)
{
    for (auto &it : abilities_) {
        if (it->GetRecordId() == id) {
            return true;
        }
    }
    return false;
}

bool MissionRecord::SupportMultWindow() const
{
    auto bottom = GetBottomAbilityRecord();
    if (bottom != nullptr) {
        return bottom->SupportMultWindow();
    }
    return false;
}

void MissionRecord::SetMissionStack(const std::shared_ptr<MissionStack> &missionStack, int stackId)
{
    CHECK_POINTER(missionStack);
    parentMissionStack_ = missionStack;
    ConfigurationHolder::Init(missionStack->GetConfiguration());
    for (auto &it : abilities_) {
        it->SetMissionStackId(stackId);
    }
}

std::shared_ptr<MissionStack> MissionRecord::GetMissionStack() const
{
    return parentMissionStack_.lock();
}

void MissionRecord::SetMissionOption(const MissionOption &option)
{
    if (option.winModeKey != option_.winModeKey) {
        HILOG_ERROR("Batch processing notify multi window mode changed.");
        for (auto &it : abilities_) {
            CHECK_POINTER(it);
            bool flag = option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
                        option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY ||
                        option.winModeKey == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING;
            // true : old is multi win, target is fullscreen.
            // false : old is fullscreen, target is multi win.
            auto key = flag ? option.winModeKey : option_.winModeKey;
            if (it->IsReady()) {
                it->NotifyMultiWinModeChanged(key, flag);
            }
        }
    }

    option_ = option;
}

const MissionOption &MissionRecord::GetMissionOption() const
{
    return option_;
}

bool MissionRecord::IsEmpty()
{
    return abilities_.empty();
}

std::shared_ptr<ConfigurationHolder> MissionRecord::GetParent()
{
    return parentMissionStack_.lock();
}

unsigned int MissionRecord::GetChildSize()
{
    return abilities_.size();
}

std::shared_ptr<ConfigurationHolder> MissionRecord::FindChild(unsigned int index)
{
    if (index < abilities_.size() && index >= 0) {
        auto iter = abilities_.begin();
        std::advance(iter, index);
        return (*iter);
    }
    return nullptr;
}

void MissionRecord::Resume(const std::shared_ptr<MissionRecord> &backup)
{
    HILOG_INFO("mission resume.");
    // backup abilities_ size = 1, singleton ability need resume
    if (std::equal(abilities_.begin(), abilities_.end(), backup->abilities_.begin()) && backup->abilities_.size() > 1) {
        HILOG_ERROR("List equality, no resume");
        return;
    }

    std::list<std::shared_ptr<AbilityRecord>> diffAbilitys;
    for (auto &ability : backup->abilities_) {
        if (abilities_.front() == ability) {
            break;
        }

        ability->SetAbilityState(AbilityState::INITIAL);
        diffAbilitys.emplace_back(ability);
    }

    abilities_.insert(abilities_.begin(), diffAbilitys.begin(), diffAbilitys.end());

    for (auto &ability : abilities_) {
        if (ability->IsAbilityState(AbilityState::INITIAL)) {
            ability->ClearFlag();
            ability->SetRestarting(true);
        }
    }
}

void MissionRecord::UpdateActiveTimestamp()
{
    activeTimestamp_ = AbilityUtil::SystemTimeMillis();
}

int64_t MissionRecord::GetActiveTimestamp() const
{
    return activeTimestamp_;
}
}  // namespace AAFwk
}  // namespace OHOS
