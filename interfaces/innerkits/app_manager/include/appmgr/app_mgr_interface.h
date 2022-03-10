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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_INTERFACE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_INTERFACE_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

#include "ability_info.h"
#include "application_info.h"
#include "app_record_id.h"
#include "bundle_info.h"
#include "iapp_state_callback.h"
#include "ams_mgr_interface.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "iapplication_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class IAppMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppMgr");

    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &app) = 0;

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) = 0;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) = 0;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) = 0;

    /**
     * CheckPermission, call CheckPermission() through proxy object, check the permission.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @param permission, check the permissions.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int CheckPermission(const int32_t recordId, const std::string &permission) = 0;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) = 0;

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() = 0;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName) = 0;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) = 0;

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) = 0;

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig) = 0;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) = 0;

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) = 0;

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) = 0;

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) = 0;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) = 0;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const BundleInfo &bundleInfo) = 0;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     * @param pid the user test process id.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(
        const std::string &msg, const int &resultCode, const std::string &bundleName, const pid_t &pid) = 0;

    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) = 0;

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) = 0;

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
        int32_t sharedFd, pid_t &renderPid) = 0;

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) = 0;

    /**
     * Post a task to the not response process.
     *
     * @param pid, the not response process id.
     */
    virtual void PostANRTaskByProcessID(const pid_t pid) = 0;

    enum class Message {
        APP_ATTACH_APPLICATION = 0,
        APP_APPLICATION_FOREGROUNDED,
        APP_APPLICATION_BACKGROUNDED,
        APP_APPLICATION_TERMINATED,
        APP_CHECK_PERMISSION,
        APP_ABILITY_CLEANED,
        APP_GET_MGR_INSTANCE,
        APP_CLEAR_UP_APPLICATION_DATA,
        APP_GET_ALL_RUNNING_PROCESSES,
        APP_GET_RUNNING_PROCESSES_BY_USER_ID,
        APP_GET_SYSTEM_MEMORY_ATTR,
        APP_ADD_ABILITY_STAGE_INFO_DONE,
        STARTUP_RESIDENT_PROCESS,
        REGISTER_APPLICATION_STATE_OBSERVER,
        UNREGISTER_APPLICATION_STATE_OBSERVER,
        GET_FOREGROUND_APPLICATIONS,
        START_USER_TEST_PROCESS,
        FINISH_USER_TEST,
        SCHEDULE_ACCEPT_WANT_DONE,
        APP_GET_ABILITY_RECORDS_BY_PROCESS_ID,
        START_RENDER_PROCESS,
        ATTACH_RENDER_PROCESS,
        POST_ANR_TASK_BY_PID,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_INTERFACE_H
