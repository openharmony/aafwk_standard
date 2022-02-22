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

#include "uri_permission_manager_stub_impl.h"

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
void UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID fromTokenId, const Security::AccessToken::AccessTokenID targetTokenId)
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerTokenId : %{pulic}u", callerTokenId);
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOG_DEBUG("caller tokenType is not native, verify failure.");
        return;
    }
    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: The param flag is invalid.");
        return;
    }
    auto uriStr = uri.ToString();
    auto search = uriMap_.find(uriStr);
    GrantInfo info = {flag, fromTokenId, targetTokenId};
    if (search == uriMap_.end()) {
        HILOG_INFO("uri is not exist, add uri and GrantInfo to map.");
        std::list<GrantInfo> infos = { info };
        uriMap_.emplace(uriStr, infos);
        return;
    }
    auto infoList = search->second;
    for (auto item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
                item.flag = flag;
            }
            HILOG_INFO("uri permission has granted, not to grant again.");
            return;
        }
    }
    HILOG_DEBUG("uri is exist, add GrantInfo to list.");
    infoList.emplace_back(info);
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID tokenId)
{
    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl:::VerifyUriPermission: The param flag is invalid.");
        return false;
    }
    unsigned int tmpFlag = 0;
    if (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        tmpFlag |= Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    }
    if (flag & Want::FLAG_AUTH_READ_URI_PERMISSION) {
        tmpFlag |= Want::FLAG_AUTH_READ_URI_PERMISSION;
    }
    auto uriStr = uri.ToString();
    auto search = uriMap_.find(uriStr);
    if (search == uriMap_.end()) {
        HILOG_DEBUG("This tokenID does not have permission for this uri.");
        return false;
    }
    for (auto item : search->second) {
        if (item.targetTokenId == tokenId &&
            (item.flag == Want::FLAG_AUTH_WRITE_URI_PERMISSION || item.flag == tmpFlag)) {
            HILOG_DEBUG("This tokenID have permission for this uri.");
            return true;
        }
    }
    HILOG_DEBUG("The application does not have permission for this URI.");
    return false;
}

void UriPermissionManagerStubImpl::RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
        auto list = iter->second;
        for (auto it = list.begin(); it != list.end();) {
            if (it->targetTokenId == tokenId) {
                HILOG_INFO("Erase an info form list.");
                list.erase(it++);
                break;
            }
        }
        if (list.size() == 0) {
            uriMap_.erase(iter++);
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS