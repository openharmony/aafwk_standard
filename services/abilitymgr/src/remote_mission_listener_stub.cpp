
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

#include "remote_mission_listener_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
RemoteMissionListenerStub::RemoteMissionListenerStub()
{}

RemoteMissionListenerStub::~RemoteMissionListenerStub()
{}

int RemoteMissionListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RemoteMissionListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("RemoteMissionListenerStub Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IRemoteMissionListener::NOTIFY_MISSION_CHANGED: {
            return NotifyMissionsChangedInner(data, reply);
        }
        case IRemoteMissionListener::NOTIFY_SNAPSHOT: {
            return NotifySnapshotInner(data, reply);
        }
        case IRemoteMissionListener::NOTIFY_NET_DISCONNECT: {
            return NotifyNetDisconnectInner(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t RemoteMissionListenerStub::NotifyMissionsChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    NotifyMissionsChanged(deviceId);
    return NO_ERROR;
}

int32_t RemoteMissionListenerStub::NotifySnapshotInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    NotifySnapshot(deviceId, missionId);
    return NO_ERROR;
}

int32_t RemoteMissionListenerStub::NotifyNetDisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t state = data.ReadInt32();
    NotifyNetDisconnect(deviceId, state);
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
