
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

#include "ability_distributed_connection.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityDistributedConnection::AbilityDistributedConnection(const sptr<AAFwk::IAbilityConnection> &conn)
{
    HILOG_INFO("%{public}s called.", __func__);
    conn_ = conn;
}

void AbilityDistributedConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (conn_ == nullptr) {
        HILOG_INFO("%{public}s conn_ == nullptr.", __func__);
        return;
    }
    conn_->OnAbilityConnectDone(element, remoteObject, resultCode);
}

void AbilityDistributedConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (conn_ == nullptr) {
        HILOG_INFO("%{public}s conn_ == nullptr.", __func__);
        return;
    }
    conn_->OnAbilityDisconnectDone(element, resultCode);
}
}  // namespace AAFwk
}  // namespace OHOS
