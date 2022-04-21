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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H

#include "iremote_object.h"
#include "refbase.h"
#include "want.h"

#include "ability_info.h"
#include "application_info.h"
#include "app_mgr_constants.h"
#include "bundle_info.h"
#include "iapp_state_callback.h"
#include "irender_scheduler.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "istart_specified_ability_response.h"

namespace OHOS {
namespace AppExecFwk {
class AppServiceManager;
class Configuration;
class AppMgrRemoteHolder;
class AppMgrClient {
public:
    AppMgrClient();
    virtual ~AppMgrClient();

    virtual AppMgrResultCode LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
        const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo, const AAFwk::Want &want);

    /**
     * Terminate ability.
     *
     * @param token Ability identify.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * Update ability state.
     *
     * @param token Ability identify.
     * @param state Ability running state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state);

    /**
     * UpdateExtensionState, call UpdateExtensionState() through the proxy object, update the extension status.
     *
     * @param token, the unique identification to update the extension.
     * @param state, extension status that needs to be updated.
     * @return
     */
    virtual AppMgrResultCode UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state);

    /**
     * Register Application state callback.
     *
     * @param callback IAppStateCallback
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback> &callback);

    /**
     * Connect service.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

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
    virtual AppMgrResultCode AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
        const int32_t connectionState);

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by user id.
     *
     * @param userId, the user id.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessesByUserId(int32_t userId);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplication(const std::string &bundleName);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @param  uid, uid.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplicationByUid(const std::string &bundleName, const int uid);

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string &bundleName);

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo> &info);

     /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId);

    /**
     * GetConfiguration
     *
     * @param info, configuration.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetConfiguration(Configuration& config);

    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject> &token);

    virtual void PrepareTerminate(const sptr<IRemoteObject> &token);

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     * @param strConfig, params string.
     */
    virtual void GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig);

    virtual void GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info);
    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId);

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);

     /**
     *  ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateConfiguration(const Configuration &config);

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService();
    #endif

    /**
     * Start a user test
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId);

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName);

    virtual void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo);

    virtual void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response);

    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag);

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens);

    /**
     * Start nweb render process, called by nweb host.
     *
     * @param renderParam, params passed to renderprocess.
     * @param ipcFd, ipc file descriptior for web browser and render process.
     * @param sharedFd, shared memory file descriptior.
     * @param renderPid, created render pid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
        int32_t sharedFd, pid_t &renderPid);

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler);

    /**
     * Post a task to the not response process.
     *
     * @param pid, the not response process id.
     */
    virtual void PostANRTaskByProcessID(const pid_t pid);

private:
    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr);
    /**
     * This function is implemented for the Unittests only. 
     *
     * @return Returns private variable remote_ of the inner class aka "AppMgrRemoteHolder".  
     */
    sptr<IRemoteObject> GetRemoteObject();

private:
    std::shared_ptr<AppMgrRemoteHolder> mgrHolder_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APP_MGR_CLIENT_H
