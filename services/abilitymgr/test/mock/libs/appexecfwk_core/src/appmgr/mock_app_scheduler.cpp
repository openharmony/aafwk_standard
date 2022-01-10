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

#include "app_scheduler.h"

#include "hilog_wrapper.h"
#include "ability_util.h"
#include "ability_manager_errors.h"
#include "ability_record.h"
#include "appmgr/app_mgr_constants.h"

namespace OHOS {
namespace AAFwk {
AppScheduler::AppScheduler()
{
    HILOG_INFO(" Test AppScheduler::AppScheduler()");
}

AppScheduler::~AppScheduler()
{
    HILOG_INFO("Test AppScheduler::~AppScheduler()");
}

bool AppScheduler::Init(const std::weak_ptr<AppStateCallback> &callback)
{
    HILOG_INFO("Test AppScheduler::Init()");
    if (!callback.lock()) {
        return false;
    }
    return true;
}

int AppScheduler::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const AppExecFwk::AbilityInfo &abilityInfo, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    HILOG_INFO("Test AppScheduler::LoadAbility()");
    if (applicationInfo.bundleName.find("com.ix.First.Test") != std::string::npos) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int AppScheduler::TerminateAbility(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::TerminateAbility()");
    return ERR_OK;
}

void AppScheduler::MoveToForground(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::MoveToForground()");
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::MoveToBackground()");
}

void AppScheduler::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    HILOG_INFO("Test AppScheduler::AbilityBehaviorAnalysis()");
}

void AppScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::KillProcessByAbilityToken()");
}

AppAbilityState AppScheduler::ConvertToAppAbilityState(const int32_t state)
{
    AppExecFwk::AbilityState abilityState = static_cast<AppExecFwk::AbilityState>(state);
    switch (abilityState) {
        case AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND: {
            return AppAbilityState::ABILITY_STATE_FOREGROUND;
        }
        case AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND: {
            return AppAbilityState::ABILITY_STATE_BACKGROUND;
        }
        default:
            return AppAbilityState::ABILITY_STATE_UNDEFINED;
    }
}

AppAbilityState AppScheduler::GetAbilityState() const
{
    return appAbilityState_;
}

void AppScheduler::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state)
{
    HILOG_INFO("Test AppScheduler::OnAbilityRequestDone()");
}

int AppScheduler::KillApplication(const std::string &bundleName)
{
    HILOG_INFO("Test AppScheduler::KillApplication()");
    return ERR_OK;
}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::AttachTimeOut()");
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Test AppScheduler::PrepareTerminate()");
}

int AppScheduler::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    HILOG_INFO("Test AppScheduler::CompelVerifyPermission()");
    return ERR_OK;
}

void AppScheduler::OnAppStateChanged(const AppExecFwk::AppProcessData &appData)
{
    HILOG_INFO("Test AppScheduler::OnAppStateChanged()");
}

void AppScheduler::GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo, std::string &strConfig)
{
    const int64_t testValue = 10;
    memoryInfo.availSysMem_ = testValue;
    memoryInfo.totalSysMem_ = testValue;
    memoryInfo.threshold_ = testValue;
}

int AppScheduler::ClearUpApplicationData(const std::string &bundleName)
{
    HILOG_INFO("Test AppScheduler::ClearUpApplicationData()");
    return ERR_OK;
}

void AppScheduler::UpdateAbilityState(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state)
{
    HILOG_INFO("Test AppScheduler::UpdateAbilityState()");
}

void AppScheduler::UpdateExtensionState(const sptr<IRemoteObject> &token, const AppExecFwk::ExtensionState state)
{
    HILOG_INFO("Test AppScheduler::UpdateExtensionState()");
}

void AppScheduler::StartupResidentProcess()
{
    HILOG_INFO("Test AppScheduler::StartupResidentProcess()");
}
}  // namespace AAFwk
}  // namespace OHOS
