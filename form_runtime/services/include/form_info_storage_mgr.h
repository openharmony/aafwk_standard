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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_STORAGE_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_STORAGE_MGR_H

#include <map>
#include <singleton.h>
#include <string>
#include "appexecfwk_errors.h"
#include "distributed_kv_data_manager.h"
#include "kvstore_death_recipient.h"

namespace OHOS {
namespace AppExecFwk {
class KvStoreDeathRecipientCallbackFormInfoStorage : public DistributedKv::KvStoreDeathRecipient {
public:
    KvStoreDeathRecipientCallbackFormInfoStorage();

    virtual ~KvStoreDeathRecipientCallbackFormInfoStorage();

    virtual void OnRemoteDied() override;
};

/**
 * @class FormInfoStorageMgr
 * Form info storage.
 */
class FormInfoStorageMgr final : public DelayedRefSingleton<FormInfoStorageMgr> {
DECLARE_DELAYED_REF_SINGLETON(FormInfoStorageMgr)

public:
    DISALLOW_COPY_AND_MOVE(FormInfoStorageMgr);

    ErrCode LoadFormInfos(std::vector<std::pair<std::string, std::string>> &formInfoStorages);

    ErrCode GetBundleFormInfos(const std::string &bundleName, std::string &formInfoStorages);

    ErrCode SaveBundleFormInfos(const std::string &bundleName, const std::string &formInfoStorages);

    ErrCode RemoveBundleFormInfos(const std::string &bundleName);

    ErrCode UpdateBundleFormInfos(const std::string &bundleName, const std::string &formInfoStorages);

    bool ResetKvStore();

private:
    void RegisterKvStoreDeathListener();

    DistributedKv::Status GetKvStore();

    bool CheckKvStore();

    DistributedKv::Status GetEntries(std::vector<DistributedKv::Entry> &allEntries);

    const DistributedKv::AppId appId_ {"form_storage"};
    const DistributedKv::StoreId storeId_ {"form_infos"};
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    const int32_t MAX_TIMES = 600;              // 1min
    const int32_t SLEEP_INTERVAL = 100 * 1000;  // 100ms
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_INFO_STORAGE_MGR_H
