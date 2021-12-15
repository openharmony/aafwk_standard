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

#ifndef OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_INTERFACE_PROXY_H
#define OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_REPLICA_INTERFACE_PROXY_H

#include <string>
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "reverse_continuation_scheduler_replica_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ReverseContinuationSchedulerReplicaProxy : public IRemoteProxy<IReverseContinuationSchedulerReplica> {
public:
    explicit ReverseContinuationSchedulerReplicaProxy(const sptr<IRemoteObject> &remoteObject);
    virtual ~ReverseContinuationSchedulerReplicaProxy() {};

    virtual void PassPrimary(const sptr<IRemoteObject> &primary) override;

    virtual bool ReverseContinuation() override;

    virtual void NotifyReverseResult(int reverseResult) override;

    virtual sptr<IRemoteObject> AsObject() override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif