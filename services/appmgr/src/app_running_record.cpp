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

#include "app_running_record.h"
#include "app_log_wrapper.h"
#include "app_mgr_service_inner.h"
#include "bytrace.h"

namespace OHOS {
namespace AppExecFwk {
int64_t AppRunningRecord::appEventId_ = 0;

RenderRecord::RenderRecord(pid_t hostPid, const std::string& renderParam,
    int32_t ipcFd, int32_t sharedFd, const std::shared_ptr<AppRunningRecord> &host)
    : hostPid_(hostPid), renderParam_(renderParam), ipcFd_(ipcFd), sharedFd_(sharedFd), host_(host)
{}

RenderRecord::~RenderRecord()
{}

std::shared_ptr<RenderRecord> RenderRecord::CreateRenderRecord(pid_t hostPid, const std::string& renderParam,
    int32_t ipcFd, int32_t sharedFd, const std::shared_ptr<AppRunningRecord> &host)
{
    if (hostPid <= 0 || renderParam.empty() || ipcFd <= 0 || sharedFd <= 0 || !host) {
        return nullptr;
    }

    auto renderRecord = std::make_shared<RenderRecord>(hostPid, renderParam, ipcFd, sharedFd, host);
    if (!renderRecord) {
        APP_LOGE("create render record failed, hostPid:%{public}d.", hostPid);
        return nullptr;
    }

    return renderRecord;
}

void RenderRecord::SetPid(pid_t pid)
{
    pid_ = pid;
}

pid_t RenderRecord::GetPid()
{
    return pid_;
}

pid_t RenderRecord::GetHostPid()
{
    return hostPid_;
}

std::string RenderRecord::GetRenderParam()
{
    return renderParam_;
}

int32_t RenderRecord::GetIpcFd()
{
    return ipcFd_;
}

int32_t RenderRecord::GetSharedFd()
{
    return sharedFd_;
}

std::shared_ptr<AppRunningRecord> RenderRecord::GetHostRecord()
{
    return host_.lock();
}

sptr<IRenderScheduler> RenderRecord::GetScheduler()
{
    return renderScheduler_;
}

void RenderRecord::SetScheduler(const sptr<IRenderScheduler> &scheduler)
{
    renderScheduler_ = scheduler;
}

void RenderRecord::SetDeathRecipient(const sptr<AppDeathRecipient> recipient)
{
    deathRecipient_ = recipient;
}

void RenderRecord::RegisterDeathRecipient()
{
    if (renderScheduler_ && deathRecipient_) {
        auto obj = renderScheduler_->AsObject();
        if (obj) {
            obj->AddDeathRecipient(deathRecipient_);
        }
    }
}

AppRunningRecord::AppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const int32_t recordId, const std::string &processName)
    : appRecordId_(recordId), processName_(processName)
{
    if (info) {
        appInfo_ = info;
        mainBundleName_ = info->bundleName;
        isLauncherApp_ = info->isLauncherApp;
        isClonedApp_ = info->isCloned;
        mainAppName_ = info->name;
    }
}

void AppRunningRecord::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    if (!appLifeCycleDeal_) {
        appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    }
    appLifeCycleDeal_->SetApplicationClient(thread);

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        APP_LOGE("moduleRecordList is empty");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
    }
}

const std::string &AppRunningRecord::GetBundleName() const
{
    return mainBundleName_;
}

bool AppRunningRecord::IsLauncherApp() const
{
    return isLauncherApp_;
}

int32_t AppRunningRecord::GetRecordId() const
{
    return appRecordId_;
}

const std::string &AppRunningRecord::GetName() const
{
    return mainAppName_;
}

bool AppRunningRecord::GetCloneInfo() const
{
    return isClonedApp_;
}

const std::string &AppRunningRecord::GetSignCode() const
{
    return signCode_;
}

void AppRunningRecord::SetSignCode(const std::string &signCode)
{
    signCode_ = signCode;
}

const std::string &AppRunningRecord::GetJointUserId() const
{
    return jointUserId_;
}

void AppRunningRecord::SetJointUserId(const std::string &jointUserId)
{
    jointUserId_ = jointUserId;
}

const std::string &AppRunningRecord::GetProcessName() const
{
    return processName_;
}

int32_t AppRunningRecord::GetUid() const
{
    return mainUid_;
}

void AppRunningRecord::SetUid(const int32_t uid)
{
    mainUid_ = uid;
}

ApplicationState AppRunningRecord::GetState() const
{
    return curState_;
}

void AppRunningRecord::SetState(const ApplicationState state)
{
    if (state >= ApplicationState::APP_STATE_END) {
        APP_LOGE("Invalid application state");
        return;
    }
    curState_ = state;
}

const std::list<std::shared_ptr<ApplicationInfo>> AppRunningRecord::GetAppInfoList()
{
    std::list<std::shared_ptr<ApplicationInfo>> appInfoList;
    for (const auto &item : appInfos_) {
        appInfoList.push_back(item.second);
    }
    return appInfoList;
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> AppRunningRecord::GetAbilities()
{
    std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> abilitysMap;
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilities = moduleRecord->GetAbilities();
        abilitysMap.insert(abilities.begin(), abilities.end());
    }
    return abilitysMap;
}

std::map<std::string, std::vector<std::shared_ptr<ModuleRunningRecord>>> &AppRunningRecord::GetModules()
{
    return hapModules_;
}

sptr<IAppScheduler> AppRunningRecord::GetApplicationClient() const
{
    return (appLifeCycleDeal_ ? appLifeCycleDeal_->GetApplicationClient() : nullptr);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecord(const std::string &abilityName) const
{
    APP_LOGI("Get ability running record by ability name.");
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilityRecord = moduleRecord->GetAbilityRunningRecord(abilityName);
        if (abilityRecord) {
            return abilityRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    APP_LOGI("Get ability running record by eventId.");
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilityRecord = moduleRecord->GetAbilityRunningRecord(eventId);
        if (abilityRecord) {
            return abilityRecord;
        }
    }

    return nullptr;
}

void AppRunningRecord::ClearAbility(const std::shared_ptr<AbilityRunningRecord> &record)
{
    if (!record) {
        APP_LOGE("Param record is null");
        return;
    }

    auto moduleRecord = GetModuleRunningRecordByToken(record->GetToken());
    if (!moduleRecord) {
        APP_LOGE("moduleRecord is not exit");
        return;
    }

    moduleRecord->ClearAbility(record);

    if (moduleRecord->GetAbilities().empty()) {
        RemoveModuleRecord(moduleRecord);
    }
}

void AppRunningRecord::RemoveModuleRecord(const std::shared_ptr<ModuleRunningRecord> &moudleRecord)
{
    APP_LOGI("Remove module record.");

    for (auto &item : hapModules_) {
        auto iter = std::find_if(item.second.begin(),
            item.second.end(),
            [&moudleRecord](const std::shared_ptr<ModuleRunningRecord> &record) { return moudleRecord == record; });
        if (iter != item.second.end()) {
            iter = item.second.erase(iter);
            if (item.second.empty()) {
                hapModules_.erase(item.first);
                appInfos_.erase(item.first);
            }
            return;
        }
    }
}

void AppRunningRecord::ForceKillApp([[maybe_unused]] const std::string &reason) const
{}

void AppRunningRecord::ScheduleAppCrash([[maybe_unused]] const std::string &description) const
{}

void AppRunningRecord::LaunchApplication(const Configuration &config)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        APP_LOGE("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        APP_LOGE("appThread is null");
        return;
    }
    AppLaunchData launchData;
    auto moduleRecords = appInfos_.find(mainBundleName_);
    if (moduleRecords != appInfos_.end()) {
        launchData.SetApplicationInfo(*(moduleRecords->second));
    }
    ProcessInfo processInfo(processName_, GetPriorityObject()->GetPid());
    launchData.SetProcessInfo(processInfo);
    launchData.SetRecordId(appRecordId_);
    launchData.SetUId(mainUid_);
    launchData.SetUserTestInfo(userTestRecord_);
    APP_LOGI("ScheduleLaunchApplication app:%{public}s", GetName().c_str());
    appLifeCycleDeal_->LaunchApplication(launchData, config);
}

void AppRunningRecord::AddAbilityStage()
{
    if (!isNewMission_) {
        APP_LOGI("Current version than supports !");
        return;
    }
    HapModuleInfo abilityStage;
    if (GetTheModuleInfoNeedToUpdated(mainBundleName_, abilityStage)) {
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
        APP_LOGI("Current Informed module : [%{public}s] | bundle : [%{public}s]",
            abilityStage.moduleName.c_str(), mainBundleName_.c_str());
        appLifeCycleDeal_->AddAbilityStage(abilityStage);
        return;
    }

    APP_LOGI("The current process[%{public}s] is updated", processName_.c_str());
}

void AppRunningRecord::AddAbilityStageBySpecifiedAbility(const std::string &bundleName)
{
    HapModuleInfo hapModuleInfo;
    if (GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo)) {
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
        appLifeCycleDeal_->AddAbilityStage(hapModuleInfo);
    }
}

void AppRunningRecord::AddAbilityStageDone()
{
    APP_LOGI("Add ability stage done. bundle %{public}s and eventId %{public}d", mainBundleName_.c_str(),
        static_cast<int>(eventId_));
    eventHandler_->RemoveEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, eventId_);
    // Should proceed to the next notification

    if (isSpecifiedAbility_) {
        ScheduleAcceptWant(moduleName_);
        return;
    }

    AddAbilityStage();
}

void AppRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        APP_LOGE("appLifeCycleDeal_ is null");
        return;
    }
    if (!ability || !ability->GetToken()) {
        APP_LOGE("abilityRecord or abilityToken is nullptr.");
        return;
    }

    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    if (!moduleRecord) {
        APP_LOGE("moduleRecord is nullptr");
        return;
    }

    moduleRecord->LaunchAbility(ability);
}

void AppRunningRecord::ScheduleTerminate()
{
    SendEvent(AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG, AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT);
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleTerminate();
    }
}

void AppRunningRecord::LaunchPendingAbilities()
{
    APP_LOGI("Launch pending abilities.");

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        APP_LOGE("moduleRecordList is empty");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
        moduleRecord->LaunchPendingAbilities();
    }
}
void AppRunningRecord::ScheduleForegroundRunning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleForegroundRunning();
    }
}

void AppRunningRecord::ScheduleBackgroundRunning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleBackgroundRunning();
    }
}

void AppRunningRecord::ScheduleProcessSecurityExit()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleProcessSecurityExit();
    }
}

void AppRunningRecord::ScheduleTrimMemory()
{
    if (appLifeCycleDeal_ && priorityObject_) {
        appLifeCycleDeal_->ScheduleTrimMemory(priorityObject_->GetTimeLevel());
    }
}

void AppRunningRecord::LowMemoryWarning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->LowMemoryWarning();
    }
}

void AppRunningRecord::AddModules(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos)
{
    APP_LOGI("Add modules");

    if (moduleInfos.empty()) {
        APP_LOGI("moduleInfos is empty.");
        return;
    }

    for (auto &iter : moduleInfos) {
        AddModule(appInfo, nullptr, nullptr, iter, nullptr);
    }
}

void AppRunningRecord::AddModule(const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const sptr<IRemoteObject> &token,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    APP_LOGI("Add module.");

    if (!appInfo) {
        APP_LOGE("appInfo is null");
        return;
    }

    std::shared_ptr<ModuleRunningRecord> moduleRecord;
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleList;

    auto initModuleRecord = [=](const std::shared_ptr<ModuleRunningRecord> &moduleRecord) {
        moduleRecord->Init(hapModuleInfo);
        moduleRecord->SetAppMgrServiceInner(appMgrServiceInner_);
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
    };

    const auto &iter = hapModules_.find(appInfo->bundleName);
    if (iter != hapModules_.end()) {
        moduleRecord = GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
            iter->second.push_back(moduleRecord);
            initModuleRecord(moduleRecord);
        }
    } else {
        moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
        moduleList.push_back(moduleRecord);
        hapModules_.emplace(appInfo->bundleName, moduleList);
        appInfos_.emplace(appInfo->bundleName, appInfo);
        initModuleRecord(moduleRecord);
    }

    if (!abilityInfo || !token) {
        APP_LOGE("abilityinfo or token is nullptr");
        return;
    }
    moduleRecord->AddAbility(token, abilityInfo, want);

    return;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRecordByModuleName(
    const std::string bundleName, const std::string &moduleName)
{
    APP_LOGI("Get module record by module name.");
    auto moduleRecords = hapModules_.find(bundleName);
    if (moduleRecords != hapModules_.end()) {
        for (auto &iter : moduleRecords->second) {
            if (iter->GetModuleName() == moduleName) {
                return iter;
            }
        }
    }

    return nullptr;
}

void AppRunningRecord::StateChangedNotifyObserver(
    const std::shared_ptr<AbilityRunningRecord> &ability, const int32_t state, bool isAbility)
{
    if (!ability) {
        APP_LOGE("ability is null");
        return;
    }
    AbilityStateData abilityStateData;
    abilityStateData.bundleName = ability->GetAbilityInfo()->applicationInfo.bundleName;
    abilityStateData.abilityName = ability->GetName();
    abilityStateData.pid = GetPriorityObject()->GetPid();
    abilityStateData.abilityState = state;
    abilityStateData.uid = ability->GetAbilityInfo()->applicationInfo.uid;
    abilityStateData.token = ability->GetToken();
    abilityStateData.abilityType = static_cast<int32_t>(ability->GetAbilityInfo()->type);

    if (isAbility && ability->GetAbilityInfo() != nullptr &&
        ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        APP_LOGI("extension type, not notify any more.");
        return;
    }
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner) {
        serviceInner->StateChangedNotifyObserver(abilityStateData, isAbility);
    }
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        APP_LOGE("token is null");
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityRunningRecordByToken(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        APP_LOGE("token is null");
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityByTerminateLists(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        APP_LOGE("moduleRecord is not exit");
        return nullptr;
    }
    return moduleRecord->GetAbilityRunningRecordByToken(token);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        APP_LOGE("moduleRecord is not exit");
        return nullptr;
    }
    return moduleRecord->GetAbilityByTerminateLists(token);
}

void AppRunningRecord::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    APP_LOGI("state is :%{public}d", static_cast<int32_t>(state));
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("can not find ability record");
        return;
    }
    if (state == abilityRecord->GetState()) {
        APP_LOGE("current state is already, no need update");
        return;
    }

    if (state == AbilityState::ABILITY_STATE_FOREGROUND) {
        AbilityForeground(abilityRecord);
    } else if (state == AbilityState::ABILITY_STATE_BACKGROUND) {
        AbilityBackground(abilityRecord);
    } else {
        APP_LOGW("wrong state");
    }
}

void AppRunningRecord::AbilityForeground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        APP_LOGE("ability is null");
        return;
    }
    AbilityState curAbilityState = ability->GetState();
    if (curAbilityState != AbilityState::ABILITY_STATE_READY &&
        curAbilityState != AbilityState::ABILITY_STATE_BACKGROUND) {
        APP_LOGE("ability state(%{public}d) error", static_cast<int32_t>(curAbilityState));
        return;
    }

    // We need schedule application to foregrounded when current application state is ready or background running.
    if (curState_ == ApplicationState::APP_STATE_READY || curState_ == ApplicationState::APP_STATE_BACKGROUND) {
        if (foregroundingAbilityTokens_.empty()) {
            ScheduleForegroundRunning();
        }
        foregroundingAbilityTokens_.push_back(ability->GetToken());
        return;
    } else if (curState_ == ApplicationState::APP_STATE_FOREGROUND) {
        // Just change ability to foreground if current application state is foreground.
        auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
        moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
        StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true);
        auto serviceInner = appMgrServiceInner_.lock();
        if (serviceInner) {
            serviceInner->OnAppStateChanged(shared_from_this(), curState_);
        }
    } else {
        APP_LOGW("wrong application state");
    }
}

void AppRunningRecord::AbilityBackground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        APP_LOGE("ability is null");
        return;
    }
    if (ability->GetState() != AbilityState::ABILITY_STATE_FOREGROUND) {
        APP_LOGE("ability state is not foreground");
        return;
    }

    // First change ability to backgrounded.
    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_BACKGROUND);
    StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND), true);
    if (curState_ == ApplicationState::APP_STATE_FOREGROUND) {
        int32_t foregroundSize = 0;
        auto abilitysMap = GetAbilities();
        for (const auto &item : abilitysMap) {
            const auto &abilityRecord = item.second;
            if (abilityRecord && abilityRecord->GetState() == AbilityState::ABILITY_STATE_FOREGROUND) {
                foregroundSize++;
                break;
            }
        }

        // Then schedule application background when all ability is not foreground.
        if (foregroundSize == 0) {
            ScheduleBackgroundRunning();
        }
    } else {
        APP_LOGW("wrong application state");
    }
}

void AppRunningRecord::PopForegroundingAbilityTokens()
{
    APP_LOGI("size:%{public}d", static_cast<int32_t>(foregroundingAbilityTokens_.size()));
    while (!foregroundingAbilityTokens_.empty()) {
        const auto &token = foregroundingAbilityTokens_.front();
        auto ability = GetAbilityRunningRecordByToken(token);
        auto moduleRecord = GetModuleRunningRecordByToken(token);
        moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
        StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true);
        foregroundingAbilityTokens_.pop_front();
    }
}

void AppRunningRecord::TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce)
{
    APP_LOGI("Terminate ability, isForce: %{public}d", static_cast<int>(isForce));

    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        APP_LOGE("can not find module record");
        return;
    }

    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true);
    moduleRecord->TerminateAbility(token, isForce);
    APP_LOGI("AppRunningRecord::TerminateAbility end");
}

void AppRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    APP_LOGI("Ability terminated.");
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        APP_LOGE("Can not find module record");
        return;
    }
    moduleRecord->AbilityTerminated(token);

    if (moduleRecord->GetAbilities().empty()) {
        RemoveModuleRecord(moduleRecord);
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        ScheduleTerminate();
    }
}

std::list<std::shared_ptr<ModuleRunningRecord>> AppRunningRecord::GetAllModuleRecord() const
{
    std::list<std::shared_ptr<ModuleRunningRecord>> moduleRecordList;
    for (const auto &item : hapModules_) {
        for (const auto &list : item.second) {
            moduleRecordList.push_back(list);
        }
    }
    APP_LOGI("GetAllModuleRecord size:%{public}zu", moduleRecordList.size());
    return moduleRecordList;
}

void AppRunningRecord::RegisterAppDeathRecipient() const
{
    if (appLifeCycleDeal_ == nullptr) {
        APP_LOGE("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        APP_LOGE("appThread is nullptr");
        return;
    }
    auto object = appLifeCycleDeal_->GetApplicationClient()->AsObject();
    if (object) {
        object->AddDeathRecipient(appDeathRecipient_);
    }
}

void AppRunningRecord::RemoveAppDeathRecipient() const
{
    if (appLifeCycleDeal_ == nullptr) {
        APP_LOGE("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        APP_LOGE("appThread is nullptr.");
        return;
    }
    auto object = appLifeCycleDeal_->GetApplicationClient()->AsObject();
    if (object) {
        object->RemoveDeathRecipient(appDeathRecipient_);
    }
}

void AppRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
    appMgrServiceInner_ = inner;

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        APP_LOGE("moduleRecordList is empty");
        return;
    }

    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetAppMgrServiceInner(appMgrServiceInner_);
    }
}

void AppRunningRecord::SetAppDeathRecipient(const sptr<AppDeathRecipient> &appDeathRecipient)
{
    appDeathRecipient_ = appDeathRecipient;
}

std::shared_ptr<PriorityObject> AppRunningRecord::GetPriorityObject()
{
    if (!priorityObject_) {
        priorityObject_ = std::make_shared<PriorityObject>();
    }

    return priorityObject_;
}

void AppRunningRecord::SendEvent(uint32_t msg, int64_t timeOut)
{
    if (!eventHandler_) {
        APP_LOGE("eventHandler_ is nullptr");
        return;
    }

    appEventId_++;
    eventId_ = appEventId_;
    APP_LOGI("eventId %{public}d", static_cast<int>(eventId_));
    eventHandler_->SendEvent(msg, appEventId_, timeOut);
}

int64_t AppRunningRecord::GetEventId() const
{
    return eventId_;
}

void AppRunningRecord::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

bool AppRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    if (!token) {
        APP_LOGE("token is nullptr");
        return false;
    }

    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        APP_LOGE("can not find module record");
        return false;
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.size() == 1) {
        return moduleRecord->IsLastAbilityRecord(token);
    }

    return false;
}

void AppRunningRecord::SetTerminating()
{
    isTerminating = true;
}

bool AppRunningRecord::IsTerminating()
{
    return isTerminating;
}

bool AppRunningRecord::IsKeepAliveApp() const
{
    return isKeepAliveApp_;
}

void AppRunningRecord::SetKeepAliveAppState(bool isKeepAlive, bool isNewMission)
{
    isKeepAliveApp_ = isKeepAlive;
    isNewMission_ = isNewMission;
}

bool AppRunningRecord::GetTheModuleInfoNeedToUpdated(const std::string bundleName, HapModuleInfo &info)
{
    bool result = false;
    auto moduleInfoVectorIter = hapModules_.find(bundleName);
    if (moduleInfoVectorIter == hapModules_.end() || moduleInfoVectorIter->second.empty()) {
        return result;
    }
    auto findCondition = [](const std::shared_ptr<ModuleRunningRecord> &record) {
        if (record) {
            return record->GetModuleRecordState() == ModuleRecordState::INITIALIZED_STATE;
        }
        return false;
    };
    auto moduleRecordIter =
        std::find_if(moduleInfoVectorIter->second.begin(), moduleInfoVectorIter->second.end(), findCondition);
    if (moduleRecordIter != moduleInfoVectorIter->second.end()) {
        (*moduleRecordIter)->GetHapModuleInfo(info);
        (*moduleRecordIter)->SetModuleRecordState(ModuleRecordState::RUNNING_STATE);
        result = true;
    }

    return result;
}

void AppRunningRecord::SetRestartResidentProcCount(int count)
{
    restartResidentProcCount_ = count;
}

void AppRunningRecord::DecRestartResidentProcCount()
{
    restartResidentProcCount_--;
}

int AppRunningRecord::GetRestartResidentProcCount() const
{
    return restartResidentProcCount_;
}

bool AppRunningRecord::CanRestartResidentProc()
{
    return (restartResidentProcCount_ > 0);
}

void AppRunningRecord::GetBundleNames(std::vector<std::string> &bundleNames)
{
    for (auto &app : appInfos_) {
        bundleNames.emplace_back(app.first);
    }
}

void AppRunningRecord::SetUserTestInfo(const UserTestRecord &record)
{
    userTestRecord_ = record;
}

void AppRunningRecord::SetSpecifiedAbilityFlagAndWant(
    const bool flag, const AAFwk::Want &want, const std::string &moduleName)
{
    isSpecifiedAbility_ = flag;
    SpecifiedWant_ = want;
    moduleName_ = moduleName;
}

bool AppRunningRecord::IsStartSpecifiedAbility() const
{
    return isSpecifiedAbility_;
}

void AppRunningRecord::ScheduleAcceptWant(const std::string &moduleName)
{
    SendEvent(
        AMSEventHandler::START_MULTI_INSTANCES_ABILITY_MSG, AMSEventHandler::START_MULTI_INSTANCES_ABILITY_TIMEOUT);
    appLifeCycleDeal_->ScheduleAcceptWant(SpecifiedWant_, moduleName);
}

void AppRunningRecord::ScheduleAcceptWantDone()
{
    eventHandler_->RemoveEvent(AMSEventHandler::START_MULTI_INSTANCES_ABILITY_MSG, appRecordId_);
}

const AAFwk::Want &AppRunningRecord::GetSpecifiedWant() const
{
    return SpecifiedWant_;
}

void AppRunningRecord::UpdateConfiguration(const Configuration &config)
{
    APP_LOGI("call %{public}s", __func__);
    if (!appLifeCycleDeal_) {
        APP_LOGI("appLifeCycleDeal_ is null");
        return;
    }
    appLifeCycleDeal_->UpdateConfiguration(config);
}

void AppRunningRecord::SetRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
    renderRecord_ = record;
}

std::shared_ptr<RenderRecord> AppRunningRecord::GetRenderRecord()
{
    return renderRecord_;
}

void AppRunningRecord::SetStartMsg(const AppSpawnStartMsg &msg)
{
    startMsg_ = msg;
}

AppSpawnStartMsg AppRunningRecord::GetStartMsg()
{
    return startMsg_;
}
}  // namespace AppExecFwk
}  // namespace OHOS