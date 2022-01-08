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

#ifndef OHOS_AAFWK_REMOTE_MISSION_LISTENER_STUB_H
#define OHOS_AAFWK_REMOTE_MISSION_LISTENER_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "remote_mission_listener_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for remote mission listener proxy.
 */
class RemoteMissionListenerStub : public IRemoteStub<IRemoteMissionListener> {
public:
    RemoteMissionListenerStub();
    virtual ~RemoteMissionListenerStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    DISALLOW_COPY_AND_MOVE(RemoteMissionListenerStub);

    int32_t NotifyMissionsChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifySnapshotInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyNetDisconnectInner(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_REMOTE_MISSION_LISTENER_STUB_H
