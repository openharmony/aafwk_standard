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

#ifndef OHOS_AAFWK_URI_PERMISSION_MANAGER_INTERFACE_H
#define OHOS_AAFWK_URI_PERMISSION_MANAGER_INTERFACE_H

#include "base/security/access_token/interfaces/innerkits/accesstoken/include/access_token.h"
#include "iremote_broker.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
class IUriPermissionManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.UriPermissionManager");

    /**
     * @brief Authorize the uri permission of fromTokenId to targetTokenId.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param fromTokenId The owner of uri.
     * @param targetTokenId The user of uri.
     */
    virtual void GrantUriPermission(const Uri &uri, unsigned int flag,
        const Security::AccessToken::AccessTokenID fromTokenId,
        const Security::AccessToken::AccessTokenID targetTokenId) = 0;

    /**
     * @brief Check whether the tokenId has URI permissions.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId The user of uri.
     * @return Returns true if the verification is successful, otherwise returns false.
     */
    virtual bool VerifyUriPermission(const Uri &uri, unsigned int flag,
        const Security::AccessToken::AccessTokenID tokenId) = 0;

    /**
     * @brief Clear user's uri authorization record.
     *
     * @param tokenId A tokenId of an application.
     */
    virtual void RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId) = 0;

    enum UriPermMgrCmd {
        // ipc id for GrantUriPermission
        ON_GRANT_URI_PERMISSION = 0,

        // ipc id for VerifyUriPermission
        ON_VERIFY_URI_PERMISSION,

        // ipc id for RemoveUriPermission
        ON_REMOVE_URI_PERMISSION,
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_URI_PERMISSION_MANAGER_INTERFACE_H
