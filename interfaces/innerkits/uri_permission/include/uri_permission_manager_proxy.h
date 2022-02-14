/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_URI_PERMISSION_MANAGER_PROXY_H
#define OHOS_AAFWK_URI_PERMISSION_MANAGER_PROXY_H

#include "iremote_proxy.h"
#include "uri_permission_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionManagerProxy : public IRemoteProxy<IUriPermissionManager> {
public:
    explicit UriPermissionManagerProxy(const sptr<IRemoteObject> &impl);
    virtual ~UriPermissionManagerProxy() = default;

    virtual void GrantUriPermission(const Uri &uri, unsigned int flag,
        const Security::AccessToken::AccessTokenID fromTokenId,
        const Security::AccessToken::AccessTokenID targetTokenId) override;

    virtual bool VerifyUriPermission(const Uri &uri, unsigned int flag,
        const Security::AccessToken::AccessTokenID tokenId) override;

    virtual void RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId) override;

private:
    static inline BrokerDelegator<UriPermissionManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_URI_PERMISSION_MANAGER_PROXY_H
