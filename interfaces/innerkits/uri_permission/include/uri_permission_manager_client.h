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

#ifndef OHOS_AAFWK_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_AAFWK_URI_PERMISSION_MANAGER_CLIENT_H

#include <functional>

#include "singleton.h"
#include "uri.h"
#include "uri_permission_manager_interface.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void()>;
class UriPermissionManagerClient : public DelayedSingleton<UriPermissionManagerClient>,
                                   public std::enable_shared_from_this<UriPermissionManagerClient> {
public:
    UriPermissionManagerClient() = default;
    ~UriPermissionManagerClient() = default;

    /**
     * @brief Authorize the uri permission of fromTokenId to targetTokenId.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param fromTokenId The owner of uri.
     * @param targetTokenId The user of uri.
     */
    void GrantUriPermission(const Uri &uri, unsigned int flag, const Security::AccessToken::AccessTokenID fromTokenId,
        const Security::AccessToken::AccessTokenID targetTokenId);
    
    /**
     * @brief Check whether the tokenId has URI permissions.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId The user of uri.
     * @return Returns true if the verification is successful, otherwise returns false.
     */
    bool VerifyUriPermission(const Uri &uri, unsigned int flag, const Security::AccessToken::AccessTokenID tokenId);

    /**
     * @brief Clear user's uri authorization record.
     *
     * @param tokenId A tokenId of an application.
     */
    void RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId);

private:
    sptr<IUriPermissionManager> ConnectUriPermService();
    void ClearProxy();
    DISALLOW_COPY_AND_MOVE(UriPermissionManagerClient);

    class UpmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        UpmsDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        ~UpmsDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    sptr<IUriPermissionManager> uriPermMgr_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_URI_PERMISSION_MANAGER_CLIENT_H
