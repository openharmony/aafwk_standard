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

#ifndef OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_H
#define OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_H

#include "ability_connect_callback_interface.h"
#include "ability_distributed_connect_callback_stub.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityDistributedConnection
 * AbilityDistributeConnect.
 */
class AbilityDistributedConnection : public AbilityDistributedConnectionStub {
public:
    AbilityDistributedConnection(const sptr<AAFwk::IAbilityConnection> &conn);
    ~AbilityDistributedConnection() = default;

    /**
     * OnAbilityDistributeConnectDone, AbilityMs notify caller ability the result of connect.
     *
     * @param element, service ability's ElementName.
     * @param remoteObject,.the session proxy of service ability.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    /**
     * OnAbilityDistributeDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    sptr<AAFwk::IAbilityConnection> conn_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_DISTRIBUTE_CONNECT_CALLBACK_H