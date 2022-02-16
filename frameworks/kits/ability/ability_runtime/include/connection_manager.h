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

#ifndef ABILITY_RUNTIME_CONNECTION_MANAGER_H
#define ABILITY_RUNTIME_CONNECTION_MANAGER_H

#include <map>
#include <vector>
#include "ability_connect_callback.h"
#include "ability_connection.h"
#include "element_name.h"
#include "errors.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
struct ConnectionInfo {
    // connection caller
    sptr<IRemoteObject> connectCaller;
    // connection receiver
    AppExecFwk::ElementName connectReceiver;
    // connection
    sptr<AbilityConnection> abilityConnection;

    ConnectionInfo(sptr<IRemoteObject> connectCaller, AppExecFwk::ElementName connectReceiver,
        sptr<AbilityConnection> abilityConnection):connectCaller(connectCaller), connectReceiver(connectReceiver),
        abilityConnection(abilityConnection)
    {
    }

    inline bool operator < (const ConnectionInfo &that) const
    {
        if (connectCaller < that.connectCaller) {
            return true;
        }
        if (connectCaller == that.connectCaller &&
            connectReceiver.GetBundleName() < that.connectReceiver.GetBundleName()) {
            return true;
        }
        if (connectCaller == that.connectCaller &&
            connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetAbilityName() < that.connectReceiver.GetAbilityName()) {
            return true;
        }

        return false;
    }
};

class ConnectionManager {
public:
    /**
     * @brief Destructor.
     *
     */
    ~ConnectionManager() = default;

    ConnectionManager(const ConnectionManager&)=delete;

    ConnectionManager& operator=(const ConnectionManager&)=delete;

    static ConnectionManager& GetInstance();

    /**
     * @brief connect ability connection.
     *
     * @param connectCaller The connection caller.
     * @param connectReceiver The connection receiver.
     * @param connectCallback The connection callback.
     * @return Returns the result of connecting ability connection.
     */
    ErrCode ConnectAbility(const sptr<IRemoteObject> &connectCaller,
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback);

    /**
     * @brief connect ability connection by user.
     *
     * @param connectCaller The connection caller.
     * @param connectReceiver The connection receiver.
     * @param accountId caller user.
     * @param connectCallback The connection callback.
     * @return Returns the result of connecting ability connection.
     */
    ErrCode ConnectAbilityWithAccount(const sptr<IRemoteObject> &connectCaller,
        const AAFwk::Want &want, int accountId, const sptr<AbilityConnectCallback> &connectCallback);

    /**
     * @brief disconnect ability connection.
     *
     * @param connectCaller The connection caller.
     * @param connectReceiver The connection receiver.
     * @param connectCallback The connection callback.
     * @return Returns the result of disconnecting ability connection.
     */
    ErrCode DisconnectAbility(const sptr<IRemoteObject> &connectCaller,
        const AppExecFwk::ElementName &connectReceiver, const sptr<AbilityConnectCallback> &connectCallback);

    /**
     * @brief check the ability connection of caller is disconnect.
     *
     * @param connectCaller The connection caller.
     * @return Returns whether the ability connection of caller is disconnect.
     */
    bool DisconnectCaller(const sptr<IRemoteObject> &connectCaller);

    /**
     * @brief check the ability connection of receiver is disconnect.
     *
     * @param connectReceiver The connection receiver.
     * @return Returns whether the ability connection of receiver is disconnect.
     */
    bool DisconnectReceiver(const AppExecFwk::ElementName &connectReceiver);

    /**
     * @brief Report the ability connection leak event.
     *
     * @param pid The process id.
     * @param tid The thread id.
     */
    void ReportConnectionLeakEvent(const int pid, const int tid);
private:
    ConnectionManager() = default;
    bool IsConnectCallerEqual(const sptr<IRemoteObject> &connectCaller, const sptr<IRemoteObject> &connectCallerOther);
    bool IsConnectReceiverEqual(const AppExecFwk::ElementName &connectReceiver,
        const AppExecFwk::ElementName &connectReceiverOther);
    ErrCode HandleCallbackTimeOut(const sptr<IRemoteObject> &connectCaller, const AAFwk::Want &want,
        const AppExecFwk::ElementName &connectReceiver, sptr<AbilityConnection> abilityConnection,
        const sptr<AbilityConnectCallback> &connectCallback);
    std::map<ConnectionInfo, std::vector<sptr<AbilityConnectCallback>>> abilityConnections_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_CONNECTION_MANAGER_H