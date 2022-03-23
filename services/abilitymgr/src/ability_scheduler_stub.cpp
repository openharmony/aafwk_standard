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

#include "abs_shared_result_set.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "ishared_result_set.h"
#include "pac_map.h"
#include "values_bucket.h"
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
    requestFuncMap_[SCHEDULE_CALL] = &AbilitySchedulerStub::CallInner;
    requestFuncMap_[SCHEDULE_GETTYPE] = &AbilitySchedulerStub::GetTypeInner;
    requestFuncMap_[SCHEDULE_RELOAD] = &AbilitySchedulerStub::ReloadInner;
    requestFuncMap_[SCHEDULE_BATCHINSERT] = &AbilitySchedulerStub::BatchInsertInner;
    requestFuncMap_[SCHEDULE_REGISTEROBSERVER] = &AbilitySchedulerStub::RegisterObserverInner;
    requestFuncMap_[SCHEDULE_UNREGISTEROBSERVER] = &AbilitySchedulerStub::UnregisterObserverInner;
    requestFuncMap_[SCHEDULE_NOTIFYCHANGE] = &AbilitySchedulerStub::NotifyChangeInner;
#ifdef SUPPORT_GRAPHICS
    requestFuncMap_[MULTI_WIN_CHANGED] = &AbilitySchedulerStub::MutiWinModeChangedInner;
#endif
    requestFuncMap_[SCHEDULE_NORMALIZEURI] = &AbilitySchedulerStub::NormalizeUriInner;
    requestFuncMap_[SCHEDULE_DENORMALIZEURI] = &AbilitySchedulerStub::DenormalizeUriInner;
    requestFuncMap_[SCHEDULE_UPDATE_CONFIGURATION] = &AbilitySchedulerStub::UpdateConfigurationInner;
    requestFuncMap_[SCHEDULE_EXECUTEBATCH] = &AbilitySchedulerStub::ExecuteBatchInner;
#ifdef SUPPORT_GRAPHICS
    requestFuncMap_[TOP_ACTIVE_ABILITY_CHANGED] = &AbilitySchedulerStub::TopActiveAbilityChangedInner;
#endif
    requestFuncMap_[NOTIFY_CONTINUATION_RESULT] = &AbilitySchedulerStub::NotifyContinuationResultInner;
    requestFuncMap_[REQUEST_CALL_REMOTE] = &AbilitySchedulerStub::CallRequestInner;
    requestFuncMap_[CONTINUE_ABILITY] = &AbilitySchedulerStub::ContinueAbilityInner;
    requestFuncMap_[DUMP_ABILITY_RUNNER_INNER] = &AbilitySchedulerStub::DumpAbilityInfoInner;
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
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
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
    return NO_ERROR;
}

int AbilitySchedulerStub::SendResultInner(MessageParcel &data, MessageParcel &reply)
{
    int requestCode = data.ReadInt32();
    int resultCode = data.ReadInt32();
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    SendResult(requestCode, resultCode, *want);
    return NO_ERROR;
}

int AbilitySchedulerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleConnectAbility(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleDisconnectAbility(*want);
    return NO_ERROR;
}

int AbilitySchedulerStub::CommandAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub want is nullptr");
        return ERR_INVALID_VALUE;
    }
    bool reStart = data.ReadBool();
    int startId = data.ReadInt32();
    HILOG_INFO("ReadInt32, startId:%{public}d", startId);
    ScheduleCommandAbility(*want, reStart, startId);
    return NO_ERROR;
}

int AbilitySchedulerStub::SaveAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    ScheduleSaveAbilityState();
    return NO_ERROR;
}

int AbilitySchedulerStub::RestoreAbilityStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<PacMap> pacMap(data.ReadParcelable<PacMap>());
    if (pacMap == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub RestoreAbilityState is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleRestoreAbilityState(*pacMap);
    return NO_ERROR;
}

int AbilitySchedulerStub::GetFileTypesInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
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
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
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
    if (fd < 0) {
        HILOG_ERROR("OpenFile fail, fd is %{pubilc}d", fd);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteFileDescriptor(fd)) {
        HILOG_ERROR("fail to WriteFileDescriptor fd");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::OpenRawFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
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
    return NO_ERROR;
}

int AbilitySchedulerStub::InsertInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Insert(*uri, *value);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("AbilitySchedulerStub::InsertInner end");
    return NO_ERROR;
}

int AbilitySchedulerStub::CallInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string method = data.ReadString();
    if (method.empty()) {
        HILOG_ERROR("ReadParcelable method is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string arg = data.ReadString();
    if (arg.empty()) {
        HILOG_ERROR("ReadParcelable arg is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AppExecFwk::PacMap> pacMap(data.ReadParcelable<AppExecFwk::PacMap>());
    if (pacMap == nullptr) {
        HILOG_ERROR("ReadParcelable pacMap is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<AppExecFwk::PacMap> result = Call(*uri, method, arg, *pacMap);
    if (!reply.WriteParcelable(result.get())) {
        HILOG_ERROR("fail to WriteParcelable pacMap error");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("AbilitySchedulerStub::CallInner end");
    return NO_ERROR;
}

int AbilitySchedulerStub::UpdatetInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Update(*uri, *value, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::DeleteInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Delete(*uri, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::QueryInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> columns;
    if (!data.ReadStringVector(&columns)) {
        HILOG_ERROR("fail to ReadStringVector columns");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto resultSet = Query(*uri, columns, *predicates);
    if (resultSet == nullptr) {
        HILOG_ERROR("fail to WriteParcelable resultSet");
        return ERR_INVALID_VALUE;
    }
    auto result = NativeRdb::ISharedResultSet::WriteToParcel(std::move(resultSet), reply);
    if (result == nullptr) {
        HILOG_ERROR("!resultSet->Marshalling(reply)");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("AbilitySchedulerStub::QueryInner end");
    return NO_ERROR;
}

int AbilitySchedulerStub::GetTypeInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string type = GetType(*uri);
    if (!reply.WriteString(type)) {
        HILOG_ERROR("fail to WriteString type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::ReloadInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<PacMap> extras(data.ReadParcelable<PacMap>());
    if (extras == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub extras is nullptr");
        return ERR_INVALID_VALUE;
    }
    bool ret = Reload(*uri, *extras);
    if (!reply.WriteBool(ret)) {
        HILOG_ERROR("fail to writeBool ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::BatchInsertInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    int count = 0;
    if (!data.ReadInt32(count)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return ERR_INVALID_VALUE;
    }

    std::vector<NativeRdb::ValuesBucket> values;
    for (int i = 0; i < count; i++) {
        std::unique_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
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
    return NO_ERROR;
}

int AbilitySchedulerStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<IDataAbilityObserver>(data.ReadParcelable<IRemoteObject>());
    if (obServer == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub obServer is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleRegisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::UnregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<IDataAbilityObserver>(data.ReadParcelable<IRemoteObject>());
    if (obServer == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub obServer is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleUnregisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::NotifyChangeInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = ScheduleNotifyChange(*uri);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::NormalizeUriInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = NormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        HILOG_ERROR("fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::DenormalizeUriInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = DenormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        HILOG_ERROR("fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilitySchedulerStub::UpdateConfigurationInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AppExecFwk::Configuration> globalConfiguration(data.ReadParcelable<AppExecFwk::Configuration>());
    if (globalConfiguration == nullptr) {
        HILOG_ERROR("AbilitySchedulerStub globalConfiguration is nullptr");
        return ERR_INVALID_VALUE;
    }
    ScheduleUpdateConfiguration(*globalConfiguration);
    return NO_ERROR;
}

#ifdef SUPPORT_GRAPHICS
int AbilitySchedulerStub::MutiWinModeChangedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t winModeKey = data.ReadInt32();
    bool flag = data.ReadBool();
    NotifyMultiWinModeChanged(winModeKey, flag);
    return NO_ERROR;
}

int AbilitySchedulerStub::TopActiveAbilityChangedInner(MessageParcel &data, MessageParcel &reply)
{
    bool flag = data.ReadBool();
    NotifyTopActiveAbilityChanged(flag);
    return NO_ERROR;
}
#endif

int AbilitySchedulerStub::ExecuteBatchInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("AbilitySchedulerStub::ExecuteBatchInner start");
    int count = 0;
    if (!data.ReadInt32(count)) {
        HILOG_ERROR("AbilitySchedulerStub::ExecuteBatchInner fail to ReadInt32 count");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("AbilitySchedulerStub::ExecuteBatchInner count:%{public}d", count);
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> operations;
    for (int i = 0; i < count; i++) {
        AppExecFwk::DataAbilityOperation *operation = data.ReadParcelable<AppExecFwk::DataAbilityOperation>();
        if (operation == nullptr) {
            HILOG_ERROR("AbilitySchedulerStub::ExecuteBatchInner operation is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        std::shared_ptr<AppExecFwk::DataAbilityOperation> dataAbilityOperation(operation);
        operations.push_back(dataAbilityOperation);
    }

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results = ExecuteBatch(operations);
    int total = (int)results.size();
    if (!reply.WriteInt32(total)) {
        HILOG_ERROR("AbilitySchedulerStub::ExecuteBatchInner fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("AbilitySchedulerStub::ExecuteBatchInner total:%{public}d", total);
    for (int i = 0; i < total; i++) {
        if (results[i] == nullptr) {
            HILOG_ERROR("AbilitySchedulerStub::ExecuteBatchInner results[i] is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        if (!reply.WriteParcelable(results[i].get())) {
            HILOG_ERROR(
                "AbilitySchedulerStub::ExecuteBatchInner fail to WriteParcelable operation, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
    }
    HILOG_INFO("AbilitySchedulerStub::ExecuteBatchInner end");
    return NO_ERROR;
}

int AbilitySchedulerStub::ContinueAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    ContinueAbility(deviceId);
    return NO_ERROR;
}

int AbilitySchedulerStub::NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = data.ReadInt32();
    NotifyContinuationResult(result);
    return NO_ERROR;
}

int AbilitySchedulerStub::DumpAbilityInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> infos;
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        HILOG_INFO("DumpAbilityInfoInner read params error");
        return NO_ERROR;
    }
    DumpAbilityInfo(params, infos);

    for (const auto & infostep:infos) {
        HILOG_INFO("DumpAbilityInfoInner infos = %{public}s", infostep.c_str());
    }

    reply.WriteInt32(infos.size());
    for (auto stack : infos) {
        reply.WriteString16(Str8ToStr16(stack));
    }

    return NO_ERROR;
}
int AbilitySchedulerStub::CallRequestInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("AbilitySchedulerStub::CallRequestInner start");

    sptr<IRemoteObject> call = CallRequest();
    if (!call) {
        HILOG_ERROR("call request return nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(NO_ERROR)) {
        HILOG_ERROR("GetAllStackInfo result error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteRemoteObject(call)) {
        HILOG_ERROR("Connect done remote object error.");
        return ERR_INVALID_VALUE;
    }

    HILOG_INFO("AbilitySchedulerStub::CallRequestInner end");

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
