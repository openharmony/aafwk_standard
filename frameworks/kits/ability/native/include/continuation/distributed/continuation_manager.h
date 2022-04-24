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

#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_MANAGER_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_MANAGER_H

#include <memory>
#include <mutex>
#include "continuation_state.h"
#include "ability_info.h"
#include "event_handler.h"
#include "iremote_object.h"
#include "want.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
class Ability;
class ContinuationHandler;
class IAbilityContinuation;
class ContinuationManager : public std::enable_shared_from_this<ContinuationManager> {
public:
    ContinuationManager();
    virtual ~ContinuationManager() = default;

    bool Init(const std::shared_ptr<Ability> &ability, const sptr<IRemoteObject> &continueToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ContinuationHandler> &continuationHandler);

    ContinuationState GetContinuationState();

    std::string GetOriginalDeviceId();

    void ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode);

    void ContinueAbility(bool reversible, const std::string &deviceId);

    bool ReverseContinueAbility();

    bool StartContinuation();

    int32_t OnContinue(WantParams &wantParams);

    int32_t OnStartAndSaveData(WantParams &wantParams);

    int32_t OnContinueAndGetContent(WantParams &wantParams);

    bool SaveData(WantParams &saveData);

    bool RestoreData(const WantParams &restoreData, bool reversible, const std::string &originalDeviceId);

    void NotifyCompleteContinuation(
        const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler);

    void CompleteContinuation(int result);

    bool RestoreFromRemote(const WantParams &restoreData);

    bool NotifyRemoteTerminated();

    void ChangeProcessStateToInit();

    enum OnContinueResult {
        AGREE = 0,
        Reject = 1,
        MISMATCH = 2
    };
private:
    enum ProgressState { INITIAL, WAITING_SCHEDULE, IN_PROGRESS };

    bool CheckContinuationIllegal();

    bool HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode);

    bool HandleContinueAbility(bool reversible, const std::string &deviceId);

    ProgressState GetProcessState();

    void ChangeProcessState(const ProgressState &newState);

    void RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState);

    void InitMainHandlerIfNeed();

    bool CheckAbilityToken();

    void CheckDmsInterfaceResult(int result, const std::string &interfaceName);

    bool DoScheduleStartContinuation();

    bool DoScheduleSaveData(WantParams &saveData);

    bool DoScheduleRestoreData(const WantParams &restoreData);

    bool DoRestoreFromRemote(const WantParams &restoreData);
#ifdef SUPPORT_GRAPHICS
    bool GetContentInfo(WantParams &wantParams);
#endif
    sptr<IRemoteObject> continueToken_ = nullptr;
    std::weak_ptr<Ability> ability_;
    std::weak_ptr<AbilityInfo> abilityInfo_;
    ProgressState progressState_ = ProgressState::INITIAL;
    bool reversible_ = false;
    ContinuationState continuationState_ = ContinuationState::LOCAL_RUNNING;
    std::string originalDeviceId_;
    std::weak_ptr<ContinuationHandler> continuationHandler_;
    std::shared_ptr<EventHandler> mainHandler_ = nullptr;
    std::mutex lock_;

    static const int TIMEOUT_MS_WAIT_DMS_SCHEDULE_START_CONTINUATION;
    static const int TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE;
    static const int TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_MANAGER_H