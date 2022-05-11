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

#include "app_scheduler_host.h"
#include "ability_info.h"
#include "appexecfwk_errors.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
AppSchedulerHost::AppSchedulerHost()
{
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleForegroundApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleBackgroundApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleTerminateApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleLowMemory;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleShrinkMemory;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleLaunchAbility;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleCleanAbility;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleLaunchApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleProfileChanged;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED)] =
        &AppSchedulerHost::HandleScheduleConfigurationUpdated;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION)] =
        &AppSchedulerHost::HandleScheduleProcessSecurityExit;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO)] =
        &AppSchedulerHost::HandleScheduleAbilityStage;
    memberFuncMap_[static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT)] =
        &AppSchedulerHost::HandleScheduleAcceptWant;
}

AppSchedulerHost::~AppSchedulerHost()
{
    memberFuncMap_.clear();
}

int AppSchedulerHost::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("AppSchedulerHost::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AppSchedulerHost::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppSchedulerHost::HandleScheduleForegroundApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleForegroundApplication();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleBackgroundApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleBackgroundApplication();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleTerminateApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleTerminateApplication();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLowMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleLowMemory();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleShrinkMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleShrinkMemory(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLaunchAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (!abilityInfo) {
        HILOG_ERROR("ReadParcelable<AbilityInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }

    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    ScheduleLaunchAbility(*abilityInfo, token, want);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleCleanAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    ScheduleCleanAbility(token);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLaunchApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<AppLaunchData> launchData(data.ReadParcelable<AppLaunchData>());
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!launchData) {
        HILOG_ERROR("ReadParcelable<launchData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!config) {
        HILOG_ERROR("ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleLaunchApplication(*launchData, *config);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleAbilityStage(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<HapModuleInfo> abilityStage(data.ReadParcelable<HapModuleInfo>());
    if (!abilityStage) {
        HILOG_ERROR("ReadParcelable<launchData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleAbilityStage(*abilityStage);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleProfileChanged(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<Profile> profile(data.ReadParcelable<Profile>());
    if (!profile) {
        HILOG_ERROR("ReadParcelable<Profile> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleProfileChanged(*profile);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleConfigurationUpdated(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<Configuration> configuration(data.ReadParcelable<Configuration>());
    if (!configuration) {
        HILOG_ERROR("ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleConfigurationUpdated(*configuration);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleProcessSecurityExit(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleProcessSecurityExit();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleAcceptWant(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto moduleName = data.ReadString();
    ScheduleAcceptWant(*want, moduleName);
    delete want;
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
