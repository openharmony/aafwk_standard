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
#include "continuation_scheduler_stub.h"
#include "ipc_types.h"
#include "string_ex.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
ContinuationSchedulerStub::ContinuationSchedulerStub()
{
    requestFuncMap_[SCHEDULE_COMPLETE_CONTINUATION] =
        &ContinuationSchedulerStub::ScheduleCompleteContinuationInner;
    requestFuncMap_[RECEIVE_REPLICA_SCHEDULER] = &ContinuationSchedulerStub::ReceiveReplicaSchedulerInner;
}

ContinuationSchedulerStub::~ContinuationSchedulerStub()
{
    requestFuncMap_.clear();
}

int ContinuationSchedulerStub::ReceiveReplicaSchedulerInner(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called begin", __func__);
    auto remoteReplica = data.ReadParcelable<IRemoteObject>();
    ReceiveReplicaScheduler(remoteReplica);
    APP_LOGI("%{public}s called end", __func__);
    return NO_ERROR;
}

int ContinuationSchedulerStub::ScheduleCompleteContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    APP_LOGI("%{public}s called begin", __func__);
    int result = data.ReadInt32();
    ScheduleCompleteContinuation(result);
    APP_LOGI("%{public}s called end", __func__);
    return NO_ERROR;
}

int ContinuationSchedulerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::u16string descriptor = ContinuationSchedulerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        APP_LOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    APP_LOGI("ContinuationSchedulerStub::OnRemoteRequest, default case, need check.");
    APP_LOGI("%{public}s called end", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS