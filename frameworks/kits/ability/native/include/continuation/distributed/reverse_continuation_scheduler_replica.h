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

#ifndef OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_H
#define OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_H

#include <memory>
#include "ability_handler.h"
#include "iremote_object.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "reverse_continuation_scheduler_replica_stub.h"

namespace OHOS {
namespace AppExecFwk {
class ReverseContinuationSchedulerReplica : public ReverseContinuationSchedulerReplicaStub {
public:
    ReverseContinuationSchedulerReplica(const std::shared_ptr<AbilityHandler> &mainHandler,
        const std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> &replicaHandler);

    virtual ~ReverseContinuationSchedulerReplica() = default;

    void PassPrimary(const sptr<IRemoteObject> &primary);

    bool ReverseContinuation();

    void NotifyReverseResult(int reverseResult);

private:
    void HandlerPassPrimary(const sptr<IRemoteObject> &primary);

    bool HandlerReverseContinuation();

    void HandlerNotifyReverseResult(int reverseResult);

private:
    std::shared_ptr<AbilityHandler> mainHandler_;
    std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> replicaHandler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif