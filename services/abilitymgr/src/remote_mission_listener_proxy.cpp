
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

#include "remote_mission_listener_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void RemoteMissionListenerProxy::NotifyMissionsChanged(const std::string& deviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        HILOG_ERROR("NotifyMissionsChanged Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("NotifyMissionsChanged Write deviceId failed.");
        return;
    }
    int result = Remote()->SendRequest(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("NotifyMissionsChanged SendRequest fail, error: %{public}d", result);
        return;
    }
}

void RemoteMissionListenerProxy::NotifySnapshot(const std::string& deviceId, int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        HILOG_ERROR("NotifySnapshot Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("NotifySnapshot Write deviceId failed.");
        return;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("NotifySnapshot Write missionId failed.");
        return;
    }
    int result = Remote()->SendRequest(IRemoteMissionListener::NOTIFY_SNAPSHOT, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("NotifySnapshot SendRequest fail, error: %{public}d", result);
        return;
    }
}

void RemoteMissionListenerProxy::NotifyNetDisconnect(const std::string& deviceId, int32_t state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        HILOG_ERROR("NotifyNetDisconnect Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("NotifyNetDisconnect Write deviceId failed.");
        return;
    }
    if (!data.WriteInt32(state)) {
        HILOG_ERROR("NotifyNetDisconnect Write missionId failed.");
        return;
    }
    int result = Remote()->SendRequest(IRemoteMissionListener::NOTIFY_NET_DISCONNECT, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("NotifyNetDisconnect SendRequest fail, error: %{public}d", result);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
