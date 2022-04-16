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

#include "app_mgr_service.h"

#include <chrono>
#include <thread>

#include <nlohmann/json.hpp>
#include <sys/types.h>

#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "hilog_wrapper.h"
#include "perf_profile.h"
#include "xcollie/watchdog.h"

#include "permission_constants.h"
#include "permission_verification.h"
#include "system_environment_information.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
using namespace std::chrono_literals;
static const int EXPERIENCE_MEM_THRESHOLD = 20;
static const int APP_MS_TIMEOUT = 180;
static const int APP_MS_BLOCK = 65;
static const float PERCENTAGE = 100.0;
const std::string TASK_ATTACH_APPLICATION = "AttachApplicationTask";
const std::string TASK_APPLICATION_FOREGROUNDED = "ApplicationForegroundedTask";
const std::string TASK_APPLICATION_BACKGROUNDED = "ApplicationBackgroundedTask";
const std::string TASK_APPLICATION_TERMINATED = "ApplicationTerminatedTask";
const std::string TASK_ABILITY_CLEANED = "AbilityCleanedTask";
const std::string TASK_ADD_APP_DEATH_RECIPIENT = "AddAppRecipientTask";
const std::string TASK_CLEAR_UP_APPLICATION_DATA = "ClearUpApplicationDataTask";
const std::string TASK_STARTUP_RESIDENT_PROCESS = "StartupResidentProcess";
const std::string TASK_ADD_ABILITY_STAGE_DONE = "AddAbilityStageDone";
const std::string TASK_START_USER_TEST_PROCESS = "StartUserTestProcess";
const std::string TASK_FINISH_USER_TEST = "FinishUserTest";
const std::string TASK_ATTACH_RENDER_PROCESS = "AttachRenderTask";
}  // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(AppMgrService, APP_MGR_SERVICE_ID, true);

AppMgrService::AppMgrService()
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    HILOG_INFO("instance created with no para");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::AppMgrService(const int32_t serviceId, bool runOnCreate) : SystemAbility(serviceId, runOnCreate)
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    HILOG_INFO("instance created");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::~AppMgrService()
{
    HILOG_INFO("instance destroyed");
}

void AppMgrService::OnStart()
{
    HILOG_INFO("ready to start service");
    if (appMgrServiceState_.serviceRunningState == ServiceRunningState::STATE_RUNNING) {
        HILOG_WARN("failed to start service since it's already running");
        return;
    }

    ErrCode errCode = Init();
    if (FAILED(errCode)) {
        HILOG_ERROR("init failed, errCode: %{public}08x", errCode);
        return;
    }
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    HILOG_INFO("start service success");
    PerfProfile::GetInstance().SetAmsLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
}

void AppMgrService::OnStop()
{
    HILOG_INFO("ready to stop service");
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_NOT_START;
    handler_.reset();
    runner_.reset();
    if (appMgrServiceInner_) {
        appMgrServiceInner_->OnStop();
    }
    HILOG_INFO("stop service success");
}

void AppMgrService::SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService)
{
    appMgrServiceInner_ = innerService;
}

AppMgrServiceState AppMgrService::QueryServiceState()
{
    if (appMgrServiceInner_) {
        appMgrServiceState_.connectionState = appMgrServiceInner_->QueryAppSpawnConnectionState();
    }
    return appMgrServiceState_;
}

ErrCode AppMgrService::Init()
{
    HILOG_INFO("ready to init");
    // start main thread message loop.
    runner_ = EventRunner::Create(Constants::APP_MGR_SERVICE_NAME);
    if (!runner_) {
        HILOG_ERROR("init failed due to create runner error");
        return ERR_INVALID_OPERATION;
    }
    if (!appMgrServiceInner_) {
        HILOG_ERROR("init failed without inner service");
        return ERR_INVALID_OPERATION;
    }
    appMgrServiceInner_->Init();
    handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrServiceInner_);

    appMgrServiceInner_->SetEventHandler(handler_);
    ErrCode openErr = appMgrServiceInner_->OpenAppSpawnConnection();
    if (FAILED(openErr)) {
        HILOG_WARN("failed to connect to AppSpawnDaemon! errCode: %{public}08x", openErr);
    }
    if (!Publish(this)) {
        HILOG_ERROR("failed to publish appmgrservice to systemAbilityMgr");
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    amsMgrScheduler_ = new (std::nothrow) AmsMgrScheduler(appMgrServiceInner_, handler_);
    if (!amsMgrScheduler_) {
        HILOG_ERROR("init failed without ams scheduler");
        return ERR_INVALID_OPERATION;
    }
    std::string threadName = Constants::APP_MGR_SERVICE_NAME + "(" + std::to_string(runner_->GetThreadId()) + ")";
    if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handler_, APP_MS_TIMEOUT) != 0) {
        HILOG_ERROR("HiviewDFX::Watchdog::GetInstance AddThread Fail");
    }
    HILOG_INFO("init success");
    return ERR_OK;
}

int32_t AppMgrService::CheckPermission(
    [[maybe_unused]] const int32_t recordId, [[maybe_unused]] const std::string &permission)
{
    HILOG_INFO("check application's permission");

    return ERR_OK;
}

void AppMgrService::AttachApplication(const sptr<IRemoteObject> &app)
{
    if (!IsReady()) {
        HILOG_ERROR("AttachApplication failed, not ready.");
        return;
    }

    pid_t pid = IPCSkeleton::GetCallingPid();
    AddAppDeathRecipient(pid);
    std::function<void()> attachApplicationFunc =
        std::bind(&AppMgrServiceInner::AttachApplication, appMgrServiceInner_, pid, iface_cast<IAppScheduler>(app));
    handler_->PostTask(attachApplicationFunc, TASK_ATTACH_APPLICATION);
}

void AppMgrService::ApplicationForegrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationForegroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationForegrounded, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationForegroundedFunc, TASK_APPLICATION_FOREGROUNDED);
}

void AppMgrService::ApplicationBackgrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationBackgroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationBackgrounded, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationBackgroundedFunc, TASK_APPLICATION_BACKGROUNDED);
}

void AppMgrService::ApplicationTerminated(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> applicationTerminatedFunc =
        std::bind(&AppMgrServiceInner::ApplicationTerminated, appMgrServiceInner_, recordId);
    handler_->PostTask(applicationTerminatedFunc, TASK_APPLICATION_TERMINATED);
}

void AppMgrService::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> abilityCleanedFunc =
        std::bind(&AppMgrServiceInner::AbilityTerminated, appMgrServiceInner_, token);
    handler_->PostTask(abilityCleanedFunc, TASK_ABILITY_CLEANED);
}

bool AppMgrService::IsReady() const
{
    if (!appMgrServiceInner_) {
        HILOG_ERROR("appMgrServiceInner is null");
        return false;
    }
    if (!handler_) {
        HILOG_ERROR("handler is null");
        return false;
    }
    return true;
}

void AppMgrService::AddAppDeathRecipient(const pid_t pid) const
{
    if (!IsReady()) {
        return;
    }
    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetEventHandler(handler_);
    appDeathRecipient->SetAppMgrServiceInner(appMgrServiceInner_);
    std::function<void()> addAppRecipientFunc =
        std::bind(&AppMgrServiceInner::AddAppDeathRecipient, appMgrServiceInner_, pid, appDeathRecipient);
    handler_->PostTask(addAppRecipientFunc, TASK_ADD_APP_DEATH_RECIPIENT);
}

void AppMgrService::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    if (!IsReady()) {
        return;
    }
    HILOG_INFO("Notify start resident process");
    std::function <void()> startupResidentProcess =
        std::bind(&AppMgrServiceInner::LoadResidentProcess, appMgrServiceInner_, bundleInfos);
    handler_->PostTask(startupResidentProcess, TASK_STARTUP_RESIDENT_PROCESS);
}

sptr<IAmsMgr> AppMgrService::GetAmsMgr()
{
    return amsMgrScheduler_;
}

int32_t AppMgrService::ClearUpApplicationData(const std::string &bundleName)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_CLEAN_APPLICATION_DATA);
        if (!isCallingPerm) {
            HILOG_ERROR("%{public}s: Permission verification failed", __func__);
            return ERR_PERMISSION_DENIED;
        }
    }

    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> clearUpApplicationDataFunc =
        std::bind(&AppMgrServiceInner::ClearUpApplicationData, appMgrServiceInner_, bundleName, uid, pid);
    handler_->PostTask(clearUpApplicationDataFunc, TASK_CLEAR_UP_APPLICATION_DATA);
    return ERR_OK;
}

int32_t AppMgrService::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningProcesses(info);
}

int32_t AppMgrService::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetProcessRunningInfosByUserId(info, userId);
}

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AppMgrService::GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig)
{
    SystemEnv::KernelSystemMemoryInfo systemMemInfo;
    SystemEnv::GetMemInfo(systemMemInfo);
    int memThreshold = 0;
    nlohmann::json memJson = nlohmann::json::parse(strConfig, nullptr, false);
    if (memJson.is_discarded()) {
        memThreshold = EXPERIENCE_MEM_THRESHOLD;
        HILOG_ERROR("%{public}s, discarded memThreshold = %{public}d", __func__, EXPERIENCE_MEM_THRESHOLD);
    } else {
        if (!memJson.contains("memoryThreshold")) {
            memThreshold = EXPERIENCE_MEM_THRESHOLD;
            HILOG_ERROR("%{public}s, memThreshold = %{public}d", __func__, EXPERIENCE_MEM_THRESHOLD);
        } else {
            memThreshold = memJson.at("memorythreshold").get<int>();
            HILOG_INFO("memThreshold = %{public}d", memThreshold);
        }
    }

    memoryInfo.availSysMem_ = systemMemInfo.GetMemFree();
    memoryInfo.totalSysMem_ = systemMemInfo.GetMemTotal();
    memoryInfo.threshold_ = static_cast<int64_t>(memoryInfo.totalSysMem_ * memThreshold / PERCENTAGE);
    memoryInfo.isSysInlowMem_ = memoryInfo.availSysMem_ < memoryInfo.threshold_;
}

void AppMgrService::AddAbilityStageDone(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    std::function <void()> addAbilityStageDone =
        std::bind(&AppMgrServiceInner::AddAbilityStageDone, appMgrServiceInner_, recordId);
    handler_->PostTask(addAbilityStageDone, TASK_ADD_ABILITY_STAGE_DONE);
}

int32_t AppMgrService::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_INFO("%{public}s begin", __func__);
    if (!IsReady()) {
        HILOG_ERROR("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterApplicationStateObserver(observer);
}

int32_t AppMgrService::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_INFO("%{public}s begin", __func__);
    if (!IsReady()) {
        HILOG_ERROR("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrService::GetForegroundApplications(std::vector<AppStateData> &list)
{
    HILOG_INFO("%{public}s begin", __func__);
    if (!IsReady()) {
        HILOG_ERROR("%{public}s begin, not ready", __func__);
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetForegroundApplications(list);
}

int AppMgrService::StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    std::function<void()> startUserTestProcessFunc =
        std::bind(&AppMgrServiceInner::StartUserTestProcess, appMgrServiceInner_, want, observer, bundleInfo, userId);
    handler_->PostTask(startUserTestProcessFunc, TASK_START_USER_TEST_PROCESS);
    return ERR_OK;
}

int AppMgrService::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    std::function<void()> finishUserTestProcessFunc =
        std::bind(&AppMgrServiceInner::FinishUserTest, appMgrServiceInner_, msg, resultCode, bundleName, callingPid);
    handler_->PostTask(finishUserTestProcessFunc, TASK_FINISH_USER_TEST);
    return ERR_OK;
}

void AppMgrService::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    if (!IsReady()) {
        return;
    }
    auto task = [=]() { appMgrServiceInner_->ScheduleAcceptWantDone(recordId, want, flag); };
    handler_->PostTask(task);
}

int AppMgrService::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAbilityRecordsByProcessID(pid, tokens);
}

int32_t AppMgrService::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, pid_t &renderPid)
{
    if (!IsReady()) {
        HILOG_ERROR("StartRenderProcess failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->StartRenderProcess(IPCSkeleton::GetCallingPid(),
        renderParam, ipcFd, sharedFd, renderPid);
}

void AppMgrService::AttachRenderProcess(const sptr<IRemoteObject> &scheduler)
{
    HILOG_DEBUG("AttachRenderProcess called.");
    if (!IsReady()) {
        HILOG_ERROR("AttachRenderProcess failed, not ready.");
        return;
    }

    auto pid = IPCSkeleton::GetCallingPid();
    auto fun = std::bind(&AppMgrServiceInner::AttachRenderProcess,
        appMgrServiceInner_, pid, iface_cast<IRenderScheduler>(scheduler));
    handler_->PostTask(fun, TASK_ATTACH_RENDER_PROCESS);
}

void AppMgrService::PostANRTaskByProcessID(const pid_t pid)
{
    HILOG_DEBUG("PostANRTaskByProcessID called.");
    if (!IsReady()) {
        HILOG_ERROR("AttachRenderProcess failed, not ready.");
        return;
    }
    auto appRecord = appMgrServiceInner_->GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord");
        return;
    }
    auto object = appRecord->GetApplicationClient();
    if (!object) {
        HILOG_ERROR("GetApplicationClient failed.");
        return;
    }
    object->ScheduleANRProcess();
}

int AppMgrService::BlockAppService()
{
    HILOG_DEBUG("%{public}s begin", __func__);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    auto task = [=]() {
        while (1) {
            HILOG_DEBUG("%{public}s begain block app service", __func__);
            std::this_thread::sleep_for(APP_MS_BLOCK*1s);
        }
    };
    handler_->PostTask(task);
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
