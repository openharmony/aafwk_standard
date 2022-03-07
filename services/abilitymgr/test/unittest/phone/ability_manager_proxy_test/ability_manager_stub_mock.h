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
#include "ability_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerStubMock : public IRemoteStub<IAbilityManager> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAbilityManagerMock");

    AbilityManagerStubMock() : code_(0)
    {}
    virtual ~AbilityManagerStubMock()
    {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;

        if (GET_RECENT_MISSION == code) {
            std::vector<AbilityMissionInfo> missionInfos;
            AbilityMissionInfo info;
            info.id = 1;
            const Want want;
            info.baseWant = want;
            AppExecFwk::ElementName baseEle("baseDevice", "baseBundle", "baseAbility");
            info.baseAbility = baseEle;
            AppExecFwk::ElementName topEle("topDevice", "topBundle", "topAbility");
            info.topAbility = topEle;
            missionInfos.emplace_back(info);

            reply.WriteInt32(missionInfos.size());
            for (auto &it : missionInfos) {
                if (!reply.WriteParcelable(&it)) {
                    return ERR_INVALID_VALUE;
                }
            }

            int res = 1;
            if (!reply.WriteInt32(res)) {
                return ERR_INVALID_VALUE;
            }
        }

        return 0;
    }

    int InvokeErrorSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return UNKNOWN_ERROR;
    }

    int code_ = 0;

    virtual int StartAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = -1)
    {
        return 0;
    }

    virtual int StartAbility(const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr)
    {
        return 0;
    }
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override
    {
        return 0;
    }
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser) override
    {
        return 0;
    }

    virtual int ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }

    virtual int ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect)
    {
        return 0;
    }

    virtual void AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
    {}

    virtual int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
    {
        return 0;
    }

    virtual int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
    {
        return 0;
    }

    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    virtual void DumpState(const std::string &args, std::vector<std::string> &state)
    {}

    virtual void DumpSysState(
            const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID)
    {}

    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
    {
        return 0;
    }

    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int GetAllStackInfo(StackInfo &stackInfo)
    {
        return 0;
    }

    virtual int GetRecentMissions(
        const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
    {
        return 0;
    }

    int GetMissionSnapshot(const int32_t missionId, MissionPixelMap &missionPixelMap)
    {
        return 0;
    }

    virtual int RemoveMission(int id)
    {
        return 0;
    }

    virtual int RemoveStack(int id)
    {
        return 0;
    }

    virtual int KillProcess(const std::string &bundleName)
    {
        return 0;
    }

    virtual int UninstallApp(const std::string &bundleName)
    {
        return 0;
    }

    virtual int MoveMissionToTop(int32_t missionId)
    {
        return 0;
    }

    bool IsFirstInMission(const sptr<IRemoteObject> &token) override
    {
        return true;
    }

    int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message) override
    {
        return 0;
    }

    int MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst) override
    {
        return 0;
    }

    int PowerOff() override
    {
        return 0;
    }

    int PowerOn() override
    {
        return 0;
    }

    int LockMission(int missionId) override
    {
        return 0;
    }
    int UnlockMission(int missionId) override
    {
        return 0;
    }

    int SetMissionDescriptionInfo(
        const sptr<IRemoteObject> &token, const MissionDescriptionInfo &missionDescriptionInfo) override
    {
        return 0;
    }

    int GetMissionLockModeState()
    {
        return 0;
    }

    int MoveMissionToFloatingStack(const MissionOption &missionOption)
    {
        return 0;
    }

    int UpdateConfiguration(const AppExecFwk::Configuration &config)

    {
        return 0;
    }

    int MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary)
    {
        return 0;
    }

    int ChangeFocusAbility(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
    {
        return 0;
    }

    int MinimizeMultiWindow(int missionId)
    {
        return 0;
    }

    int MaximizeMultiWindow(int missionId)
    {
        return 0;
    }

    int GetFloatingMissions(std::vector<AbilityMissionInfo> &list)
    {
        return 0;
    }

    int CloseMultiWindow(int missionId)
    {
        return 0;
    }

    int SetMissionStackSetting(const StackSetting &stackSetting)
    {
        return 0;
    }

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
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    virtual int ReleaseAbility(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element) override
    {
        return 0;
    }
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId, MissionSnapshot& snapshot)
    {
        return 0;
    }
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
    {
        return 0;
    }

    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }

    virtual bool IsRunningInStabilityTest() override
    {
        return true;
    }

    virtual int SendANRProcessID(int pid) override
    {
        return 0;
    }

    MOCK_METHOD2(TerminateAbilityByCaller, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD4(StartAbility, int(const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, int requestCode));
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
    MOCK_METHOD1(GetSystemMemoryAttr, void(AppExecFwk::SystemMemoryAttr &memoryInfo));
    MOCK_METHOD3(StartContinuation, int(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams));
    MOCK_METHOD2(ContinueAbility, int(const std::string &deviceId, int32_t missionId));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string &deviceId, int32_t sessionId, bool isSuccess));

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
    MOCK_METHOD2(MoveMissionToFront, int(int32_t missionId, const StartOptions &startOptions));
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject> &token, const std::string &label));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo> &info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo> &info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo> &info));
    MOCK_METHOD3(StartAbilityByCall, int(const Want &, const sptr<IAbilityConnection> &, const sptr<IRemoteObject> &));

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override
    {
        return 0;
    }

    virtual int FinishUserTest(const std::string &msg, const int &resultCode, const std::string &bundleName) override
    {
        return 0;
    }

    virtual int GetCurrentTopAbility(sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }

    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }

    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
