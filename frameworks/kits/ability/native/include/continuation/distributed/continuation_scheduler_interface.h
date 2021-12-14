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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_INTERFACE_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_INTERFACE_H

#include <string>

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
class IContinuationScheduler : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityshell.ContinuationScheduler");
    /**
     * @brief Receive a scheduler which could handle reverse continuation.
     *        Remote side will pass an scheduler before continuation completed if this continuation is
     *        reversible. This method will not be called if this continuation is not reversible.
     * @param remoteReplica A scheduler to handle reverse continuation request.
     */
    virtual void ReceiveReplicaScheduler(const sptr<IRemoteObject> &remoteReplica) = 0;

    /**
     * @brief Called by DMS when the continuing device finished.
     * @param result Zero indicate the continuation is success, otherwise integer less than zero.
     */
    virtual void ScheduleCompleteContinuation(int result) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_ABILITY_INCLUDE_DISTRIBUTED_CONTINUATION_SCHEDULE_FOR_DMS_INTERFACE_H