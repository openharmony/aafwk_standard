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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_H

#include <string>
#include <mutex>
#include <memory>

#include "distribute_schedule_handler_interface.h"
#include "continuation_scheduler_stub.h"
#include "event_handler.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationScheduler : public ContinuationSchedulerStub {

public:
    ContinuationScheduler(
        std::weak_ptr<IDistributeScheduleHandler> &callback, std::shared_ptr<EventHandler> &mainHandler);
    ~ContinuationScheduler() = default;
    void ReceiveReplicaScheduler(const sptr<IRemoteObject> &remoteReplica);
    void ScheduleCompleteContinuation(int result);

private:
    void HandleReceiveReplicaScheduler(const sptr<IRemoteObject> &remoteReplica);
    void HandleCompleteContinuation(int result);
    bool GetCallback(std::shared_ptr<IDistributeScheduleHandler> &callback);

    static std::mutex mutex_;
    std::shared_ptr<EventHandler> mainHandler_;
    std::weak_ptr<IDistributeScheduleHandler> callback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_H