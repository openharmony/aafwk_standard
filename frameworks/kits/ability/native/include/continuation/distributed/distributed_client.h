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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CLIENT_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CLIENT_H


#include <string>
#include <mutex>

#include "ability.h"
#include "ability_info.h"
#include "want_params.h"
#include "want.h"
#include "iremote_broker.h"
#include "distributed_errors.h"

#include "distributed_sched_proxy.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::DistributedSchedule;

class DistributedClient {
public:
    DistributedClient();
    virtual ~DistributedClient();
    static std::shared_ptr<DistributedClient> GetInstance();
    ErrCode Connect();
    ErrCode NotifyCompleteContinuation(
        const std::u16string &devId, int32_t sessionId, bool isSuccess, const sptr<IRemoteObject> &reverseScheduler);

private:
    static std::mutex mutex_;
    static std::shared_ptr<DistributedClient> instance_;
    sptr<IRemoteObject> remoteObject_;

    std::shared_ptr<OHOS::DistributedSchedule::DistributedSchedProxy> dmsProxy_;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif