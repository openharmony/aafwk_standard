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

#include "datashare_stub_impl.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<JsDataShareExtAbility> DataShareStubImpl::GetOwner()
{
    std::shared_ptr<JsDataShareExtAbility> owner = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        owner = extension_.lock();
    }
    return owner;
}

std::vector<std::string> DataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::string> ret;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->GetFileTypes(uri, mimeTypeFilter);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = -1;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->OpenFile(uri, mode);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = -1;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->OpenRawFile(uri, mode);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = 0;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->Insert(uri, value);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = 0;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->Update(uri, value, predicates);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = 0;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->Delete(uri, predicates);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> DataShareStubImpl::Query(const Uri &uri,
    std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret = nullptr;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->Query(uri, columns, predicates);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

std::string DataShareStubImpl::GetType(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::string ret = "";
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->GetType(uri);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = 0;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->BatchInsert(uri, values);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    bool ret = false;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->RegisterObserver(uri, dataObserver);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    bool ret = false;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->UnregisterObserver(uri, dataObserver);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::NotifyChange(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    bool ret = false;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        ret = extension->NotifyChange(uri);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

Uri DataShareStubImpl::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        urivalue = extension->NormalizeUri(uri);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return urivalue;
}

Uri DataShareStubImpl::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        urivalue = extension->DenormalizeUri(uri);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return urivalue;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> DataShareStubImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    std::function<void()> syncTaskFunc = [&, client = sptr<DataShareStubImpl>(this)]() {
        auto extension = client->GetOwner();
        if (extension == nullptr) {
            HILOG_ERROR("%{public}s end failed.", __func__);
            return;
        }
        results = extension->ExecuteBatch(operations);
    };
    uvQueue_->SyncCall(syncTaskFunc);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return results;
}
} // namespace AppExecFwk
} // namespace OHOS
