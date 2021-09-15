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
#include "continuation_scheduler.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {

std::mutex ContinuationScheduler::mutex_;
ContinuationScheduler::ContinuationScheduler(
    std::weak_ptr<IDistributeScheduleHandler> &callback, std::shared_ptr<EventHandler> &mainHandler)
{
    if (mainHandler == nullptr) {
        APP_LOGE("Handler can not be nullptr.");
    }
    callback_ = callback;
    mainHandler_ = mainHandler;
}

void ContinuationScheduler::ScheduleCompleteContinuation(int result)
{
    APP_LOGI("%{public}s called begin", __func__);
    auto task = [continuationScheduler = this, result]() {
        continuationScheduler->HandleCompleteContinuation(result);
    };

    std::lock_guard<std::mutex> lock(mutex_);

    if (mainHandler_ == nullptr) {
        APP_LOGE("ContinuationScheduler::ScheduleCompleteContinuation mainHandler_ == nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        APP_LOGE("ContinuationScheduler::ScheduleCompleteContinuation PostTask error");
        return;
    }
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationScheduler::ReceiveReplicaScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteReplica == nullptr) {
        APP_LOGE("ContinuationScheduler::ReceiveReplicaScheduler remoteReplica is nullptr");
        return;
    }

    auto task = [continuationScheduler = this, remoteReplica]() {
        continuationScheduler->HandleReceiveReplicaScheduler(remoteReplica);
    };

    std::lock_guard<std::mutex> lock(mutex_);

    if (mainHandler_ == nullptr) {
        APP_LOGE("ContinuationScheduler::ReceiveReplicaScheduler mainHandler_ == nullptr");
        return;
    }

    bool ret = mainHandler_->PostTask(task);
    if (!ret) {
        APP_LOGE("ContinuationScheduler::ReceiveReplicaScheduler PostTask error");
        return;
    }
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationScheduler::HandleReceiveReplicaScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<IDistributeScheduleHandler> callback = nullptr;
    if (GetCallback(callback)) {
        callback->HandleReceiveRemoteScheduler(remoteReplica);
        return;
    }
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationScheduler::HandleCompleteContinuation(int result)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<IDistributeScheduleHandler> callback = nullptr;
    if (GetCallback(callback)) {
        callback->HandleCompleteContinuation(result);
        return;
    }
    APP_LOGI("%{public}s called end", __func__);
}

bool ContinuationScheduler::GetCallback(std::shared_ptr<IDistributeScheduleHandler> &callback)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (callback != nullptr) {
        APP_LOGE("ContinuationScheduler::GetCallback callback is not inited");
        return false;
    }

    callback = callback_.lock();
    if (callback == nullptr) {
        APP_LOGE("ContinuationScheduler::GetCallback callback get failed");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS