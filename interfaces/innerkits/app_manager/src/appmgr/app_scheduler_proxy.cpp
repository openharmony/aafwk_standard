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

#include "app_scheduler_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AppSchedulerProxy::AppSchedulerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppScheduler>(impl)
{}

bool AppSchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppSchedulerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AppSchedulerProxy::ScheduleForegroundApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleBackgroundApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION),
            data,
            reply,
            option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleTerminateApplication()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLowMemory()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleShrinkMemory(const int32_t level)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(level);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION),
        data,
        reply,
        option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&info);

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            HILOG_ERROR("Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    if (!data.WriteParcelable(want.get())) {
        HILOG_ERROR("write want fail.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        HILOG_ERROR("Failed to write token");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config)
{
    HILOG_INFO("AppSchedulerProxy ScheduleLaunchApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&launchData)) {
        HILOG_ERROR("WriteParcelable launchData failed");
        return ;
    }

    if (!data.WriteParcelable(&config)) {
        HILOG_ERROR("WriteParcelable config failed");
        return ;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }

    HILOG_INFO("AppSchedulerProxy ScheduleLaunchApplication end");
}

void AppSchedulerProxy::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_INFO("AppSchedulerProxy ScheduleAbilityStage start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&abilityStage)) {
        return ;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_INFO("AppSchedulerProxy ScheduleAbilityStage end");
}

void AppSchedulerProxy::ScheduleProfileChanged(const Profile &profile)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&profile);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleConfigurationUpdated(const Configuration &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&config);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleProcessSecurityExit()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppSchedulerProxy::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteString(moduleName)) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
