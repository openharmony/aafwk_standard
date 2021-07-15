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

#include "ability_scheduler_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "pac_map.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
AbilitySchedulerStub::AbilitySchedulerStub()
{
    requestFuncMap_[SCHEDULE_ABILITY_TRANSACTION] = &AbilitySchedulerStub::AbilityTransactionInner;
    requestFuncMap_[SEND_RESULT] = &AbilitySchedulerStub::SendResultInner;
    requestFuncMap_[SCHEDULE_ABILITY_CONNECT] = &AbilitySchedulerStub::ConnectAbilityInner;
    requestFuncMap_[SCHEDULE_ABILITY_DISCONNECT] = &AbilitySchedulerStub::DisconnectAbilityInner;
    requestFuncMap_[SCHEDULE_ABILITY_COMMAND] = &AbilitySchedulerStub::CommandAbilityInner;
    requestFuncMap_[SCHEDULE_SAVE_ABILITY_STATE] = &AbilitySchedulerStub::SaveAbilityStateInner;
    requestFuncMap_[SCHEDULE_RESTORE_ABILITY_STATE] = &AbilitySchedulerStub::RestoreAbilityStateInner;
    requestFuncMap_[SCHEDULE_GETFILETYPES] = &AbilitySchedulerStub::GetFileTypesInner;
    requestFuncMap_[SCHEDULE_OPENFILE] = &AbilitySchedulerStub::OpenFileInner;
    requestFuncMap_[SCHEDULE_OPENRAWFILE] = &AbilitySchedulerStub::OpenRawFileInner;
    requestFuncMap_[SCHEDULE_INSERT] = &AbilitySchedulerStub::InsertInner;
    requestFuncMap_[SCHEDULE_UPDATE] = &AbilitySchedulerStub::UpdatetInner;
    requestFuncMap_[SCHEDULE_DELETE] = &AbilitySchedulerStub::DeleteInner;
    requestFuncMap_[SCHEDULE_QUERY] = &AbilitySchedulerStub::QueryInner;
    requestFuncMap_[SCHEDULE_GETTYPE] = &AbilitySchedulerStub::GetTypeInner;
    requestFuncMap_[SCHEDULE_RELOAD] = &AbilitySchedulerStub::ReloadInner;
    requestFuncMap_[SCHEDULE_BATCHINSERT] = &AbilitySchedulerStub::BatchInsertInner;
    requestFuncMap_[DISPLAY_UNLOCK_MISSION_MESSAGE] = &AbilitySchedulerStub::DisplayUnlockMissionMessageInner;
}

AbilitySchedulerStub::~AbilitySchedulerStub()
{
    requestFuncMap_.clear();
}

int AbilitySchedulerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("AbilitySchedulerStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = AbilitySchedulerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("AbilitySchedulerStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilitySchedulerStub::AbilityTransactionInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<LifeCycleStateInfo> stateInfo(data.ReadParcelable<LifeCycleStateInfo>());
    if (!stateInfo) {
        HILOG_ERROR("ReadParcelable<LifeCycleStateInfo> failed");
        return ERR_INVALID_VALUE;
    }
    ScheduleAbilityTransaction(*want, *stateInfo);
    delete want;
    return NO_ERROR;
}

int AbilitySchedulerStub::SendResultInner(MessageParcel &data, MessageParcel &reply)
{
    int requestCode = data.ReadInt32();
    int resultCode = data.ReadInt32();
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    SendResult(requestCode, resultCode, *want);
    delete want;
    return NO_ERROR;
}

int AbilitySchedulerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleConnectAbility(*want);
    delete want;
    return NO_ERROR;
}

int AbilitySchedulerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleDisconnectAbility(*want);
    delete want;
    return NO_ERROR;
}

int AbilitySchedulerStub::CommandAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    bool reStart = data.ReadBool();
    int startId = data.ReadInt32();
    ScheduleCommandAbility(*want, reStart, startId);
    delete want;
    return NO_ERROR;
}

int AbilitySchedulerStub::SaveAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    return NO_ERROR;
}

int AbilitySchedulerStub::RestoreAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("RestoreAbilityStateInner");
    return NO_ERROR;
}

int AbilitySchedulerStub::GetFileTypesInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mimeTypeFilter = data.ReadString();
    if (mimeTypeFilter.empty()) {
        HILOG_ERROR("AbilitySchedulerStub mimeTypeFilter is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> types = GetFileTypes(*uri, mimeTypeFilter);
    if (!reply.WriteStringVector(types)) {
        HILOG_ERROR("fail to WriteStringVector types");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenFileInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        HILOG_ERROR("AbilitySchedulerStub mode is nullptr");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenFile(*uri, mode);
    if (!reply.WriteFileDescriptor(fd)) {
        HILOG_ERROR("fail to WriteFileDescriptor fd");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenRawFileInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        HILOG_ERROR("AbilitySchedulerStub mode is nullptr");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenRawFile(*uri, mode);
    if (!reply.WriteInt32(fd)) {
        HILOG_ERROR("fail to WriteInt32 fd");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    return NO_ERROR;
}

int AbilitySchedulerStub::InsertInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    ValuesBucket *value = data.ReadParcelable<ValuesBucket>();
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Insert(*uri, *value);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    delete value;
    return NO_ERROR;
}

int AbilitySchedulerStub::UpdatetInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    ValuesBucket *value = data.ReadParcelable<ValuesBucket>();
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    DataAbilityPredicates *predicates = data.ReadParcelable<DataAbilityPredicates>();
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Update(*uri, *value, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    delete value;
    delete predicates;
    return NO_ERROR;
}

int AbilitySchedulerStub::DeleteInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    DataAbilityPredicates *predicates = data.ReadParcelable<DataAbilityPredicates>();
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Delete(*uri, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    delete predicates;
    return NO_ERROR;
}

int AbilitySchedulerStub::QueryInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> columns;
    if (!data.ReadStringVector(&columns)) {
        HILOG_ERROR("fail to ReadStringVector columns");
        return ERR_INVALID_VALUE;
    }
    DataAbilityPredicates *predicates = data.ReadParcelable<DataAbilityPredicates>();
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<ResultSet> resultSet = Query(*uri, columns, *predicates);
    ResultSet *resultSetPtr = resultSet.get();
    if (resultSetPtr == nullptr || !reply.WriteParcelable(resultSetPtr)) {
        HILOG_ERROR("fail to WriteParcelable resultSet");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    delete predicates;
    return NO_ERROR;
}

int AbilitySchedulerStub::GetTypeInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string type = GetType(*uri);
    if (!reply.WriteString(type)) {
        HILOG_ERROR("fail to WriteString type");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    return NO_ERROR;
}

int AbilitySchedulerStub::ReloadInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    PacMap *extras = data.ReadParcelable<PacMap>();
    if (extras == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub extras is nullptr");
        return ERR_INVALID_VALUE;
    }
    bool ret = Reload(*uri, *extras);
    if (!reply.WriteBool(ret)) {
        HILOG_ERROR("fail to writeBool ret");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    delete extras;
    return NO_ERROR;
}

int AbilitySchedulerStub::BatchInsertInner(MessageParcel &data, MessageParcel &reply)
{
    Uri *uri = data.ReadParcelable<Uri>();
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    int count = 0;
    if (!data.ReadInt32(count)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return ERR_INVALID_VALUE;
    }

    std::vector<ValuesBucket> values;
    for (int i = 0; i < count; i++) {
        ValuesBucket *value = data.ReadParcelable<ValuesBucket>();
        if (value == nullptr) {
            HILOG_ERROR("AbilitySchedulerStub value is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        values.emplace_back(*value);
    }

    int ret = BatchInsert(*uri, values);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    delete uri;
    return NO_ERROR;
}

int AbilitySchedulerStub::DisplayUnlockMissionMessageInner(MessageParcel &data, MessageParcel &reply)
{
    DisplayUnlockMissionMessage();
    return NO_ERROR;
}

void AbilitySchedulerRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("recv AbilitySchedulerRecipient death notice");

    if (handler_) {
        handler_(remote);
    }
}

AbilitySchedulerRecipient::AbilitySchedulerRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

AbilitySchedulerRecipient::~AbilitySchedulerRecipient()
{}

}  // namespace AAFwk
}  // namespace OHOS
