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

#include "data_ability_helper.h"

#include "ability_scheduler_interface.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "app_log_wrapper.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
std::string SchemeOhos = "dataability";
std::mutex DataAbilityHelper::oplock_;
using IAbilityScheduler = OHOS::AAFwk::IAbilityScheduler;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri,
    const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    APP_LOGI("DataAbilityHelper::DataAbilityHelper start");
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
    APP_LOGI("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const std::shared_ptr<Uri> &uri, const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    APP_LOGI("DataAbilityHelper::DataAbilityHelper start");
    token_ = context->GetToken();
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
    APP_LOGI("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<Context> &context)
{
    APP_LOGI("DataAbilityHelper::DataAbilityHelper only with context start");
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    APP_LOGI("DataAbilityHelper::DataAbilityHelper only with context end");
}

DataAbilityHelper::DataAbilityHelper(const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri,
    const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy)
{
    APP_LOGI("DataAbilityHelper::DataAbilityHelper start");
    token_ = token;
    uri_ = uri;
    tryBind_ = false;
    dataAbilityProxy_ = dataAbilityProxy;
    isSystemCaller_ = true;
    if (isSystemCaller_ && dataAbilityProxy_) {
        AddDataAbilityDeathRecipient(dataAbilityProxy_->AsObject());
    }
    APP_LOGI("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const sptr<IRemoteObject> &token)
{
    APP_LOGI("DataAbilityHelper::DataAbilityHelper only with token_start");
    token_ = token;
    isSystemCaller_ = true;
    APP_LOGI("DataAbilityHelper::DataAbilityHelper only with token end");
}

void DataAbilityHelper::AddDataAbilityDeathRecipient(const sptr<IRemoteObject> &token)
{
    APP_LOGI("DataAbilityHelper::AddDataAbilityDeathRecipient start.");
    if (token != nullptr && callerDeathRecipient_ != nullptr) {
        APP_LOGI("token RemoveDeathRecipient.");
        token->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        callerDeathRecipient_ =
            new DataAbilityDeathRecipient(std::bind(&DataAbilityHelper::OnSchedulerDied, this, std::placeholders::_1));
    }
    if (token != nullptr) {
        APP_LOGI("token AddDeathRecipient.");
        token->AddDeathRecipient(callerDeathRecipient_);
    }
    APP_LOGI("DataAbilityHelper::AddDataAbilityDeathRecipient end.");
}

void DataAbilityHelper::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("'%{public}s start':", __func__);
    std::lock_guard<std::mutex> guard(lock_);
    auto object = remote.promote();
    object = nullptr;
    dataAbilityProxy_ = nullptr;
    uri_ = nullptr;
    APP_LOGI("DataAbilityHelper::OnSchedulerDied end.");
}

/**
 * @brief Creates a DataAbilityHelper instance without specifying the Uri based on the given Context.
 *
 * @param context Indicates the Context object on OHOS.
 *
 * @return Returns the created DataAbilityHelper instance where Uri is not specified.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(const std::shared_ptr<Context> &context)
{
    APP_LOGI("DataAbilityHelper::Creator with context start.");
    if (context == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(context);
    if (ptrDataAbilityHelper == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator with context end.");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}

/**
 * @brief Creates a DataAbilityHelper instance with the Uri specified based on the given Context.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataAbilityHelper instance with a specified Uri.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri)
{
    APP_LOGI("DataAbilityHelper::Creator with context uri called.");
    return DataAbilityHelper::Creator(context, uri, false);
}

/**
 * @brief Creates a DataAbilityHelper instance with the Uri specified based on the given Context.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataAbilityHelper instance with a specified Uri.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const std::shared_ptr<Uri> &uri)
{
    APP_LOGI("DataAbilityHelper::Creator with context uri called.");
    return DataAbilityHelper::Creator(context, uri, false);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelper instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 * @param tryBind Specifies whether the exit of the corresponding Data ability process causes the exit of the
 * client process.
 *
 * @return Returns the created DataAbilityHelper instance.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    APP_LOGI("DataAbilityHelper::Creator with context uri tryBind called start.");
    if (context == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, the Scheme is not dataability, Scheme: "
                 "%{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    APP_LOGI("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper =
        new (std::nothrow) DataAbilityHelper(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelper == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator with context uri tryBind called end.");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelper instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 * @param tryBind Specifies whether the exit of the corresponding Data ability process causes the exit of the
 * client process.
 *
 * @return Returns the created DataAbilityHelper instance.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    APP_LOGI("DataAbilityHelper::Creator with context uri tryBind called start.");
    if (context == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, the Scheme is not dataability, Scheme: "
                 "%{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    APP_LOGI("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper =
        new (std::nothrow) DataAbilityHelper(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelper == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (context, uri, tryBind) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator with context uri tryBind called end.");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}

/**
 * @brief Creates a DataAbilityHelper instance without specifying the Uri based.
 *
 * @param token Indicates the System token.
 *
 * @return Returns the created DataAbilityHelper instance where Uri is not specified.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(const sptr<IRemoteObject> &token)
{
    APP_LOGI("DataAbilityHelper::Creator with token start.");
    if (token == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (token) failed, token == nullptr");
        return nullptr;
    }

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(token);
    if (ptrDataAbilityHelper == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (token) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator with token end.");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelper instance.
 *
 * @param token Indicates the System token.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataAbilityHelper instance.
 */
std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri)
{
    APP_LOGI("DataAbilityHelper::Creator with token uri called start.");
    if (token == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (token, uri) failed, token == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (token, uri) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        APP_LOGE("DataAbilityHelper::Creator (token, uri) failed, the Scheme is not dataability, Scheme: "
                 "%{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), false, token);
    if (dataAbilityProxy == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    APP_LOGI("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(token, uri, dataAbilityProxy);
    if (ptrDataAbilityHelper == nullptr) {
        APP_LOGE("DataAbilityHelper::Creator (token, uri) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    APP_LOGI("DataAbilityHelper::Creator with token uri called end.");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}

/**
 * @brief Releases the client resource of the Data ability.
 * You should call this method to releases client resource after the data operations are complete.
 *
 * @return Returns true if the resource is successfully released; returns false otherwise.
 */
bool DataAbilityHelper::Release()
{
    APP_LOGI("DataAbilityHelper::Release start.");
    if (uri_ == nullptr) {
        APP_LOGE("DataAbilityHelper::Release failed, uri_ is nullptr");
        return false;
    }

    APP_LOGI("DataAbilityHelper::Release before ReleaseDataAbility.");
    int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
    if (err != ERR_OK) {
        APP_LOGE("DataAbilityHelper::Release failed to ReleaseDataAbility err = %{public}d", err);
        return false;
    }

    APP_LOGI("DataAbilityHelper::Release after ReleaseDataAbility.");
    std::lock_guard<std::mutex> guard(lock_);
    dataAbilityProxy_ = nullptr;
    uri_.reset();
    APP_LOGI("DataAbilityHelper::Release end.");
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
std::vector<std::string> DataAbilityHelper::GetFileTypes(Uri &uri, const std::string &mimeTypeFilter)
{
    APP_LOGI("DataAbilityHelper::GetFileTypes start.");
    std::vector<std::string> matchedMIMEs;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return matchedMIMEs;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::GetFileTypes before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::GetFileTypes after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::GetFileTypes failed dataAbility == nullptr");
            return matchedMIMEs;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }

    APP_LOGI("DataAbilityHelper::GetFileTypes before dataAbilityProxy->GetFileTypes.");
    matchedMIMEs = dataAbilityProxy->GetFileTypes(uri, mimeTypeFilter);
    APP_LOGI("DataAbilityHelper::GetFileTypes after dataAbilityProxy->GetFileTypes.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::GetFileTypes before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::GetFileTypes after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::GetFileTypes failed to ReleaseDataAbility err = %{public}d", err);
        }
    }

    APP_LOGI("DataAbilityHelper::GetFileTypes end.");
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
int DataAbilityHelper::OpenFile(Uri &uri, const std::string &mode)
{
    APP_LOGI("DataAbilityHelper::OpenFile start.");
    int fd = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::OpenFile before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::OpenFile after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::OpenFile failed dataAbility == nullptr");
            return fd;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }

    APP_LOGI("DataAbilityHelper::OpenFile before dataAbilityProxy->OpenFile.");
    fd = dataAbilityProxy->OpenFile(uri, mode);
    APP_LOGI("DataAbilityHelper::OpenFile after dataAbilityProxy->OpenFile.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::OpenFile before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::OpenFile after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::OpenFile failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::OpenFile end.");
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
int DataAbilityHelper::OpenRawFile(Uri &uri, const std::string &mode)
{
    APP_LOGI("DataAbilityHelper::OpenRawFile start.");
    int fd = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::OpenRawFile before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::OpenRawFile after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::OpenRawFile failed dataAbility == nullptr");
            return fd;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::OpenRawFile before dataAbilityProxy->OpenRawFile.");
    fd = dataAbilityProxy->OpenRawFile(uri, mode);
    APP_LOGI("DataAbilityHelper::OpenRawFile after dataAbilityProxy->OpenRawFile.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::OpenRawFile before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::OpenRawFile after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::OpenRawFile failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::OpenRawFile end.");
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
int DataAbilityHelper::Insert(Uri &uri, const NativeRdb::ValuesBucket &value)
{
    APP_LOGI("DataAbilityHelper::Insert start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Insert before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::Insert after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::Insert failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::Insert before dataAbilityProxy->Insert.");
    index = dataAbilityProxy->Insert(uri, value);
    APP_LOGI("DataAbilityHelper::Insert after dataAbilityProxy->Insert.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Insert before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::Insert after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::Insert failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::Insert end.");
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
int DataAbilityHelper::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataAbilityHelper::Update start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Update before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::Update after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::Update failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::Update before dataAbilityProxy->Update.");
    index = dataAbilityProxy->Update(uri, value, predicates);
    APP_LOGI("DataAbilityHelper::Update after dataAbilityProxy->Update.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Update before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::Update after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::Update failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::Update end.");
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
int DataAbilityHelper::Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataAbilityHelper::Delete start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Delete before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::Delete after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::Delete failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::Delete before dataAbilityProxy->Delete.");
    index = dataAbilityProxy->Delete(uri, predicates);
    APP_LOGI("DataAbilityHelper::Delete after dataAbilityProxy->Delete.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Delete before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::Delete after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::Delete failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::Delete end.");
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
std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityHelper::Query(
    Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    APP_LOGI("DataAbilityHelper::Query start.");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;

    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return resultset;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Query before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::Query after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::Query failed dataAbility == nullptr");
            return resultset;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::Query before dataAbilityProxy->Query.");
    resultset = dataAbilityProxy->Query(uri, columns, predicates);
    APP_LOGI("DataAbilityHelper::Query after dataAbilityProxy->Query.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Query before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::Query after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::Query failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::Query end.");
    return resultset;
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string DataAbilityHelper::GetType(Uri &uri)
{
    APP_LOGI("DataAbilityHelper::GetType start.");
    std::string type;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return type;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::GetType before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::GetType after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::GetType failed dataAbility == nullptr");
            return type;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::GetType before dataAbilityProxy->GetType.");
    type = dataAbilityProxy->GetType(uri);
    APP_LOGI("DataAbilityHelper::GetType after dataAbilityProxy->GetType.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::GetType before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::GetType after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::GetType failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::GetType end.");
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
bool DataAbilityHelper::Reload(Uri &uri, const PacMap &extras)
{
    APP_LOGI("DataAbilityHelper::Reload start.");
    bool ret = false;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return ret;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Reload before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::Reload after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::Reload failed dataAbility == nullptr");
            return ret;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::Reload before dataAbilityProxy->Reload.");
    ret = dataAbilityProxy->Reload(uri, extras);
    APP_LOGI("DataAbilityHelper::Reload after dataAbilityProxy->Reload.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::Reload before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::Reload after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::Reload failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::Reload end.");
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
int DataAbilityHelper::BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    APP_LOGI("DataAbilityHelper::BatchInsert start.");
    int ret = -1;
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return ret;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::BatchInsert before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::BatchInsert after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::BatchInsert failed dataAbility == nullptr");
            return ret;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::BatchInsert before dataAbilityProxy->BatchInsert.");
    ret = dataAbilityProxy->BatchInsert(uri, values);
    APP_LOGI("DataAbilityHelper::BatchInsert after dataAbilityProxy->BatchInsert.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::BatchInsert before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::BatchInsert after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::BatchInsert failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::BatchInsert end.");
    return ret;
}

bool DataAbilityHelper::CheckUriParam(const Uri &uri)
{
    APP_LOGI("DataAbilityHelper::CheckUriParam start.");
    Uri checkUri(uri.ToString());
    if (!CheckOhosUri(checkUri)) {
        APP_LOGE("DataAbilityHelper::CheckUriParam failed. CheckOhosUri uri failed");
        return false;
    }

    auto uriSp = uri_; // do not directly use uri_ here, otherwise, it will crash.
    if (uriSp != nullptr) {
        if (!CheckOhosUri(*uriSp)) {
            APP_LOGE("DataAbilityHelper::CheckUriParam failed. CheckOhosUri uriSp failed");
            return false;
        }

        std::vector<std::string> checkSegments;
        checkUri.GetPathSegments(checkSegments);

        std::vector<std::string> segments;
        uriSp->GetPathSegments(segments);

        if (checkSegments.empty() || segments.empty() || checkSegments[0] != segments[0]) {
            APP_LOGE("DataAbilityHelper::CheckUriParam failed. the dataability in uri doesn't equal the one in uriSp.");
            return false;
        }
    }
    APP_LOGI("DataAbilityHelper::CheckUriParam end.");
    return true;
}

bool DataAbilityHelper::CheckOhosUri(const Uri &uri)
{
    APP_LOGI("DataAbilityHelper::CheckOhosUri start.");
    Uri checkUri(uri.ToString());
    if (checkUri.GetScheme() != SchemeOhos) {
        APP_LOGE("DataAbilityHelper::CheckOhosUri failed. uri is not a dataability one.");
        return false;
    }

    std::vector<std::string> segments;
    checkUri.GetPathSegments(segments);
    if (segments.empty()) {
        APP_LOGE("DataAbilityHelper::CheckOhosUri failed. There is no segments in the uri.");
        return false;
    }

    if (checkUri.GetPath() == "") {
        APP_LOGE("DataAbilityHelper::CheckOhosUri failed. The path in the uri is empty.");
        return false;
    }
    APP_LOGI("DataAbilityHelper::CheckOhosUri end.");
    return true;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataAbilityHelper::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("DataAbilityHelper::RegisterObserver start.");
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
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
            registerMap_.emplace(dataObserver, dataAbilityProxy);
            uriMap_.emplace(dataObserver, tmpUri.GetPath());
        } else {
            auto path = uriMap_.find(dataObserver);
            if (path->second != tmpUri.GetPath()) {
                APP_LOGE("DataAbilityHelper::RegisterObserver failed input uri's path is not equal the one the "
                         "observer used");
                return;
            }
            dataAbilityProxy = dataability->second;
        }
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        APP_LOGE("DataAbilityHelper::RegisterObserver failed dataAbility == nullptr");
        registerMap_.erase(dataObserver);
        uriMap_.erase(dataObserver);
        return;
    }
    dataAbilityProxy->ScheduleRegisterObserver(uri, dataObserver);
    APP_LOGI("DataAbilityHelper::RegisterObserver end.");
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataAbilityHelper::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    APP_LOGI("DataAbilityHelper::UnregisterObserver start.");
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
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            return;
        }
        auto path = uriMap_.find(dataObserver);
        if (path->second != tmpUri.GetPath()) {
            APP_LOGE("DataAbilityHelper::UnregisterObserver failed input uri's path is not equal the one the "
                     "observer used");
            return;
        }
        dataAbilityProxy = dataability->second;
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        APP_LOGE("DataAbilityHelper::UnregisterObserver failed dataAbility == nullptr");
        return;
    }

    dataAbilityProxy->ScheduleUnregisterObserver(uri, dataObserver);
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::UnregisterObserver before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
        APP_LOGI("DataAbilityHelper::UnregisterObserver after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::UnregisterObserver failed to ReleaseDataAbility err = %{public}d", err);
        }
        dataAbilityProxy_ = nullptr;
    }
    registerMap_.erase(dataObserver);
    uriMap_.erase(dataObserver);
    APP_LOGI("DataAbilityHelper::UnregisterObserver end.");
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
void DataAbilityHelper::NotifyChange(const Uri &uri)
{
    APP_LOGI("DataAbilityHelper::NotifyChange start.");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (dataAbilityProxy == nullptr) {
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::NotifyChange failed dataAbility == nullptr");
            return;
        }
    }

    dataAbilityProxy->ScheduleNotifyChange(uri);

    if (uri_ == nullptr) {
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::NotifyChange failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::NotifyChange end.");
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri DataAbilityHelper::NormalizeUri(Uri &uri)
{
    APP_LOGI("DataAbilityHelper::NormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::NormalizeUri before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::NormalizeUri after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::NormalizeUri failed dataAbility == nullptr");
            return urivalue;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::NormalizeUri before dataAbilityProxy->NormalizeUri.");
    urivalue = dataAbilityProxy->NormalizeUri(uri);
    APP_LOGI("DataAbilityHelper::NormalizeUri after dataAbilityProxy->NormalizeUri.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::NormalizeUri before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::NormalizeUri after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::NormalizeUri failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::NormalizeUri end.");
    return urivalue;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed to
 * this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found in the
 * current environment.
 */
Uri DataAbilityHelper::DenormalizeUri(Uri &uri)
{
    APP_LOGI("DataAbilityHelper::DenormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        APP_LOGE("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::DenormalizeUri before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::DenormalizeUri after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::DenormalizeUri failed dataAbility == nullptr");
            return urivalue;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    APP_LOGI("DataAbilityHelper::DenormalizeUri before dataAbilityProxy->DenormalizeUri.");
    urivalue = dataAbilityProxy->DenormalizeUri(uri);
    APP_LOGI("DataAbilityHelper::DenormalizeUri after dataAbilityProxy->DenormalizeUri.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::DenormalizeUri before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::DenormalizeUri after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::DenormalizeUri failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::DenormalizeUri end.");
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityHelper::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    APP_LOGI("DataAbilityHelper::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (!CheckUriParam(uri)) {
        APP_LOGE("DataAbilityHelper::ExecuteBatch. CheckUriParam uri failed");
        return results;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::ExecuteBatch before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        APP_LOGI("DataAbilityHelper::ExecuteBatch after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            APP_LOGE("DataAbilityHelper::ExecuteBatch failed dataAbility == nullptr");
            return results;
        }
    }

    APP_LOGI("DataAbilityHelper::ExecuteBatch before dataAbilityProxy->ExecuteBatch.");
    results = dataAbilityProxy->ExecuteBatch(operations);
    APP_LOGI("DataAbilityHelper::ExecuteBatch after dataAbilityProxy->ExecuteBatch.");
    if (uri_ == nullptr) {
        APP_LOGI("DataAbilityHelper::ExecuteBatch before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        APP_LOGI("DataAbilityHelper::ExecuteBatch after ReleaseDataAbility.");
        if (err != ERR_OK) {
            APP_LOGE("DataAbilityHelper::ExecuteBatch failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    APP_LOGI("DataAbilityHelper::ExecuteBatch end");
    return results;
}

void DataAbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("recv DataAbilityDeathRecipient death notice");
    if (handler_) {
        handler_(remote);
    }
    APP_LOGI("DataAbilityHelper::OnRemoteDied end.");
}

DataAbilityDeathRecipient::DataAbilityDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataAbilityDeathRecipient::~DataAbilityDeathRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS