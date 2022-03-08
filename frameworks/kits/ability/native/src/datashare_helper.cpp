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

#include "datashare_helper.h"

#include "ability_scheduler_interface.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "app_log_wrapper.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "idatashare.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string SCHEME_DATASHARE = "datashare";
constexpr int INVALID_VALUE = -1;
}  // namespace

std::mutex DataShareHelper::oplock_;
DataShareHelper::DataShareHelper(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const AAFwk::Want &want)
{
    APP_LOGI("DataShareHelper::DataShareHelper with context and want start");
    token_ = context->GetToken();
    want_ = want;
    uri_ = nullptr;
    dataShareProxy_ = nullptr;
    dataShareConnection_ = DataShareConnection::GetInstance();
    APP_LOGI("DataShareHelper::DataShareHelper with context and want end");
}

DataShareHelper::DataShareHelper(const std::shared_ptr<Context> &context, const AAFwk::Want &want,
    const std::shared_ptr<Uri> &uri, const sptr<IDataShare> &dataShareProxy)
{
    APP_LOGI("DataShareHelper::DataShareHelper start");
    token_ = context->GetToken();
    context_ = std::shared_ptr<Context>(context);
    want_ = want;
    uri_ = uri;
    dataShareProxy_ = dataShareProxy;
    dataShareConnection_ = DataShareConnection::GetInstance();
    APP_LOGI("DataShareHelper::DataShareHelper end");
}

DataShareHelper::DataShareHelper(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const AAFwk::Want &want, const std::shared_ptr<Uri> &uri, const sptr<IDataShare> &dataShareProxy)
{
    APP_LOGI("DataShareHelper::DataShareHelper start");
    token_ = context->GetToken();
    want_ = want;
    uri_ = uri;
    dataShareProxy_ = dataShareProxy;
    dataShareConnection_ = DataShareConnection::GetInstance();
    APP_LOGI("DataShareHelper::DataShareHelper end");
}

void DataShareHelper::AddDataShareDeathRecipient(const sptr<IRemoteObject> &token)
{
    APP_LOGI("DataShareHelper::AddDataShareDeathRecipient start.");
    if (token != nullptr && callerDeathRecipient_ != nullptr) {
        APP_LOGI("token RemoveDeathRecipient.");
        token->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        callerDeathRecipient_ =
            new DataShareDeathRecipient(std::bind(&DataShareHelper::OnSchedulerDied, this, std::placeholders::_1));
    }
    if (token != nullptr) {
        APP_LOGI("token AddDeathRecipient.");
        token->AddDeathRecipient(callerDeathRecipient_);
    }
    APP_LOGI("DataShareHelper::AddDataShareDeathRecipient end.");
}

void DataShareHelper::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("'%{public}s start':", __func__);
    auto object = remote.promote();
    object = nullptr;
    dataShareProxy_ = nullptr;
    uri_ = nullptr;
    APP_LOGI("DataShareHelper::OnSchedulerDied end.");
}

/**
 * @brief Creates a DataShareHelper instance without specifying the Uri based on the given Context.
 *
 * @param context Indicates the Context object on OHOS.
 * @param want Indicates the Want containing information about the target extension ability to connect.
 *
 * @return Returns the created DataShareHelper instance where Uri is not specified.
 */
std::shared_ptr<DataShareHelper> DataShareHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const AAFwk::Want &want)
{
    APP_LOGI("DataShareHelper::Creator with context start.");
    if (context == nullptr) {
        APP_LOGE("DataShareHelper::Creator (context) failed, context == nullptr");
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator before ConnectDataShareExtAbility.");
    sptr<DataShareConnection> dataShareConnection = DataShareConnection::GetInstance();
    if (!dataShareConnection->IsExtAbilityConnected()) {
        dataShareConnection->ConnectDataShareExtAbility(want, context->GetToken());
    }
    APP_LOGI("DataShareHelper::Creator after ConnectDataShareExtAbility.");

    DataShareHelper *ptrDataShareHelper = new (std::nothrow) DataShareHelper(context, want);
    if (ptrDataShareHelper == nullptr) {
        APP_LOGE("DataShareHelper::Creator (context) failed, create DataShareHelper failed");
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator with context end.");
    return std::shared_ptr<DataShareHelper>(ptrDataShareHelper);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (data share for short) and the associated client process in
 * a DataShareHelper instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param want Indicates the Want containing information about the target extension ability to connect.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataShareHelper instance.
 */
std::shared_ptr<DataShareHelper> DataShareHelper::Creator(
    const std::shared_ptr<Context> &context, const AAFwk::Want &want, const std::shared_ptr<Uri> &uri)
{
    APP_LOGI("DataShareHelper::Creator with context, want and uri called start.");
    if (context == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SCHEME_DATASHARE) {
        APP_LOGE("DataShareHelper::Creator failed, the Scheme is not datashare, Scheme: %{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator before ConnectDataShareExtAbility.");
    sptr<IDataShare> dataShareProxy = nullptr;

    sptr<DataShareConnection> dataShareConnection = DataShareConnection::GetInstance();
    if (!dataShareConnection->IsExtAbilityConnected()) {
        dataShareConnection->ConnectDataShareExtAbility(want, context->GetToken());
    }
    dataShareProxy = dataShareConnection->GetDataShareProxy();
    if (dataShareProxy == nullptr) {
        APP_LOGW("DataShareHelper::Creator get invalid dataShareProxy");
    }
    APP_LOGI("DataShareHelper::Creator after ConnectDataShareExtAbility.");

    DataShareHelper *ptrDataShareHelper = new (std::nothrow) DataShareHelper(context, want, uri, dataShareProxy);
    if (ptrDataShareHelper == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, create DataShareHelper failed");
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator with context, want and uri called end.");
    return std::shared_ptr<DataShareHelper>(ptrDataShareHelper);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (data share for short) and the associated client process in
 * a DataShareHelper instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param want Indicates the Want containing information about the target extension ability to connect.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataShareHelper instance.
 */
std::shared_ptr<DataShareHelper> DataShareHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const AAFwk::Want &want,
    const std::shared_ptr<Uri> &uri)
{
    APP_LOGI("DataShareHelper::Creator with runtime context, want and uri called start.");
    if (context == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SCHEME_DATASHARE) {
        APP_LOGE("DataShareHelper::Creator failed, the Scheme is not datashare, Scheme: %{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator before ConnectDataShareExtAbility.");
    sptr<IDataShare> dataShareProxy = nullptr;

    sptr<DataShareConnection> dataShareConnection = DataShareConnection::GetInstance();
    if (!dataShareConnection->IsExtAbilityConnected()) {
        dataShareConnection->ConnectDataShareExtAbility(want, context->GetToken());
    }
    dataShareProxy = dataShareConnection->GetDataShareProxy();
    if (dataShareProxy == nullptr) {
        APP_LOGW("DataShareHelper::Creator get invalid dataShareProxy");
    }
    APP_LOGI("DataShareHelper::Creator after ConnectDataShareExtAbility.");

    DataShareHelper *ptrDataShareHelper = new (std::nothrow) DataShareHelper(context, want, uri, dataShareProxy);
    if (ptrDataShareHelper == nullptr) {
        APP_LOGE("DataShareHelper::Creator failed, create DataShareHelper failed");
        return nullptr;
    }

    APP_LOGI("DataShareHelper::Creator with runtime context, want and uri called end.");
    return std::shared_ptr<DataShareHelper>(ptrDataShareHelper);
}

/**
 * @brief Releases the client resource of the data share.
 * You should call this method to releases client resource after the data operations are complete.
 *
 * @return Returns true if the resource is successfully released; returns false otherwise.
 */
bool DataShareHelper::Release()
{
    APP_LOGI("DataShareHelper::Release start.");
    if (uri_ == nullptr) {
        APP_LOGE("DataShareHelper::Release failed, uri_ is nullptr");
        return false;
    }

    APP_LOGI("DataShareHelper::Release before DisconnectDataShareExtAbility.");
    if (dataShareConnection_->IsExtAbilityConnected()) {
        dataShareConnection_->DisconnectDataShareExtAbility();
    }
    APP_LOGI("DataShareHelper::Release after DisconnectDataShareExtAbility.");
    dataShareProxy_ = nullptr;
    uri_.reset();
    APP_LOGI("DataShareHelper::Release end.");
    return true;
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> DataShareHelper::GetFileTypes(Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("DataShareHelper::GetFileTypes start.");
    std::vector<std::string> matchedMIMEs;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return matchedMIMEs;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::GetFileTypes before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::GetFileTypes after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return matchedMIMEs;
    }

    APP_LOGI("DataShareHelper::GetFileTypes before dataShareProxy_->GetFileTypes.");
    matchedMIMEs = dataShareProxy_->GetFileTypes(uri, mimeTypeFilter);
    APP_LOGI("DataShareHelper::GetFileTypes after dataShareProxy_->GetFileTypes.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::GetFileTypes before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::GetFileTypes after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }

    APP_LOGI("DataShareHelper::GetFileTypes end.");
    return matchedMIMEs;
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
int DataShareHelper::OpenFile(Uri &uri, const std::string &mode)
{
    APP_LOGI("DataShareHelper::OpenFile start.");
    int fd = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::OpenFile before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::OpenFile after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return fd;
    }

    APP_LOGI("DataShareHelper::OpenFile before dataShareProxy_->OpenFile.");
    fd = dataShareProxy_->OpenFile(uri, mode);
    APP_LOGI("DataShareHelper::OpenFile after dataShareProxy_->OpenFile.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::OpenFile before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::OpenFile after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::OpenFile end.");
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
int DataShareHelper::OpenRawFile(Uri &uri, const std::string &mode)
{
    APP_LOGI("DataShareHelper::OpenRawFile start.");
    int fd = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::OpenRawFile before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::OpenRawFile after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return fd;
    }

    APP_LOGI("DataShareHelper::OpenRawFile before dataShareProxy_->OpenRawFile.");
    fd = dataShareProxy_->OpenRawFile(uri, mode);
    APP_LOGI("DataShareHelper::OpenRawFile after dataShareProxy_->OpenRawFile.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::OpenRawFile before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::OpenRawFile after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::OpenRawFile end.");
    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int DataShareHelper::Insert(Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("DataShareHelper::Insert start.");
    int index = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Insert before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::Insert after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return index;
    }

    APP_LOGI("DataShareHelper::Insert before dataShareProxy_->Insert.");
    index = dataShareProxy_->Insert(uri, value);
    APP_LOGI("DataShareHelper::Insert after dataShareProxy_->Insert.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Insert before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::Insert after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::Insert end.");
    return index;
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
int DataShareHelper::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataShareHelper::Update start.");
    int index = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Update before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::Update after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return index;
    }

    APP_LOGI("DataShareHelper::Update before dataShareProxy_->Update.");
    index = dataShareProxy_->Update(uri, value, predicates);
    APP_LOGI("DataShareHelper::Update after dataShareProxy_->Update.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Update before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::Update after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::Update end.");
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
int DataShareHelper::Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataShareHelper::Delete start.");
    int index = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Delete before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::Delete after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return index;
    }

    APP_LOGI("DataShareHelper::Delete before dataShareProxy_->Delete.");
    index = dataShareProxy_->Delete(uri, predicates);
    APP_LOGI("DataShareHelper::Delete after dataShareProxy_->Delete.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Delete before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::Delete after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::Delete end.");
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
std::shared_ptr<NativeRdb::AbsSharedResultSet> DataShareHelper::Query(
    Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataShareHelper::Query start.");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;

    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return resultset;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Query before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::Query after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return resultset;
    }

    APP_LOGI("DataShareHelper::Query before dataShareProxy_->Query.");
    resultset = dataShareProxy_->Query(uri, columns, predicates);
    APP_LOGI("DataShareHelper::Query after dataShareProxy_->Query.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::Query before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::Query after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::Query end.");
    return resultset;
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the data share. This method should be
 * implemented by a data share. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string DataShareHelper::GetType(Uri &uri)
{
    APP_LOGI("DataShareHelper::GetType start.");
    std::string type;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return type;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::GetType before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::GetType after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return type;
    }

    APP_LOGI("DataShareHelper::GetType before dataShareProxy_->GetType.");
    type = dataShareProxy_->GetType(uri);
    APP_LOGI("DataShareHelper::GetType after dataShareProxy_->GetType.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::GetType before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::GetType after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::GetType end.");
    return type;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int DataShareHelper::BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("DataShareHelper::BatchInsert start.");
    int ret = INVALID_VALUE;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return ret;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::BatchInsert before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::BatchInsert after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return ret;
    }

    APP_LOGI("DataShareHelper::BatchInsert before dataShareProxy_->BatchInsert.");
    ret = dataShareProxy_->BatchInsert(uri, values);
    APP_LOGI("DataShareHelper::BatchInsert after dataShareProxy_->BatchInsert.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::BatchInsert before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::BatchInsert after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::BatchInsert end.");
    return ret;
}

bool DataShareHelper::CheckUriParam(const Uri &uri)
{
    APP_LOGI("DataShareHelper::CheckUriParam start.");
    Uri checkUri(uri.ToString());
    if (!CheckOhosUri(checkUri)) {
        APP_LOGE("DataShareHelper::CheckUriParam failed. CheckOhosUri uri failed");
        return false;
    }

    if (uri_ != nullptr) {
        if (!CheckOhosUri(*uri_)) {
            APP_LOGE("DataShareHelper::CheckUriParam failed. CheckOhosUri uri_ failed");
            return false;
        }

        std::vector<std::string> checkSegments;
        checkUri.GetPathSegments(checkSegments);

        std::vector<std::string> segments;
        uri_->GetPathSegments(segments);

        if (checkSegments[0] != segments[0]) {
            APP_LOGE("DataShareHelper::CheckUriParam failed. the datashare in uri doesn't equal the one in uri_.");
            return false;
        }
    }
    APP_LOGI("DataShareHelper::CheckUriParam end.");
    return true;
}

bool DataShareHelper::CheckOhosUri(const Uri &uri)
{
    APP_LOGI("DataShareHelper::CheckOhosUri start.");
    Uri checkUri(uri.ToString());
    if (checkUri.GetScheme() != SCHEME_DATASHARE) {
        APP_LOGE("DataShareHelper::CheckOhosUri failed. uri is not a datashare one.");
        return false;
    }

    std::vector<std::string> segments;
    checkUri.GetPathSegments(segments);
    if (segments.empty()) {
        APP_LOGE("DataShareHelper::CheckOhosUri failed. There is no segments in the uri.");
        return false;
    }

    if (checkUri.GetPath() == "") {
        APP_LOGE("DataShareHelper::CheckOhosUri failed. The path in the uri is empty.");
        return false;
    }
    APP_LOGI("DataShareHelper::CheckOhosUri end.");
    return true;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataShareHelper::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("DataShareHelper::RegisterObserver start.");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    if (dataObserver == nullptr) {
        APP_LOGE("%{public}s called. dataObserver is nullptr", __func__);
        return;
    }

    Uri tmpUri(uri.ToString());
    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<IDataShare> dataShareProxy = nullptr;
    if (uri_ == nullptr) {
        auto datashare = registerMap_.find(dataObserver);
        if (datashare == registerMap_.end()) {
            if (!dataShareConnection_->IsExtAbilityConnected()) {
                dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
            }
            dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
            registerMap_.emplace(dataObserver, dataShareProxy);
            uriMap_.emplace(dataObserver, tmpUri.GetPath());
        } else {
            auto path = uriMap_.find(dataObserver);
            if (path->second != tmpUri.GetPath()) {
                APP_LOGE("DataShareHelper::RegisterObserver failed input uri's path is not equal the one the "
                         "observer used");
                return;
            }
            dataShareProxy = datashare->second;
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy == nullptr) {
        APP_LOGE("DataShareHelper::RegisterObserver failed dataShareProxy == nullptr");
        registerMap_.erase(dataObserver);
        uriMap_.erase(dataObserver);
        return;
    }
    dataShareProxy->RegisterObserver(uri, dataObserver);
    APP_LOGI("DataShareHelper::RegisterObserver end.");
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataShareHelper::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("DataShareHelper::UnregisterObserver start.");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    if (dataObserver == nullptr) {
        APP_LOGE("%{public}s called. dataObserver is nullptr", __func__);
        return;
    }

    Uri tmpUri(uri.ToString());
    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<IDataShare> dataShareProxy = nullptr;
    if (uri_ == nullptr) {
        auto datashare = registerMap_.find(dataObserver);
        if (datashare == registerMap_.end()) {
            return;
        }
        auto path = uriMap_.find(dataObserver);
        if (path->second != tmpUri.GetPath()) {
            APP_LOGE("DataShareHelper::UnregisterObserver failed input uri's path is not equal the one the "
                     "observer used");
            return;
        }
        dataShareProxy = datashare->second;
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy == nullptr) {
        APP_LOGE("DataShareHelper::UnregisterObserver failed dataShareProxy == nullptr");
        return;
    }

    dataShareProxy->UnregisterObserver(uri, dataObserver);
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::UnregisterObserver before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::UnregisterObserver after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    registerMap_.erase(dataObserver);
    uriMap_.erase(dataObserver);
    APP_LOGI("DataShareHelper::UnregisterObserver end.");
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
void DataShareHelper::NotifyChange(const Uri &uri)
{
    APP_LOGI("DataShareHelper::NotifyChange start.");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    if (dataShareProxy_ == nullptr) {
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return;
    }

    dataShareProxy_->NotifyChange(uri);

    if (uri_ == nullptr) {
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::NotifyChange end.");
}

/**
 * @brief Converts the given uri that refer to the data share into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the data share even if the
 * context has changed. If you implement URI normalization for a data share, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the data share must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this data share does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the data share supports URI normalization; returns null otherwise.
 */
Uri DataShareHelper::NormalizeUri(Uri &uri)
{
    APP_LOGI("DataShareHelper::NormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::NormalizeUri before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::NormalizeUri after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return urivalue;
    }

    APP_LOGI("DataShareHelper::NormalizeUri before dataShareProxy_->NormalizeUri.");
    urivalue = dataShareProxy_->NormalizeUri(uri);
    APP_LOGI("DataShareHelper::NormalizeUri after dataShareProxy_->NormalizeUri.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::NormalizeUri before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::NormalizeUri after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::NormalizeUri end.");
    return urivalue;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed to
 * this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found in
 * the current environment.
 */
Uri DataShareHelper::DenormalizeUri(Uri &uri)
{
    APP_LOGI("DataShareHelper::DenormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::DenormalizeUri before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::DenormalizeUri after ConnectDataShareExtAbility.");
        if (isSystemCaller_ && dataShareProxy_) {
            AddDataShareDeathRecipient(dataShareProxy_->AsObject());
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return urivalue;
    }

    APP_LOGI("DataShareHelper::DenormalizeUri before dataShareProxy_->DenormalizeUri.");
    urivalue = dataShareProxy_->DenormalizeUri(uri);
    APP_LOGI("DataShareHelper::DenormalizeUri after dataShareProxy_->DenormalizeUri.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::DenormalizeUri before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::DenormalizeUri after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::DenormalizeUri end.");
    return urivalue;
}

void DataShareDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("recv DataShareDeathRecipient death notice");
    if (handler_) {
        handler_(remote);
    }
    APP_LOGI("DataShareHelper::OnRemoteDied end.");
}

DataShareDeathRecipient::DataShareDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataShareDeathRecipient::~DataShareDeathRecipient()
{}

std::vector<std::shared_ptr<DataAbilityResult>> DataShareHelper::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    APP_LOGI("DataShareHelper::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (!CheckUriParam(uri)) {
        APP_LOGE("DataShareHelper::ExecuteBatch. CheckUriParam uri failed");
        return results;
    }
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::ExecuteBatch before ConnectDataShareExtAbility.");
        if (!dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->ConnectDataShareExtAbility(want_, token_);
        }
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
        APP_LOGI("DataShareHelper::ExecuteBatch after ConnectDataShareExtAbility.");
        if (dataShareProxy_ == nullptr) {
            APP_LOGE("DataShareHelper::ExecuteBatch failed dataShareProxy_ == nullptr");
            return results;
        }
    } else {
        dataShareProxy_ = dataShareConnection_->GetDataShareProxy();
    }

    if (dataShareProxy_ == nullptr) {
        APP_LOGE("%{public}s failed with invalid dataShareProxy_", __func__);
        return results;
    }

    APP_LOGI("DataShareHelper::ExecuteBatch before dataShareProxy_->ExecuteBatch.");
    results = dataShareProxy_->ExecuteBatch(operations);
    APP_LOGI("DataShareHelper::ExecuteBatch after dataShareProxy_->ExecuteBatch.");
    if (uri_ == nullptr) {
        APP_LOGI("DataShareHelper::ExecuteBatch before DisconnectDataShareExtAbility.");
        if (dataShareConnection_->IsExtAbilityConnected()) {
            dataShareConnection_->DisconnectDataShareExtAbility();
        }
        APP_LOGI("DataShareHelper::ExecuteBatch after DisconnectDataShareExtAbility.");
        dataShareProxy_ = nullptr;
    }
    APP_LOGI("DataShareHelper::ExecuteBatch end");
    return results;
}
}  // namespace AppExecFwk
}  // namespace OHOS