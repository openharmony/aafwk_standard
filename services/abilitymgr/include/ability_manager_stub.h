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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_STUB_H
#define OHOS_AAFWK_ABILITY_MANAGER_STUB_H

#include "ability_manager_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityManagerStub
 * AbilityManagerStub.
 */
class AbilityManagerStub : public IRemoteStub<IAbilityManager> {
public:
    AbilityManagerStub();
    ~AbilityManagerStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override;
private:
    void FirstStepInit();
    void SecondStepInit();
    void ThirdStepInit();
    int TerminateAbilityInner(MessageParcel &data, MessageParcel &reply);
    int TerminateAbilityByCallerInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply);
    int AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply);
    int AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int TerminateAbilityResultInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInner(MessageParcel &data, MessageParcel &reply);
    int AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int KillProcessInner(MessageParcel &data, MessageParcel &reply);
    int UninstallAppInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply);
    int ConnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DumpStateInner(MessageParcel &data, MessageParcel &reply);
    int DumpSysStateInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int UpdateConfigurationInner(MessageParcel &data, MessageParcel &reply);

    int GetWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int SendWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int CancelWantSenderInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply);

    int RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply);
    int GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply);

    int GetSystemMemoryAttrInner(MessageParcel &data, MessageParcel &reply);
    int GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply);
    int IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply);
    int ClearUpApplicationDataInner(MessageParcel &data, MessageParcel &reply);

    int ContinueMissionInner(MessageParcel &data, MessageParcel &reply);
    int ContinueAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);

    int LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfoInner(MessageParcel &data, MessageParcel &reply);
    int CleanMissionInner(MessageParcel &data, MessageParcel &reply);
    int CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply);

	// for new version ability (call ability)
    int StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseInner(MessageParcel &data, MessageParcel &reply);
    int StartUserInner(MessageParcel &data, MessageParcel &reply);
    int StopUserInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionLabelInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionIconInner(MessageParcel &data, MessageParcel &reply);
    int GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply);

    int StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply);
    int SendANRProcessIDInner(MessageParcel &data, MessageParcel &reply);

    int SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply);

    int StartUserTestInner(MessageParcel &data, MessageParcel &reply);
    int FinishUserTestInner(MessageParcel &data, MessageParcel &reply);
    int GetCurrentTopAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);

    int IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply);

    int ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply);

    using RequestFuncType = int (AbilityManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;

    int BlockAmsServiceInner(MessageParcel &data, MessageParcel &reply);
    int BlockAbilityInner(MessageParcel &data, MessageParcel &reply);
    int BlockAppServiceInner(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_STUB_H
