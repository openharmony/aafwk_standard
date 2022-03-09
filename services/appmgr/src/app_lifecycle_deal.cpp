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

#include "app_lifecycle_deal.h"
#include "bytrace.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppLifeCycleDeal::AppLifeCycleDeal()
{}

AppLifeCycleDeal::~AppLifeCycleDeal()
{}

void AppLifeCycleDeal::LaunchApplication(const AppLaunchData &launchData_, const Configuration &config)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("AppLifeCycleDeal ScheduleLaunchApplication");
    if (appThread_) {
        appThread_->ScheduleLaunchApplication(launchData_, config);
    }
}

void AppLifeCycleDeal::AddAbilityStage(const HapModuleInfo &abilityStage)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleAbilityStage(abilityStage);
}

void AppLifeCycleDeal::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    if (appThread_) {
        appThread_->ScheduleLaunchAbility(*(ability->GetAbilityInfo()), ability->GetToken(),
            ability->GetWant());
    }
}

void AppLifeCycleDeal::ScheduleTerminate()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleTerminateApplication();
}

void AppLifeCycleDeal::ScheduleForegroundRunning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleForegroundApplication();
}

void AppLifeCycleDeal::ScheduleBackgroundRunning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleBackgroundApplication();
}

void AppLifeCycleDeal::ScheduleTrimMemory(int32_t timeLevel)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleShrinkMemory(timeLevel);
}

void AppLifeCycleDeal::LowMemoryWarning()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleLowMemory();
}

void AppLifeCycleDeal::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }
    appThread_->ScheduleCleanAbility(token);
}

void AppLifeCycleDeal::ScheduleProcessSecurityExit()
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleProcessSecurityExit();
}

void AppLifeCycleDeal::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    appThread_ = thread;
}

sptr<IAppScheduler> AppLifeCycleDeal::GetApplicationClient() const
{
    return appThread_;
}

void AppLifeCycleDeal::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    if (!appThread_) {
        HILOG_ERROR("appThread_ is nullptr");
        return;
    }

    appThread_->ScheduleAcceptWant(want, moduleName);
}

void AppLifeCycleDeal::UpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("call %{public}s", __func__);
    if (!appThread_) {
        HILOG_INFO("appThread_ is null");
        return;
    }
    appThread_->ScheduleConfigurationUpdated(config);
}
}  // namespace AppExecFwk
}  // namespace OHOS