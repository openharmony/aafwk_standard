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

#include "appexecfwk_errors.h"
#include "form_info.h"
#include "form_mgr_stub.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
const int32_t LIMIT_PARCEL_SIZE = 1024;

void SplitString(const std::string &source, std::vector<std::string> &strings)
{
    size_t splitSize = (source.size() / LIMIT_PARCEL_SIZE);
    if ((source.size() % LIMIT_PARCEL_SIZE) != 0) {
        splitSize++;
    }
    HILOG_DEBUG("the dump string split into %{public}zu size", splitSize);
    for (size_t i = 0; i < splitSize; i++) {
        size_t start = LIMIT_PARCEL_SIZE * i;
        strings.emplace_back(source.substr(start, LIMIT_PARCEL_SIZE));
    }
}

FormMgrStub::FormMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_ADD_FORM)] =
        &FormMgrStub::HandleAddForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DELETE_FORM)] =
        &FormMgrStub::HandleDeleteForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_RELEASE_FORM)] =
        &FormMgrStub::HandleReleaseForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_UPDATE_FORM)] =
        &FormMgrStub::HandleUpdateForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_REQUEST_FORM)] =
        &FormMgrStub::HandleRequestForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_NOTIFY_FORM_WHETHER_VISIBLE)] =
        &FormMgrStub::HandleNotifyWhetherVisibleForms;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_CAST_TEMP_FORM)] =
        &FormMgrStub::HandleCastTempForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_STORAGE_FORM_INFOS)] =
        &FormMgrStub::HandleDumpStorageFormInfos;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_NAME)] =
        &FormMgrStub::HandleDumpFormInfoByBundleName;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_INFOS_BY_ID)] =
        &FormMgrStub::HandleDumpFormInfoByFormId;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_FORM_TIMER_INFO_BY_ID)] =
        &FormMgrStub::HandleDumpFormTimerByFormId;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_SET_NEXT_REFRESH_TIME)] =
        &FormMgrStub::HandleSetNextRefreshTime;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_LIFECYCLE_UPDATE)] =
        &FormMgrStub::HandleLifecycleUpdate;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_MESSAGE_EVENT)] =
        &FormMgrStub::HandleMessageEvent;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_BATCH_ADD_FORM_RECORDS_ST)] =
        &FormMgrStub::HandleBatchAddFormRecords;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_CLEAR_FORM_RECORDS_ST)] =
        &FormMgrStub::HandleClearFormRecords;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DISTRIBUTED_DATA_ADD_FORM__ST)] =
        &FormMgrStub::HandleDistributedDataAddForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DISTRIBUTED_DATA_DELETE_FORM__ST)] =
        &FormMgrStub::HandleDistributedDataDeleteForm;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_DELETE_INVALID_FORMS)] =
        &FormMgrStub::HandleDeleteInvalidForms;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_ACQUIRE_FORM_STATE)] =
        &FormMgrStub::HandleAcquireFormState;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_GET_ALL_FORMS_INFO)] =
        &FormMgrStub::HandleGetAllFormsInfo;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_GET_FORMS_INFO_BY_APP)] =
        &FormMgrStub::HandleGetFormsInfoByApp;
    memberFuncMap_[static_cast<uint32_t>(IFormMgr::Message::FORM_MGR_GET_FORMS_INFO_BY_MODULE)] =
        &FormMgrStub::HandleGetFormsInfoByModule;
}

FormMgrStub::~FormMgrStub()
{
    memberFuncMap_.clear();
}

/**
 * @brief handle remote request.
 * @param data input param.
 * @param reply output param.
 * @param option message option.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("FormMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = FormMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("%{public}s failed, local descriptor is not equal to remote", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
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
/**
 * @brief handle AddForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleAddForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<FormReqInfo>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        HILOG_ERROR("%{public}s, failed to RemoteObject invalidate", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    FormJsInfo formInfo;
    int32_t result = AddForm(formId, *want, client, formInfo);
    reply.WriteInt32(result);
    reply.WriteParcelable(&formInfo);

    return result;
}
/**
 * @brief handle DeleteForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDeleteForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t result = DeleteForm(formId, client);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle ReleaseForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleReleaseForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    bool delCache = data.ReadBool();

    int32_t result = ReleaseForm(formId, client, delCache);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle UpdateForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleUpdateForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::unique_ptr<FormProviderData> formBindingData(data.ReadParcelable<FormProviderData>());
    int32_t result = UpdateForm(formId, *formBindingData);
    reply.WriteInt32(result);
    return result;
}
/**
     * @brief handle SetNextRefreshTime message.
     * @param data input param.
     * @param reply output param.
     * @return Returns ERR_OK on success, others on failure.
     */
int32_t FormMgrStub::HandleSetNextRefreshTime(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    int64_t nextTime = data.ReadInt64();
    int32_t result = SetNextRefreshTime(formId, nextTime);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle LifecycleUpdate message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleLifecycleUpdate(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int64_t> formIds;
    bool ret = data.ReadInt64Vector(&formIds);
    if (!ret) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        HILOG_ERROR("%{public}s, failed to get remote object.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t updateType = data.ReadInt32();
    int32_t result = LifecycleUpdate(formIds, client, updateType);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle RequestForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleRequestForm(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);

    int64_t formId = data.ReadInt64();

    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        HILOG_ERROR("%{public}s, failed to get remote object.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = RequestForm(formId, client, *want);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief handle NotifyVisibleForms message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleNotifyWhetherVisibleForms(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int64_t> formIds;
    bool ret = data.ReadInt64Vector(&formIds);
    if (!ret) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t formVisibleType = data.ReadInt32();

    int32_t result = NotifyWhetherVisibleForms(formIds, client, formVisibleType);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief handle CastTempForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleCastTempForm(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = CastTempForm(formId, client);
    reply.WriteInt32(result);
    return result;
}
/**
 * @brief Handle DumpStorageFormInfos message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpStorageFormInfos(MessageParcel &data, MessageParcel &reply)
{
    std::string formInfos;
    int32_t result = DumpStorageFormInfos(formInfos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(formInfos, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            HILOG_ERROR("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    return result;
}
/**
 * @brief Handle DumpFormInfoByBundleName message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormInfoByBundleName(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    std::string formInfos;
    int32_t result = DumpFormInfoByBundleName(bundleName, formInfos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        HILOG_DEBUG("%{public}s, formInfos: %{public}s", __func__, formInfos.c_str());
        std::vector<std::string> dumpInfos;
        SplitString(formInfos, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            HILOG_ERROR("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    return result;
}
/**
 * @brief Handle DumpFormInfoByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormInfoByFormId(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::string formInfo;
    int32_t result = DumpFormInfoByFormId(formId, formInfo);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(formInfo, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            HILOG_ERROR("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}
/**
 * @brief Handle DumpFormTimerByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDumpFormTimerByFormId(MessageParcel &data, MessageParcel &reply)
{
    int64_t formId = data.ReadInt64();
    std::string isTimingService;
    int32_t result = DumpFormTimerByFormId(formId, isTimingService);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        std::vector<std::string> dumpInfos;
        SplitString(isTimingService, dumpInfos);
        if (!reply.WriteStringVector(dumpInfos)) {
            HILOG_ERROR("%{public}s, failed to WriteStringVector<dumpInfos>", __func__);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}

/**
 * @brief Handle DumpFormInfoByFormId message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleMessageEvent(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    int64_t formId = data.ReadInt64();
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        HILOG_ERROR("%{public}s, failed to get remote object.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = MessageEvent(formId, *want, client);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle BatchAddFormRecords message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleBatchAddFormRecords(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = BatchAddFormRecords(*want);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle BatchDeleteFormRecords message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleClearFormRecords(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    int32_t result = ClearFormRecords();
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle DistributedDataAddForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDistributedDataAddForm(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);

    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<Want>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = DistributedDataAddForm(*want);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle DistributedDataDeleteForm message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDistributedDataDeleteForm(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);

    std::string formId = data.ReadString();
    if (formId.empty()) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable<int64_t>", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t result = DistributedDataDeleteForm(formId);
    reply.WriteInt32(result);
    return result;
}

/**
 * @brief Handle DeleteInvalidForms message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleDeleteInvalidForms(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::vector<int64_t> formIds;
    if (!data.ReadInt64Vector(&formIds)) {
        HILOG_ERROR("%{public}s, failed to ReadInt64Vector", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s, failed to get remote object.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t numFormsDeleted = 0;
    int32_t result = DeleteInvalidForms(formIds, callerToken, numFormsDeleted);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("%{public}s, failed to write result", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!reply.WriteInt32(numFormsDeleted)) {
        HILOG_ERROR("%{public}s, failed to write numFormsDeleted", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return result;
}

/**
 * @brief Handle AcquireFormState message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleAcquireFormState(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormStateInfo stateInfo {};
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("%{public}s, failed to ReadParcelable want", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s, failed to get remote object.", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t result = AcquireFormState(*want, callerToken, stateInfo);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("%{public}s, failed to write result", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!reply.WriteInt32((int32_t)stateInfo.state)) {
        HILOG_ERROR("%{public}s, failed to write state", __func__);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return result;
}

/**
 * @brief Handle GetAllFormsInfo message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleGetAllFormsInfo(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::vector<FormInfo> infos;
    int32_t result = GetAllFormsInfo(infos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        if (!WriteParcelableVector(infos, reply)) {
            HILOG_ERROR("write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}

/**
 * @brief Handle GetFormsInfoByApp message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleGetFormsInfoByApp(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string bundleName = data.ReadString();
    std::vector<FormInfo> infos;
    int32_t result = GetFormsInfoByApp(bundleName, infos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        if (!WriteParcelableVector(infos, reply)) {
            HILOG_ERROR("write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}

/**
 * @brief Handle GetFormsInfoByModule message.
 * @param data input param.
 * @param reply output param.
 * @return Returns ERR_OK on success, others on failure.
 */
int32_t FormMgrStub::HandleGetFormsInfoByModule(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string bundleName = data.ReadString();
    std::string moduleName = data.ReadString();
    std::vector<FormInfo> infos;
    int32_t result = GetFormsInfoByModule(bundleName, moduleName, infos);
    reply.WriteInt32(result);
    if (result == ERR_OK) {
        if (!WriteParcelableVector(infos, reply)) {
            HILOG_ERROR("write failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return result;
}

/**
 * @brief Write a parcelabe vector objects to the proxy node.
 * @param parcelableVector Indicates the objects to be write.
 * @param reply Indicates the reply to be sent;
 * @return Returns true if objects send successfully; returns false otherwise.
 */
template<typename T>
bool FormMgrStub::WriteParcelableVector(std::vector<T> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        HILOG_ERROR("write ParcelableVector failed");
        return false;
    }

    for (auto &parcelable: parcelableVector) {
        if (!reply.WriteParcelable(&parcelable)) {
            HILOG_ERROR("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
