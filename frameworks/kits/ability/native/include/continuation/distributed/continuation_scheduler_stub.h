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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_STUB_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_STUB_H

#include <string>
#include <map>
#include "continuation_scheduler_interface.h"
#include "iremote_stub.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationSchedulerStub : public IRemoteStub<IContinuationScheduler> {
public:
    ContinuationSchedulerStub();
    ~ContinuationSchedulerStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int ReceiveReplicaSchedulerInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCompleteContinuationInner(MessageParcel &data, MessageParcel &reply);
    using RequestFuncType = int (ContinuationSchedulerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;

    enum {
        SCHEDULE_COMPLETE_CONTINUATION = 1,
        RECEIVE_REPLICA_SCHEDULER = 2,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_STUB_H