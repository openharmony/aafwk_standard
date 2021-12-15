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
#ifndef OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_INTERFACE_H
#define OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_INTERFACE_H

#include "want.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
class IReverseContinuationSchedulerPrimary : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.ReverseContinuationSchedulerPrimary");
    IReverseContinuationSchedulerPrimary() = default;
    virtual ~IReverseContinuationSchedulerPrimary() = default;

    /**
     * @brief Replica call this method when it terminated.
     */
    virtual void NotifyReplicaTerminated() = 0;

    /**
     * @brief Replica call this method to notify primary go on.
     *
     * @param want Contains data to be restore.
     * @return True if success, otherwise false.
     */
    virtual bool ContinuationBack(const AAFwk::Want &want) = 0;

    enum {
        NOTIFY_REPLICA_TERMINATED = 1,
        CONTINUATION_BACK,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_INTERFACE_H */