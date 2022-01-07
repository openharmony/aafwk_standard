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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_MOCK_ABILITY_MANAGER_STUB_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_MOCK_ABILITY_MANAGER_STUB_H

#include "gmock/gmock.h"

#include "string_ex.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string STRING_DEVICE = "device";
const std::string STRING_ABILITY_NAME = "ability";
const std::string STRING_ABILITY_NAME_INVALID = "invalid_ability";
const std::string STRING_BUNDLE_NAME = "bundle";
const std::string STRING_BUNDLE_NAME_INVALID = "invalid_bundle";
const std::string STRING_RECORD_ID = "1024";
const std::string STRING_RECORD_ID_INVALID = "2048";
const std::string STRING_STATE_ON = "on";
const std::string STRING_STATE_ON_INVALID = "invalid_on";
const std::string STRING_STATE_OFF = "off";
const std::string STRING_STATE_OFF_INVALID = "invalid_off";
}  // namespace

class MockAbilityManagerStub : public AbilityManagerStub {
public:
    int StartAbility(const Want &want, int requestCode = -1);

    MOCK_METHOD3(StartAbility, int(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant));
    MOCK_METHOD1(MinimizeAbility, int(const sptr<IRemoteObject> &token));
    MOCK_METHOD3(ConnectAbility,
        int(const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD1(DisconnectAbility, int(const sptr<IAbilityConnection> &connect));
    MOCK_METHOD3(AcquireDataAbility,
        sptr<IAbilityScheduler>(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(
        ReleaseDataAbility, int(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject> &token, int32_t windowToken));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject> &token, int state, const PacMap &));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject> &token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject> &token));

    void DumpState(const std::string &args, std::vector<std::string> &state);

    MOCK_METHOD2(TerminateAbilityResult, int(const sptr<IRemoteObject> &token, int startId));

    int StopServiceAbility(const Want &want);

    MOCK_METHOD2(TerminateAbilityByCaller, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD1(GetAllStackInfo, int(StackInfo &stackInfo));
    MOCK_METHOD3(
        GetRecentMissions, int(const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList));
    MOCK_METHOD2(GetMissionSnapshot, int(const int32_t missionId, MissionPixelMap &missionPixelMap));
    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(RemoveMission, int(int id));
    MOCK_METHOD1(RemoveStack, int(int id));
    MOCK_METHOD1(KillProcess, int(const std::string &bundleName));
    MOCK_METHOD1(UninstallApp, int(const std::string &bundleName));

    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject> &token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject> &token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string &permission, int pid, int uid, std::string &message));
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
    int PowerOff();
    int PowerOn();
    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD2(SetMissionDescriptionInfo, int(const sptr<IRemoteObject> &token, const MissionDescriptionInfo &info));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD1(UpdateConfiguration, int(const AppExecFwk::Configuration &));
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
    MOCK_METHOD1(SetShowOnLockScreen, int(bool isAllow));
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
    MOCK_METHOD1(ClearUpApplicationData, int(const std::string &));

    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
public:
    std::string powerState_;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_MOCK_ABILITY_MANAGER_STUB_H
