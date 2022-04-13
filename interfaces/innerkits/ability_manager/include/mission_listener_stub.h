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

#ifndef OHOS_AAFWK_MISSION_LISTENER_STUB_H
#define OHOS_AAFWK_MISSION_LISTENER_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <vector>

#include "mission_listener_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class MissionListenerStub
 * MissionListener Stub.
 */
class MissionListenerStub : public IRemoteStub<IMissionListener> {
public:
    MissionListenerStub();
    virtual ~MissionListenerStub() = default;

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(MissionListenerStub);

    int OnMissionCreatedInner(MessageParcel &data, MessageParcel &reply);
    int OnMissionDestroyedInner(MessageParcel &data, MessageParcel &reply);
    int OnMissionSnapshotChangedInner(MessageParcel &data, MessageParcel &reply);
    int OnMissionMovedToFrontInner(MessageParcel &data, MessageParcel &reply);
    int OnMissionIconUpdatedInner(MessageParcel &data, MessageParcel &reply);

    using MissionListenerFunc = int (MissionListenerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::vector<MissionListenerFunc> vecMemberFunc_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_MISSION_LISTENER_STUB_H
