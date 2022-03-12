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

#include "remote_client_manager.h"

#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"


namespace OHOS {
namespace AppExecFwk {
RemoteClientManager::RemoteClientManager()
    : appSpawnClient_(std::make_shared<AppSpawnClient>()), nwebSpawnClient_(std::make_shared<AppSpawnClient>(true))
{}

RemoteClientManager::~RemoteClientManager()
{}

std::shared_ptr<AppSpawnClient> RemoteClientManager::GetSpawnClient()
{
    if (appSpawnClient_) {
        return appSpawnClient_;
    }
    return nullptr;
}

void RemoteClientManager::SetSpawnClient(const std::shared_ptr<AppSpawnClient> &appSpawnClient)
{
    appSpawnClient_ = appSpawnClient;
}

sptr<IBundleMgr> RemoteClientManager::GetBundleManager()
{
    if (bundleManager_ == nullptr) {
        sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager != nullptr) {
            bundleManager_ =
                iface_cast<AppExecFwk::IBundleMgr>(systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
        } else {
            HILOG_ERROR("AppMgrServiceInner::GetBundleManager fail to get SAMGR");
        }
    }
    return bundleManager_;
}

void RemoteClientManager::SetBundleManager(sptr<IBundleMgr> bundleManager)
{
    bundleManager_ = bundleManager;
}

std::shared_ptr<AppSpawnClient> RemoteClientManager::GetNWebSpawnClient()
{
    return nwebSpawnClient_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
