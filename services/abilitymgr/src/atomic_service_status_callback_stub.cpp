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

#include "atomic_service_status_callback_stub.h"

#include "ability_manager_interface.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
AtomicServiceStatusCallbackStub::AtomicServiceStatusCallbackStub()
{
    vecMemberFunc_.resize(IAtomicServiceStatusCallbackCmd::CMD_MAX);
    vecMemberFunc_[IAtomicServiceStatusCallbackCmd::ON_FREE_INSTALL_DONE] =
        &AtomicServiceStatusCallbackStub::OnInstallFinishedInner;
    vecMemberFunc_[IAtomicServiceStatusCallbackCmd::ON_REMOTE_FREE_INSTALL_DONE] =
        &AtomicServiceStatusCallbackStub::OnRemoteInstallFinishedInner;
}

int AtomicServiceStatusCallbackStub::OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply)
{
    auto resultCode = data.ReadInt32();
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("AtomicServiceStatusCallbackStub want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto userId = data.ReadInt32();
    OnInstallFinished(resultCode, *want, userId);
    return NO_ERROR;
}

int AtomicServiceStatusCallbackStub::OnRemoteInstallFinishedInner(MessageParcel &data, MessageParcel &reply)
{
    auto resultCode = data.ReadInt32();
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("AtomicServiceStatusCallbackStub want is nullptr");
        return ERR_INVALID_VALUE;
    }

    want->SetParam(FROM_REMOTE_KEY, true);
    auto userId = data.ReadInt32();
    OnRemoteInstallFinished(resultCode, *want, userId);
    return NO_ERROR;
}

int AtomicServiceStatusCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AtomicServiceStatusCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code < IAtomicServiceStatusCallbackCmd::CMD_MAX && code >= 0) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS
