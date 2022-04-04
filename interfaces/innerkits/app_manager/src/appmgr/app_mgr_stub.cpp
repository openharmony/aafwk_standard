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

#include "app_mgr_stub.h"

#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "ability_info.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "appexecfwk_errors.h"
#include "bytrace.h"
#include "hilog_wrapper.h"
#include "iapp_state_callback.h"
#include "want.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
AppMgrStub::AppMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ATTACH_APPLICATION)] =
        &AppMgrStub::HandleAttachApplication;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_FOREGROUNDED)] =
        &AppMgrStub::HandleApplicationForegrounded;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_BACKGROUNDED)] =
        &AppMgrStub::HandleApplicationBackgrounded;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_TERMINATED)] =
        &AppMgrStub::HandleApplicationTerminated;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_CHECK_PERMISSION)] =
        &AppMgrStub::HandleCheckPermission;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ABILITY_CLEANED)] =
        &AppMgrStub::HandleAbilityCleaned;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_MGR_INSTANCE)] = &AppMgrStub::HandleGetAmsMgr;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_CLEAR_UP_APPLICATION_DATA)] =
        &AppMgrStub::HandleClearUpApplicationData;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_ALL_RUNNING_PROCESSES)] =
        &AppMgrStub::HandleGetAllRunningProcesses;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_RUNNING_PROCESSES_BY_USER_ID)] =
        &AppMgrStub::HandleGetProcessRunningInfosByUserId;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_SYSTEM_MEMORY_ATTR)] =
        &AppMgrStub::HandleGetSystemMemoryAttr;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_ADD_ABILITY_STAGE_INFO_DONE)] =
        &AppMgrStub::HandleAddAbilityStageDone;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::STARTUP_RESIDENT_PROCESS)] =
        &AppMgrStub::HandleStartupResidentProcess;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::REGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleRegisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleUnregisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::GET_FOREGROUND_APPLICATIONS)] =
        &AppMgrStub::HandleGetForegroundApplications;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::START_USER_TEST_PROCESS)] =
        &AppMgrStub::HandleStartUserTestProcess;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::FINISH_USER_TEST)] =
        &AppMgrStub::HandleFinishUserTest;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::SCHEDULE_ACCEPT_WANT_DONE)] =
        &AppMgrStub::HandleScheduleAcceptWantDone;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID)] =
        &AppMgrStub::HandleGetAbilityRecordsByProcessID;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::START_RENDER_PROCESS)] =
        &AppMgrStub::HandleStartRenderProcess;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::ATTACH_RENDER_PROCESS)] =
        &AppMgrStub::HandleAttachRenderProcess;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::POST_ANR_TASK_BY_PID)] =
        &AppMgrStub::HandlePostANRTaskByProcessID;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::BLOCK_APP_SERVICE)] =
        &AppMgrStub::HandleBlockAppServiceDone;
}

AppMgrStub::~AppMgrStub()
{
    memberFuncMap_.clear();
}

int AppMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("AppMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AppMgrStub::GetDescriptor();
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

int32_t AppMgrStub::HandleAttachApplication(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    AttachApplication(client);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationForegrounded(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationForegrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationBackgrounded(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationBackgrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationTerminated(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    ApplicationTerminated(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleCheckPermission(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t recordId = data.ReadInt32();
    std::string permission = data.ReadString();
    int32_t result = CheckPermission(recordId, permission);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAbilityCleaned(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityCleaned(token);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAmsMgr(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t result = NO_ERROR;
    sptr<IAmsMgr> amsMgr = GetAmsMgr();
    if (!amsMgr) {
        HILOG_ERROR("abilitymgr instance is nullptr");
        result = ERR_NO_INIT;
    } else {
        if (!reply.WriteRemoteObject(amsMgr->AsObject())) {
            HILOG_ERROR("failed to reply abilitymgr instance to client, for write parcel error");
            result = ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleClearUpApplicationData(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t result = ClearUpApplicationData(bundleName);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningProcesses(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::vector<RunningProcessInfo> info;
    auto result = GetAllRunningProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessRunningInfosByUserId(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t userId = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetProcessRunningInfosByUserId(info, userId);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetSystemMemoryAttr(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    SystemMemoryAttr memoryInfo;
    std::string strConfig;
    data.ReadString(strConfig);
    GetSystemMemoryAttr(memoryInfo, strConfig);
    if (reply.WriteParcelable(&memoryInfo)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAddAbilityStageDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t recordId = data.ReadInt32();
    AddAbilityStageDone(recordId);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartupResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t infoSize = data.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppExecFwk::BundleInfo> bundleInfo(data.ReadParcelable<AppExecFwk::BundleInfo>());
        if (!bundleInfo) {
            HILOG_ERROR("Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        bundleInfos.emplace_back(*bundleInfo);
    }
    StartupResidentProcess(bundleInfos);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    int32_t result = RegisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    int32_t result = UnregisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}


int32_t AppMgrStub::HandleGetForegroundApplications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppStateData> appStateDatas;
    int32_t result = GetForegroundApplications(appStateDatas);
    reply.WriteInt32(appStateDatas.size());
    for (auto &it : appStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleStartUserTestProcess(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<BundleInfo> bundleInfo(data.ReadParcelable<BundleInfo>());
    if (bundleInfo == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    int32_t userId = data.ReadInt32();
    int32_t result = StartUserTestProcess(*want, observer, *bundleInfo, userId);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::HandleFinishUserTest(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int resultCode = data.ReadInt32();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::GetForegroundApplications(std::vector<AppStateData> &list)
{
    return NO_ERROR;
}

int32_t AppMgrStub::HandleScheduleAcceptWantDone(MessageParcel &data, MessageParcel &reply)
{
    auto recordId = data.ReadInt32();
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadString();

    ScheduleAcceptWantDone(recordId, *want, flag);
    delete want;
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAbilityRecordsByProcessID(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    std::vector<sptr<IRemoteObject>> tokens;
    auto result = GetAbilityRecordsByProcessID(pid, tokens);
    reply.WriteInt32(tokens.size());
    for (auto &it : tokens) {
        if (!reply.WriteRemoteObject(it)) {
            HILOG_ERROR("failed to write query result.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    std::string renderParam = data.ReadString();
    int32_t ipcFd = data.ReadFileDescriptor();
    int32_t sharedFd = data.ReadFileDescriptor();
    int32_t renderPid = 0;
    int32_t result = StartRenderProcess(renderParam, ipcFd, sharedFd, renderPid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(renderPid)) {
        HILOG_ERROR("write renderPid error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleAttachRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> scheduler = data.ReadRemoteObject();
    AttachRenderProcess(scheduler);
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePostANRTaskByProcessID(MessageParcel &data, MessageParcel &reply)
{
    auto pid = data.ReadInt32();
    PostANRTaskByProcessID(pid);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleBlockAppServiceDone(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s", __func__);
    int32_t result = BlockAppService();
    reply.WriteInt32(result);
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
