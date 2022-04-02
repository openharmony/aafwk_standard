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

#include "data_ability_helper.h"

#include "ability_scheduler_interface.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "bytrace.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
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
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper start");
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const std::shared_ptr<Uri> &uri, const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper start");
    token_ = context->GetToken();
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<Context> &context)
{
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper only with context start");
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper only with context end");
}

DataAbilityHelper::DataAbilityHelper(const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri,
    const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy)
{
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper start");
    token_ = token;
    uri_ = uri;
    tryBind_ = false;
    dataAbilityProxy_ = dataAbilityProxy;
    isSystemCaller_ = true;
    if (isSystemCaller_ && dataAbilityProxy_) {
        AddDataAbilityDeathRecipient(dataAbilityProxy_->AsObject());
    }
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper end");
}

DataAbilityHelper::DataAbilityHelper(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper only with token_start");
    token_ = token;
    isSystemCaller_ = true;
    HILOG_INFO("DataAbilityHelper::DataAbilityHelper only with token end");
}

void DataAbilityHelper::AddDataAbilityDeathRecipient(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("DataAbilityHelper::AddDataAbilityDeathRecipient start.");
    if (token != nullptr && callerDeathRecipient_ != nullptr) {
        HILOG_INFO("token RemoveDeathRecipient.");
        token->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        callerDeathRecipient_ =
            new DataAbilityDeathRecipient(std::bind(&DataAbilityHelper::OnSchedulerDied, this, std::placeholders::_1));
    }
    if (token != nullptr) {
        HILOG_INFO("token AddDeathRecipient.");
        token->AddDeathRecipient(callerDeathRecipient_);
    }
    HILOG_INFO("DataAbilityHelper::AddDataAbilityDeathRecipient end.");
}

void DataAbilityHelper::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("'%{public}s start':", __func__);
    std::lock_guard<std::mutex> guard(lock_);
    auto object = remote.promote();
    object = nullptr;
    dataAbilityProxy_ = nullptr;
    uri_ = nullptr;
    HILOG_INFO("DataAbilityHelper::OnSchedulerDied end.");
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
    HILOG_INFO("DataAbilityHelper::Creator with context start.");
    if (context == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(context);
    if (ptrDataAbilityHelper == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator with context end.");
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
    HILOG_INFO("DataAbilityHelper::Creator with context uri called.");
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
    HILOG_INFO("DataAbilityHelper::Creator with context uri called.");
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
    HILOG_INFO("DataAbilityHelper::Creator with context uri tryBind called start.");
    if (context == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, the Scheme is not dataability, Scheme: "
                 "%{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    HILOG_INFO("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper =
        new (std::nothrow) DataAbilityHelper(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelper == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator with context uri tryBind called end.");
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
    HILOG_INFO("DataAbilityHelper::Creator with context uri tryBind called start.");
    if (context == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, context == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, the Scheme is not dataability, Scheme: "
            "%{public}s", uri->GetScheme().c_str());
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    HILOG_INFO("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper =
        new (std::nothrow) DataAbilityHelper(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelper == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (context, uri, tryBind) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator with context uri tryBind called end.");
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
    HILOG_INFO("DataAbilityHelper::Creator with token start.");
    if (token == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (token) failed, token == nullptr");
        return nullptr;
    }

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(token);
    if (ptrDataAbilityHelper == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (token) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator with token end.");
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
    HILOG_INFO("DataAbilityHelper::Creator with token uri called start.");
    if (token == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (token, uri) failed, token == nullptr");
        return nullptr;
    }

    if (uri == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (token, uri) failed, uri == nullptr");
        return nullptr;
    }

    if (uri->GetScheme() != SchemeOhos) {
        HILOG_ERROR("DataAbilityHelper::Creator (token, uri) failed, the Scheme is not dataability, Scheme: "
                 "%{public}s",
            uri->GetScheme().c_str());
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator before AcquireDataAbility.");
    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), false, token);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator failed get dataAbilityProxy");
        return nullptr;
    }
    HILOG_INFO("DataAbilityHelper::Creator after AcquireDataAbility.");

    DataAbilityHelper *ptrDataAbilityHelper = new (std::nothrow) DataAbilityHelper(token, uri, dataAbilityProxy);
    if (ptrDataAbilityHelper == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Creator (token, uri) failed, create DataAbilityHelper failed");
        return nullptr;
    }

    HILOG_INFO("DataAbilityHelper::Creator with token uri called end.");
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
    HILOG_INFO("DataAbilityHelper::Release start.");
    if (uri_ == nullptr) {
        HILOG_ERROR("DataAbilityHelper::Release failed, uri_ is nullptr");
        return false;
    }

    HILOG_INFO("DataAbilityHelper::Release before ReleaseDataAbility.");
    int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("DataAbilityHelper::Release failed to ReleaseDataAbility err = %{public}d", err);
        return false;
    }

    HILOG_INFO("DataAbilityHelper::Release after ReleaseDataAbility.");
    std::lock_guard<std::mutex> guard(lock_);
    dataAbilityProxy_ = nullptr;
    uri_.reset();
    HILOG_INFO("DataAbilityHelper::Release end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::GetFileTypes start.");
    std::vector<std::string> matchedMIMEs;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return matchedMIMEs;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::GetFileTypes before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::GetFileTypes after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::GetFileTypes failed dataAbility == nullptr");
            return matchedMIMEs;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }

    HILOG_INFO("DataAbilityHelper::GetFileTypes before dataAbilityProxy->GetFileTypes.");
    matchedMIMEs = dataAbilityProxy->GetFileTypes(uri, mimeTypeFilter);
    HILOG_INFO("DataAbilityHelper::GetFileTypes after dataAbilityProxy->GetFileTypes.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::GetFileTypes before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::GetFileTypes after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::GetFileTypes failed to ReleaseDataAbility err = %{public}d", err);
        }
    }

    HILOG_INFO("DataAbilityHelper::GetFileTypes end.");
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
    HILOG_INFO("DataAbilityHelper::OpenFile start.");
    int fd = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::OpenFile before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::OpenFile after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::OpenFile failed dataAbility == nullptr");
            return fd;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }

    HILOG_INFO("DataAbilityHelper::OpenFile before dataAbilityProxy->OpenFile.");
    fd = dataAbilityProxy->OpenFile(uri, mode);
    HILOG_INFO("DataAbilityHelper::OpenFile after dataAbilityProxy->OpenFile.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::OpenFile before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::OpenFile after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::OpenFile failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::OpenFile end.");
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
    HILOG_INFO("DataAbilityHelper::OpenRawFile start.");
    int fd = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return fd;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::OpenRawFile before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::OpenRawFile after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::OpenRawFile failed dataAbility == nullptr");
            return fd;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::OpenRawFile before dataAbilityProxy->OpenRawFile.");
    fd = dataAbilityProxy->OpenRawFile(uri, mode);
    HILOG_INFO("DataAbilityHelper::OpenRawFile after dataAbilityProxy->OpenRawFile.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::OpenRawFile before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::OpenRawFile after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::OpenRawFile failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::OpenRawFile end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::Insert start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Insert before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Insert after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Insert failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Insert before dataAbilityProxy->Insert.");
    index = dataAbilityProxy->Insert(uri, value);
    HILOG_INFO("DataAbilityHelper::Insert after dataAbilityProxy->Insert.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Insert before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Insert after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Insert failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Insert end.");
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityHelper::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    HILOG_INFO("DataAbilityHelper::Call start.");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return result;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Call before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Call after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Call failed dataAbility == nullptr");
            return nullptr;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Call before dataAbilityProxy->Insert.");
    result = dataAbilityProxy->Call(uri, method, arg, pacMap);
    HILOG_INFO("DataAbilityHelper::Call after dataAbilityProxy->Insert.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Call before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Call after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Call failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Call end.");
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
int DataAbilityHelper::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::Update start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Update before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Update after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Update failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Update before dataAbilityProxy->Update.");
    index = dataAbilityProxy->Update(uri, value, predicates);
    HILOG_INFO("DataAbilityHelper::Update after dataAbilityProxy->Update.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Update before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Update after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Update failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Update end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::Delete start.");
    int index = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return index;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Delete before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Delete after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Delete failed dataAbility == nullptr");
            return index;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Delete before dataAbilityProxy->Delete.");
    index = dataAbilityProxy->Delete(uri, predicates);
    HILOG_INFO("DataAbilityHelper::Delete after dataAbilityProxy->Delete.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Delete before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Delete after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Delete failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Delete end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::Query start.");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;

    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return resultset;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Query before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Query after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Query failed dataAbility == nullptr");
            return resultset;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Query before dataAbilityProxy->Query.");
    resultset = dataAbilityProxy->Query(uri, columns, predicates);
    HILOG_INFO("DataAbilityHelper::Query after dataAbilityProxy->Query.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Query before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Query after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Query failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Query end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::GetType start.");
    std::string type;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return type;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::GetType before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::GetType after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::GetType failed dataAbility == nullptr");
            return type;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::GetType before dataAbilityProxy->GetType.");
    type = dataAbilityProxy->GetType(uri);
    HILOG_INFO("DataAbilityHelper::GetType after dataAbilityProxy->GetType.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::GetType before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::GetType after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::GetType failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::GetType end.");
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
    HILOG_INFO("DataAbilityHelper::Reload start.");
    bool ret = false;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return ret;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Reload before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::Reload after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::Reload failed dataAbility == nullptr");
            return ret;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::Reload before dataAbilityProxy->Reload.");
    ret = dataAbilityProxy->Reload(uri, extras);
    HILOG_INFO("DataAbilityHelper::Reload after dataAbilityProxy->Reload.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::Reload before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::Reload after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::Reload failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::Reload end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::BatchInsert start.");
    int ret = -1;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return ret;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::BatchInsert before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::BatchInsert after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::BatchInsert failed dataAbility == nullptr");
            return ret;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::BatchInsert before dataAbilityProxy->BatchInsert.");
    ret = dataAbilityProxy->BatchInsert(uri, values);
    HILOG_INFO("DataAbilityHelper::BatchInsert after dataAbilityProxy->BatchInsert.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::BatchInsert before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::BatchInsert after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::BatchInsert failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::BatchInsert end.");
    return ret;
}

bool DataAbilityHelper::CheckUriParam(const Uri &uri)
{
    HILOG_INFO("DataAbilityHelper::CheckUriParam start.");
    Uri checkUri(uri.ToString());
    if (!CheckOhosUri(checkUri)) {
        HILOG_ERROR("DataAbilityHelper::CheckUriParam failed. CheckOhosUri uri failed");
        return false;
    }

    // do not directly use uri_ here, otherwise, it will probably crash.
    std::vector<std::string> segments;
    {
        std::lock_guard<std::mutex> guard(lock_);
        if (!uri_) {
            HILOG_INFO("DataAbilityHelper::CheckUriParam uri_ is nullptr, no need check");
            return true;
        }

        if (!CheckOhosUri(*uri_)) {
            HILOG_ERROR("DataAbilityHelper::CheckUriParam failed. CheckOhosUri uri_ failed");
            return false;
        }

        uri_->GetPathSegments(segments);
    }

    std::vector<std::string> checkSegments;
    checkUri.GetPathSegments(checkSegments);

    if (checkSegments.empty() || segments.empty() || checkSegments[0] != segments[0]) {
        HILOG_ERROR("DataAbilityHelper::CheckUriParam failed. dataability in uri doesn't equal the one in uri_.");
        return false;
    }

    HILOG_INFO("DataAbilityHelper::CheckUriParam end.");
    return true;
}

bool DataAbilityHelper::CheckOhosUri(const Uri &uri)
{
    HILOG_INFO("DataAbilityHelper::CheckOhosUri start.");
    Uri checkUri(uri.ToString());
    if (checkUri.GetScheme() != SchemeOhos) {
        HILOG_ERROR("DataAbilityHelper::CheckOhosUri failed. uri is not a dataability one.");
        return false;
    }

    std::vector<std::string> segments;
    checkUri.GetPathSegments(segments);
    if (segments.empty()) {
        HILOG_ERROR("DataAbilityHelper::CheckOhosUri failed. There is no segments in the uri.");
        return false;
    }

    if (checkUri.GetPath() == "") {
        HILOG_ERROR("DataAbilityHelper::CheckOhosUri failed. The path in the uri is empty.");
        return false;
    }
    HILOG_INFO("DataAbilityHelper::CheckOhosUri end.");
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
    HILOG_INFO("DataAbilityHelper::RegisterObserver start.");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    if (dataObserver == nullptr) {
        HILOG_ERROR("%{public}s called. dataObserver is nullptr", __func__);
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
                HILOG_ERROR("DataAbilityHelper::RegisterObserver failed input uri's path is not equal the one the "
                         "observer used");
                return;
            }
            dataAbilityProxy = dataability->second;
        }
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("DataAbilityHelper::RegisterObserver failed dataAbility == nullptr");
        registerMap_.erase(dataObserver);
        uriMap_.erase(dataObserver);
        return;
    }
    dataAbilityProxy->ScheduleRegisterObserver(uri, dataObserver);
    HILOG_INFO("DataAbilityHelper::RegisterObserver end.");
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataAbilityHelper::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("DataAbilityHelper::UnregisterObserver start.");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    if (dataObserver == nullptr) {
        HILOG_ERROR("%{public}s called. dataObserver is nullptr", __func__);
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
            HILOG_ERROR("DataAbilityHelper::UnregisterObserver failed input uri's path is not equal the one the "
                     "observer used");
            return;
        }
        dataAbilityProxy = dataability->second;
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("DataAbilityHelper::UnregisterObserver failed dataAbility == nullptr");
        return;
    }

    dataAbilityProxy->ScheduleUnregisterObserver(uri, dataObserver);
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::UnregisterObserver before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
        HILOG_INFO("DataAbilityHelper::UnregisterObserver after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::UnregisterObserver failed to ReleaseDataAbility err = %{public}d", err);
        }
        dataAbilityProxy_ = nullptr;
    }
    registerMap_.erase(dataObserver);
    uriMap_.erase(dataObserver);
    HILOG_INFO("DataAbilityHelper::UnregisterObserver end.");
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
void DataAbilityHelper::NotifyChange(const Uri &uri)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::NotifyChange start.");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (dataAbilityProxy == nullptr) {
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::NotifyChange failed dataAbility == nullptr");
            return;
        }
    }

    dataAbilityProxy->ScheduleNotifyChange(uri);

    if (uri_ == nullptr) {
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::NotifyChange failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::NotifyChange end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::NormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::NormalizeUri before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::NormalizeUri after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::NormalizeUri failed dataAbility == nullptr");
            return urivalue;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::NormalizeUri before dataAbilityProxy->NormalizeUri.");
    urivalue = dataAbilityProxy->NormalizeUri(uri);
    HILOG_INFO("DataAbilityHelper::NormalizeUri after dataAbilityProxy->NormalizeUri.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::NormalizeUri before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::NormalizeUri after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::NormalizeUri failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::NormalizeUri end.");
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
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("DataAbilityHelper::DenormalizeUri start.");
    Uri urivalue("");
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("%{public}s called. CheckUriParam uri failed", __func__);
        return urivalue;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::DenormalizeUri before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::DenormalizeUri after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::DenormalizeUri failed dataAbility == nullptr");
            return urivalue;
        }
        if (isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    HILOG_INFO("DataAbilityHelper::DenormalizeUri before dataAbilityProxy->DenormalizeUri.");
    urivalue = dataAbilityProxy->DenormalizeUri(uri);
    HILOG_INFO("DataAbilityHelper::DenormalizeUri after dataAbilityProxy->DenormalizeUri.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::DenormalizeUri before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::DenormalizeUri after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::DenormalizeUri failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::DenormalizeUri end.");
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityHelper::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_INFO("DataAbilityHelper::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("DataAbilityHelper::ExecuteBatch. CheckUriParam uri failed");
        return results;
    }

    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::ExecuteBatch before AcquireDataAbility.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        HILOG_INFO("DataAbilityHelper::ExecuteBatch after AcquireDataAbility.");
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("DataAbilityHelper::ExecuteBatch failed dataAbility == nullptr");
            return results;
        }
    }

    HILOG_INFO("DataAbilityHelper::ExecuteBatch before dataAbilityProxy->ExecuteBatch.");
    results = dataAbilityProxy->ExecuteBatch(operations);
    HILOG_INFO("DataAbilityHelper::ExecuteBatch after dataAbilityProxy->ExecuteBatch.");
    if (uri_ == nullptr) {
        HILOG_INFO("DataAbilityHelper::ExecuteBatch before ReleaseDataAbility.");
        int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("DataAbilityHelper::ExecuteBatch after ReleaseDataAbility.");
        if (err != ERR_OK) {
            HILOG_ERROR("DataAbilityHelper::ExecuteBatch failed to ReleaseDataAbility err = %{public}d", err);
        }
    }
    HILOG_INFO("DataAbilityHelper::ExecuteBatch end");
    return results;
}

void DataAbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("recv DataAbilityDeathRecipient death notice");
    if (handler_) {
        handler_(remote);
    }
    HILOG_INFO("DataAbilityHelper::OnRemoteDied end.");
}

DataAbilityDeathRecipient::DataAbilityDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataAbilityDeathRecipient::~DataAbilityDeathRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS