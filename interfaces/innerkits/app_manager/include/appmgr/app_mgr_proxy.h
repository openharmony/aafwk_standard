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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_CLIENT_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_CLIENT_H

#include "iremote_proxy.h"
#include "want.h"

#include "app_mgr_interface.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrProxy : public IRemoteProxy<IAppMgr> {
public:
    explicit AppMgrProxy(const sptr<IRemoteObject> &impl);
    virtual ~AppMgrProxy() = default;

    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &obj) override;

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) override;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) override;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) override;

    /**
     * CheckPermission, call CheckPermission() through proxy object, check the permission.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @param permission, check the permissions.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t CheckPermission(const int32_t recordId, const std::string &permission) override;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) override;

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() override;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName) override;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override;

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override;

    /**
     * Get system memory information.
     * @param SystemMemoryAttr, memory information.
     */
    virtual void GetSystemMemoryAttr(SystemMemoryAttr &memoryInfo, std::string &strConfig) override;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) override;

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override;

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) override;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const BundleInfo &bundleInfo, int32_t userId) override;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;

    virtual void ScheduleAcceptWantDone(
        const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override;

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns true on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override;

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
        int32_t sharedFd, pid_t &renderPid) override;

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderApp, information needed to start the Application.
     */
    virtual void AttachRenderProcess(const sptr<IRemoteObject> &renderApp) override;

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override;

    /**
     * Post a task to the not response process.
     *
     * @param pid, the not response process id.
     */
    virtual void PostANRTaskByProcessID(const pid_t pid) override;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() override;
    #endif

private:
    bool SendTransactCmd(IAppMgr::Message code, MessageParcel &data, MessageParcel &reply);
    bool WriteInterfaceToken(MessageParcel &data);
    template<typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    static inline BrokerDelegator<AppMgrProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_APP_MGR_CLIENT_H
