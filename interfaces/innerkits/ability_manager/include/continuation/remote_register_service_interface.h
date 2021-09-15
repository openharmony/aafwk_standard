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

#ifndef FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_INTERFACE_H
#define FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_INTERFACE_H

#include <string>
#include <memory>
#include "iremote_broker.h"
#include "extra_params.h"
#include "connect_callback_interface.h"

namespace OHOS {
namespace AppExecFwk {
class IRemoteRegisterService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.RemoteRegisterService");

    IRemoteRegisterService() = default;
    virtual ~IRemoteRegisterService() = default;

    /**
     * register to controlcenter continuation register service.
     *
     * @param bundleName bundlename of ability.
     * @param extras filter with supported device list.
     * @param callback callback for device connect and disconnect.
     */
    virtual int Register(const std::string &bundleName, const sptr<IRemoteObject> &token, const ExtraParams &extras,
        const sptr<IConnectCallback> &callback) = 0;

    /**
     * unregister to controlcenter continuation register service.
     *
     * @param registerToken token from register return value.
     */
    virtual bool Unregister(int registerToken) = 0;

    /**
     * notify continuation status to controlcenter continuation register service.
     *
     * @param registerToken token from register.
     * @param deviceId deviceid.
     * @param status device status.
     */
    virtual bool UpdateConnectStatus(int registerToken, const std::string &deviceId, int status) = 0;

    /**
     * notify controlcenter continuation register service to show device list.
     *
     * @param registerToken token from register
     * @param extras filter with supported device list.
     */
    virtual bool ShowDeviceList(int registerToken, const ExtraParams &extras) = 0;

    enum {
        // ipc id for register
        COMMAND_REGISTER = 1,
        COMMAND_UNREGISTER,
        COMMAND_UPDATE_CONNECT_STATUS,
        COMMAND_SHOW_DEVICE_LIST
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_INTERFACE_H
