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

#include "mission_continue_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

MissionContinueStub::MissionContinueStub()
{}

MissionContinueStub::~MissionContinueStub()
{}

int32_t MissionContinueStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        HILOG_ERROR("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IMissionContinue::NOTIFY_CONTINUATION_RESULT: {
            int32_t result = data.ReadInt32();
            HILOG_INFO("NOTIFY_CONTINUATION_RESULT result: %{public}d", result);
            OnContinueDone(result);
            return NO_ERROR;
        }
        default: {
            HILOG_WARN("MissionContinueStub::OnRemoteRequest code: %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
} // namespace AAFwk
} // namespace OHOS