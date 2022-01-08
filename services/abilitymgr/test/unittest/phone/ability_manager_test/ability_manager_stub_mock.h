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

#ifndef ABILITY_UNITTEST_ABILITY_MANAGER_STUB_MOCK_H
#define ABILITY_UNITTEST_ABILITY_MANAGER_STUB_MOCK_H
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "hilog_wrapper.h"
#include "ability_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerStubMock : public IRemoteStub<IAbilityManager> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAbilityManagerMock");

    AbilityManagerStubMock()
    {}
    virtual ~AbilityManagerStubMock()
    {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD2(StartAbility, int(const Want &, int));
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject> &, int, const Want *));
    MOCK_METHOD1(MinimizeAbility, int(const sptr<IRemoteObject> &);
    MOCK_METHOD3(ConnectAbility, int(const Want &, const sptr<IAbilityConnection> &, const sptr<IRemoteObject> &));

    MOCK_METHOD1(DisconnectAbility, int(const sptr<IAbilityConnection> &));
    MOCK_METHOD3(AcquireDataAbility, sptr<IAbilityScheduler>(const Uri &, bool, const sptr<IRemoteObject> &));
    MOCK_METHOD2(ReleaseDataAbility, int(sptr<IAbilityScheduler>, const sptr<IRemoteObject> &));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject> &, int32_t));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler> &, const sptr<IRemoteObject> &));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject> &, int, const PacMap &));
    MOCK_METHOD2(ScheduleConnectAbilityDone, int(const sptr<IRemoteObject> &, const sptr<IRemoteObject> &));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject> &));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject> &));
    MOCK_METHOD2(DumpState, void(const std::string &, std::vector<std::string> &));
    MOCK_METHOD2(TerminateAbilityResult, int(const sptr<IRemoteObject> &, int));
    MOCK_METHOD1(StopServiceAbility, int(const Want &));
    MOCK_METHOD1(GetAllStackInfo, int(StackInfo &));
    MOCK_METHOD3(GetRecentMissions, int(const int32_t, const int32_t, std::vector<AbilityMissionInfo> &));
    MOCK_METHOD2(GetMissionSnapshot, int(const int32_t, MissionSnapshotInfo &));
    MOCK_METHOD1(RemoveMission, int(int));
    MOCK_METHOD1(RemoveStack, int(int));
    MOCK_METHOD1(MoveMissionToTop, int(int32_t));
    MOCK_METHOD1(KillProcess, int(const std::string &));
    MOCK_METHOD1(UninstallApp, int(const std::string &));
    MOCK_METHOD2(TerminateAbilityByCaller, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD3(StartAbility, int(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject> &token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject> &token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string &permission, int pid, int uid, std::string &message));
    MOCK_METHOD0(PowerOff, int());
    MOCK_METHOD0(PowerOn, int());
    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD2(SetMissionDescriptionInfo, int(const sptr<IRemoteObject> &token, const MissionDescriptionInfo &info));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD1(UpdateConfiguration, int(const AppExecFwk::Configuration &));
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
    MOCK_METHOD1(GetFloatingMissions, int(std::vector<AbilityMissionInfo> &list));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(SetMissionStackSetting, int(const StackSetting &stackSetting));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(ChangeFocusTest, void(const std::vector<int> missionId));
    MOCK_METHOD1(TerminateAbilityTest, void(int id));
    MOCK_METHOD1(MoveMissionToEnd, int(int id));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
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
    MOCK_METHOD3(StartContinuation, int(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, const int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams));
    MOCK_METHOD2(ContinueAbility, int(const std::string &deviceId, int32_t missionId));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string &deviceId, int32_t sessionId, bool isSuccess));
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
