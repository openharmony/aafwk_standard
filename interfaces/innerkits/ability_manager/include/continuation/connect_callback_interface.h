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
#ifndef FOUNDATION_APPEXECFWK_OHOS_ICONNECT_CALLBACK_H
#define FOUNDATION_APPEXECFWK_OHOS_ICONNECT_CALLBACK_H
#include <string>
#include "iremote_broker.h"

using string = std::string;
namespace OHOS {
namespace AppExecFwk {
/**
 * interface for device connect callback.
 */
class IConnectCallback : public IRemoteBroker {
public:
    IConnectCallback() = default;
    virtual ~IConnectCallback() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.iconnectcallback");
    /**
     * @brief Connect device interface.
     * @param deviceId indicates the device ID of the connection.
     * @param deviceType indicates the device type of the connection.
     * @return none.
     */
    virtual void Connect(const string &deviceId, const string &deviceType) = 0;
    /**
     * @brief disconnect device interface.
     * @param deviceId indicates the device ID of the disconnect.
     * @param deviceType indicates the device type of the connectdisconnection.
     * @return none.
     */
    virtual void Disconnect(const string &deviceId) = 0;

    // connect status
    enum {
        COMMAND_CONNECT = 1,
        COMMAND_DISCONNECT,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ICONNECT_CALLBACK_H
