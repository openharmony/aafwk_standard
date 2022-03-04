/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "app_mgr_service_inner.h"

#include <csignal>
#include <securec.h>
#include <sys/stat.h>
#include <unistd.h>

#include "app_log_wrapper.h"
#include "application_state_observer_stub.h"
#include "datetime_ex.h"
#include "perf_profile.h"

#include "app_process_data.h"
#include "bundle_constants.h"
#include "bytrace.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "os_account_manager.h"
#include "permission/permission_kit.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "locale_config.h"
#include "uri_permission_manager_client.h"


namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Security;

namespace {
// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 millias second
constexpr int64_t MICROSECONDS = 1000000;
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delaytime setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
const std::string CLASS_NAME = "ohos.app.MainThread";
const std::string FUNC_NAME = "main";
const std::string SO_PATH = "system/lib64/libmapleappkit.z.so";
const std::string RENDER_PARAM = "invalidparam";
const int32_t SIGNAL_KILL = 9;
const std::string REQ_PERMISSION = "ohos.permission.LOCATION_IN_BACKGROUND";
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t USER_SCALE = 200000;
#define ENUM_TO_STRING(s) #s

constexpr int32_t BASE_USER_RANGE = 200000;

constexpr ErrCode APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, 0x01);
constexpr ErrCode ERR_ALREADY_EXIST_RENDER = APPMGR_ERR_OFFSET + 100; // error code for already exist render.
const std::string EVENT_NAME_LIFECYCLE_TIMEOUT = "LIFECYCLE_TIMEOUT";
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";

// Msg length is less than 48 characters
const std::string EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT = "Terminate Ability TimeOut!";
const std::string EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT = "Terminate Application TimeOut!";
const std::string EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT = "Add Ability Stage TimeOut!";
const std::string EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT = "Start Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT = "Start Process Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_DEFAULT = "AppMgrServiceInner HandleTimeOut!";


int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}
}  // namespace

using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;
using OHOS::AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
using OHOS::AppExecFwk::Constants::ROOT_UID;

AppMgrServiceInner::AppMgrServiceInner()
    : appProcessManager_(std::make_shared<AppProcessManager>()),
      remoteClientManager_(std::make_shared<RemoteClientManager>()),
      appRunningManager_(std::make_shared<AppRunningManager>()),
      configuration_(std::make_shared<Configuration>())
{}

void AppMgrServiceInner::Init()
{
    GetGlobalConfiguration();
}

AppMgrServiceInner::~AppMgrServiceInner()
{}

void AppMgrServiceInner::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AAFwk::Want> &want)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!CheckLoadabilityConditions(token, abilityInfo, appInfo)) {
        APP_LOGE("CheckLoadabilityConditions failed");
        return;
    }

    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    if (!GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo)) {
        APP_LOGE("GetBundleAndHapInfo failed");
        return;
    }

    std::string processName;
    MakeProcessName(processName, abilityInfo, appInfo, hapModuleInfo);
    APP_LOGI("processName = [%{public}s]", processName.c_str());

    auto appRecord =
        appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        appRecord =
            CreateAppRunningRecord(token, preToken, appInfo, abilityInfo,
                processName, bundleInfo, hapModuleInfo, want);
        if (!appRecord) {
            APP_LOGE("CreateAppRunningRecord failed, appRecord is nullptr");
            return;
        }
        bool isColdStart = want == nullptr ? false : want->GetBoolParam("coldStart", false);
        StartProcess(abilityInfo->applicationName, processName, isColdStart, appRecord,
            abilityInfo->applicationInfo.uid, abilityInfo->applicationInfo.bundleName);
    } else {
        StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want);
    }
    PerfProfile::GetInstance().SetAbilityLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
    PerfProfile::GetInstance().Reset();
}

bool AppMgrServiceInner::CheckLoadabilityConditions(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    if (!token || !abilityInfo || !appInfo) {
        APP_LOGE("param error");
        return false;
    }
    if (abilityInfo->name.empty() || appInfo->name.empty()) {
        APP_LOGE("error abilityInfo or appInfo");
        return false;
    }
    if (abilityInfo->applicationName != appInfo->name) {
        APP_LOGE("abilityInfo and appInfo have different appName, don't load for it");
        return false;
    }

    return true;
}

void AppMgrServiceInner::MakeProcessName(std::string &processName, const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, HapModuleInfo &hapModuleInfo)
{
    if (!abilityInfo || !appInfo) {
        return;
    }
    if (!abilityInfo->process.empty()) {
        processName = abilityInfo->process;
        return;
    }
    MakeProcessName(processName, appInfo, hapModuleInfo);
}

void AppMgrServiceInner::MakeProcessName(
    std::string &processName, const std::shared_ptr<ApplicationInfo> &appInfo, HapModuleInfo &hapModuleInfo)
{
    if (!appInfo) {
        return;
    }
    if (!appInfo->process.empty()) {
        processName = appInfo->process;
        return;
    }
    // check after abilityInfo, because abilityInfo contains extension process.
    if (hapModuleInfo.isStageBasedModel && !hapModuleInfo.process.empty()) {
        processName = hapModuleInfo.process;
        APP_LOGI("Stage mode, Make processName:%{public}s", processName.c_str());
        return;
    }
    processName = appInfo->bundleName;
}

bool AppMgrServiceInner::GetBundleAndHapInfo(const AbilityInfo &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    APP_LOGI("AppMgrServiceInner GetBundleAndHapInfo start!");
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return false;
    }

    auto userId = GetUserIdByUid(abilityInfo.applicationInfo.uid);
    bool bundleMgrResult = bundleMgr_->GetBundleInfo(appInfo->bundleName,
        BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId);
    if (!bundleMgrResult) {
        APP_LOGE("GetBundleInfo is fail");
        return false;
    }
    bundleMgrResult = bundleMgr_->GetHapModuleInfo(abilityInfo, hapModuleInfo);
    if (!bundleMgrResult) {
        APP_LOGE("GetHapModuleInfo is fail");
        return false;
    }

    return true;
}

void AppMgrServiceInner::AttachApplication(const pid_t pid, const sptr<IAppScheduler> &app)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (pid <= 0) {
        APP_LOGE("invalid pid:%{public}d", pid);
        return;
    }
    if (!app) {
        APP_LOGE("app client is null");
        return;
    }
    APP_LOGI("attach application pid:%{public}d", pid);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("no such appRecord");
        return;
    }
    appRecord->SetApplicationClient(app);
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        LaunchApplication(appRecord);
    }
    appRecord->RegisterAppDeathRecipient();
}

void AppMgrServiceInner::LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        APP_LOGE("appRecord is null");
        return;
    }

    if (!configuration_) {
        APP_LOGE("configuration_ is null");
        return;
    }

    if (appRecord->GetState() != ApplicationState::APP_STATE_CREATE) {
        APP_LOGE("wrong app state:%{public}d", appRecord->GetState());
        return;
    }
    appRecord->LaunchApplication(*configuration_);
    appRecord->SetState(ApplicationState::APP_STATE_READY);

    // There is no ability when the resident process starts
    // The status of all resident processes is ready
    // There is no process of switching the foreground, waiting for his first ability to start
    if (appRecord->IsKeepAliveApp()) {
        appRecord->AddAbilityStage();
        return;
    }

    if (appRecord->IsStartSpecifiedAbility()) {
        appRecord->AddAbilityStageBySpecifiedAbility(appRecord->GetBundleName());
        return;
    }
    appRecord->LaunchPendingAbilities();
}

void AppMgrServiceInner::AddAbilityStageDone(const int32_t recordId)
{
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    appRecord->AddAbilityStageDone();
}

void AppMgrServiceInner::ApplicationForegrounded(const int32_t recordId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    appRecord->PopForegroundingAbilityTokens();
    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_READY || appState == ApplicationState::APP_STATE_BACKGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND);
    } else if (appState == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    } else {
        APP_LOGW("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(),
            static_cast<ApplicationState>(appState));
    }

    // push the foregrounded app front of RecentAppList.
    PushAppFront(recordId);
    APP_LOGI("application is foregrounded");
}

void AppMgrServiceInner::ApplicationBackgrounded(const int32_t recordId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND);
    } else if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    } else {
        APP_LOGW("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(),
            static_cast<ApplicationState>(appRecord->GetState()));
    }

    APP_LOGI("application is backgrounded");
}

void AppMgrServiceInner::ApplicationTerminated(const int32_t recordId)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("get app record failed");
        return;
    }
    // Maybe can't get in here
    if (appRecord->IsKeepAliveApp()) {
        return;
    }
    if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
        APP_LOGE("current state is not background");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED);
    appRunningManager_->RemoveAppRunningRecordById(recordId);
    RemoveAppFromRecentListById(recordId);
    OnProcessDied(appRecord);

    APP_LOGI("application is terminated");
}

int32_t AppMgrServiceInner::KillApplication(const std::string &bundleName)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    if (VerifyProcessPermission() == ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;

    if (!appRunningManager_->ProcessExitByBundleName(bundleName, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    if (VerifyProcessPermission() == ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        APP_LOGE("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    APP_LOGI("uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUserId(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES, userId) ==
        ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        APP_LOGE("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_NO_INIT;
    }

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!bundleMgr_->CheckIsSystemAppByUid(callerUid)) {
        APP_LOGE("caller is not systemApp, callerUid %{public}d", callerUid);
        return ERR_INVALID_VALUE;
    }

    APP_LOGI("userId value is %{public}d", userId);
    int uid = bundleMgr_->GetUidByBundleName(bundleName, userId);
    APP_LOGI("uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        APP_LOGI("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

void AppMgrServiceInner::ClearUpApplicationData(const std::string &bundleName, int32_t callerUid, pid_t callerPid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto userId = GetUserIdByUid(callerUid);
    APP_LOGI("userId:%{public}d", userId);
    ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, userId);
}

void AppMgrServiceInner::ClearUpApplicationDataByUserId(
    const std::string &bundleName, int32_t callerUid, pid_t callerPid, const int userId)
{
    if (callerPid <= 0) {
        APP_LOGE("invalid callerPid:%{public}d", callerPid);
        return;
    }
    if (callerUid <= 0) {
        APP_LOGE("invalid callerUid:%{public}d", callerUid);
        return;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return;
    }

    // request to clear user information permission.
    int32_t result =
        Permission::PermissionKit::RemoveUserGrantedReqPermissions(bundleName, userId);
    if (result) {
        APP_LOGE("RemoveUserGrantedReqPermissions failed");
        return;
    }
    // 2.delete bundle side user data
    if (!bundleMgr_->CleanBundleDataFiles(bundleName, userId)) {
        APP_LOGE("Delete bundle side user data is fail");
        return;
    }
    // 3.kill application
    // 4.revoke user rights
    result = KillApplicationByUserId(bundleName, userId);
    if (result < 0) {
        APP_LOGE("Kill Application by bundle name is fail");
        return;
    }
    NotifyAppStatusByCallerUid(bundleName, userId, callerUid,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
}

int32_t AppMgrServiceInner::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();

    std::vector<int32_t> ids;
    auto result = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    APP_LOGD("ids size : %{public}d", static_cast<int>(ids.size()));

    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        int32_t userId = static_cast<int32_t>(appRecord->GetUid() / USER_SCALE);
        if ((std::find(ids.begin(), ids.end(), userId) != ids.end()) && (result == ERR_OK)) {
            if (isPerm) {
                GetRunningProcesses(appRecord, info);
            } else {
                auto applicationInfo = appRecord->GetApplicationInfo();
                if (!applicationInfo) {
                    continue;
                }
                auto callingTokenId = IPCSkeleton::GetCallingTokenID();
                auto tokenId = applicationInfo->accessTokenId;
                if (callingTokenId == tokenId) {
                    GetRunningProcesses(appRecord, info);
                }
            }
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        int32_t userIdTemp = static_cast<int32_t>(appRecord->GetUid() / USER_SCALE);
        if (userIdTemp == userId) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

void AppMgrServiceInner::GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RunningProcessInfo> &info)
{
    RunningProcessInfo runningProcessInfo;
    runningProcessInfo.processName_ = appRecord->GetProcessName();
    runningProcessInfo.pid_ = appRecord->GetPriorityObject()->GetPid();
    runningProcessInfo.uid_ = appRecord->GetUid();
    runningProcessInfo.state_ = static_cast<AppProcessState>(appRecord->GetState());
    appRecord->GetBundleNames(runningProcessInfo.bundleNames);
    info.emplace_back(runningProcessInfo);
}

int32_t AppMgrServiceInner::KillProcessByPid(const pid_t pid) const
{
    int32_t ret = -1;
    if (pid > 0) {
        APP_LOGI("kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
    }
    return ret;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillis() - startTime;
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        if (CheckAllProcessExist(pids)) {
            return true;
        }
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        delayTime = SystemTimeMillis() - startTime;
    }
    return false;
}

bool AppMgrServiceInner::GetAllPids(std::list<pid_t> &pids)
{
    for (const auto &appTaskInfo : appProcessManager_->GetRecentAppList()) {
        if (appTaskInfo) {
            auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
            if (appRecord) {
                pids.push_back(appTaskInfo->GetPid());
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }
    return (pids.empty() ? false : true);
}

bool AppMgrServiceInner::process_exist(pid_t &pid)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/status", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) == 0) {
        return true;
    }
    return false;
}

bool AppMgrServiceInner::CheckAllProcessExist(std::list<pid_t> &pids)
{
    for (auto iter = pids.begin(); iter != pids.end();) {
        if (!process_exist(*iter)) {
            iter = pids.erase(iter);
        } else {
            iter++;
        }
    }
    if (pids.empty()) {
        return true;
    }
    return false;
}

int64_t AppMgrServiceInner::SystemTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByPid(const pid_t pid) const
{
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::CreateAppRunningRecord(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        return nullptr;
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    if (!appRecord) {
        return nullptr;
    }

    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want);
    if (want) {
        appRecord->SetDebugApp(want->GetBoolParam("debugApp", false));
    }

    if (preToken) {
        auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
        if (abilityRecord) {
            abilityRecord->SetPreToken(preToken);
        }
    }

    return appRecord;
}

void AppMgrServiceInner::TerminateAbility(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("AppMgrServiceInner::TerminateAbility begin");
    if (!token) {
        APP_LOGE("AppMgrServiceInner::TerminateAbility token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("AppMgrServiceInner::TerminateAbility app is not exist!");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    }

    if (appRunningManager_) {
        appRunningManager_->TerminateAbility(token);
    }
}

void AppMgrServiceInner::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("can not find ability record!");
        return;
    }
    if (state == abilityRecord->GetState()) {
        APP_LOGE("current state is already, no need update!");
        return;
    }
    auto type = abilityRecord->GetAbilityInfo()->type;
    if (type == AppExecFwk::AbilityType::SERVICE &&
        (state == AbilityState::ABILITY_STATE_CREATE ||
        state == AbilityState::ABILITY_STATE_TERMINATED ||
        state == AbilityState::ABILITY_STATE_CONNECTED ||
        state == AbilityState::ABILITY_STATE_DISCONNECTED)) {
        APP_LOGI("StateChangedNotifyObserver service type, state:%{public}d", static_cast<int32_t>(state));
        appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), true);
        return;
    }
    if (state > AbilityState::ABILITY_STATE_BACKGROUND || state < AbilityState::ABILITY_STATE_FOREGROUND) {
        APP_LOGE("state is not foreground or background!");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    }

    appRecord->UpdateAbilityState(token, state);
}

void AppMgrServiceInner::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("can not find ability record!");
        return;
    }
    appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), false);
}

void AppMgrServiceInner::OnStop()
{
    appRunningManager_->ClearAppRunningRecordMap();
    CloseAppSpawnConnection();
}

ErrCode AppMgrServiceInner::OpenAppSpawnConnection()
{
    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->OpenConnection();
    }
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppMgrServiceInner::CloseAppSpawnConnection() const
{
    if (remoteClientManager_->GetSpawnClient()) {
        remoteClientManager_->GetSpawnClient()->CloseConnection();
    }
}

SpawnConnectionState AppMgrServiceInner::QueryAppSpawnConnectionState() const
{
    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->QueryConnectionState();
    }
    return SpawnConnectionState::STATE_NOT_CONNECT;
}

const std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> &AppMgrServiceInner::GetRecordMap() const
{
    return appRunningManager_->GetAppRunningRecordMap();
}

void AppMgrServiceInner::SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient)
{
    remoteClientManager_->SetSpawnClient(std::move(spawnClient));
}

void AppMgrServiceInner::SetBundleManager(sptr<IBundleMgr> bundleManager)
{
    remoteClientManager_->SetBundleManager(bundleManager);
}

void AppMgrServiceInner::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        APP_LOGE("%{public}s: Not abilityMgr call.", __func__);
        return;
    }
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (callback != nullptr) {
        appStateCallbacks_.push_back(callback);
    }
}

void AppMgrServiceInner::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility,       // 0:false,1:true
    const int32_t perceptibility,   // 0:false,1:true
    const int32_t connectionState)  // 0:false,1:true
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        APP_LOGE("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app record is not exist for ability token");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        APP_LOGE("ability record is not exist for ability previous token");
        return;
    }
    if (preToken) {
        abilityRecord->SetPreToken(preToken);
    }
    abilityRecord->SetVisibility(visibility);
    abilityRecord->SetPerceptibility(perceptibility);
    abilityRecord->SetConnectionState(connectionState);
}

void AppMgrServiceInner::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        APP_LOGE("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        APP_LOGE("app record is not exist for ability token");
        return;
    }

    // befor exec ScheduleProcessSecurityExit return
    // The resident process won't let him die
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    std::list<pid_t> pids;
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillis())) {
            int32_t result = KillProcessByPid(pid);
            if (result < 0) {
                APP_LOGE("KillProcessByAbilityToken kill process is fail");
                return;
            }
        }
    }
}

void AppMgrServiceInner::KillProcessesByUserId(int32_t userId)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    int64_t startTime = SystemTimeMillis();
    std::list<pid_t> pids;
    if (!appRunningManager_->GetPidsByUserId(userId, pids)) {
        APP_LOGI("The process corresponding to the userId did not start");
        return;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGI("The remote process exited successfully ");
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("KillProcessByPid is failed. pid: %{public}d", *iter);
            return;
        }
    }
}

void AppMgrServiceInner::StartAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<AppRunningRecord> &appRecord,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("already create appRecord, just start ability");
    if (!appRecord) {
        APP_LOGE("appRecord is null");
        return;
    }

    if (abilityInfo->launchMode == LaunchMode::SINGLETON) {
        auto abilityRecord = appRecord->GetAbilityRunningRecord(abilityInfo->name);
        if (abilityRecord) {
            APP_LOGW("same ability info in singleton launch mode, will not add ability");
            return;
        }
    }

    auto ability = appRecord->GetAbilityRunningRecordByToken(token);
    if (ability && preToken) {
        APP_LOGE("Ability is already started");
        ability->SetPreToken(preToken);
        return;
    }

    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_SUSPENDED) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    }

    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    if (!moduleRecord) {
        APP_LOGE("add moduleRecord failed");
        return;
    }

    ability = moduleRecord->GetAbilityRunningRecordByToken(token);
    if (!ability) {
        APP_LOGE("add ability failed");
        return;
    }

    if (preToken != nullptr) {
        ability->SetPreToken(preToken);
    }

    if (appState == ApplicationState::APP_STATE_CREATE) {
        APP_LOGE("in create state, don't launch ability");
        return;
    }
    appRecord->LaunchAbility(ability);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

void AppMgrServiceInner::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGD("begin");
    if (!token) {
        APP_LOGE("token is null!");
        return;
    }

    auto appRecord = appRunningManager_->GetTerminatingAppRunningRecord(token);
    if (!appRecord) {
        APP_LOGE("app is not exist!");
        return;
    }

    appRecord->AbilityTerminated(token);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppRecordId(const int32_t recordId) const
{
    const auto &iter = appRunningManager_->GetAppRunningRecordMap().find(recordId);
    if (iter != appRunningManager_->GetAppRunningRecordMap().end()) {
        return iter->second;
    }
    return nullptr;
}

void AppMgrServiceInner::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state)
{
    APP_LOGD("begin, state:%{public}d", static_cast<int32_t>(state));
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }

    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAppStateChanged(WrapAppProcessData(appRecord, state));
        }
    }

    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        AppStateData data = WrapAppStateData(appRecord, state);
        APP_LOGD("OnForegroundApplicationChanged, size:%{public}d, name:%{public}s, uid:%{public}d, state:%{public}d",
            (int32_t)appStateObservers_.size(), data.bundleName.c_str(), data.uid, data.state);
        std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
        for (const auto &observer : appStateObservers_) {
            if (observer != nullptr) {
                observer->OnForegroundApplicationChanged(data);
            }
        }
    }

    if (state == ApplicationState::APP_STATE_CREATE || state == ApplicationState::APP_STATE_TERMINATED) {
        AppStateData data = WrapAppStateData(appRecord, state);
        APP_LOGI("OnApplicationStateChanged, size:%{public}d, name:%{public}s, uid:%{public}d, state:%{public}d",
            (int32_t)appStateObservers_.size(), data.bundleName.c_str(), data.uid, data.state);
        std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
        for (const auto &observer : appStateObservers_) {
            if (observer != nullptr) {
                observer->OnApplicationStateChanged(data);
            }
        }
    }
}

AppProcessData AppMgrServiceInner::WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppProcessData processData;
    auto appInfoList = appRecord->GetAppInfoList();
    for (const auto &list : appInfoList) {
        AppData data;
        data.appName = list->name;
        data.uid = list->uid;
        processData.appDatas.push_back(data);
    }
    processData.processName = appRecord->GetProcessName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.appState = state;
    return processData;
}

AppStateData AppMgrServiceInner::WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppStateData appStateData;
    appStateData.bundleName = appRecord->GetBundleName();
    appStateData.state = static_cast<int32_t>(state);
    appStateData.uid = appRecord->GetUid();
    return appStateData;
}

ProcessData AppMgrServiceInner::WrapProcessData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    ProcessData processData;
    processData.bundleName = appRecord->GetBundleName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.uid = appRecord->GetUid();
    return processData;
}

void AppMgrServiceInner::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    APP_LOGD("begin, state:%{public}d", static_cast<int32_t>(state));
    if (!ability) {
        APP_LOGE("ability is null");
        return;
    }
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityRequestDone(ability->GetToken(), state);
        }
    }
}

void AppMgrServiceInner::StateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility)
{
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    APP_LOGD("bundle:%{public}s, ability:%{public}s, state:%{public}d, pid:%{public}d,"
        "uid:%{public}d, abilityType:%{public}d",
        abilityStateData.bundleName.c_str(), abilityStateData.abilityName.c_str(),
        abilityStateData.abilityState, abilityStateData.pid, abilityStateData.uid,
        abilityStateData.abilityType);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            if (isAbility) {
                observer->OnAbilityStateChanged(abilityStateData);
            } else {
                observer->OnExtensionStateChanged(abilityStateData);
            }
        }
    }
}

void AppMgrServiceInner::OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGD("OnProcessCreated begin.");
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    APP_LOGD("OnProcessCreated, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.uid, data.pid, (int32_t)appStateObservers_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            observer->OnProcessCreated(data);
        }
    }
    APP_LOGD("end");
}

void AppMgrServiceInner::OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGD("OnProcessDied begin.");
    if (!appRecord) {
        APP_LOGE("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    APP_LOGD("OnProcessDied, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.uid, data.pid, (int32_t)appStateObservers_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (const auto &observer : appStateObservers_) {
        if (observer != nullptr) {
            observer->OnProcessDied(data);
        }
    }
    APP_LOGD("end");
}

void AppMgrServiceInner::StartProcess(const std::string &appName, const std::string &processName, bool coldStart,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int uid, const std::string &bundleName)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!remoteClientManager_->GetSpawnClient() || !appRecord) {
        APP_LOGE("appSpawnClient or apprecord is null");
        return;
    }

    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return;
    }

    auto userId = GetUserIdByUid(uid);
    AppSpawnStartMsg startMsg;
    BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool bundleMgrResult = bundleMgr_->GetBundleInfos(AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfos, userId);
    if (!bundleMgrResult) {
        APP_LOGE("GetBundleInfo is fail");
        return;
    }

    auto isExist = [&bundleName, &uid](const AppExecFwk::BundleInfo &bundleInfo) {
        return bundleInfo.name == bundleName && bundleInfo.uid == uid;
    };
    auto bundleInfoIter = std::find_if(bundleInfos.begin(), bundleInfos.end(), isExist);
    if (bundleInfoIter == bundleInfos.end()) {
        APP_LOGE("Get target fail.");
        return;
    }
    startMsg.uid = (*bundleInfoIter).uid;
    startMsg.gid = (*bundleInfoIter).gid;
    startMsg.accessTokenId = (*bundleInfoIter).applicationInfo.accessTokenId;
    startMsg.apl = (*bundleInfoIter).applicationInfo.appPrivilegeLevel;
    startMsg.bundleName = bundleName;
    startMsg.renderParam = RENDER_PARAM;
    startMsg.coldStart = coldStart;
    APP_LOGD("StartProcess accessTokenId:%{public}d, apl:%{public}s, bundleName:%{public}s coldStart:%{public}d",
        startMsg.accessTokenId, startMsg.apl.c_str(), bundleName.c_str(), coldStart);

    bundleMgrResult = bundleMgr_->GetBundleGidsByUid(bundleName, uid, startMsg.gids);
    if (!bundleMgrResult) {
        APP_LOGE("GetBundleGids is fail");
        return;
    }
    startMsg.procName = processName;
    startMsg.soPath = SO_PATH;

    PerfProfile::GetInstance().SetAppForkStartTime(GetTickCount());
    pid_t pid = 0;
    ErrCode errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        APP_LOGE("failed to spawn new app process, errCode %{public}08x", errCode);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }
    APP_LOGI("newPid:%{public}d uid:%{public}d", pid, startMsg.uid);
    appRecord->GetPriorityObject()->SetPid(pid);
    appRecord->SetUid(startMsg.uid);
    appRecord->SetStartMsg(startMsg);
    appRecord->SetAppMgrServiceInner(weak_from_this());
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE);
    AddAppToRecentList(appName, appRecord->GetProcessName(), pid, appRecord->GetRecordId());
    OnProcessCreated(appRecord);
    PerfProfile::GetInstance().SetAppForkEndTime(GetTickCount());
}

void AppMgrServiceInner::RemoveAppFromRecentList(const std::string &appName, const std::string &processName)
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    auto appTaskInfo = appProcessManager_->GetAppTaskInfoByProcessName(appName, processName);
    if (!appTaskInfo) {
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
    if (!appRecord) {
        appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
        return;
    }

    // Do not delete resident processes, berfor exec ScheduleProcessSecurityExit
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    startTime = SystemTimeMillis();
    pids.push_back(appTaskInfo->GetPid());
    appRecord->ScheduleProcessSecurityExit();
    if (!WaitForRemoteProcessExit(pids, startTime)) {
        int32_t result = KillProcessByPid(appTaskInfo->GetPid());
        if (result < 0) {
            APP_LOGE("RemoveAppFromRecentList kill process is fail");
            return;
        }
    }
    appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
}

const std::list<const std::shared_ptr<AppTaskInfo>> &AppMgrServiceInner::GetRecentAppList() const
{
    return appProcessManager_->GetRecentAppList();
}

void AppMgrServiceInner::ClearRecentAppList()
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    if (GetAllPids(pids)) {
        return;
    }

    startTime = SystemTimeMillis();
    if (WaitForRemoteProcessExit(pids, startTime)) {
        appProcessManager_->ClearRecentAppList();
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        int32_t result = KillProcessByPid(*iter);
        if (result < 0) {
            APP_LOGE("ClearRecentAppList kill process is fail");
            return;
        }
    }
    appProcessManager_->ClearRecentAppList();
}

void AppMgrServiceInner::OnRemoteDied(const wptr<IRemoteObject> &remote, bool isRenderProcess)
{
    APP_LOGE("On remote died.");
    if (isRenderProcess) {
        OnRenderRemoteDied(remote);
        return;
    }

    auto appRecord = appRunningManager_->OnRemoteDied(remote);
    if (!appRecord) {
        return;
    }

    FinishUserTestLocked("App died", -1, appRecord);

    // clear uri permission
    auto upmClient = AAFwk::UriPermissionManagerClient::GetInstance();
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo && upmClient) {
        upmClient->RemoveUriPermission(appInfo->accessTokenId);
    }

    for (const auto &item : appRecord->GetAbilities()) {
        const auto &abilityRecord = item.second;
        appRecord->StateChangedNotifyObserver(abilityRecord,
            static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true);
    }
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    OnProcessDied(appRecord);

    // kill render if exist.
    auto renderRecord = appRecord->GetRenderRecord();
    if (renderRecord && renderRecord->GetPid() > 0) {
        APP_LOGD("Kill render process when webviehost died.");
        KillProcessByPid(renderRecord->GetPid());
    }

    if (appRecord->IsKeepAliveApp()) {
        appRecord->DecRestartResidentProcCount();
        if (appRecord->CanRestartResidentProc()) {
            auto restartProcss = [appRecord, innerService = shared_from_this()]() {
                innerService->RestartResidentProcess(appRecord);
            };

            if (!eventHandler_) {
                APP_LOGE("eventHandler_ is nullptr");
                return;
            }
            eventHandler_->PostTask(restartProcss, "RestartResidentProcess");
        }
    }
}

void AppMgrServiceInner::PushAppFront(const int32_t recordId)
{
    appProcessManager_->PushAppFront(recordId);
}

void AppMgrServiceInner::RemoveAppFromRecentListById(const int32_t recordId)
{
    appProcessManager_->RemoveAppFromRecentListById(recordId);
}

void AppMgrServiceInner::AddAppToRecentList(
    const std::string &appName, const std::string &processName, const pid_t pid, const int32_t recordId)
{
    appProcessManager_->AddAppToRecentList(appName, processName, pid, recordId);
}

const std::shared_ptr<AppTaskInfo> AppMgrServiceInner::GetAppTaskInfoById(const int32_t recordId) const
{
    return appProcessManager_->GetAppTaskInfoById(recordId);
}

void AppMgrServiceInner::AddAppDeathRecipient(const pid_t pid, const sptr<AppDeathRecipient> &appDeathRecipient) const
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::shared_ptr<AppRunningRecord> appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord) {
        appRecord->SetAppDeathRecipient(appDeathRecipient);
    }
}

void AppMgrServiceInner::HandleTimeOut(const InnerEvent::Pointer &event)
{
    APP_LOGI("handle time out");
    if (!appRunningManager_ || event == nullptr) {
        APP_LOGE("appRunningManager or event is nullptr");
        return;
    }
    SendHiSysEvent(event->GetInnerEventId(), event->GetParam());
    switch (event->GetInnerEventId()) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            appRunningManager_->HandleTerminateTimeOut(event->GetParam());
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            HandleTerminateApplicationTimeOut(event->GetParam());
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            HandleAddAbilityStageTimeOut(event->GetParam());
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            HandleStartSpecifiedAbilityTimeOut(event->GetParam());
            break;
        default:
            break;
    }
}

void AppMgrServiceInner::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

void AppMgrServiceInner::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s called", __func__);
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->HandleAbilityAttachTimeOut(token);
}

void AppMgrServiceInner::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    APP_LOGI("Prepare terminate");
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->PrepareTerminate(token);
}

void AppMgrServiceInner::HandleTerminateApplicationTimeOut(const int64_t eventId)
{
    APP_LOGI("handle terminate application time out");
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr");
        return;
    }

    auto abilityRecord = appRecord->GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        APP_LOGE("abilityRecord is nullptr");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED);
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        int32_t result = KillProcessByPid(pid);
        if (result < 0) {
            APP_LOGE("KillProcessByAbilityToken kill process is fail");
            return;
        }
    }
    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    OnProcessDied(appRecord);
}

void AppMgrServiceInner::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    APP_LOGI("called add ability stage info time out!");
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    KillApplicationByRecord(appRecord);
}

int AppMgrServiceInner::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    APP_LOGI("compel verify permission");
    message = ENUM_TO_STRING(PERMISSION_NOT_GRANTED);
    if (!remoteClientManager_) {
        APP_LOGE("remoteClientManager_ is nullptr");
        return ERR_NO_INIT;
    }
    if (permission.empty()) {
        APP_LOGI("permission is empty, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    if (pid == getpid()) {
        APP_LOGI("pid is my pid, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    int userId = Constants::DEFAULT_USERID;
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("app record is nullptr");
        return PERMISSION_NOT_GRANTED;
    }
    auto bundleName = appRecord->GetBundleName();
    if (appRecord->GetCloneInfo()) {
        userId = Constants::C_UESRID;
    }
    auto bundleMgr = remoteClientManager_->GetBundleManager();
    if (bundleMgr == nullptr) {
        APP_LOGE("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    auto bmsUid = bundleMgr->GetUidByBundleName(bundleName, userId);
    if (bmsUid == ROOT_UID || bmsUid == SYSTEM_UID) {
        APP_LOGI("uid is root or system, PERMISSION_GRANTED");
        message = ENUM_TO_STRING(PERMISSION_GRANTED);
        return ERR_OK;
    }
    if (bmsUid != uid) {
        APP_LOGI("check uid != bms uid, PERMISSION_NOT_GRANTED");
        return PERMISSION_NOT_GRANTED;
    }
    auto result = bundleMgr->CheckPermissionByUid(bundleName, permission, userId);
    if (result != PERMISSION_GRANTED) {
        return PERMISSION_NOT_GRANTED;
    }
    message = ENUM_TO_STRING(PERMISSION_GRANTED);
    return ERR_OK;
}

void AppMgrServiceInner::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    APP_LOGI("%{public}s called", __func__);
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return;
    }

    appRunningManager_->GetRunningProcessInfoByToken(token, info);
}

void AppMgrServiceInner::LoadResidentProcess(const std::vector<AppExecFwk::BundleInfo> &infos)
{
    APP_LOGI("%{public}s called", __func__);
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        APP_LOGE("%{public}s: Not SA call.", __func__);
        return;
    }

    APP_LOGI("bundle info size: [%{public}d]", static_cast<int>(infos.size()));
    StartResidentProcess(infos, -1);
}

void AppMgrServiceInner::StartResidentProcess(const std::vector<BundleInfo> &infos, int restartCount)
{
    APP_LOGI("start resident process");
    if (infos.empty()) {
        APP_LOGE("infos is empty!");
        return;
    }

    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    for (auto &bundle : infos) {
        auto processName = bundle.applicationInfo.process.empty() ?
            bundle.applicationInfo.bundleName : bundle.applicationInfo.process;
        APP_LOGI("processName = [%{public}s]", processName.c_str());

        // Inspection records
        auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
            bundle.applicationInfo.name, processName, bundle.applicationInfo.uid, bundle);
        if (appRecord) {
            APP_LOGI("processName [%{public}s] Already exists ", processName.c_str());
            continue;
        }
        StartEmptyResidentProcess(bundle, processName, restartCount);
    }
}

void AppMgrServiceInner::StartEmptyResidentProcess(
    const BundleInfo &info, const std::string &processName, int restartCount)
{
    APP_LOGI("start bundle [%{public}s | processName [%{public}s]]", info.name.c_str(), processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        APP_LOGI("Failed to start resident process!");
        return;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        APP_LOGE("start process [%{public}s] failed!", processName.c_str());
        return;
    }

    StartProcess(appInfo->name, processName, false, appRecord, appInfo->uid, appInfo->bundleName);

    // If it is empty, the startup failed
    if (!appRecord) {
        APP_LOGE("start process [%{public}s] failed!", processName.c_str());
        return;
    }

    bool isStageBased = false;
    bool moduelJson = false;
    if (!info.hapModuleInfos.empty()) {
        isStageBased = info.hapModuleInfos.back().isStageBasedModel;
        moduelJson = info.hapModuleInfos.back().isModuleJson;
    }
    APP_LOGI("StartEmptyResidentProcess stage:%{public}d moduel:%{public}d size:%{public}d",
        isStageBased, moduelJson, (int32_t)info.hapModuleInfos.size());
    appRecord->SetKeepAliveAppState(true, isStageBased);

    if (restartCount > 0) {
        APP_LOGI("StartEmptyResidentProcess restartCount : [%{public}d], ", restartCount);
        appRecord->SetRestartResidentProcCount(restartCount);
    }

    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    APP_LOGI("StartEmptyResidentProcess oK pid : [%{public}d], ", appRecord->GetPriorityObject()->GetPid());
}

bool AppMgrServiceInner::CheckRemoteClient()
{
    if (!remoteClientManager_) {
        APP_LOGE("remoteClientManager_ is null");
        return false;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        APP_LOGE("appSpawnClient is null");
        return false;
    }

    if (!remoteClientManager_->GetBundleManager()) {
        APP_LOGE("GetBundleManager fail");
        return false;
    }
    return true;
}

void AppMgrServiceInner::RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        APP_LOGE("restart resident process failed!");
        return;
    }

    auto bundleMgr = remoteClientManager_->GetBundleManager();
    BundleInfo bundleInfo;
    if (!bundleMgr->GetBundleInfo(appRecord->GetBundleName(), BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo)) {
        APP_LOGE("GetBundleInfo fail");
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    APP_LOGI("the resident process [%{public}s] remaining restarts num is [%{public}d]",
        appRecord->GetProcessName().c_str(), (int)appRecord->GetRestartResidentProcCount());
    StartResidentProcess(infos, appRecord->GetRestartResidentProcCount());
}

void AppMgrServiceInner::NotifyAppStatus(const std::string &bundleName, const std::string &eventData)
{
    APP_LOGI("%{public}s called, bundle name is %{public}s, event is %{public}s",
        __func__, bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void AppMgrServiceInner::NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t userId,
    const int32_t callerUid, const std::string &eventData)
{
    APP_LOGI("%{public}s called, bundle name is %{public}s, , userId is %{public}d, event is %{public}s",
        __func__, bundleName.c_str(), userId, eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, userId);
    want.SetParam(Constants::UID, callerUid);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

int32_t AppMgrServiceInner::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (VerifyObserverPermission() == ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::recursive_mutex> lockRegister(observerLock_);
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    if (ObserverExist(observer)) {
        APP_LOGE("Observer exist.");
        return ERR_INVALID_VALUE;
    }
    appStateObservers_.push_back(observer);
    APP_LOGI("%{public}s appStateObservers_ size:%{public}d", __func__, (int32_t)appStateObservers_.size());
    AddObserverDeathRecipient(observer);
    return ERR_OK;
}

int32_t AppMgrServiceInner::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (VerifyObserverPermission() == ERR_PERMISSION_DENIED) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::recursive_mutex> lockUnregister(observerLock_);
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<sptr<IApplicationStateObserver>>::iterator it;
    for (it = appStateObservers_.begin(); it != appStateObservers_.end(); ++it) {
        if ((*it)->AsObject() == observer->AsObject()) {
            appStateObservers_.erase(it);
            APP_LOGI("%{public}s appStateObservers_ size:%{public}d", __func__, (int32_t)appStateObservers_.size());
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    APP_LOGE("Observer not exist.");
    return ERR_INVALID_VALUE;
}

bool AppMgrServiceInner::ObserverExist(const sptr<IApplicationStateObserver> &observer)
{
    if (observer == nullptr) {
        APP_LOGE("Observer nullptr");
        return false;
    }
    for (int i = 0; i < (int)appStateObservers_.size(); i++) {
        if (appStateObservers_[i]->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

void AppMgrServiceInner::AddObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (observer == nullptr || observer->AsObject() == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        APP_LOGE("This death recipient has been added.");
        return;
    } else {
        sptr<IRemoteObject::DeathRecipient> deathRecipient = new ApplicationStateObserverRecipient(
            std::bind(&AppMgrServiceInner::OnObserverDied, this, std::placeholders::_1));
        observer->AsObject()->AddDeathRecipient(deathRecipient);
        recipientMap_.emplace(observer->AsObject(), deathRecipient);
    }
}

void AppMgrServiceInner::RemoveObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    APP_LOGI("%{public}s begin", __func__);
    if (observer == nullptr || observer->AsObject() == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
        return;
    }
}

void AppMgrServiceInner::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("%{public}s begin", __func__);
    auto object = remote.promote();
    if (object == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    if (eventHandler_) {
        auto task = [object, appManager = this]() {appManager->HandleObserverDiedTask(object);};
        eventHandler_->PostTask(task, TASK_ON_CALLBACK_DIED);
    }
}

void AppMgrServiceInner::HandleObserverDiedTask(const sptr<IRemoteObject> &observer)
{
    APP_LOGI("Handle call back died task.");
    if (observer == nullptr) {
        APP_LOGE("observer nullptr.");
        return;
    }
    sptr<IApplicationStateObserver> object = iface_cast<IApplicationStateObserver>(observer);
    UnregisterApplicationStateObserver(object);
}

int32_t AppMgrServiceInner::GetForegroundApplications(std::vector<AppStateData> &list)
{
    APP_LOGI("%{public}s, begin.", __func__);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        APP_LOGE("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    appRunningManager_->GetForegroundApplications(list);
    return ERR_OK;
}

int AppMgrServiceInner::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo)
{
    APP_LOGI("Enter");
    if (!observer) {
        APP_LOGE("observer nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }

    HapModuleInfo hapModuleInfo;
    if (GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo)) {
        APP_LOGE("Failed to get HapModuleInfo for TestRunner");
        return ERR_INVALID_VALUE;
    }

    std::string processName;
    MakeProcessName(processName, std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo), hapModuleInfo);
    APP_LOGI("processName = [%{public}s]", processName.c_str());

    // Inspection records
    auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
        bundleInfo.applicationInfo.name, processName, bundleInfo.applicationInfo.uid, bundleInfo);
    if (appRecord) {
        APP_LOGI("processName [%{public}s] Already exists ", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    return StartEmptyProcess(want, observer, bundleInfo, processName);
}

int AppMgrServiceInner::GetHapModuleInfoForTestRunner(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    APP_LOGI("Enter");
    if (!observer) {
        APP_LOGE("observer nullptr.");
        return ERR_INVALID_VALUE;
    }

    bool moduelJson = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduelJson = bundleInfo.hapModuleInfos.back().isModuleJson;
    }
    if (moduelJson) {
        std::string moudleName;
        auto testRunnerName = want.GetStringParam("-s unittest");
        auto pos = testRunnerName.find(":");
        if (pos != std::string::npos) {
            moudleName = testRunnerName.substr(0, pos);
        } else {
            UserTestAbnormalFinish(observer, "No module name isn't unspecified.");
            return ERR_INVALID_VALUE;
        }

        bool found = false;
        for (auto item : bundleInfo.hapModuleInfos) {
            if (item.moduleName == moudleName) {
                hapModuleInfo = item;
                found = true;
                break;
            }
        }
        if (!found) {
            UserTestAbnormalFinish(observer, "The specified module name is not found.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int AppMgrServiceInner::UserTestAbnormalFinish(const sptr<IRemoteObject> &observer, const std::string &msg)
{
    sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(observer);
    if (!observerProxy) {
        APP_LOGE("Failed to get ITestObserver proxy");
        return ERR_INVALID_VALUE;
    }
    observerProxy->TestFinished(msg, -1);
    return ERR_OK;
}

int AppMgrServiceInner::StartEmptyProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &info, const std::string &processName)
{
    APP_LOGI("enter bundle [%{public}s | processName [%{public}s]]", info.name.c_str(), processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        APP_LOGE("Failed to start the process being tested!");
        return ERR_INVALID_VALUE;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        APP_LOGE("start process [%{public}s] failed!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<UserTestRecord> testRecord = std::make_shared<UserTestRecord>();
    if (!testRecord) {
        APP_LOGE("Failed to make UserTestRecord!");
        return ERR_INVALID_VALUE;
    }
    testRecord->want = want;
    testRecord->observer = observer;
    testRecord->isFinished = false;
    appRecord->SetUserTestInfo(testRecord);

    StartProcess(appInfo->name, processName, false, appRecord, appInfo->uid, appInfo->bundleName);

    // If it is empty, the startup failed
    if (!appRecord) {
        APP_LOGE("start process [%{public}s] failed!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    APP_LOGI("StartEmptyProcess OK pid : [%{public}d]", appRecord->GetPriorityObject()->GetPid());

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTest(
    const std::string &msg, const int &resultCode, const std::string &bundleName, const pid_t &pid)
{
    APP_LOGI("Enter");
    if (bundleName.empty()) {
        APP_LOGE("Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("no such appRecord");
        return ERR_INVALID_VALUE;
    }

    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        APP_LOGE("unstart user test");
        return ERR_INVALID_VALUE;
    }

    FinishUserTestLocked(msg, resultCode, appRecord);

    int ret = KillApplication(bundleName);
    if (ret) {
        APP_LOGE("Failed to kill process.");
        return ret;
    }

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTestLocked(
    const std::string &msg, const int &resultCode, std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGI("Enter");
    if (!appRecord) {
        APP_LOGE("Invalid appRecord");
        return ERR_INVALID_VALUE;
    }

    std::unique_lock<std::mutex> lck(userTestLock_);
    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        APP_LOGW("unstart user test");
        return ERR_INVALID_VALUE;
    }
    if (!userTestRecord->isFinished) {
        sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(userTestRecord->observer);
        if (!observerProxy) {
            APP_LOGE("Failed to get ITestObserver proxy");
            return ERR_INVALID_VALUE;
        }
        observerProxy->TestFinished(msg, resultCode);

        userTestRecord->isFinished = true;
    }

    return ERR_OK;
}

void AppMgrServiceInner::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    APP_LOGD("Start specified ability.");
    if (!CheckRemoteClient()) {
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    if (!appInfo) {
        APP_LOGE("appInfo is nullptr.");
        return;
    }

    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo)) {
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    if (!abilityInfoPtr) {
        APP_LOGE("abilityInfoPtr is nullptr.");
        return;
    }
    MakeProcessName(processName, abilityInfoPtr, appInfo, hapModuleInfo);

    std::vector<HapModuleInfo> hapModules;
    hapModules.emplace_back(hapModuleInfo);

    std::shared_ptr<AppRunningRecord> appRecord;
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        // new app record
        appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
        if (!appRecord) {
            APP_LOGE("start process [%{public}s] failed!", processName.c_str());
            return;
        }
        appRecord->SetEventHandler(eventHandler_);
        appRecord->SendEventForSpecifiedAbility(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG,
            AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT);
        bool coldStart = want.GetBoolParam("coldStart", false);
        StartProcess(appInfo->name, processName, coldStart, appRecord, appInfo->uid, appInfo->bundleName);

        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        appRecord->AddModules(appInfo, hapModules);
    } else {
        APP_LOGD("process is exist");
        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            APP_LOGD("module record is nullptr, add modules");
            appRecord->AddModules(appInfo, hapModules);
            appRecord->AddAbilityStageBySpecifiedAbility(appInfo->bundleName);
        } else {
            APP_LOGD("schedule accept want");
            appRecord->ScheduleAcceptWant(hapModuleInfo.moduleName);
        }
    }
}

void AppMgrServiceInner::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!response) {
        APP_LOGE("response is nullptr, register failed.");
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        APP_LOGE("%{public}s: Not abilityMgr call.", __func__);
        return;
    }

    startSpecifiedAbilityResponse_ = response;
}

void AppMgrServiceInner::ScheduleAcceptWantDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    APP_LOGD("Schedule accept want done, flag: %{public}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        APP_LOGE("Get app record failed.");
        return;
    }
    appRecord->ScheduleAcceptWantDone();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnAcceptWantResponse(want, flag);
    }
}

void AppMgrServiceInner::HandleStartSpecifiedAbilityTimeOut(const int64_t eventId)
{
    APP_LOGD("called start specified ability time out!");
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::UpdateConfiguration(const Configuration &config)
{
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is null");
        return;
    }

    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    int size = changeKeyV.size();
    APP_LOGI("changeKeyV size :%{public}d", size);
    if (!changeKeyV.empty()) {
        configuration_->Merge(changeKeyV, config);
        // all app
        appRunningManager_->UpdateConfiguration(config);
    }
}

void AppMgrServiceInner::GetGlobalConfiguration()
{
    if (!configuration_) {
        APP_LOGE("configuration_ is null");
        return;
    }
    // Currently only this interface is known
    auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
    APP_LOGI("current global language is : %{public}s", language.c_str());
    configuration_->AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, language);

    // Assign to default colormode "light"
    APP_LOGI("current global colormode is : %{public}s", ConfigurationInner::COLOR_MODE_LIGHT.c_str());
    configuration_->AddItem(GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);
}

std::shared_ptr<AppExecFwk::Configuration> AppMgrServiceInner::GetConfiguration()
{
    return configuration_;
}

void AppMgrServiceInner::KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    APP_LOGD("Kill application by appRecord.");

    if (!appRecord) {
        APP_LOGD("appRecord is nullptr.");
        return;
    }

    auto pid = appRecord->GetPriorityObject()->GetPid();
    appRecord->SetTerminating();
    appRecord->ScheduleProcessSecurityExit();

    auto startTime = SystemTimeMillis();
    std::list<pid_t> pids = {pid};
    if (WaitForRemoteProcessExit(pids, startTime)) {
        APP_LOGD("The remote process exited successfully");
        return;
    }

    auto result = KillProcessByPid(pid);
    if (result < 0) {
        APP_LOGE("Kill application by app record, pid: %{public}d", pid);
    }
}

void AppMgrServiceInner::SendHiSysEvent(const int32_t innerEventId, const int64_t eventId)
{
    APP_LOGD("called AppMgrServiceInner SendHiSysEvent!");
    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        APP_LOGE("appRecord is nullptr");
        return;
    }

    std::string eventName = EVENT_NAME_LIFECYCLE_TIMEOUT;
    std::string pidStr = std::to_string(appRecord->GetPriorityObject()->GetPid());
    std::string uidStr = std::to_string(appRecord->GetUid());
    std::string packageName = appRecord->GetBundleName();
    std::string processName = appRecord->GetProcessName();
    std::string msg;
    switch (innerEventId) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT;
            break;
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT;
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT;
            break;
        default:
            msg = EVENT_MESSAGE_DEFAULT;
            break;
    }

    APP_LOGD("SendHiSysEvent, eventName = %{public}s, uidStr = %{public}s, pidStr = %{public}s, \
        packageName = %{public}s, processName = %{public}s, msg = %{public}s",
        eventName.c_str(),
        uidStr.c_str(),
        pidStr.c_str(),
        packageName.c_str(),
        processName.c_str(),
        msg.c_str());

    OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_PID, pidStr,
        EVENT_KEY_UID, uidStr,
        EVENT_KEY_PACKAGE_NAME, packageName,
        EVENT_KEY_PROCESS_NAME, processName,
        EVENT_KEY_MESSAGE, msg);
}

int AppMgrServiceInner::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        APP_LOGE("no such appRecord");
        return ERR_NAME_NOT_FOUND;
    }
    for (auto &item : appRecord->GetAbilities()) {
        tokens.emplace_back(item.first);
    }
    return ERR_OK;
}

int AppMgrServiceInner::VerifyProcessPermission()
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        APP_LOGE("%{public}s: Permission verification succeeded", __func__);
        return ERR_OK;
    }
    APP_LOGE("%{public}s: Permission verification failed", __func__);
    return ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyAccountPermission(const std::string &permissionName, const int userId)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    const int currentUserId = getuid() / Constants::BASE_USER_RANGE;
    if (userId != currentUserId) {
        auto isCallingPermAccount = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS);
        if (!isCallingPermAccount) {
            APP_LOGE("%{public}s: Permission accounts verification failed", __func__);
            return ERR_PERMISSION_DENIED;
        }
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(permissionName);
    if (isCallingPerm) {
        APP_LOGD("%{public}s: Permission verification succeeded", __func__);
        return ERR_OK;
    }
    APP_LOGE("%{public}s: Permission verification failed", __func__);
    return ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyObserverPermission()
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_RUNNING_STATE_OBSERVER);
    if (isCallingPerm) {
        APP_LOGE("%{public}s: Permission verification succeeded", __func__);
        return ERR_OK;
    }
    APP_LOGE("%{public}s: Permission verification failed", __func__);
    return ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::StartRenderProcess(const pid_t hostPid, const std::string &renderParam,
    int32_t ipcFd, int32_t sharedFd, pid_t &renderPid)
{
    APP_LOGI("start render process, webview hostpid:%{public}d", hostPid);
    if (hostPid <= 0 || renderParam.empty() || ipcFd <= 0 || sharedFd <= 0) {
        APP_LOGE("invalid param, hostPid:%{public}d, renderParam:%{public}s, ipcFd:%{public}d, sharedFd:%{public}d",
            hostPid, renderParam.c_str(), ipcFd, sharedFd);
        return ERR_INVALID_VALUE;
    }

    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is , not start render process");
        return ERR_INVALID_VALUE;
    }

    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        APP_LOGE("no such appRecord, hostpid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto renderRecord = appRecord->GetRenderRecord();
    if (renderRecord) {
        APP_LOGW("already exit render process,do not request again, renderPid:%{public}d", renderRecord->GetPid());
        renderPid = renderRecord->GetPid();
        return ERR_ALREADY_EXIST_RENDER;
    }

    renderRecord = RenderRecord::CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, appRecord);
    if (!renderRecord) {
        APP_LOGE("create render record failed, hostpid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    return StartRenderProcessImpl(renderRecord, appRecord, renderPid);
}

void AppMgrServiceInner::AttachRenderProcess(const pid_t pid, const sptr<IRenderScheduler> &scheduler)
{
    APP_LOGD("attach render process start");
    if (pid <= 0) {
        APP_LOGE("invalid render process pid:%{public}d", pid);
        return;
    }
    if (!scheduler) {
        APP_LOGE("render scheduler is null");
        return;
    }

    if (!appRunningManager_) {
        APP_LOGE("appRunningManager_ is null");
        return;
    }

    APP_LOGI("attach render process pid:%{public}d", pid);
    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (!appRecord) {
        APP_LOGE("no such app Record, pid:%{public}d", pid);
        return;
    }

    auto renderRecord = appRecord->GetRenderRecord();
    if (!renderRecord) {
        APP_LOGE("no such render Record, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetEventHandler(eventHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsRenderProcess(true);
    renderRecord->SetScheduler(scheduler);
    renderRecord->SetDeathRecipient(appDeathRecipient);
    renderRecord->RegisterDeathRecipient();

    // notify fd to render process
    scheduler->NotifyBrowserFd(renderRecord->GetIpcFd(), renderRecord->GetSharedFd());
}

int AppMgrServiceInner::StartRenderProcessImpl(const std::shared_ptr<RenderRecord> &renderRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &renderPid)
{
    if (!renderRecord || !appRecord) {
        APP_LOGE("renderRecord or appRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto webviewSpawnClient = remoteClientManager_->GetWebviewSpawnClient();
    if (!webviewSpawnClient) {
        APP_LOGE("webviewSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    startMsg.renderParam = renderRecord->GetRenderParam();
    pid_t pid = 0;
    ErrCode errCode = webviewSpawnClient->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        APP_LOGE("failed to spawn new render process, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }
    renderPid = pid;
    appRecord->SetRenderRecord(renderRecord);
    renderRecord->SetPid(pid);
    APP_LOGI("start render process successed, hostPid:%{public}d, pid:%{public}d uid:%{public}d",
        renderRecord->GetHostPid(), pid, startMsg.uid);
    return 0;
}

void AppMgrServiceInner::OnRenderRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGE("On render remote died.");
    if (appRunningManager_) {
        appRunningManager_->OnRemoteRenderDied(remote);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
