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

#ifndef OHOS_AAFWK_APP_SCHEDULER_H
#define OHOS_AAFWK_APP_SCHEDULER_H

#include <memory>
#include <unordered_set>

#include "ability_info.h"
// #include "app_process_data.h"
#include "appmgr/app_mgr_client.h"
#include "appmgr/app_state_callback_host.h"
#include "application_info.h"
#include "iremote_object.h"
#include "refbase.h"
#include "singleton.h"
#include "system_memory_attr.h"

namespace OHOS {
namespace AAFwk {
/**
 * @enum AppAbilityState
 * AppAbilityState defines the life cycle state of app ability.
 */
enum class AppAbilityState {
    ABILITY_STATE_UNDEFINED = 0,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_END,
};

enum class AppState {
    BEGIN = 0,
    READY,
    FOREGROUND,
    BACKGROUND,
    SUSPENDED,
    TERMINATED,
    END,
};

struct AppInfo {
    std::string appName;
    std::string processName;
    int32_t uid;
    AppState state;
};
/**
 * @class AppStateCallback
 * AppStateCallback.
 */
class AppStateCallback {
public:
    AppStateCallback()
    {}
    virtual ~AppStateCallback()
    {}

    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) = 0;

    virtual void OnAppStateChanged(const AppInfo &info) = 0;
};

/**
 * @class AppScheduler
 * AppScheduler , access app manager service.
 */
class AppScheduler : virtual RefBase, public AppExecFwk::AppStateCallbackHost {

    DECLARE_DELAYED_SINGLETON(AppScheduler)
public:
    /**
     * init app scheduler.
     * @param callback, app state call back.
     * @return true on success ,false on failure.
     */
    bool Init(const std::weak_ptr<AppStateCallback> &callback);

    /**
     * load ability with token, ability info and application info.
     *
     * @param token, the token of ability.
     * @param preToken, the token of ability's caller.
     * @param abilityinfo, ability info.
     * @param application, application info.
     * @return true on success ,false on failure.
     */
    int LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const AppExecFwk::AbilityInfo &abilityinfo, const AppExecFwk::ApplicationInfo &application);

    /**
     * terminate ability with token.
     *
     * @param token, the token of ability.
     * @return true on success ,false on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * move ability to forground.
     *
     * @param token, the token of ability.
     */
    void MoveToForground(const sptr<IRemoteObject> &token);

    /**
     * move ability to background.
     *
     * @param token, the token of ability.
     */
    void MoveToBackground(const sptr<IRemoteObject> &token);

    /**
     * Update ability state.
     *
     * @param token, the token of ability.
     * @param state, ability state.
     */
    void UpdateAbilityState(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    void UpdateExtensionState(const sptr<IRemoteObject> &token, const AppExecFwk::ExtensionState state);

    /**
     * AbilityBehaviorAnalysis, ability behavior analysis assistant process optimization.
     *
     * @param token, the unique identification to start the ability.
     * @param preToken, the unique identification to call the ability.
     * @param visibility, the visibility information about windows info.
     * @param perceptibility, the Perceptibility information about windows info.
     * @param connectionState, the service ability connection state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    void AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const int32_t visibility, const int32_t perceptibility, const int32_t connectionState);

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     */
    void KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the process by user id.
     *
     * @param userId, the user id.
     */
    void KillProcessesByUserId(int32_t userId);

    /**
     * convert ability state to app ability state.
     *
     * @param state, the state of ability.
     */
    AppAbilityState ConvertToAppAbilityState(const int32_t state);

    /**
     * get ability state.
     *
     * @return state, the state of app ability.
     */
    AppAbilityState GetAbilityState() const;

    /**
     * kill the application
     *
     * @param bundleName.
     */
    int KillApplication(const std::string &bundleName);

    /**
     * clear the application data
     *
     * @param bundleName.
     */
    int ClearUpApplicationData(const std::string &bundleName);

    void AttachTimeOut(const sptr<IRemoteObject> &token);

    void PrepareTerminate(const sptr<IRemoteObject> &token);

    /**
     * Checks whether a specified permission has been granted to the process identified by pid and uid
     *
     * @param permission Indicates the permission to check.
     * @param pid Indicates the ID of the process to check.
     * @param uid Indicates the UID of the process to check.
     * @param message Describe success or failure
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message);

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    void GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo, std::string &strConfig);

    /**
     * Start a resident process
     */
    void StartupResidentProcess();

protected:
    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AppExecFwk::AbilityState state) override;

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual void OnAppStateChanged(const AppExecFwk::AppProcessData &appData) override;

private:
    std::recursive_mutex lock_;
    bool isInit_  {false};
    std::weak_ptr<AppStateCallback> callback_;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    AppAbilityState appAbilityState_ = AppAbilityState::ABILITY_STATE_UNDEFINED;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_APP_SCHEDULER_H
