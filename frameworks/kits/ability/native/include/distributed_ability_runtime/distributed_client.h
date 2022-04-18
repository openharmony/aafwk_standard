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

#ifndef FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CLIENT_H
#define FOUNDATION_AAFWK_STANDARD_FRAMEWORKS_KITS_ABILITY_NATIVE_INCLUDE_CONTINUATION_DISTRIBUTED_CLIENT_H


#include <string>

#include "mission_info.h"
#include "mission_snapshot.h"
#include "want_params.h"
#include "want.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
class DistributedClient {
public:
    DistributedClient() = default;
    virtual ~DistributedClient() = default;
    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken);
    int32_t ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams);
    int32_t StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status, uint32_t accessToken);
    int32_t NotifyCompleteContinuation(const std::u16string &devId, int32_t sessionId, bool isSuccess);
    int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken);
    int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid, uint32_t accessToken);
    int32_t StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag);
    int32_t StopSyncRemoteMissions(const std::string& devId);
    int32_t RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj);
    int32_t UnRegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj);
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions,
        std::vector<AAFwk::MissionInfo>& missionInfos);
    int32_t GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
        std::unique_ptr<MissionSnapshot>& missionSnapshot);
    int32_t StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken);
    int32_t ReleaseRemoteAbility(const sptr<IRemoteObject>& connect, const AppExecFwk::ElementName &element);
    int32_t StartRemoteFreeInstall(const OHOS::AAFwk::Want& want,
        int32_t callerUid, int32_t requestCode, uint32_t accessToken, const sptr<IRemoteObject>& callback);
    enum {
        START_REMOTE_ABILITY = 1,
        CONNECT_REMOTE_ABILITY = 6,
        DISCONNECT_REMOTE_ABILITY = 7,
        START_CONTINUATION = 11,
        NOTIFY_COMPLETE_CONTINUATION = 12,
        CONTINUE_MISSION = 36,
        GET_MISSION_INFOS = 80,
        REGISTER_MISSION_LISTENER = 84,
        UNREGISTER_MISSION_LISTENER = 85,
        START_SYNC_MISSIONS = 92,
        STOP_SYNC_MISSIONS = 98,
        GET_REMOTE_MISSION_SNAPSHOT_INFO = 99,
        START_REMOTE_ABILITY_BY_CALL = 150,
        RELEASE_REMOTE_ABILITY = 151,
        START_REMOTE_FREE_INSTALL = 200,
    };
private:
    sptr<IRemoteObject> GetDmsProxy();
    bool ReadMissionInfosFromParcel(Parcel& parcel, std::vector<AAFwk::MissionInfo>& missionInfos);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
