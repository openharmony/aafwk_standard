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
#include "distributed_client.h"

#include "app_log_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<DistributedClient> DistributedClient::instance_ = nullptr;
std::mutex DistributedClient::mutex_;

std::shared_ptr<DistributedClient> DistributedClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<DistributedClient>();
        }
    }
    return instance_;
}

DistributedClient::DistributedClient()
{}

DistributedClient::~DistributedClient()
{}

ErrCode DistributedClient::Connect()
{
    APP_LOGI("%{public}s called begin", __func__);
    std::lock_guard<std::mutex> lock(mutex_);

    if (remoteObject_ == nullptr) {
        sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            APP_LOGE("%{public}s:fail to get System Ability Manager", __func__);
            return GET_SYSTEM_ABILITY_SERVICE_FAILED;
        }
        remoteObject_ = systemManager->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
        if (remoteObject_ == nullptr) {
            APP_LOGE("%{public}s:fail to get Distributed Ability Service", __func__);
            return GET_DISTRIBUTED_ABILITY_SERVICE_FAILED;
        }
    }

    if (dmsProxy_ == nullptr) {
        dmsProxy_ = std::make_shared<OHOS::DistributedSchedule::DistributedSchedProxy>(remoteObject_);
    }

    APP_LOGI("%{public}s called end", __func__);
    return ERR_OK;
}

ErrCode DistributedClient::NotifyCompleteContinuation(
    const std::u16string &devId, int32_t sessionId, bool isSuccess, const sptr<IRemoteObject> &reverseScheduler)
{
    APP_LOGI("%{public}s called", __func__);
    if (remoteObject_ == nullptr) {
        ErrCode err = Connect();
        if (err != ERR_OK) {
            return DISTRIBUTED_ABILITY_SERVICE_NOT_CONNECTED;
        }
    }

    // there need a params for reverseScheduler
    dmsProxy_->NotifyCompleteContinuation(devId, sessionId, isSuccess);
    return 0;
}
}  // namespace AppExecFwk
}  // namespace OHOS
