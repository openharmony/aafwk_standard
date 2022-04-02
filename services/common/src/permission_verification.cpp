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

#include "permission_verification.h"

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"

namespace OHOS {
namespace AAFwk {
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName)
{
    HILOG_DEBUG("VerifyCallingPermission permission %{public}s", permissionName.c_str());
    auto callerToken = GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_ERROR("permission %{public}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    HILOG_DEBUG("verify AccessToken success");
    return true;
}

bool PermissionVerification::IsSACall()
{
    HILOG_DEBUG("AmsMgrScheduler::IsSACall is called.");
    auto callerToken = GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOG_DEBUG("caller tokenType is native, verify success");
        return true;
    }
    HILOG_DEBUG("Not SA called.");
    return false;
}

bool PermissionVerification::VerifyRunningInfoPerm()
{
    if (IsSACall()) {
        HILOG_DEBUG("%{public}s: the interface called by SA.", __func__);
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_GET_RUNNING_INFO)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

bool PermissionVerification::VerifyControllerPerm()
{
    if (IsSACall()) {
        HILOG_DEBUG("%{public}s: the interface called by SA.", __func__);
        return true;
    }
    if (VerifyCallingPermission(PermissionConstants::PERMISSION_SET_ABILITY_CONTROLLER)) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return true;
    }
    HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
    return false;
}

unsigned int PermissionVerification::GetCallingTokenID()
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerToken : %{private}u", callerToken);
    return callerToken;
}
}  // namespace AAFwk
}  // namespace OHOS