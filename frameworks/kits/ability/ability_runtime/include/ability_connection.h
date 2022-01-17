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

#ifndef ABILITY_RUNTIME_ABILITY_CONNECTION_H
#define ABILITY_RUNTIME_ABILITY_CONNECTION_H

#include "ability_connect_callback.h"
#include "ability_connect_callback_stub.h"
#include "want.h"
#include "element_name.h"

#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    /**
     * @brief Constructor.
     *
     */
    AbilityConnection() = default;

    /**
     * @brief Constructor.
     * @param abilityConnectCallback is used to notify caller ability that connect or disconnect is complete
     */
    explicit AbilityConnection(const sptr<AbilityConnectCallback> &abilityConnectCallback);

    /**
     * @brief Destructor.
     *
     */
    ~AbilityConnection() = default;

    /**
     * OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
     *
     * @param element, service ability's ElementName.
     * @param remoteObject,.the session proxy of service ability.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)  override;

    /**
     * set abilityConnectCallback
     *
     * @param abilityConnectCallback is used to notify caller ability that connect or disconnect is complete
    */
    void SetConnectCallback(sptr<AbilityConnectCallback> abilityConnectCallback);

    void SetRemoteObject(const sptr<IRemoteObject> &remoteObject);

    void SetResultCode(int resultCode);

    sptr<IRemoteObject> GetRemoteObject();

    int GetResultCode();
private:
    sptr<AbilityConnectCallback> abilityConnectCallback_;
    sptr<IRemoteObject> remoteObject_ = nullptr;
    int resultCode_ = -1;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_ABILITY_CONNECTION_H