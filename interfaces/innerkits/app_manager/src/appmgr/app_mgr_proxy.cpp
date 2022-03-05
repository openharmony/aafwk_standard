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

#include "app_mgr_proxy.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"

#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AppMgrProxy::AppMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppMgr>(impl)
{}

bool AppMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppMgrProxy::GetDescriptor())) {
        APP_LOGE("write interface token failed");
        return false;
    }
    return true;
}

void AppMgrProxy::AttachApplication(const sptr<IRemoteObject> &obj)
{
    APP_LOGD("AppMgrProxy::AttachApplication start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(obj.GetRefPtr());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_ATTACH_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationForegrounded(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_FOREGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationBackgrounded(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_BACKGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

void AppMgrProxy::ApplicationTerminated(const int32_t recordId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_TERMINATED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

int32_t AppMgrProxy::CheckPermission(const int32_t recordId, const std::string &permission)
{
    APP_LOGD("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_PERMISSION_DENIED;
    }
    data.WriteInt32(recordId);
    data.WriteString(permission);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_CHECK_PERMISSION), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGE("SendRequest is failed, error code: %{public}d", ret);
        return ERR_PERMISSION_DENIED;
    }
    APP_LOGD("end");
    return reply.ReadInt32();
}

void AppMgrProxy::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(token.GetRefPtr());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_ABILITY_CLEANED), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
    }
    APP_LOGD("end");
}

sptr<IAmsMgr> AppMgrProxy::GetAmsMgr()
{
    APP_LOGD("begin to get Ams instance");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_MGR_INSTANCE, data, reply)) {
        return nullptr;
    }
    sptr<IRemoteObject> object = reply.ReadParcelable<IRemoteObject>();
    sptr<IAmsMgr> amsMgr = iface_cast<IAmsMgr>(object);
    if (!amsMgr) {
        APP_LOGE("ams instance is nullptr");
        return nullptr;
    }
    APP_LOGD("get ams instance success");
    return amsMgr;
}

int32_t AppMgrProxy::ClearUpApplicationData(const std::string &bundleName)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_CLEAR_UP_APPLICATION_DATA), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    APP_LOGD("end");
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_ALL_RUNNING_PROCESSES, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        APP_LOGE("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    APP_LOGD("end");
    return result;
}

int32_t AppMgrProxy::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    APP_LOGD("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(userId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        APP_LOGE("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_RUNNING_PROCESSES_BY_USER_ID, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        APP_LOGE("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    APP_LOGD("end");
    return result;
}

bool AppMgrProxy::SendTransactCmd(IAppMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        APP_LOGE("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        APP_LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AppMgrProxy::GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteString(strConfig)) {
        APP_LOGE("want write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::APP_GET_SYSTEM_MEMORY_ATTR, data, reply)) {
        APP_LOGE("SendTransactCmd failed");
        return;
    }

    std::shared_ptr<SystemMemoryAttr> remoteRetsult(reply.ReadParcelable<SystemMemoryAttr>());
    if (remoteRetsult == nullptr) {
        APP_LOGE("recv SystemMemoryAttr failed");
        return;
    }

    memoryInfo = *remoteRetsult;
}

void AppMgrProxy::AddAbilityStageDone(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId)) {
        APP_LOGE("want write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::APP_ADD_ABILITY_STAGE_INFO_DONE, data, reply)) {
        APP_LOGE("SendTransactCmd failed");
        return;
    }
    return;
}

void AppMgrProxy::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        APP_LOGE("write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            APP_LOGE("write bundle info failed");
            return;
        }
    }

    if (!SendTransactCmd(IAppMgr::Message::STARTUP_RESIDENT_PROCESS, data, reply)) {
        APP_LOGE("SendTransactCmd failed");
        return;
    }
    return;
}

template<typename T>
int AppMgrProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            APP_LOGE("Read Parcelable infos failed");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    APP_LOGD("get parcelable infos success");
    return NO_ERROR;
}

int AppMgrProxy::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer)
{
    if (!observer) {
        APP_LOGE("observer null");
        return ERR_INVALID_VALUE;
    }
    APP_LOGD("RegisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(observer->AsObject())) {
        APP_LOGE("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::REGISTER_APPLICATION_STATE_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        APP_LOGE("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::UnregisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer)
{
    if (!observer) {
        APP_LOGE("observer null");
        return ERR_INVALID_VALUE;
    }
    APP_LOGD("UnregisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(observer->AsObject())) {
        APP_LOGE("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_APPLICATION_STATE_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        APP_LOGE("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::GetForegroundApplications(std::vector<AppStateData> &list)
{
    APP_LOGD("GetForegroundApplications start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::GET_FOREGROUND_APPLICATIONS),
        data, reply, option);
    if (error != NO_ERROR) {
        APP_LOGE("GetForegroundApplications fail, error: %{public}d", error);
        return error;
    }
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppStateData> info(reply.ReadParcelable<AppStateData>());
        if (!info) {
            APP_LOGE("Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        list.emplace_back(*info);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&want)) {
        APP_LOGE("want write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(observer)) {
        APP_LOGE("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&bundleInfo)) {
        APP_LOGE("bundleInfo write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::START_USER_TEST_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::FinishUserTest(
    const std::string &msg, const int &resultCode, const std::string &bundleName, const pid_t &pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(msg)) {
        APP_LOGE("msg write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(resultCode)) {
        APP_LOGE("resultCode:WriteInt32 fail.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        APP_LOGE("bundleName write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid)) {
        APP_LOGE("pid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::FINISH_USER_TEST), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AppMgrProxy::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId) || !data.WriteParcelable(&want) || !data.WriteString(flag)) {
        APP_LOGE("want write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::SCHEDULE_ACCEPT_WANT_DONE, data, reply)) {
        APP_LOGE("SendTransactCmd failed");
        return;
    }
}

int AppMgrProxy::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID, data, reply)) {
        return ERR_NULL_OBJECT;
    }
    int32_t infoSize = reply.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        auto iRemote = reply.ReadRemoteObject();
        tokens.emplace_back(iRemote);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, pid_t &renderPid)
{
    if (renderParam.empty() || ipcFd <= 0 || sharedFd <= 0) {
        APP_LOGE("Invalid params, renderParam:%{public}s, ipcFd:%{public}d, sharedFd:%{public}d",
            renderParam.c_str(), ipcFd, sharedFd);
        return -1;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        APP_LOGE("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(renderParam)) {
        APP_LOGE("want paramSize failed.");
        return -1;
    }

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd)) {
        APP_LOGE("want fd failed, ipcFd:%{public}d, sharedFd:%{public}d", ipcFd, sharedFd);
        return -1;
    }

    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::START_RENDER_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        APP_LOGW("StartRenderProcess SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    renderPid = reply.ReadInt32();
    if (result != 0) {
        APP_LOGW("StartRenderProcess failed, result: %{public}d", ret);
        return ret;
    }
    return 0;
}

void AppMgrProxy::AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler)
{
    if (!renderScheduler) {
        APP_LOGE("renderScheduler is null");
        return;
    }

    APP_LOGD("AttachRenderProcess start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(renderScheduler)) {
        APP_LOGE("renderScheduler write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::ATTACH_RENDER_PROCESS, data, reply)) {
        APP_LOGE("SendTransactCmd ATTACH_RENDER_PROCESS failed");
        return;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
