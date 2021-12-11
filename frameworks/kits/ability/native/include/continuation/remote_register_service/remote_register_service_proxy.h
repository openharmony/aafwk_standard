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

#ifndef FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_PROXY_H

#include "remote_register_service_interface.h"
#include <iremote_object.h>
#include <map>
#include "message_parcel.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class RemoteRegisterServiceProxy : public IRemoteProxy<IRemoteRegisterService> {
public:
    explicit RemoteRegisterServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IRemoteRegisterService>(impl)
    {}
    virtual ~RemoteRegisterServiceProxy() = default;

    /**
     * register to controlcenter continuation register service.
     *
     * @param bundleName bundlename of ability.
     * @param extras filter with supported device list.
     * @param callback callback for device connect and disconnect.
     */
    virtual int Register(const std::string &bundleName, const sptr<IRemoteObject> &token, const ExtraParams &extras,
        const sptr<IConnectCallback> &callback) override;

    /**
     * unregister to controlcenter continuation register service.
     *
     * @param registerToken token from register return value.
     */
    virtual bool Unregister(int registerToken) override;

    /**
     * notify continuation status to controlcenter continuation register service.
     *
     * @param registerToken token from register.
     * @param deviceId deviceid.
     * @param status device status.
     */
    virtual bool UpdateConnectStatus(int registerToken, const std::string &deviceId, int status) override;

    /**
     * notify controlcenter continuation register service to show device list.
     *
     * @param registerToken token from register
     * @param extras filter with supported device list.
     */
    virtual bool ShowDeviceList(int registerToken, const ExtraParams &extras) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_REMOTE_REGISTER_SERVICE_PROXY_H
