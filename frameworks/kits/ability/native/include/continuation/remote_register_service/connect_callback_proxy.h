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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONNECT_CALLBACK_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_CONNECT_CALLBACK_PROXY_H
#include <string>
#include "connect_callback_interface.h"
#include "iremote_proxy.h"

using string = std::string;
using IRemoteObject = OHOS::IRemoteObject;

namespace OHOS {
namespace AppExecFwk {
/**
 * interface for device connect callback.
 */
class ConnectCallbackProxy : public IRemoteProxy<IConnectCallback> {
public:
    /**
     * @brief Remote device sends connection request.
     * @param deviceId indicators id of connection device.
     * @param deviceType indicators type of connection device.
     * @return none
     */
    virtual void Connect(const string &deviceId, const string &deviceType) override;
    /**
     * @brief Remote device sends disconnection request.
     * @param deviceId indicators id of disconnection device.
     * @return none
     */
    virtual void Disconnect(const string &deviceId) override;

private:
    void RemoteRequest(MessageParcel &data, int commandDisconnect);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ICONNECT_CALLBACK_H
