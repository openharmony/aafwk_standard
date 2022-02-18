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

#include "app_mgr_client.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"

#include "app_mgr_interface.h"
#include "app_service_manager.h"

namespace OHOS {
namespace AppExecFwk {

AppMgrClient::AppMgrClient()
{}

AppMgrClient::~AppMgrClient()
{}

AppMgrResultCode AppMgrClient::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo, const AAFwk::Want &want)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::TerminateAbility(const sptr<IRemoteObject> &token)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::Reset()
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
    const int32_t connectionState)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillProcessesByUserId(int32_t userId)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillApplication(const std::string &bundleName)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationData(const std::string &bundleName)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::SetAppFreezingTime(int time)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::GetAppFreezingTime(int &time)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::ConnectAppMgrService()
{
    return AppMgrResultCode::RESULT_OK;
}

// void AppMgrClient::SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
// {}

void AppMgrClient::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{}

void AppMgrClient::PrepareTerminate(const sptr<IRemoteObject> &token)
{}

int AppMgrClient::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    return AppMgrResultCode::RESULT_OK;
}

void AppMgrClient::GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig)
{}

}  // namespace AppExecFwk
}  // namespace OHOS
