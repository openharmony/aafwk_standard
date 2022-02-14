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

#ifndef OHOS_AAFWK_URI_PERMISSION_MANAGER_STUB_IMPL_H
#define OHOS_AAFWK_URI_PERMISSION_MANAGER_STUB_IMPL_H

#include <map>

#include "uri.h"
#include "uri_permission_manager_stub.h"

namespace OHOS {
namespace AAFwk {
struct GrantInfo {
    unsigned int flag;
    int32_t fromTokenId;
    int32_t targetTokenId;
};
class UriPermissionManagerStubImpl : public UriPermissionManagerStub {
public:
    UriPermissionManagerStubImpl() = default;
    virtual ~UriPermissionManagerStubImpl() = default;

    void GrantUriPermission(const Uri &uri, unsigned int flag, const Security::AccessToken::AccessTokenID fromTokenId,
        const Security::AccessToken::AccessTokenID targetTokenId) override;

    bool VerifyUriPermission(const Uri &uri, unsigned int flag,
        const Security::AccessToken::AccessTokenID tokenId) override;

    void RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId) override;

private:
    std::map<std::string, std::list<GrantInfo>> uriMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_URI_PERMISSION_MANAGER_STUB_IMPL_H
