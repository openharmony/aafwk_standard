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
#include "ability_scheduler_proxy.h"

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
bool AbilitySchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilitySchedulerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AbilitySchedulerProxy::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &stateInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&want);
    data.WriteParcelable(&stateInfo);
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleAbilityTransaction fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::SendResult(int requestCode, int resultCode, const Want &resultWant)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(requestCode);
    data.WriteInt32(resultCode);
    if (!data.WriteParcelable(&resultWant)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SEND_RESULT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("SendResult fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleConnectAbility(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_CONNECT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleConnectAbility fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleDisconnectAbility(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleDisconnectAbility fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!data.WriteBool(restart)) {
        HILOG_ERROR("fail to WriteBool");
        return;
    }
    HILOG_INFO("WriteInt32,startId:%{public}d", startId);
    if (!data.WriteInt32(startId)) {
        HILOG_ERROR("fail to WriteInt32");
        return;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleCommandAbility fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleSaveAbilityState()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_SAVE_ABILITY_STATE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleSaveAbilityState fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleRestoreAbilityState(const PacMap &inState)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&inState)) {
        HILOG_ERROR("WriteParcelable error");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleRestoreAbilityState fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&config)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_UPDATE_CONFIGURATION, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ScheduleRestoreAbilityState fail to SendRequest. err: %{public}d", err);
    }
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> AbilitySchedulerProxy::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> types;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return types;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return types;
    }

    if (!data.WriteString(mimeTypeFilter)) {
        HILOG_ERROR("fail to WriteString mimeTypeFilter");
        return types;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_GETFILETYPES, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("GetFileTypes fail to SendRequest. err: %{public}d", err);
    }

    if (!reply.ReadStringVector(&types)) {
        HILOG_ERROR("fail to ReadStringVector types");
    }

    return types;
}

/**
 * @brief Opens a file in a specified remote path.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the file descriptor.
 */
int AbilitySchedulerProxy::OpenFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        HILOG_ERROR("fail to WriteString mode");
        return fd;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_OPENFILE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("OpenFile fail to SendRequest. err: %{public}d", err);
        return fd;
    }

    fd = reply.ReadFileDescriptor();
    if (fd == -1) {
        HILOG_ERROR("fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int AbilitySchedulerProxy::OpenRawFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        HILOG_ERROR("fail to WriteString mode");
        return fd;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_OPENRAWFILE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("OpenFile fail to SendRequest. err: %{public}d", err);
        return fd;
    }

    if (!reply.ReadInt32(fd)) {
        HILOG_ERROR("fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int AbilitySchedulerProxy::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&value)) {
        HILOG_ERROR("fail to WriteParcelable value");
        return index;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_INSERT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("Insert fail to SendRequest. err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
std::shared_ptr<AppExecFwk::PacMap> AbilitySchedulerProxy::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteString(method)) {
        HILOG_ERROR("fail to WriteString method");
        return nullptr;
    }

    if (!data.WriteString(arg)) {
        HILOG_ERROR("fail to WriteString arg");
        return nullptr;
    }

    if (!data.WriteParcelable(&pacMap)) {
        HILOG_ERROR("fail to WriteParcelable pacMap");
        return nullptr;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_CALL, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("Call fail to SendRequest. err: %{public}d", err);
        return nullptr;
    }
    std::shared_ptr<AppExecFwk::PacMap> result(reply.ReadParcelable<AppExecFwk::PacMap>());
    if (!result) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return nullptr;
    }
    return result;
}

/**
 * @brief Updates data records in the database.
 *
 * @param uri Indicates the path of data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records updated.
 */
int AbilitySchedulerProxy::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&value)) {
        HILOG_ERROR("fail to WriteParcelable value");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return index;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_UPDATE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("Update fail to SendRequest. err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records deleted.
 */
int AbilitySchedulerProxy::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return index;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_DELETE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("Delete fail to SendRequest. err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of data to query.
 * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the query result.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilitySchedulerProxy::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteStringVector(columns)) {
        HILOG_ERROR("fail to WriteStringVector columns");
        return nullptr;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return nullptr;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_QUERY, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("Query fail to SendRequest. err: %{public}d", err);
        return nullptr;
    }
    return OHOS::NativeRdb::ISharedResultSet::ReadFromParcel(reply);
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string AbilitySchedulerProxy::GetType(const Uri &uri)
{
    std::string type;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return type;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return type;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_GETTYPE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("GetFileTypes fail to SendRequest. err: %{public}d", err);
        return type;
    }

    type = reply.ReadString();
    if (type.empty()) {
        HILOG_ERROR("fail to ReadString type");
        return type;
    }

    return type;
}

/**
 * @brief Reloads data in the database.
 *
 * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
 * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
 * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
 * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool AbilitySchedulerProxy::Reload(const Uri &uri, const PacMap &extras)
{
    bool ret = false;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return ret;
    }

    if (!data.WriteParcelable(&extras)) {
        HILOG_ERROR("fail to WriteParcelable extras");
        return ret;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_RELOAD, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("GetFileTypes fail to SendRequest. err: %{public}d", err);
        return ret;
    }

    ret = reply.ReadBool();
    if (!ret) {
        HILOG_ERROR("fail to ReadBool ret");
        return ret;
    }

    return ret;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int AbilitySchedulerProxy::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return ret;
    }

    int count = (int)values.size();
    if (!data.WriteInt32(count)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ret;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(&values[i])) {
            HILOG_ERROR("fail to WriteParcelable ret, index = %{public}d", i);
            return ret;
        }
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_BATCHINSERT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("GetFileTypes fail to SendRequest. err: %{public}d", err);
        return ret;
    }

    if (!reply.ReadInt32(ret)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return ret;
    }

    return ret;
}

/**
 * @brief notify multi window mode changed.
 *
 * @param winModeKey Indicates ability Window display mode.
 * @param flag Indicates this ability has been enter this mode.
 */
void AbilitySchedulerProxy::NotifyMultiWinModeChanged(int32_t winModeKey, bool flag)
{
#ifdef SUPPORT_GRAPHICS
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(winModeKey)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!data.WriteBool(flag)) {
        HILOG_ERROR("fail to WriteBool");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::MULTI_WIN_CHANGED, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("NotifyMultiWinModeChanged fail to SendRequest. err: %{public}d", err);
    }
#endif
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilitySchedulerProxy::ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        HILOG_ERROR("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    int32_t result = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_REGISTEROBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        HILOG_INFO("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        HILOG_ERROR("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilitySchedulerProxy::ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        HILOG_ERROR("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    int32_t result = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_UNREGISTEROBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        HILOG_INFO("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        HILOG_ERROR("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
bool AbilitySchedulerProxy::ScheduleNotifyChange(const Uri &uri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    int32_t result = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_NOTIFYCHANGE, data, reply, option);
    if (result == ERR_NONE) {
        HILOG_INFO("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        HILOG_ERROR("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

/**
 * @brief notify this ability is top active ability.
 *
 * @param flag true: Indicates this ability is top active ability
 */
void AbilitySchedulerProxy::NotifyTopActiveAbilityChanged(bool flag)
{
#ifdef SUPPORT_GRAPHICS
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteBool(flag)) {
        HILOG_ERROR("fail to WriteBool");
        return;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::TOP_ACTIVE_ABILITY_CHANGED, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("NotifyTopActiveAbilityChanged fail to SendRequest. err: %{public}d", err);
    }
#endif
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
 * any method that is called on the Data ability must require normalization verification and denormalization. The
 * default implementation of this method returns null, indicating that this Data ability does not support URI
 * normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri AbilitySchedulerProxy::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return urivalue;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_NORMALIZEURI, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("NormalizeUri fail to SendRequest. err: %{public}d", err);
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return Uri("");
    }
    return *info;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
 * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
 * in the current environment.
 */
Uri AbilitySchedulerProxy::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return urivalue;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_DENORMALIZEURI, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("DenormalizeUri fail to SendRequest. err: %{public}d", err);
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return Uri("");
    }
    return *info;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> AbilitySchedulerProxy::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("AbilitySchedulerProxy::ExecuteBatch start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results;
    results.clear();

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to Writer token");
        return results;
    }

    int count = (int)operations.size();
    if (!data.WriteInt32(count)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to WriteInt32 ret");
        return results;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(operations[i].get())) {
            HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to WriteParcelable ret, index = %{public}d", i);
            return results;
        }
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::SCHEDULE_EXECUTEBATCH, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to SendRequest. err: %{public}d", err);
        return results;
    }

    int total = 0;
    if (!reply.ReadInt32(total)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to ReadInt32 count %{public}d", total);
        return results;
    }

    for (int i = 0; i < total; i++) {
        AppExecFwk::DataAbilityResult *result = reply.ReadParcelable<AppExecFwk::DataAbilityResult>();
        if (result == nullptr) {
            HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch result is nullptr, index = %{public}d", i);
            return results;
        }
        std::shared_ptr<AppExecFwk::DataAbilityResult> dataAbilityResult(result);
        results.push_back(dataAbilityResult);
    }
    HILOG_INFO("AbilitySchedulerProxy::ExecuteBatch end %{public}d", total);
    return results;
}

void AbilitySchedulerProxy::ContinueAbility(const std::string& deviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("ContinueAbility fail to write token");
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("ContinueAbility fail to write deviceId");
        return;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::CONTINUE_ABILITY, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("ContinueAbility fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::NotifyContinuationResult(int32_t result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("NotifyContinuationResult fail to write token");
        return;
    }
    if (!data.WriteInt32(result)) {
        HILOG_ERROR("NotifyContinuationResult fail to write result");
        return;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::NOTIFY_CONTINUATION_RESULT, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("NotifyContinuationResult fail to SendRequest. err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("DumpAbilityRunner fail to write token");
        return;
    }

    if (!data.WriteStringVector(params)) {
        HILOG_ERROR("DumpAbilityRunner fail to write params");
        return;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::DUMP_ABILITY_RUNNER_INNER, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("DumpAbilityRunner fail to SendRequest. err: %{public}d", err);
    }

    int32_t stackNum = reply.ReadInt32();
    for (int i = 0; i < stackNum; i++) {
        std::string stac = Str16ToStr8(reply.ReadString16());
        info.emplace_back(stac);
    }
}

sptr<IRemoteObject> AbilitySchedulerProxy::CallRequest()
{
    HILOG_INFO("AbilitySchedulerProxy::CallRequest start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    int32_t err = Remote()->SendRequest(IAbilityScheduler::REQUEST_CALL_REMOTE, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("CallRequest fail to SendRequest. err: %{public}d", err);
        return nullptr;
    }

    int32_t result = reply.ReadInt32();
    if (result != ERR_OK) {
        HILOG_ERROR("CallRequest failed, err %{public}d", result);
        return nullptr;
    }
    auto call = reply.ReadRemoteObject();
    if (call == nullptr) {
        HILOG_ERROR("CallRequest failed, err remoteObject is nullptr");
        return nullptr;
    }

    HILOG_INFO("AbilitySchedulerProxy::CallRequest end");
    return call;
}

int AbilitySchedulerProxy::BlockAbility()
{
    HILOG_INFO("AbilitySchedulerProxy::BlockAbility start");
    int ret = -1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }
    int32_t err = Remote()->SendRequest(IAbilityScheduler::BLOCK_ABILITY_INNER, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("BlockAbility fail to SendRequest. err: %d", err);
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        HILOG_ERROR("fail to ReadInt32 ret");
        return ret;
    }
    return ret;
}
}  // namespace AAFwk
}  // namespace OHOS