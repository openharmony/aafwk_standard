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

#ifndef OHOS_AAFWK_ABILITY_RECORD_H
#define OHOS_AAFWK_ABILITY_RECORD_H

#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "ability_connect_callback_interface.h"
#include "ability_info.h"
#include "ability_start_setting.h"
#include "ability_token_stub.h"
#include "ability_window_configuration.h"
#include "app_scheduler.h"
#include "application_info.h"
#include "ability_record_info.h"
#include "call_container.h"
#include "lifecycle_deal.h"
#include "lifecycle_state_info.h"
#include "want.h"
#include "window_info.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
using Closure = std::function<void()>;

class AbilityRecord;
class MissionRecord;
class ConnectionRecord;
class Mission;
class MissionList;
class CallContainer;

const std::string ABILITY_TOKEN_NAME = "AbilityToken";
const std::string LINE_SEPARATOR = "\n";
constexpr int32_t API_VERSION_7 = 7;

/**
 * @class Token
 * Token is identification of ability and used to interact with kit and wms.
 */
class Token : public AbilityTokenStub {
public:
    explicit Token(std::weak_ptr<AbilityRecord> abilityRecord);
    virtual ~Token();

    std::shared_ptr<AbilityRecord> GetAbilityRecord() const;
    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token);

private:
    std::weak_ptr<AbilityRecord> abilityRecord_;  // ability of this token
};

/**
 * @class AbilityResult
 * Record requestCode of for-result start mode and result.
 */
class AbilityResult {
public:
    AbilityResult() = default;
    AbilityResult(int requestCode, int resultCode, const Want &resultWant)
        : requestCode_(requestCode), resultCode_(resultCode), resultWant_(resultWant)
    {}
    virtual ~AbilityResult()
    {}

    int requestCode_ = -1;  // requestCode of for-result start mode
    int resultCode_ = -1;   // resultCode of for-result start mode
    Want resultWant_;       // for-result start mode ability will send the result to caller
};

/**
 * @class CallerRecord
 * Record caller ability of for-result start mode and result.
 */
class CallerRecord {
public:
    CallerRecord() = default;
    CallerRecord(int requestCode, std::weak_ptr<AbilityRecord> caller) : requestCode_(requestCode), caller_(caller)
    {}
    virtual ~CallerRecord()
    {}

    int GetRequestCode()
    {
        return requestCode_;
    }
    std::shared_ptr<AbilityRecord> GetCaller()
    {
        return caller_.lock();
    }

private:
    int requestCode_ = -1;  // requestCode of for-result start mode
    std::weak_ptr<AbilityRecord> caller_;
};

/**
 * @class AbilityRequest
 * Wrap parameters of starting ability.
 */
enum AbilityCallType {
    INVALID_TYPE = 0,
    CALL_REQUEST_TYPE,
};
struct AbilityRequest {
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo appInfo;
    int32_t uid = 0;
    int requestCode = -1;
    bool restart = false;

    // call ability
    int callerUid = -1;
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = nullptr;

    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    int32_t compatibleVersion = 0;
    std::string specifiedFlag;

    bool IsNewVersion() const
    {
        return compatibleVersion > API_VERSION_7;
    }

    bool IsContinuation() const
    {
        auto flags = want.GetFlags();
        if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
            return true;
        }
        return false;
    }

    bool IsCallType(const AbilityCallType & type) const
    {
        return (callType == type);
    }

    void Dump(std::vector<std::string> &state)
    {
        std::string dumpInfo = "      want [" + want.ToUri() + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      app name [" + abilityInfo.applicationName + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      main name [" + abilityInfo.name + "]";
        state.push_back(dumpInfo);
        dumpInfo = "      request code [" + std::to_string(requestCode) + "]";
        state.push_back(dumpInfo);
    }
};

// new version
enum ResolveResultType {
    OK_NO_REMOTE_OBJ = 0,
    OK_HAS_REMOTE_OBJ,
    NG_INNER_ERROR,
};
/**
 * @class AbilityRecord
 * AbilityRecord records ability info and states and used to schedule ability life.
 */
class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode = -1, int32_t apiVersion = 1);

    virtual ~AbilityRecord();

    /**
     * CreateAbilityRecord.
     *
     * @param abilityRequest,create ability record.
     * @return Returns ability record ptr.
     */
    static std::shared_ptr<AbilityRecord> CreateAbilityRecord(const AbilityRequest &abilityRequest);

    /**
     * Init ability record.
     *
     * @return Returns true on success, others on failure.
     */
    bool Init();

    /**
     * load ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int LoadAbility();

    /**
     * foreground the ability.
     *
     */
    void ForegroundAbility();

    /**
     * process request of foregrounding the ability.
     *
     */
    void ProcessForegroundAbility();

    /**
     * move the ability to back ground.
     *
     * @param task timeout task.
     */
    void BackgroundAbility(const Closure &task);

    /**
     * terminate ability.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility();

    /**
     * set ability's mission record.
     *
     * @param missionRecord, mission record.
     */
    void SetMissionRecord(const std::shared_ptr<MissionRecord> &missionRecord);

    /**
     * set ability's mission stack.
     *
     * @param missionStack, mission stack.
     */
    void SetMissionStackId(const int stackId);

    /**
     * Get ability's mission stack id.
     */
    int GetMissionStackId() const;

    /**
     * get ability's mission record.
     *
     * @return missionRecord, mission record.
     */
    std::shared_ptr<MissionRecord> GetMissionRecord() const;

    int GetMissionRecordId() const;

    /**
     * get ability's info.
     *
     * @return ability info.
     */
    const AppExecFwk::AbilityInfo &GetAbilityInfo() const;

    /**
     * get application's info.
     *
     * @return application info.
     */
    const AppExecFwk::ApplicationInfo &GetApplicationInfo() const;

    /**
     * set ability's state.
     *
     * @param state, ability's state.
     */
    void SetAbilityState(AbilityState state);

    /**
     * get ability's state.
     *
     * @return ability state.
     */
    AbilityState GetAbilityState() const;

    /**
     * set ability scheduler for accessing ability thread.
     *
     * @param scheduler , ability scheduler.
     */
    void SetScheduler(const sptr<IAbilityScheduler> &scheduler);

    /**
     * get ability's token.
     *
     * @return ability's token.
     */
    sptr<Token> GetToken() const;

    /**
     * set ability's previous ability record.
     *
     * @param abilityRecord , previous ability record
     */
    void SetPreAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * get ability's previous ability record.
     *
     * @return previous ability record
     */
    std::shared_ptr<AbilityRecord> GetPreAbilityRecord() const;

    /**
     * set ability's next ability record.
     *
     * @param abilityRecord , next ability record
     */
    void SetNextAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * get ability's previous ability record.
     *
     * @return previous ability record
     */
    std::shared_ptr<AbilityRecord> GetNextAbilityRecord() const;

    /**
     * set ability's back ability record.
     *
     * @param abilityRecord , back ability record
     */
    void SetBackAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * get ability's back ability record.
     *
     * @return back ability record
     */
    std::shared_ptr<AbilityRecord> GetBackAbilityRecord() const;

    /**
     * set event id.
     *
     * @param eventId
     */
    void SetEventId(int64_t eventId);

    /**
     * get event id.
     *
     * @return eventId
     */
    int64_t GetEventId() const;

    /**
     * check whether the ability is ready.
     *
     * @return true : ready ,false: not ready
     */
    bool IsReady() const;

    /**
     * check whether the ability 's window is attached.
     *
     * @return true : attached ,false: not attached
     */
    bool IsWindowAttached() const;

    /**
     * check whether the ability is launcher.
     *
     * @return true : lanucher ,false: not lanucher
     */
    bool IsLauncherAbility() const;

    /**
     * check whether the ability is terminating.
     *
     * @return true : yes ,false: not
     */
    bool IsTerminating() const;

    /**
     * check whether the ability force to terminate.
     *
     * @return true : yes ,false: not
     */
    bool IsForceTerminate() const;
    void SetForceTerminate(bool flag);

    /**
     * set the ability is terminating.
     *
     */
    void SetTerminatingState();

    /**
     * set the ability is new want flag.
     *
     * @return isNewWant
     */
    void SetIsNewWant(bool isNewWant);

    /**
     * check whether the ability is new want flag.
     *
     * @return true : yes ,false: not
     */
    bool IsNewWant() const;

    /**
     * check whether the ability is created by connect ability mode.
     *
     * @return true : yes ,false: not
     */
    bool IsCreateByConnect() const;

    /**
     * set the ability is created by connect ability mode.
     *
     */
    void SetCreateByConnectMode();

    /**
     * active the ability.
     *
     */
    virtual void Activate();

    /**
     * process request of activing the ability.
     *
     */
    void ProcessActivate();

    /**
     * process request of activing the ability in moving.
     *
     */
    void ProcessActivateInMoving();

    /**
     * process request of inactiving the ability.
     *
     */
    void ProcessInactivate();

    /**
     * process request of inactiving the ability in moving.
     *
     */
    void ProcessInactivateInMoving();

    /**
     * inactive the ability.
     *
     */
    virtual void Inactivate();

    /**
     * move the ability to back ground.
     *
     */
    virtual void MoveToBackground(const Closure &task);

    /**
     * terminate the ability.
     *
     */
    void Terminate(const Closure &task);

    /**
     * connect the ability.
     *
     */
    void ConnectAbility();

    /**
     * disconnect the ability.
     *
     */
    void DisconnectAbility();

    /**
     * Command the ability.
     *
     */
    void CommandAbility();

    /**
     * save ability state.
     *
     */
    void SaveAbilityState();
    void SaveAbilityState(const PacMap &inState);

    /**
     * restore ability state.
     *
     */
    void RestoreAbilityState();

    /**
     * notify top active ability updated.
     *
     */
    void TopActiveAbilityChanged(bool flag);

    /**
     * set the want for start ability.
     *
     */
    void SetWant(const Want &want);

    /**
     * get the want for start ability.
     *
     */
    const Want &GetWant() const;

    /**
     * get request code of the ability to start.
     *
     */
    int GetRequestCode() const;

    /**
     * set the result object of the ability which one need to be terminated.
     *
     */
    void SetResult(const std::shared_ptr<AbilityResult> &result);

    /**
     * get the result object of the ability which one need to be terminated.
     *
     */
    std::shared_ptr<AbilityResult> GetResult() const;

    /**
     * send result object to caller ability thread.
     *
     */
    void SendResult();

    /**
     * send result object to caller ability.
     *
     */
    void SendResultToCallers();

    /**
     * save result object to caller ability.
     *
     */
    void SaveResultToCallers(const int resultCode, const Want *resultWant);

    /**
     * add connect record to the list.
     *
     */
    void AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord);

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordList() const;

    /**
     * get the list of connect record.
     *
     */
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectingRecordList();

    /**
     * remove the connect record from list.
     *
     */
    void RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord);

    /**
     * check whether connect list is empty.
     *
     */
    bool IsConnectListEmpty();

    /**
     * add ability's window info to record.
     *
     */
    void AddWindowInfo(int windowToken);

    /**
     * remove ability's window info from record.
     *
     */
    void RemoveWindowInfo();

    /**
     * get ability's window info from record.
     *
     */
    std::shared_ptr<WindowInfo> GetWindowInfo() const;

    /**
     * add caller record
     *
     */
    void AddCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode);

    /**
     * get caller record to list.
     *
     */
    std::list<std::shared_ptr<CallerRecord>> GetCallerRecordList() const;

    /**
     * get connecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetConnectingRecord() const;

    /**
     * get disconnecting record from list.
     *
     */
    std::shared_ptr<ConnectionRecord> GetDisconnectingRecord() const;

    /**
     * convert ability state (enum type to string type).
     *
     */
    static std::string ConvertAbilityState(const AbilityState &state);

    static std::string ConvertAppState(const AppState &state);

    /**
     * convert life cycle state to ability state .
     *
     */
    static int ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state);

    /**
     * get the ability record id.
     *
     */
    inline int GetRecordId() const
    {
        return recordId_;
    }

    /**
     * dump ability info.
     *
     */
    void Dump(std::vector<std::string> &info);

    /**
     * dump ability state info.
     *
     */
    void DumpAbilityState(std::vector<std::string> &info, bool isClient);

    void SetStartTime();

    int64_t GetStartTime() const;

    /**
     * dump service info.
     *
     */
    void DumpService(std::vector<std::string> &info, bool isClient = false) const;

    /**
     * get ability record info.
     *
     */
    void GetAbilityRecordInfo(AbilityRecordInfo &recordInfo);

    /**
     * set aconnect remote object.
     *
     */
    void SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject);

    /**
     * get connect remote object.
     *
     */
    sptr<IRemoteObject> GetConnRemoteObject() const;

    void AddStartId();
    int GetStartId() const;

    void SetIsUninstallAbility();
    /**
     * Determine whether ability is uninstalled
     *
     * @return true: uninstalled false: installed
     */
    bool IsUninstallAbility() const;

    void SetKernalSystemAbility();
    bool IsKernalSystemAbility() const;

    void SetLauncherRoot();
    bool IsLauncherRoot() const;

    bool IsAbilityState(const AbilityState &state) const;
    bool IsActiveState() const;

    bool SupportMultWindow() const;
    void NotifyMultiWinModeChanged(const AbilityWindowConfiguration &winModeKey, bool flag);
    void SetInMovingState(bool isMoving);
    bool GetInMovingState() const;

    bool IsToEnd() const;
    void SetToEnd(bool isToEnd);

    void SetStartSetting(const std::shared_ptr<AbilityStartSetting> &setting);
    std::shared_ptr<AbilityStartSetting> GetStartSetting() const;

    void SetPowerState(const bool isPower);
    bool GetPowerState() const;

    void SetRestarting(const bool isRestart);
    bool IsRestarting() const;
    void SetAppState(const AppState &state);
    AppState GetAppState() const;

    void ClearFlag();

    void SetLockScreenState(const bool isLock);
    bool GetLockScreenState() const;
    void SetMovingBackgroundFlag(bool isMoving);
    bool IsMovingBackground() const;

    void SetLockScreenRoot();
    bool IsLockScreenRoot() const;
    void SetPowerStateLockScreen(const bool isPower);
    bool GetPowerStateLockScreen() const;

    bool IsNewVersion();
    void SetLaunchReason(const LaunchReason &reason);
    void SetLastExitReason(const LastExitReason &reason);
    void ContinueAbility(const std::string& deviceId);
    void NotifyContinuationResult(const int32_t result);
    std::shared_ptr<MissionList> GetOwnedMissionList() const;

    void SetUseNewMission();
    void SetMission(const std::shared_ptr<Mission> &mission);
    void SetMissionList(const std::shared_ptr<MissionList> &missionList);
    std::shared_ptr<Mission> GetMission() const;
    int32_t GetMissionId() const;

    void SetUid(int32_t uid);
    int32_t GetUid();
    void SetSwitchingPause(bool state);
    bool IsSwitchingPause();

    // new version
    ResolveResultType Resolve(const AbilityRequest &abilityRequest);
    bool Release(const sptr<IAbilityConnection> & connect);
    bool IsNeedToCallRequest() const;
    bool IsStartedByCall() const;
    void SetStartedByCall(const bool isFlag);
    bool CallRequest();
    bool IsStartToBackground() const;
    void SetStartToBackground(const bool flag);

    void SetSpecifiedFlag(const std::string &flag);
    std::string GetSpecifiedFlag() const;

protected:
    void SendEvent(uint32_t msg, uint32_t timeOut);

    sptr<Token> token_ = {};                               // used to interact with kit and wms
    std::unique_ptr<LifecycleDeal> lifecycleDeal_ = {};    // life manager used to schedule life
    AbilityState currentState_ = AbilityState::INITIAL;    // current life state
    Want want_ = {};                                       // want to start this ability
    LifeCycleStateInfo lifeCycleStateInfo_;                // target life state info
    static int64_t g_abilityRecordEventId_;
    int64_t eventId_ = 0;                                  // post event id

private:
    /**
     * get the type of ability.
     *
     */
    void GetAbilityTypeString(std::string &typeStr);
    void OnSchedulerDied(const wptr<IRemoteObject> &remote);

    static int64_t abilityRecordId;
    int recordId_ = 0;                                // record id
    AppExecFwk::AbilityInfo abilityInfo_ = {};             // the ability info get from BMS
    AppExecFwk::ApplicationInfo applicationInfo_ = {};     // the ability info get from BMS
    std::weak_ptr<MissionRecord> missionRecord_ = {};      // mission of this ability
    std::weak_ptr<AbilityRecord> preAbilityRecord_ = {};   // who starts this ability record
    std::weak_ptr<AbilityRecord> nextAbilityRecord_ = {};  // ability that started by this ability
    std::weak_ptr<AbilityRecord> backAbilityRecord_ = {};  // who back to this ability record
    int64_t startTime_ = 0;                           // records first time of ability start
    bool isReady_ = false;                            // is ability thread attached?
    bool isWindowAttached_ = false;                   // Is window of this ability attached?
    bool isLauncherAbility_ = false;                  // is launcher?

    sptr<IAbilityScheduler> scheduler_ = {};       // kit scheduler
    bool isTerminating_ = false;              // is terminating ?
    std::shared_ptr<WindowInfo> windowInfo_;  // add window info
    bool isCreateByConnect_ = false;          // is created by connect ability mode?
    bool isToEnd_ = false;                    // is to end ?

    int requestCode_ = -1;  // requestCode_: >= 0 for-result start mode; <0 for normal start mode in default.
    sptr<IRemoteObject::DeathRecipient> schedulerDeathRecipient_ = {};  // scheduler binderDied Recipient

    /**
     * result_: ability starts with for-result mode will send result before being terminated.
     * Its caller will receive results before active.
     * Now we assume only one result generate when terminate.
     */
    std::shared_ptr<AbilityResult> result_ = {};

    // service(ability) can be connected by multi-pages(abilites), so need to store this service's connections
    std::list<std::shared_ptr<ConnectionRecord>> connRecordList_ = {};
    // service(ability) onConnect() return proxy of service ability
    sptr<IRemoteObject> connRemoteObject_ = {};
    int startId_ = 0;  // service(ability) start id

    // page(ability) can be started by multi-pages(abilites), so need to store this ability's caller
    std::list<std::shared_ptr<CallerRecord>> callerList_ = {};

    bool isUninstall_ = false;
    bool isForceTerminate_ = false;
    const static std::map<AbilityState, std::string> stateToStrMap;
    const static std::map<AbilityLifeCycleState, AbilityState> convertStateMap;
    const static std::map<AppState, std::string> appStateToStrMap_;

    bool isKernalSystemAbility_ = false;
    bool isLauncherRoot_ = false;
    bool isPowerState_ = false;  // ability to change state when poweroff and poweron.
    bool isLockScreenState_ = false; // ability to change state when lockscreen.

    PacMap stateDatas_;             // ability saved ability state data
    bool isRestarting_ = false;     // is restarting ?
    bool isInMovingState_ = false;  // whether complete multi window moving state.
    bool isMovingBackground_ = false;
    bool isLockScreenRoot_ = false;
    bool isPowerStateLockScreen_ = false;
    AppState appState_ = AppState::BEGIN;

    int32_t compatibleVersion_ = 0; // > 7 new version, <= 7 old version.
    int32_t uid_ = 0;
    std::weak_ptr<MissionList> missionList_;
    std::weak_ptr<Mission> mission_;
    int32_t missionId_ = -1;
    bool isSwitchingPause_ = false;
	
	// new version
    std::shared_ptr<CallContainer> callContainer_ = nullptr;
    bool isStartedByCall_ = false;
    bool isStartToBackground_ = false;

    int32_t restartCount_ = -1;
    int32_t restratMax_ = -1;
    std::string specifiedFlag_;
};

class AbilityRecordNew : public AbilityRecord {
public:
    AbilityRecordNew(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode = -1, int32_t apiVersion = 1);

    ~AbilityRecordNew();

    void Activate() override;
    void Inactivate() override;
    void MoveToBackground(const Closure &task) override;

    void ForegroundNew();
    void BackgroundNew(const Closure &task);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_RECORD_H
