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
#include "reverse_continuation_scheduler_replica.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplica::ReverseContinuationSchedulerReplica(
    const std::shared_ptr<AbilityHandler> &mainHandler,
    const std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> &replicaHandler)
{
    mainHandler_ = mainHandler;
    replicaHandler_ = replicaHandler;
}
void ReverseContinuationSchedulerReplica::PassPrimary(const sptr<IRemoteObject> &primary)
{
    APP_LOGI("%{public}s called begin", __func__);
    auto task = [reverseContinuationSchedulerReplica = this, primary]() {
        reverseContinuationSchedulerReplica->HandlerPassPrimary(primary);
    };

    if (mainHandler_ == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::PassPrimary mainHandler_ == nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        APP_LOGE("ReverseContinuationSchedulerReplica::PassPrimary PostTask error");
    }
    APP_LOGI("%{public}s called end", __func__);
}
bool ReverseContinuationSchedulerReplica::ReverseContinuation()
{
    APP_LOGI("%{public}s called begin", __func__);
    auto task = [reverseContinuationSchedulerReplica = this]() {
        reverseContinuationSchedulerReplica->HandlerReverseContinuation();
    };

    if (mainHandler_ == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::ReverseContinuation mainHandler_ == nullptr");
        return false;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        APP_LOGE("ReverseContinuationSchedulerReplica::ReverseContinuation PostTask error");
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}
void ReverseContinuationSchedulerReplica::NotifyReverseResult(int reverseResult)
{
    APP_LOGI("%{public}s called begin", __func__);
    auto task = [reverseContinuationSchedulerReplica = this, reverseResult]() {
        reverseContinuationSchedulerReplica->HandlerNotifyReverseResult(reverseResult);
    };

    if (mainHandler_ == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::NotifyReverseResult mainHandler_ == nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        APP_LOGE("ReverseContinuationSchedulerReplica::NotifyReverseResult PostTask error");
    }
    APP_LOGI("%{public}s called end", __func__);
}

void ReverseContinuationSchedulerReplica::HandlerPassPrimary(const sptr<IRemoteObject> &primary)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::PassPrimary get replicaHandlerTmp is nullptr");
        return;
    }
    replicaHandlerTmp->PassPrimary(primary);
    APP_LOGI("%{public}s called end", __func__);
}

bool ReverseContinuationSchedulerReplica::HandlerReverseContinuation()
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::PassPrimary get replicaHandlerTmp is nullptr");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return replicaHandlerTmp->ReverseContinuation();
}

void ReverseContinuationSchedulerReplica::HandlerNotifyReverseResult(int reverseResult)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandlerTmp = nullptr;
    replicaHandlerTmp = replicaHandler_.lock();
    if (replicaHandlerTmp == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplica::PassPrimary get replicaHandlerTmp is nullptr");
        return;
    }
    replicaHandlerTmp->NotifyReverseResult(reverseResult);
    APP_LOGI("%{public}s called end", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS