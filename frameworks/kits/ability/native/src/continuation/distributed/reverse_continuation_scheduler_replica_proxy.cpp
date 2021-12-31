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
#include "reverse_continuation_scheduler_replica_proxy.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplicaProxy::ReverseContinuationSchedulerReplicaProxy(
    const sptr<IRemoteObject> &remoteObject)
    : IRemoteProxy<IReverseContinuationSchedulerReplica>(remoteObject)
{}

sptr<IRemoteObject> ReverseContinuationSchedulerReplicaProxy::AsObject()
{
    sptr<IRemoteObject> remoteObject = Remote();

    return remoteObject;
}
void ReverseContinuationSchedulerReplicaProxy::PassPrimary(const sptr<IRemoteObject> &primary)
{
    APP_LOGI("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::PassPrimary write interface token failed");
        return;
    }
    if (!data.WriteRemoteObject(primary)) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::PassPrimary write parcel callback failed");
        return;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::PassPrimary Remote() is NULL");
        return;
    }
    if (!remoteObject->SendRequest(
            static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::PASS_PRIMARY), data, reply, option)) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::PassPrimary SendRequest return false");
    }
    APP_LOGI("%{public}s called end", __func__);
}
bool ReverseContinuationSchedulerReplicaProxy::ReverseContinuation()
{
    APP_LOGI("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::ReverseContinuation write interface token failed");
        return false;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::ReverseContinuation Remote() is nullptr");
        return false;
    }
    if (!remoteObject->SendRequest(
            static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::REVERSE_CONTINUATION),
            data,
            reply,
            option)) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::ReverseContinuation SendRequest return false");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}
void ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult(int reverseResult)
{
    APP_LOGI("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult write interface token failed");
        return;
    }
    if (!data.WriteInt32(reverseResult)) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult write parcel flags failed");
        return;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult Remote() is NULL");
        return;
    }
    if (!remoteObject->SendRequest(
            static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::NOTIFY_REVERSE_RESULT),
            data,
            reply,
            option)) {
        APP_LOGE("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult SendRequest return false");
    }
    APP_LOGI("%{public}s called end", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS