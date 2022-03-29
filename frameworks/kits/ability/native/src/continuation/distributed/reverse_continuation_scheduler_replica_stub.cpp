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
#include "reverse_continuation_scheduler_replica_stub.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplicaStub::ReverseContinuationSchedulerReplicaStub()
{
    continuationFuncMap_[static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::PASS_PRIMARY)] =
        &ReverseContinuationSchedulerReplicaStub::PassPrimaryInner;
    continuationFuncMap_[static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::REVERSE_CONTINUATION)] =
        &ReverseContinuationSchedulerReplicaStub::ReverseContinuationInner;
    continuationFuncMap_[static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::NOTIFY_REVERSE_RESULT)] =
        &ReverseContinuationSchedulerReplicaStub::NotifyReverseResultInner;
}
ReverseContinuationSchedulerReplicaStub::~ReverseContinuationSchedulerReplicaStub()
{
    continuationFuncMap_.clear();
}
int32_t ReverseContinuationSchedulerReplicaStub::PassPrimaryInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called begin", __func__);
    sptr<IRemoteObject> primary = nullptr;
    if (data.ReadBool()) {
        primary = data.ReadRemoteObject();
    }
    PassPrimary(primary);
    HILOG_INFO("%{public}s called end", __func__);
    return NO_ERROR;
}
int32_t ReverseContinuationSchedulerReplicaStub::ReverseContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called begin", __func__);
    ReverseContinuation();
    HILOG_INFO("%{public}s called end", __func__);
    return NO_ERROR;
}
int32_t ReverseContinuationSchedulerReplicaStub::NotifyReverseResultInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s called begin", __func__);
    int reverseResult = data.ReadInt32();
    NotifyReverseResult(reverseResult);
    HILOG_INFO("%{public}s called end", __func__);
    return NO_ERROR;
}

int ReverseContinuationSchedulerReplicaStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("%{public}s called begin, code = %{public}u, flags= %{public}d.", __func__, code, option.GetFlags());
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != ReverseContinuationSchedulerReplicaStub::GetDescriptor()) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaStub::OnRemoteRequest token is invalid");
        return ERR_INVALID_STATE;
    }

    auto itFunc = continuationFuncMap_.find(code);
    if (itFunc != continuationFuncMap_.end()) {
        auto continuationFunc = itFunc->second;
        if (continuationFunc != nullptr) {
            return (this->*continuationFunc)(data, reply);
        }
    }
    HILOG_INFO("%{public}s called end", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS