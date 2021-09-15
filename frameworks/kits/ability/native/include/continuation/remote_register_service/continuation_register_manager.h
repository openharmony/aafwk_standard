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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_H

#include <string>
#include "continuation_register_manager_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationRegisterManagerProxy;
class ContinuationRegisterManager : public IContinuationRegisterManager {
public:
    ContinuationRegisterManager() = default;
    virtual ~ContinuationRegisterManager() = default;

    void Init(const std::shared_ptr<ContinuationRegisterManagerProxy> &continuationRegisterManagerProxy);
    /**
     * register to controlcenter continuation register service.
     *
     * @param bundleName bundlename of ability.
     * @param parameter filter with supported device list.
     * @param callback callback for device connect and disconnect.
     * @param requestCallback callback for this request, -1 means failed, otherwise is register token.
     */
    virtual void Register(const std::string &bundleName, const ExtraParams &parameter,
        const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
        const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * unregister to controlcenter continuation register service.
     *
     * @param token token from register return value.
     * @param requestCallback callback for this request, -1 means failed, otherwise succeeded.
     */
    virtual void Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * notify continuation status to controlcenter continuation register service.
     *
     * @param token token from register.
     * @param deviceId deviceid.
     * @param status device status.
     * @param requestCallback callback for this request, -1 means failed, otherwise successed.
     */
    virtual void UpdateConnectStatus(int token, const std::string &deviceId, int status,
        const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * notify controlcenter continuation register service to show device list.
     *
     * @param token token from register
     * @param parameter filter with supported device list.
     * @param requestCallback callback for this request, -1 means failed, otherwise successed.
     */
    virtual void ShowDeviceList(
        int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * disconnect to controlcenter continuation register service.
     */
    virtual void Disconnect(void) override;

private:
    std::shared_ptr<ContinuationRegisterManagerProxy> continuationRegisterManagerProxy_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_H