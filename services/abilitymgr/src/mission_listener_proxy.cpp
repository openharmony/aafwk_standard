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

#include "mission_listener_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void MissionListenerProxy::OnMissionCreated(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_CREATED);
}

void MissionListenerProxy::OnMissionDestroyed(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_DESTROYED);
}

void MissionListenerProxy::OnMissionSnapshotChanged(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_SNAPSHOT_CHANGED);
}

void MissionListenerProxy::OnMissionMovedToFront(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_MOVED_TO_FRONT);
}

#ifdef SUPPORT_GRAPHICS
void MissionListenerProxy::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (!icon) {
        HILOG_ERROR("invalid mission icon.");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    HILOG_INFO("mission_listener_proxy, OnMissionIconUpdated,missionId:%{public}d", missionId);
    if (!data.WriteInterfaceToken(IMissionListener::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed when proxy call OnMissionIconUpdated.");
        return;
    }

    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("Write missionId failed.");
        return;
    }

    if (!data.WriteParcelable(icon.get())) {
        HILOG_ERROR("write icon failed.");
        return;
    }

    int error = Remote()->SendRequest(IMissionListener::ON_MISSION_ICON_UPDATED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("SendRequest icon updated fail, error: %{public}d", error);
        return;
    }
}
#endif

void MissionListenerProxy::SendRequestCommon(int32_t missionId, IMissionListener::MissionListenerCmd cmd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    HILOG_INFO("mission_listener_proxy, sendrequest, cmd:%{public}d, missionId:%{public}d", cmd, missionId);
    if (!data.WriteInterfaceToken(IMissionListener::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("Write missionId error.");
        return;
    }

    int error = Remote()->SendRequest(cmd, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnMissionCreated fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS