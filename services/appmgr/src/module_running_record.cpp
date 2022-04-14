/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "module_running_record.h"
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ModuleRunningRecord::ModuleRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const std::shared_ptr<AMSEventHandler> &eventHandler)
    : appInfo_(info), eventHandler_(eventHandler)
{}

ModuleRunningRecord::~ModuleRunningRecord()
{}

void ModuleRunningRecord::Init(const HapModuleInfo &info)
{
    owenInfo_ = info;
    owenState_ = ModuleRecordState::INITIALIZED_STATE;
}

const std::string &ModuleRunningRecord::GetModuleName() const
{
    return owenInfo_.moduleName;
}

const std::shared_ptr<ApplicationInfo> ModuleRunningRecord::GetAppInfo()
{
    return appInfo_;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        HILOG_ERROR("token is null");
        return nullptr;
    }
    const auto &iter = abilities_.find(token);
    if (iter != abilities_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::AddAbility(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    HILOG_INFO("Add ability.");
    if (!token || !abilityInfo) {
        HILOG_ERROR("Param abilityInfo or token is null");
        return nullptr;
    }
    if (GetAbilityRunningRecordByToken(token)) {
        HILOG_ERROR("AbilityRecord already exists and no need to add");
        return nullptr;
    }
    auto abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    abilityRecord->SetWant(want);
    abilities_.emplace(token, abilityRecord);
    return abilityRecord;
}

bool ModuleRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Is last ability record.");
    if (!token) {
        HILOG_ERROR("token is nullptr");
        return false;
    }

    return ((abilities_.size() == 1) && (abilities_.find(token) != abilities_.end()));
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> &ModuleRunningRecord::GetAbilities()
    const
{
    return abilities_;
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        HILOG_ERROR("GetAbilityByTerminateLists error, token is null");
        return nullptr;
    }
    const auto &iter = terminateAbilitys_.find(token);
    if (iter != terminateAbilitys_.end()) {
        return iter->second;
    }
    return nullptr;
}

void ModuleRunningRecord::ClearAbility(const std::shared_ptr<AbilityRunningRecord> &record)
{
    HILOG_INFO("Clear ability.");
    if (!record) {
        HILOG_ERROR("Param record is null");
        return;
    }
    if (!GetAbilityRunningRecordByToken(record->GetToken())) {
        HILOG_ERROR("Param record is not exist");
        return;
    }
    abilities_.erase(record->GetToken());
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityRunningRecord(const std::string &abilityName) const
{
    HILOG_INFO("Get ability running record by ability name.");
    const auto &iter = std::find_if(abilities_.begin(), abilities_.end(), [&abilityName](const auto &pair) {
        return pair.second->GetName() == abilityName;
    });
    return ((iter == abilities_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AbilityRunningRecord> ModuleRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    HILOG_INFO("Get ability running record by eventId.");
    const auto &iter = std::find_if(abilities_.begin(), abilities_.end(), [eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    if (iter != abilities_.end()) {
        return iter->second;
    }

    const auto &finder = std::find_if(terminateAbilitys_.begin(),
        terminateAbilitys_.end(),
        [eventId](const auto &pair) { return pair.second->GetEventId() == eventId; });
    if (finder != terminateAbilitys_.end()) {
        return finder->second;
    }
    return nullptr;
}

void ModuleRunningRecord::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!ability) {
        HILOG_ERROR("ability is null");
        return;
    }
    AbilityState oldState = ability->GetState();
    ability->SetState(state);
    HILOG_INFO("Ability state change from %{public}d to %{public}d, name is %{public}s.", 
        oldState, state, ability->GetName().c_str());
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner) {
        serviceInner->OnAbilityStateChanged(ability, state);
    }
}

void ModuleRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HILOG_INFO("Launch ability.");
    if (!ability || !ability->GetToken()) {
        HILOG_ERROR("null abilityRecord or abilityToken");
        return;
    }
    const auto &iter = abilities_.find(ability->GetToken());
    if (iter != abilities_.end() && appLifeCycleDeal_->GetApplicationClient()) {
        HILOG_INFO("Schedule launch ability, name is %{public}s.", ability->GetName().c_str());
        appLifeCycleDeal_->LaunchAbility(ability);
        ability->SetState(AbilityState::ABILITY_STATE_READY);
    }
}

void ModuleRunningRecord::LaunchPendingAbilities()
{
    HILOG_INFO("Launch pending abilities.");

    if (abilities_.empty()) {
        HILOG_ERROR("abilities_ is empty");
        return;
    }

    for (auto item : abilities_) {
        HILOG_INFO("state : %{public}d", item.second->GetState());
        if (item.second->GetState() == AbilityState::ABILITY_STATE_CREATE) {
            LaunchAbility(item.second);
        }
    }
}

void ModuleRunningRecord::TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce)
{
    HILOG_INFO("Terminate ability.");
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }

    terminateAbilitys_.emplace(token, abilityRecord);
    abilities_.erase(token);

    SendEvent(
        AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG, AMSEventHandler::TERMINATE_ABILITY_TIMEOUT, abilityRecord);

    if (!isForce) {
        auto curAbilityState = abilityRecord->GetState();
        if (curAbilityState != AbilityState::ABILITY_STATE_BACKGROUND) {
            HILOG_ERROR("current state(%{public}d) error", static_cast<int32_t>(curAbilityState));
            return;
        }
    }

    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleCleanAbility(token);
    } else {
        HILOG_WARN("appLifeCycleDeal_ is null");
        auto serviceInner = appMgrServiceInner_.lock();
        auto appRunningRecord = appRunningRecord_.lock();
        if (serviceInner) {
            serviceInner->ClearAppRunningData(appRunningRecord, true);
        }
    }

    HILOG_INFO("ModuleRunningRecord::TerminateAbility end");
}

void ModuleRunningRecord::SendEvent(
    uint32_t msg, int64_t timeOut, const std::shared_ptr<AbilityRunningRecord> &abilityRecord)
{
    HILOG_INFO("Send event");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    AppRunningRecord::appEventId_++;
    abilityRecord->SetEventId(AppRunningRecord::appEventId_);
    eventHandler_->SendEvent(msg, AppRunningRecord::appEventId_, timeOut);
}

void ModuleRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Ability terminated.");
    if (!token) {
        HILOG_ERROR("token is null");
        return;
    }

    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    auto abilityRecord = GetAbilityByTerminateLists(token);
    if (!abilityRecord) {
        HILOG_ERROR("ModuleRunningRecord::AbilityTerminated can not find ability record");
        return;
    }

    eventHandler_->RemoveEvent(AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG, abilityRecord->GetEventId());
    terminateAbilitys_.erase(token);
}

void ModuleRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
    appMgrServiceInner_ = inner;
}

ModuleRecordState ModuleRunningRecord::GetModuleRecordState()
{
    return owenState_;
}

void ModuleRunningRecord::SetModuleRecordState(const ModuleRecordState &state)
{
    owenState_ = state;
}

void ModuleRunningRecord::GetHapModuleInfo(HapModuleInfo &info)
{
    info = owenInfo_;
}

void ModuleRunningRecord::SetApplicationClient(std::shared_ptr<AppLifeCycleDeal> &appLifeCycleDeal)
{
    appLifeCycleDeal_ = appLifeCycleDeal;
}

ModuleRecordState ModuleRunningRecord::GetState() const
{
    return owenState_;
}

void ModuleRunningRecord::SetAppRunningRecord(const std::shared_ptr<AppRunningRecord> &appRunningRecord)
{
    appRunningRecord_ = appRunningRecord;
}
}  // namespace AppExecFwk
}  // namespace OHOS
