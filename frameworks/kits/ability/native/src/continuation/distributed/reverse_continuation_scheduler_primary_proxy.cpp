
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

#include "reverse_continuation_scheduler_primary_proxy.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

ReverseContinuationSchedulerPrimaryProxy::ReverseContinuationSchedulerPrimaryProxy(
    const sptr<IRemoteObject> &remoteObject)
    : IRemoteProxy<IReverseContinuationSchedulerPrimary>(remoteObject)
{}

/**
 * @brief Replica call this method when it terminated.
 */
void ReverseContinuationSchedulerPrimaryProxy::NotifyReplicaTerminated()
{
    HILOG_INFO("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerPrimaryProxy::GetDescriptor())) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::NotifyReplicaTerminated write interface token failed");
        return;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::NotifyReplicaTerminated Remote() is nullptr");
        return;
    }
    if (!remoteObject->SendRequest(
        IReverseContinuationSchedulerPrimary::NOTIFY_REPLICA_TERMINATED, data, reply, option)) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::NotifyReplicaTerminated SendRequest return false");
        return;
    }
    HILOG_INFO("%{public}s called end", __func__);
}

/**
 * @brief Replica call this method to notify primary go on.
 *
 * @param want Contains data to be restore.
 * @return True if success, otherwise false.
 */
bool ReverseContinuationSchedulerPrimaryProxy::ContinuationBack(const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerPrimaryProxy::GetDescriptor())) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::ContinuationBack write interface token failed");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::ContinuationBack fail to WriteParcelable");
        return false;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::ContinuationBack Remote() is nullptr");
        return false;
    }
    if (!remoteObject->SendRequest(IReverseContinuationSchedulerPrimary::CONTINUATION_BACK, data, reply, option)) {
        HILOG_ERROR("ReverseContinuationSchedulerPrimaryProxy::ContinuationBack SendRequest return false");
        return false;
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS