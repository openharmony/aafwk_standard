/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include "form_info_storage_mgr.h"
#include <thread>
#include <unistd.h>
#include "form_mgr_errors.h"
#include "hilog_wrapper.h"
#include "kvstore_death_recipient_callback.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t CHECK_TIMES = 300;
const int32_t CHECK_INTERVAL = 100000;  // 100ms
}  // namespace

KvStoreDeathRecipientCallbackFormInfoStorage::KvStoreDeathRecipientCallbackFormInfoStorage()
{
    HILOG_INFO("create kvstore death recipient callback instance");
}

KvStoreDeathRecipientCallbackFormInfoStorage::~KvStoreDeathRecipientCallbackFormInfoStorage()
{
    HILOG_INFO("destroy kvstore death recipient callback instance");
}

void KvStoreDeathRecipientCallbackFormInfoStorage::OnRemoteDied()
{
    HILOG_INFO("OnRemoteDied, register data change listener begin");
    std::thread([] {
        int32_t times = 0;
        FormInfoStorageMgr &formInfoStorageMgr = FormInfoStorageMgr::GetInstance();
        while (times < CHECK_TIMES) {
            times++;
            // init kvStore.
            if (formInfoStorageMgr.ResetKvStore()) {
                // register data change listener again.
                HILOG_INFO("current times is %{public}d", times);
                break;
            }
            usleep(CHECK_INTERVAL);
        }
    }).detach();

    HILOG_INFO("OnRemoteDied, register data change listener end");
}

FormInfoStorageMgr::FormInfoStorageMgr()
{
    DistributedKv::Status status = GetKvStore();
    if (status == DistributedKv::Status::IPC_ERROR) {
        status = GetKvStore();
        HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    RegisterKvStoreDeathListener();
    HILOG_INFO("FormInfoStorageMgr is created");
}

FormInfoStorageMgr::~FormInfoStorageMgr()
{
    dataManager_.CloseKvStore(appId_, kvStorePtr_);
}

ErrCode FormInfoStorageMgr::LoadFormInfos(std::vector<std::pair<std::string, std::string>> &formInfoStorages)
{
    HILOG_INFO("FormInfoStorageMgr load all form infos");
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    DistributedKv::Status status;
    std::vector<DistributedKv::Entry> allEntries;
    status = GetEntries(allEntries);
    if (status == DistributedKv::Status::IPC_ERROR) {
        status = GetEntries(allEntries);
        HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("get entries error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    for (const auto &item: allEntries) {
        formInfoStorages.emplace_back(item.key.ToString(), item.value.ToString());
    }

    return ERR_OK;
}

ErrCode FormInfoStorageMgr::GetBundleFormInfos(const std::string &bundleName, std::string &formInfoStorages)
{
    if (bundleName.empty()) {
        HILOG_ERROR("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_INFO("FormInfoStorageMgr get form info, bundleName=%{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    DistributedKv::Status status = DistributedKv::Status::ERROR;
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Key key(bundleName);
    if (kvStorePtr_) {
        // sync call GetEntries, the callback will be trigger at once
        status = kvStorePtr_->GetEntries(key, allEntries);
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("get entries error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    if (allEntries.empty()) {
        HILOG_ERROR("%{public}s not match any FormInfo", bundleName.c_str());
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    formInfoStorages = allEntries.front().value.ToString();
    return ERR_OK;
}

ErrCode FormInfoStorageMgr::SaveBundleFormInfos(const std::string &bundleName, const std::string &formInfoStorages)
{
    if (bundleName.empty()) {
        HILOG_ERROR("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_INFO("FormInfoStorageMgr save form info, bundleName=%{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Value value(formInfoStorages);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
        if (status == DistributedKv::Status::IPC_ERROR) {
            status = kvStorePtr_->Put(key, value);
            HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("save formInfoStorages to kvStore error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}

ErrCode FormInfoStorageMgr::RemoveBundleFormInfos(const std::string &bundleName)
{
    if (bundleName.empty()) {
        HILOG_ERROR("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_INFO("FormInfoStorageMgr remove form info, bundleName=%{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Delete(key);
        if (status == DistributedKv::Status::IPC_ERROR) {
            status = kvStorePtr_->Delete(key);
            HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
        }
    }

    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("remove formInfoStorages from kvStore error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}

ErrCode FormInfoStorageMgr::UpdateBundleFormInfos(const std::string &bundleName, const std::string &formInfoStorages)
{
    if (bundleName.empty()) {
        HILOG_ERROR("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_INFO("FormInfoStorageMgr update form info, bundleName=%{public}s", bundleName.c_str());
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            HILOG_ERROR("kvStore is nullptr");
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }
    DistributedKv::Key key(bundleName);
    DistributedKv::Status status;
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    status = kvStorePtr_->Delete(key);
    if (status == DistributedKv::Status::IPC_ERROR) {
        status = kvStorePtr_->Delete(key);
        HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    if (status != DistributedKv::Status::SUCCESS && status != DistributedKv::Status::KEY_NOT_FOUND) {
        HILOG_ERROR("update formInfoStorages to kvStore error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    DistributedKv::Value value(formInfoStorages);
    status = kvStorePtr_->Put(key, value);
    if (status == DistributedKv::Status::IPC_ERROR) {
        status = kvStorePtr_->Put(key, value);
        HILOG_WARN("distribute database ipc error and try to call again, result = %{public}d", status);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("update formInfoStorages to kvStore error: %{public}d", status);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}

DistributedKv::Status FormInfoStorageMgr::GetKvStore()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        HILOG_ERROR("return error: %{public}d", status);
    } else {
        HILOG_INFO("get kvStore success");
    }
    return status;
}

bool FormInfoStorageMgr::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }
    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        HILOG_INFO("CheckKvStore, Times: %{public}d", tryTimes);
        usleep(SLEEP_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

void FormInfoStorageMgr::RegisterKvStoreDeathListener()
{
    HILOG_INFO("register kvStore death listener");
    std::shared_ptr<DistributedKv::KvStoreDeathRecipient> callback =
        std::make_shared<KvStoreDeathRecipientCallbackFormInfoStorage>();
    dataManager_.RegisterKvStoreServiceDeathRecipient(callback);
}

bool FormInfoStorageMgr::ResetKvStore()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    kvStorePtr_ = nullptr;
    DistributedKv::Status status = GetKvStore();
    if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
        return true;
    }
    HILOG_WARN("failed");
    return false;
}

DistributedKv::Status FormInfoStorageMgr::GetEntries(std::vector<DistributedKv::Entry> &allEntries)
{
    DistributedKv::Status status = DistributedKv::Status::ERROR;
    // if prefix is empty, get all entries.
    DistributedKv::Key key("");
    if (kvStorePtr_) {
        // sync call GetEntries, the callback will be trigger at once
        status = kvStorePtr_->GetEntries(key, allEntries);
    }
    HILOG_INFO("get all entries status: %{public}d", status);
    return status;
}
}  // namespace AppExecFwk
}  // namespace OHOS
