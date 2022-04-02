/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_connection.h"

#include <unistd.h>

#include "connection_manager.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const int DIED = -1;
AbilityConnection::AbilityConnection(const sptr<AbilityConnectCallback> &abilityConnectCallback)
{
    abilityConnectCallback_ = abilityConnectCallback;
}

void AbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityConnectCallback_ == nullptr) {
        HILOG_ERROR("%{public}s abilityConnectCallback is nullptr.", __func__);
        return;
    }
    SetRemoteObject(remoteObject);
    SetResultCode(resultCode);
    abilityConnectCallback_->OnAbilityConnectDone(element, remoteObject, resultCode);
    HILOG_DEBUG("%{public}s end, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str());
}

void AbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_DEBUG("%{public}s begin, resultCode:%{public}d.", __func__, resultCode);
    if (abilityConnectCallback_ == nullptr) {
        HILOG_ERROR("%{public}s abilityConnectCallback is nullptr.", __func__);
        return;
    }
    // if resultCode < 0 that means the connectReceiver is died
    if (resultCode == DIED) {
        bool ret = ConnectionManager::GetInstance().DisconnectReceiver(element);
        if (ret) {
            ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
            HILOG_INFO("The service connection is not disconnected.");
        }
        abilityConnectCallback_->OnAbilityDisconnectDone(element, DIED + 1);
    } else {
        abilityConnectCallback_->OnAbilityDisconnectDone(element, resultCode);
    }
    HILOG_DEBUG("%{public}s end, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str());
}

void AbilityConnection::SetConnectCallback(sptr<AbilityConnectCallback> abilityConnectCallback)
{
    abilityConnectCallback_ = abilityConnectCallback;
}

void AbilityConnection::SetRemoteObject(const sptr<IRemoteObject> &remoteObject)
{
    remoteObject_ = remoteObject;
}

void AbilityConnection::SetResultCode(int resultCode)
{
    resultCode_ = resultCode;
}

sptr<IRemoteObject> AbilityConnection::GetRemoteObject()
{
    return remoteObject_;
}

int AbilityConnection::GetResultCode()
{
    return resultCode_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS