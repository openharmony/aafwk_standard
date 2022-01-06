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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_HANDLER_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_HANDLER_H

#include <string>

#include "distribute_schedule_handler_interface.h"
#include "continuation_manager.h"
#include "reverse_continuation_scheduler_primary_proxy.h"
#include "reverse_continuation_scheduler_replica_proxy.h"
#include "reverse_continuation_scheduler_primary_stub.h"
#include "ability.h"
#include "want_params.h"
#include "want.h"
#include "reverse_continuation_scheduler_recipient.h"
#include "reverse_continuation_scheduler_replica_interface.h"
#include "reverse_continuation_scheduler_primary_interface.h"
#include "reverse_continuation_scheduler_primary_handler_interface.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "iremote_broker.h"

using Want = OHOS::AAFwk::Want;
using Ability = OHOS::AppExecFwk::Ability;
namespace OHOS {
namespace AppExecFwk {
class ContinuationHandler : public IDistributeScheduleHandler,
                            public IReverseContinuationSchedulerPrimaryHandler,
                            public IReverseContinuationSchedulerReplicaHandler {
public:
    ContinuationHandler(std::weak_ptr<ContinuationManager> &continuationManager, std::weak_ptr<Ability> &ability);
    virtual ~ContinuationHandler() = default;
    virtual bool HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId) override;
    virtual void HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica) override;
    virtual void HandleCompleteContinuation(int result) override;
    virtual void PassPrimary(const sptr<IRemoteObject> &Primary) override;
    virtual bool ReverseContinuation() override;
    virtual void NotifyReverseResult(int reverseResult) override;
    virtual bool ContinuationBack(const Want &want) override;
    virtual void NotifyReplicaTerminated() override;
    void NotifyTerminationToPrimary();
    void SetReversible(bool reversible);
    void SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo);
    void SetPrimaryStub(const sptr<IRemoteObject> &Primary);
    bool ReverseContinueAbility();

    static const std::string ORIGINAL_DEVICE_ID;

private:
    void OnReplicaDied(const wptr<IRemoteObject> &remote);
    void ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo);
    void CleanUpAfterReverse();
    Want SetWantParams(const WantParams &wantParams);

    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    std::weak_ptr<Ability> ability_;
    std::weak_ptr<ContinuationManager> continuationManager_;
    bool reversible_ = false;
    sptr<IReverseContinuationSchedulerReplica> remoteReplicaProxy_ = nullptr;
    sptr<IReverseContinuationSchedulerPrimary> remotePrimaryProxy_ = nullptr;
    sptr<IRemoteObject> remotePrimaryStub_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> schedulerDeathRecipient_ = nullptr;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CONTINUATION_HANDLER_H