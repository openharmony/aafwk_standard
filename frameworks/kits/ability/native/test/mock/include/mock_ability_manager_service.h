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

#ifndef OHOS_AAFWK_ABILITY_MOCK_MANAGER_SERVICE_H
#define OHOS_AAFWK_ABILITY_MOCK_MANAGER_SERVICE_H

#include <memory>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>

#include "ability_manager_stub.h"
#include "iremote_object.h"

#include "gmock/gmock.h"

namespace OHOS {
namespace AAFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class MockAbilityManagerService : public AbilityManagerStub {
public:
    MockAbilityManagerService();
    ~MockAbilityManagerService();
    int StartAbility(const Want &want, int requestCode = -1) override;
    int StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode = -1) override
    {
        return 0;
    }
    int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode = -1, const Want *resultWant = nullptr) override;
    int MinimizeAbility(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int ConnectAbility(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;
    int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken) override;

    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override;
    int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;
    int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override;
    int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override;

    void DumpState(const std::string &args, std::vector<std::string> &info) override;

    int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId) override;
    int StopServiceAbility(const Want &want) override;
    int PowerOff() override;
    int PowerOn() override;

    int LockMission(int missionId) override
    {
        return 0;
    };
    int UnlockMission(int missionId) override
    {
        return 0;
    };

    int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const MissionDescriptionInfo &missionDescriptionInfo) override
    {
        return 0;
    };

    int GetMissionLockModeState()
    {
        return 0;
    }

    int UpdateConfiguration(const AppExecFwk::Configuration &config)
    {
        return 0;
    }

    MOCK_METHOD1(GetAllStackInfo, int(StackInfo &stackInfo));
    MOCK_METHOD2(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(SendWantSender, int(const sptr<IWantSender> &target, const SenderInfo &senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender> &sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender> &target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender> &target, std::shared_ptr<Want> &want));

    MOCK_METHOD4(StartAbility, int(const Want &want, const AbilityStartSetting &abilityStartSetting,
                                   const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD1(MoveMissionToFloatingStack, int(const MissionOption &missionOption));
    MOCK_METHOD2(MoveMissionToSplitScreenStack, int(const MissionOption &primary, const MissionOption &secondary));
    MOCK_METHOD2(
        ChangeFocusAbility, int(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken));
    MOCK_METHOD1(MinimizeMultiWindow, int(int missionId));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(GetFloatingMissions, int(std::vector<AbilityMissionInfo> &list));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(SetMissionStackSetting, int(const StackSetting &stackSetting));
    MOCK_METHOD1(GetPendinTerminateAbilityTestgRequestWant, void(int id));
    MOCK_METHOD1(GetSystemMemoryAttr, void(AppExecFwk::SystemMemoryAttr &memoryInfo));
    MOCK_METHOD2(StartContinuation, int(const Want &want, const sptr<IRemoteObject> &abilityToken));
    MOCK_METHOD2(NotifyContinuationResult, int(const sptr<IRemoteObject> &abilityToken, const int32_t result));

    MOCK_METHOD1(LockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(UnlockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(RegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD1(UnRegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD3(
        GetMissionInfos, int(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos));
    MOCK_METHOD3(GetMissionInfo, int(const std::string& deviceId, int32_t missionId, MissionInfo &missionInfo));
    MOCK_METHOD1(CleanMission, int(int32_t missionId));
    MOCK_METHOD0(CleanAllMissions, int());
    MOCK_METHOD1(MoveMissionToFront, int(int32_t missionId));

    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
    
    int RemoveMission(int id) override;

    int RemoveStack(int id) override;
    sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }

    int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList) override
    {
        return 0;
    }

    int GetMissionSnapshot(const int32_t missionId, MissionPixelMap &missionPixelMap)
    {
        return 0;
    }

    int MoveMissionToTop(int32_t missionId) override;

    int KillProcess(const std::string &bundleName) override;

    int UninstallApp(const std::string &bundleName) override;

    int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode) override
    {
        return 0;
    }

    virtual int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst);

    virtual bool IsFirstInMission(const sptr<IRemoteObject> &token);

    virtual int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message);

    virtual int SetShowOnLockScreen(bool isAllow) override
    {
        return 0;
    }

    virtual int ClearUpApplicationData(const std::string &bundleName) override
    {
        return 0;
    }

    virtual int StartUser(int userId) override
    {
        return 0;
    }

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override
    {
        return 0;
    }

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }

    virtual int StopSyncRemoteMissions(const std::string& devId) override
    {
        return 0;
    }

    enum RequestCode {
        E_STATE_INITIAL = 0,
        E_STATE_INACTIVE,
        E_STATE_ACTIVE,
        E_STATE_BACKGROUND,
        E_STATE_SUSPENDED,
    };

    sptr<IAbilityScheduler> abilityScheduler_;  // kit interface used to schedule ability life
    Want want_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MOCK_MANAGER_SERVICE_H
