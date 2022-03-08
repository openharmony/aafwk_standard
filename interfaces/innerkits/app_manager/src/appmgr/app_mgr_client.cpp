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

#include "app_mgr_client.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"

#include "app_mgr_interface.h"
#include "app_service_manager.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppMgrClient::AppMgrClient()
{
    SetServiceManager(std::make_unique<AppServiceManager>());
}

AppMgrClient::~AppMgrClient()
{}

AppMgrResultCode AppMgrClient::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo, const AAFwk::Want &want)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            // From here, separate AbilityInfo and ApplicationInfo from AA.
            std::shared_ptr<AbilityInfo> abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
            std::shared_ptr<ApplicationInfo> appInfoPtr = std::make_shared<ApplicationInfo>(appInfo);
            std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>(want);
            amsService->LoadAbility(token, preToken, abilityInfoPtr, appInfoPtr, wantPtr);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::TerminateAbility(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->TerminateAbility(token);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateAbilityState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateExtensionState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->RegisterAppStateCallback(callback);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
    const int32_t connectionState)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessByAbilityToken(token);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessesByUserId(int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessesByUserId(userId);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplication(const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplication(bundleName);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplicationByUid(bundleName, uid);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationData(const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        int32_t result = service->ClearUpApplicationData(bundleName);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        int32_t result = service->GetAllRunningProcesses(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        int32_t result = service->GetProcessRunningInfosByUserId(info, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetConfiguration(Configuration& config)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->GetConfiguration(config);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ConnectAppMgrService()
{
    if (!serviceManager_) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    remote_ = serviceManager_->GetAppMgrService();
    if (!remote_) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::RESULT_OK;
}

void AppMgrClient::SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
{
    serviceManager_ = std::move(serviceMgr);
}

void AppMgrClient::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->AbilityAttachTimeOut(token);
}

void AppMgrClient::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->PrepareTerminate(token);
}

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AppMgrClient::GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }

    service->GetSystemMemoryAttr(memoryInfo, strConfig);
}

void AppMgrClient::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->GetRunningProcessInfoByToken(token, info);
        }
    }
}

void AppMgrClient::AddAbilityStageDone(const int32_t recordId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }

    service->AddAbilityStageDone(recordId);
}

void AppMgrClient::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }

    service->StartupResidentProcess(bundleInfos);
}

int AppMgrClient::StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->StartUserTestProcess(want, observer, bundleInfo);
}

int AppMgrClient::FinishUserTest(
    const std::string &msg, const int &resultCode, const std::string &bundleName, const pid_t &pid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->FinishUserTest(msg, resultCode, bundleName, pid);
}

void AppMgrClient::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->StartSpecifiedAbility(want, abilityInfo);
}

void AppMgrClient::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->RegisterStartSpecifiedAbilityResponse(response);
}

void AppMgrClient::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return;
    }

    service->ScheduleAcceptWantDone(recordId, want, flag);
}

AppMgrResultCode AppMgrClient::UpdateConfiguration(const Configuration &config)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    amsService->UpdateConfiguration(config);
    return AppMgrResultCode::RESULT_OK;
}

int AppMgrClient::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service == nullptr) {
        HILOG_ERROR("service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }

    return service->GetAbilityRecordsByProcessID(pid, tokens);
}

int AppMgrClient::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, pid_t &renderPid)
{
    if (!remote_) {
        ConnectAppMgrService();
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        return service->StartRenderProcess(renderParam, ipcFd, sharedFd, renderPid);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

void AppMgrClient::AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler)
{
    if (!renderScheduler) {
        HILOG_INFO("renderScheduler is nullptr");
        return;
    }

    if (!remote_) {
        ConnectAppMgrService();
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(remote_);
    if (service != nullptr) {
        HILOG_INFO("AttachRenderProcess");
        service->AttachRenderProcess(renderScheduler->AsObject());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
