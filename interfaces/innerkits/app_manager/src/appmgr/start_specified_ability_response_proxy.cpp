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

#include "start_specified_ability_response_proxy.h"
#include "ipc_types.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
StartSpecifiedAbilityResponseProxy::StartSpecifiedAbilityResponseProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStartSpecifiedAbilityResponse>(impl)
{}

bool StartSpecifiedAbilityResponseProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(StartSpecifiedAbilityResponseProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void StartSpecifiedAbilityResponseProxy::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("On accept want by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want) || !data.WriteString(flag)) {
        HILOG_ERROR("Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_ACCEPT_WANT_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void StartSpecifiedAbilityResponseProxy::OnTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("On timeout response by proxy.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        IStartSpecifiedAbilityResponse::Message::ON_TIMEOUT_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
