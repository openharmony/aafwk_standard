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
std::vector<std::string> DataShareStubImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::string> ret;
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return ret;
    }
    ret = extension->GetFileTypes(uri, mimeTypeFilter);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->OpenFile(uri, mode);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->OpenRawFile(uri, mode);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->Insert(uri, value);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->Update(uri, value, predicates);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->Delete(uri, predicates);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> DataShareStubImpl::Query(const Uri &uri,
    std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return nullptr;
    }
    auto ret = extension->Query(uri, columns, predicates);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

std::string DataShareStubImpl::GetType(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return "";
    }
    auto ret = extension->GetType(uri);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

int DataShareStubImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return -1;
    }
    auto ret = extension->BatchInsert(uri, values);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return false;
    }
    bool ret = extension->RegisterObserver(uri, dataObserver);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return false;
    }
    bool ret = extension->UnregisterObserver(uri, dataObserver);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

bool DataShareStubImpl::NotifyChange(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return false;
    }
    bool ret = extension->NotifyChange(uri);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return ret;
}

Uri DataShareStubImpl::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return urivalue;
    }
    urivalue = extension->NormalizeUri(uri);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return urivalue;
}

Uri DataShareStubImpl::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return urivalue;
    }
    urivalue = extension->DenormalizeUri(uri);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return urivalue;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> DataShareStubImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    auto extension = extension_.lock();
    if (extension == nullptr) {
        HILOG_INFO("%{public}s end failed.", __func__);
        return results;
    }
    results = extension->ExecuteBatch(operations);
    HILOG_INFO("%{public}s end successfully.", __func__);
    return results;
}
} // namespace AppExecFwk
} // namespace OHOS
