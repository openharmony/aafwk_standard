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
#ifndef OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_H
#define OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_H

#include <memory>
#include "reverse_continuation_scheduler_primary_handler_interface.h"
#include "reverse_continuation_scheduler_primary_stub.h"
#include "ability_handler.h"

namespace OHOS {
namespace AppExecFwk {
class ReverseContinuationSchedulerPrimary : public ReverseContinuationSchedulerPrimaryStub {
public:
    ReverseContinuationSchedulerPrimary(
        const std::weak_ptr<IReverseContinuationSchedulerPrimaryHandler> &continuationHandler,
        const std::shared_ptr<AbilityHandler> &mainHandler);
    virtual ~ReverseContinuationSchedulerPrimary() = default;

    /**
     * @brief Replica call this method when it terminated.
     */
    void NotifyReplicaTerminated() override;

    /**
     * @brief Replica call this method to notify primary go on.
     *
     * @param want Contains data to be restore.
     * @return True if success, otherwise false.
     */
    bool ContinuationBack(const AAFwk::Want &want) override;

private:
    void HandlerNotifyReplicaTerminated();
    void HandlerContinuationBack(const AAFwk::Want &want);

    std::weak_ptr<IReverseContinuationSchedulerPrimaryHandler> continuationHandler_;
    const std::shared_ptr<AbilityHandler> mainHandler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_REVERSE_CONTINUATION_SCHEDULER_PRIMARY_H */