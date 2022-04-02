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

#include "ams_mgr_stub.h"

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

namespace OHOS {
namespace AppExecFwk {
AmsMgrStub::AmsMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY)] = &AmsMgrStub::HandleLoadAbility;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY)] =
        &AmsMgrStub::HandleTerminateAbility;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE)] =
        &AmsMgrStub::HandleUpdateAbilityState;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE)] =
        &AmsMgrStub::HandleUpdateExtensionState;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK)] =
        &AmsMgrStub::HandleRegisterAppStateCallback;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::ABILITY_BEHAVIOR_ANALYSIS)] =
        &AmsMgrStub::HandleAbilityBehaviorAnalysis;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN)] =
        &AmsMgrStub::HandleKillProcessByAbilityToken;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID)] =
        &AmsMgrStub::HandleKillProcessesByUserId;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT)] =
        &AmsMgrStub::HandleKillProcessWithAccount;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION)] = &AmsMgrStub::HandleKillApplication;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT)] =
        &AmsMgrStub::HandleAbilityAttachTimeOut;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY)] =
        &AmsMgrStub::HandlePrepareTerminate;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID)] =
        &AmsMgrStub::HandleKillApplicationByUid;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN)] =
        &AmsMgrStub::HandleGetRunningProcessInfoByToken;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY)] =
        &AmsMgrStub::HandleStartSpecifiedAbility;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE)] =
        &AmsMgrStub::HandleRegisterStartSpecifiedAbilityResponse;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::UPDATE_CONFIGURATION)] =
        &AmsMgrStub::HandleUpdateConfiguration;
    memberFuncMap_[static_cast<uint32_t>(IAmsMgr::Message::GET_CONFIGURATION)] =
        &AmsMgrStub::HandleGetConfiguration;
}

AmsMgrStub::~AmsMgrStub()
{
    memberFuncMap_.clear();
}

int AmsMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("AmsMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AmsMgrStub::GetDescriptor();
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

ErrCode AmsMgrStub::HandleLoadAbility(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToke = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        preToke = data.ReadRemoteObject();
    }
    std::shared_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (!abilityInfo) {
        HILOG_ERROR("ReadParcelable<AbilityInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<ApplicationInfo> appInfo(data.ReadParcelable<ApplicationInfo>());
    if (!appInfo) {
        HILOG_ERROR("ReadParcelable<ApplicationInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (!want) {
        HILOG_ERROR("ReadParcelable want failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    LoadAbility(token, preToke, abilityInfo, appInfo, want);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleTerminateAbility(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    TerminateAbility(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateAbilityState(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateAbilityState(token, static_cast<AbilityState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateExtensionState(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateExtensionState(token, static_cast<ExtensionState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleRegisterAppStateCallback(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IAppStateCallback> callback = nullptr;
    if (data.ReadBool()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        callback = iface_cast<IAppStateCallback>(obj);
    }
    RegisterAppStateCallback(callback);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAbilityBehaviorAnalysis(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> preToke = nullptr;
    if (data.ReadBool()) {
        preToke = data.ReadRemoteObject();
    }
    int32_t visibility = data.ReadInt32();
    int32_t Perceptibility = data.ReadInt32();
    int32_t connectionState = data.ReadInt32();

    AbilityBehaviorAnalysis(token, preToke, visibility, Perceptibility, connectionState);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessByAbilityToken(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();

    KillProcessByAbilityToken(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByUserId(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    int32_t userId = data.ReadInt32();

    KillProcessesByUserId(userId);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessWithAccount(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("enter");

    BYTRACE(BYTRACE_TAG_APP);

    std::string bundleName = data.ReadString();
    int accountId = data.ReadInt32();

    HILOG_INFO("bundleName = %{public}s, accountId = %{public}d", bundleName.c_str(), accountId);

    int32_t result = KillProcessWithAccount(bundleName, accountId);
    reply.WriteInt32(result);

    HILOG_INFO("end");

    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplication(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t result = KillApplication(bundleName);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplicationByUid(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int uid = data.ReadInt32();
    int32_t result = KillApplicationByUid(bundleName, uid);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleAbilityAttachTimeOut(MessageParcel &data, MessageParcel &reply)
{
    BYTRACE(BYTRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityAttachTimeOut(token);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandlePrepareTerminate(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    PrepareTerminate(token);
    return NO_ERROR;
}

void AmsMgrStub::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{}

int32_t AmsMgrStub::HandleGetRunningProcessInfoByToken(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto token = data.ReadRemoteObject();
    GetRunningProcessInfoByToken(token, processInfo);
    if (reply.WriteParcelable(&processInfo)) {
        HILOG_ERROR("process info write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleStartSpecifiedAbility(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    AbilityInfo *abilityInfo = data.ReadParcelable<AbilityInfo>();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr.");
        delete want;
        return ERR_INVALID_VALUE;
    }
    StartSpecifiedAbility(*want, *abilityInfo);
    delete want;
    delete abilityInfo;
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterStartSpecifiedAbilityResponse(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    sptr<IStartSpecifiedAbilityResponse> response = iface_cast<IStartSpecifiedAbilityResponse>(obj);
    RegisterStartSpecifiedAbilityResponse(response);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleUpdateConfiguration(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (config) {
        UpdateConfiguration(*config);
        return NO_ERROR;
    }
    return UNKNOWN_ERROR;
}

int32_t AmsMgrStub::HandleGetConfiguration(MessageParcel &data, MessageParcel &reply)
{
    Configuration config;
    int ret = GetConfiguration(config);
    if (ret != ERR_OK) {
        HILOG_ERROR("GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&config)) {
        HILOG_ERROR("GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS