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

#ifndef FOUNDATION_APPEXECFWK_CONTINUATION_CONNECTOR_H
#define FOUNDATION_APPEXECFWK_CONTINUATION_CONNECTOR_H

#include <string>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>

#include "iremote_broker.h"
#include "element_name.h"
#include "refbase.h"
#include "context.h"
#include "extra_params.h"
#include "continuation_request.h"
#include "ability_connect_callback_stub.h"
#include "continuation/kits/continuation_device_callback_interface.h"
#include "continuation/remote_register_service_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationConnector : public AAFwk::AbilityConnectionStub {
public:
    ContinuationConnector(const std::weak_ptr<Context> &context);
    virtual ~ContinuationConnector() = default;

    /**
     * @brief get singleton of Class ContinuationConnector
     *
     * @param context: the running context for appcontext
     *
     * @return The singleton of ContinuationConnector
     */
    static sptr<ContinuationConnector> GetInstance(const std::weak_ptr<Context> &context);

    /**
     * @brief This method is called back to receive the connection result after an ability calls the
     * Ability#connectAbility(Want, IAbilityConnection) method to connect it to a Service ability.
     *
     * @param element: Indicates information about the connected Service ability.
     * @param remote: Indicates the remote proxy object of the Service ability.
     * @param resultCode: Indicates the connection result code. The value 0 indicates a successful connection, and any
     * other value indicates a connection failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    /**
     * @brief This method is called back to receive the disconnection result after the connected Service ability crashes
     * or is killed. If the Service ability exits unexpectedly, all its connections are disconnected, and each ability
     * previously connected to it will call onAbilityDisconnectDone.
     *
     * @param element: Indicates information about the disconnected Service ability.
     * @param resultCode: Indicates the disconnection result code. The value 0 indicates a successful disconnection, and
     * any other value indicates a disconnection failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    /**
     * @brief bind remote ability of RemoteRegisterService.
     *
     * @param request: request for continuation.
     */
    void BindRemoteRegisterAbility(const std::shared_ptr<AppExecFwk::ContinuationRequest> &request);

    /**
     * @brief unbind remote ability of RemoteRegisterService.
     */
    void UnbindRemoteRegisterAbility();

    /**
     * @brief check whether connected to remote register service.
     *
     * @return bool true if connected, otherwise false.
     */
    bool IsAbilityConnected();

    /**
     * @brief unregister to control center continuation register service.
     *
     * @param token token from register return value.
     *
     * @return bool result of unregister.
     */
    bool Unregister(int token);

    /**
     * @brief notify continuation status to control center continuation register service.
     *
     * @param token token from register.
     * @param deviceId device id.
     * @param status device status.
     *
     * @return bool result of updateConnectStatus.
     */
    bool UpdateConnectStatus(int token, const std::string &deviceId, int status);

    /**
     * @brief notify control center continuation register service to show device list.
     *
     * @param token token from register
     * @param parameter filter with supported device list.
     * @return bool result of showDeviceList.
     */
    bool ShowDeviceList(int token, const AppExecFwk::ExtraParams &parameter);

    /**
     * @brief register to control center continuation register service.
     *
     * @param context ability context.
     * @param bundleName bundle name of ability.
     * @param parameter filter with supported device list.
     * @param callback callback for device connect and disconnect.
     *
     * @return int token.
     */
    int Register(std::weak_ptr<Context> &context, const std::string bundleName,
        const AppExecFwk::ExtraParams &parameter, std::shared_ptr<IContinuationDeviceCallback> &callback);

private:
    /**
     * @brief bind remote ability of RemoteRegisterService.
     */
    inline void BindRemoteRegisterAbility();

private:
    static const std::string CONNECTOR_DEVICE_ID;
    static const std::string CONNECTOR_ABILITY_NAME;
    static const std::string CONNECTOR_BUNDLE_NAME;
    static sptr<ContinuationConnector> instance_;
    static std::mutex mutex_;
    std::weak_ptr<Context> context_;
    std::vector<std::shared_ptr<ContinuationRequest>> continuationRequestList_;
    std::atomic<bool> isConnected_ = {false};
    sptr<IRemoteRegisterService> remoteRegisterService_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_CONTINUATION_CONNECTOR_H