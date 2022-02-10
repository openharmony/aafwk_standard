/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "datashare_proxy.h"

#include <ipc_types.h>
#include <string_ex.h>

#include "abs_shared_result_set.h"
#include "app_log_wrapper.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "ipc_types.h"
#include "ishared_result_set.h"
#include "pac_map.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
std::vector<std::string> DataShareProxy::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("%{public}s begin.", __func__);
    std::vector<std::string> types;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return types;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return types;
    }

    if (!data.WriteString(mimeTypeFilter)) {
        APP_LOGE("fail to WriteString mimeTypeFilter");
        return types;
    }

    int32_t err = Remote()->SendRequest(CMD_GET_FILE_TYPES, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("GetFileTypes fail to SendRequest. err: %d", err);
    }

    if (!reply.ReadStringVector(&types)) {
        APP_LOGE("fail to ReadStringVector types");
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return types;
}

int DataShareProxy::OpenFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("%{public}s begin.", __func__);
    int fd = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        APP_LOGE("fail to WriteString mode");
        return fd;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_OPEN_FILE, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("OpenFile fail to SendRequest. err: %d", err);
        return fd;
    }

    fd = reply.ReadFileDescriptor();
    if (fd == -1) {
        APP_LOGE("fail to ReadFileDescriptor fd");
        return fd;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return fd;
}

int DataShareProxy::OpenRawFile(const Uri &uri, const std::string &mode)
{
    APP_LOGI("%{public}s begin.", __func__);
    int fd = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        APP_LOGE("fail to WriteString mode");
        return fd;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_OPEN_RAW_FILE, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("OpenRawFile fail to SendRequest. err: %d", err);
        return fd;
    }

    if (!reply.ReadInt32(fd)) {
        APP_LOGE("fail to ReadInt32 fd");
        return fd;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return fd;
}

int DataShareProxy::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("%{public}s begin.", __func__);
    int index = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&value)) {
        APP_LOGE("fail to WriteParcelable value");
        return index;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_INSERT, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("Insert fail to SendRequest. err: %d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        APP_LOGE("fail to ReadInt32 index");
        return index;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return index;
}

int DataShareProxy::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("%{public}s begin.", __func__);
    int index = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&value)) {
        APP_LOGE("fail to WriteParcelable value");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        APP_LOGE("fail to WriteParcelable predicates");
        return index;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_UPDATE, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("Update fail to SendRequest. err: %d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        APP_LOGE("fail to ReadInt32 index");
        return index;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return index;
}

int DataShareProxy::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("%{public}s begin.", __func__);
    int index = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        APP_LOGE("fail to WriteParcelable predicates");
        return index;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_DELETE, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("Delete fail to SendRequest. err: %d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        APP_LOGE("fail to ReadInt32 index");
        return index;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return index;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> DataShareProxy::Query(const Uri &uri,
    std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("%{public}s begin.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteStringVector(columns)) {
        APP_LOGE("fail to WriteStringVector columns");
        return nullptr;
    }

    if (!data.WriteParcelable(&predicates)) {
        APP_LOGE("fail to WriteParcelable predicates");
        return nullptr;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_QUERY, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("Query fail to SendRequest. err: %d", err);
        return nullptr;
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return OHOS::NativeRdb::ISharedResultSet::ReadFromParcel(reply);
}

std::string DataShareProxy::GetType(const Uri &uri)
{
    APP_LOGI("%{public}s begin.", __func__);
    std::string type;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return type;
    }
    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return type;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_GET_TYPE, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("GetFileTypes fail to SendRequest. err: %d", err);
        return type;
    }

    type = reply.ReadString();
    if (type.empty()) {
        APP_LOGE("fail to ReadString type");
        return type;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return type;
}

int DataShareProxy::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("%{public}s begin.", __func__);
    int ret = -1;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return ret;
    }

    int count = values.size();
    if (!data.WriteInt32(count)) {
        APP_LOGE("fail to WriteInt32 ret");
        return ret;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(&values[i])) {
            APP_LOGE("fail to WriteParcelable ret, index = %{public}d", i);
            return ret;
        }
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_BATCH_INSERT, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("GetFileTypes fail to SendRequest. err: %d", err);
        return ret;
    }

    if (!reply.ReadInt32(ret)) {
        APP_LOGE("fail to ReadInt32 index");
        return ret;
    }

    APP_LOGI("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareProxy::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("%{public}s begin.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        APP_LOGE("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = Remote()->SendRequest(CMD_REGISTER_OBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        APP_LOGI("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
    } else {
        APP_LOGE("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
    APP_LOGI("%{public}s end.", __func__);
    return true;
}

bool DataShareProxy::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("%{public}s begin.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteParcelable(dataObserver->AsObject())) {
        APP_LOGE("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = Remote()->SendRequest(CMD_UNREGISTER_OBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        APP_LOGI("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
    } else {
        APP_LOGE("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return true;
}

bool DataShareProxy::NotifyChange(const Uri &uri)
{
    APP_LOGI("%{public}s begin.", __func__);
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = Remote()->SendRequest(CMD_NOTIFY_CHANGE, data, reply, option);
    if (result == ERR_NONE) {
        APP_LOGI("%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
    } else {
        APP_LOGE("%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return true;
}

Uri DataShareProxy::NormalizeUri(const Uri &uri)
{
    APP_LOGI("%{public}s begin.", __func__);
    Uri urivalue("");
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return urivalue;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_NORMALIZE_URI, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("NormalizeUri fail to SendRequest. err: %d", err);
        return urivalue;
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        APP_LOGE("ReadParcelable value is nullptr.");
        return urivalue;
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return *info;
}

Uri DataShareProxy::DenormalizeUri(const Uri &uri)
{
    APP_LOGI("%{public}s begin.", __func__);
    Uri urivalue("");
    MessageParcel data;
    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        APP_LOGE("fail to WriteParcelable uri");
        return urivalue;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_DENORMALIZE_URI, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("DenormalizeUri fail to SendRequest. err: %d", err);
        return urivalue;
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        APP_LOGE("ReadParcelable value is nullptr.");
        return urivalue;
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return *info;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> DataShareProxy::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    APP_LOGI("%{public}s begin.", __func__);
    MessageParcel data;
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results;
    results.clear();

    if (!data.WriteInterfaceToken(DataShareProxy::GetDescriptor())) {
        APP_LOGE("%{public}s WriteInterfaceToken failed", __func__);
        return results;
    }

    int count = operations.size();
    if (!data.WriteInt32(count)) {
        APP_LOGE("fail to WriteInt32 ret");
        return results;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(operations[i].get())) {
            APP_LOGE("fail to WriteParcelable ret, index = %{public}d", i);
            return results;
        }
    }

    MessageParcel reply;
    MessageOption option;
    int32_t err = Remote()->SendRequest(CMD_EXECUTE_BATCH, data, reply, option);
    if (err != NO_ERROR) {
        APP_LOGE("fail to SendRequest. err: %{public}d", err);
        return results;
    }

    int total = 0;
    if (!reply.ReadInt32(total)) {
        APP_LOGE("fail to ReadInt32 count %{public}d", total);
        return results;
    }

    for (int i = 0; i < total; i++) {
        AppExecFwk::DataAbilityResult *result = reply.ReadParcelable<AppExecFwk::DataAbilityResult>();
        if (result == nullptr) {
            APP_LOGE("result is nullptr, index = %{public}d", i);
            return results;
        }
        std::shared_ptr<AppExecFwk::DataAbilityResult> dataAbilityResult(result);
        results.push_back(dataAbilityResult);
    }
    APP_LOGI("%{public}s end successfully.", __func__);
    return results;
}
} // namespace AAFwk
} // namespace OHOS
