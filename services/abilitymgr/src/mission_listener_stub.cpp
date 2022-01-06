/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mission_listener_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
MissionListenerStub::MissionListenerStub()
{
    vecMemberFunc_.resize(IMissionListener::MISSION_LINSTENER_CMD_MAX);
    vecMemberFunc_[ON_MISSION_CREATED] = &MissionListenerStub::OnMissionCreatedInner;
    vecMemberFunc_[ON_MISSION_DESTROYED] = &MissionListenerStub::OnMissionDestroyedInner;
    vecMemberFunc_[ON_MISSION_SNAPSHOT_CHANGED] = &MissionListenerStub::OnMissionSnapshotChangedInner;
    vecMemberFunc_[ON_MISSION_MOVED_TO_FRONT] = &MissionListenerStub::OnMissionMovedToFrontInner;
}

int MissionListenerStub::OnMissionCreatedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionCreated(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionDestroyedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionDestroyed(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionSnapshotChangedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionSnapshotChanged(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionMovedToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionMovedToFront(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = MissionListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code < IMissionListener::MISSION_LINSTENER_CMD_MAX && code >= 0) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS