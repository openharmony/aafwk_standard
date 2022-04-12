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

#include "form_mgr_service.h"

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>

#include "appexecfwk_errors.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_info_mgr.h"
#include "form_mgr_adapter.h"
#include "form_task_mgr.h"
#include "form_timer_mgr.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_verification.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::chrono;

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<FormMgrService>::GetInstance().get());

const std::string NAME_FORM_MGR_SERVICE = "FormMgrService";

FormMgrService::FormMgrService()
    : SystemAbility(FORM_MGR_SERVICE_ID, true),
      state_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr),
      handler_(nullptr)
{
}

FormMgrService::~FormMgrService()
{
    if (formSysEventReceiver_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(formSysEventReceiver_);
        formSysEventReceiver_ = nullptr;
    }
}

bool FormMgrService::IsReady() const
{
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return false;
    }
    if (!handler_) {
        HILOG_ERROR("%{public}s fail, handler is null", __func__);
        return false;
    }

    return true;
}

/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::AddForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo)
{
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, add form permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().AddForm(formId, want, callerToken, formInfo);
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, delete form permission denied", __func__);
        return ret;
    }

    return FormMgrAdapter::GetInstance().DeleteForm(formId, callerToken);
}

/**
 * @brief Release forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, release form permission denied", __func__);
        return ret;
    }

    return FormMgrAdapter::GetInstance().ReleaseForm(formId, callerToken, delCache);
}

/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param formBindingData Form binding data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::UpdateForm(const int64_t formId, const FormProviderData &formBindingData)
{
    // get IBundleMgr
    sptr<IBundleMgr> bundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("%{public}s error, failed to get bundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }
    std::string callerBundleName;
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!IN_PROCESS_CALL(bundleMgr->GetBundleNameForUid(callingUid, callerBundleName))) {
        HILOG_ERROR("GetFormsInfoByModule, failed to get form config info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }
    return FormMgrAdapter::GetInstance().UpdateForm(formId, callerBundleName, formBindingData);
}

/**
 * @brief Request form with formId and want, send formId and want to form manager service.
 * @param formId The Id of the form to update.
 * @param callerToken Caller ability token.
 * @param want The want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, request form permission denied", __func__);
        return ret;
    }

    return FormMgrAdapter::GetInstance().RequestForm(formId, callerToken, want);
}

/**
 * @brief set next refresh time.
 * @param formId The id of the form.
 * @param bundleManager the bundle manager ipc object.
 * @param nextTime next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    HILOG_INFO("%{public}s called.", __func__);

    return FormMgrAdapter::GetInstance().SetNextRefreshTime(formId, nextTime);
}


/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * @param formIds The Id list of the forms to notify.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds,
    const sptr<IRemoteObject> &callerToken, const int32_t formVisibleType)
{
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, event notify visible permission denied", __func__);
        return ret;
    }

    return FormMgrAdapter::GetInstance().NotifyWhetherVisibleForms(formIds, callerToken, formVisibleType);
}

/**
 * @brief temp form to normal form.
 * @param formId The Id of the form.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, cast temp form permission denied", __func__);
        return ret;
    }

    return FormMgrAdapter::GetInstance().CastTempForm(formId, callerToken);
}

/**
 * @brief lifecycle update.
 * @param formIds formIds of hostclient.
 * @param callerToken Caller ability token.
 * @param updateType update type,enable or disable.
 * @return Returns true on success, false on failure.
 */
int FormMgrService::LifecycleUpdate(const std::vector<int64_t> &formIds,
    const sptr<IRemoteObject> &callerToken, const int32_t updateType)
{
    HILOG_INFO("lifecycleUpdate.");

    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, delete form permission denied", __func__);
        return ret;
    }

    if (updateType == ENABLE_FORM_UPDATE) {
        return FormMgrAdapter::GetInstance().EnableUpdateForm(formIds, callerToken);
    } else {
        return FormMgrAdapter::GetInstance().DisableUpdateForm(formIds, callerToken);
    }
}
/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpStorageFormInfos(std::string &formInfos)
{
    return FormMgrAdapter::GetInstance().DumpStorageFormInfos(formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos)
{
    return FormMgrAdapter::GetInstance().DumpFormInfoByBundleName(bundleName, formInfos);
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo)
{
    return FormMgrAdapter::GetInstance().DumpFormInfoByFormId(formId, formInfo);
}
/**
 * @brief Dump form timer by form id.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService)
{
    return FormMgrAdapter::GetInstance().DumpFormTimerByFormId(formId, isTimingService);
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrService::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, request form permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().MessageEvent(formId, want, callerToken);
}

/**
 * @brief Process js router event.
 * @param formId Indicates the unique id of form.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrService::RouterEvent(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, request form permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().RouterEvent(formId);
}

/**
 * @brief Batch add forms to form records for st limit value test.
 * @param want The want of the form to add.
 * @return Returns forms count to add.
 */
int FormMgrService::BatchAddFormRecords(const Want &want)
{
    return FormMgrAdapter::GetInstance().BatchAddFormRecords(want);
}
/**
 * @brief Clear form records for st limit value test.
 * @return Returns forms count to delete.
 */
int FormMgrService::ClearFormRecords()
{
    return FormMgrAdapter::GetInstance().ClearFormRecords();
}
/**
 * @brief Start envent for the form manager service.
 */
void FormMgrService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_WARN("%{public}s fail, Failed to start service since it's already running", __func__);
        return;
    }

    HILOG_INFO("Form Mgr Service start...");
    ErrCode errCode = Init();
    if (errCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, Failed to init, errCode: %{public}08x", __func__, errCode);
        return;
    }

    state_ = ServiceRunningState::STATE_RUNNING;

    HILOG_INFO("Form Mgr Service start success.");
}
/**
 * @brief Stop envent for the form manager service.
 */
void FormMgrService::OnStop()
{
    HILOG_INFO("stop service");

    state_ = ServiceRunningState::STATE_NOT_START;

    if (handler_) {
        handler_.reset();
    }

    if (runner_) {
        runner_.reset();
    }
}
/**
 * @brief initialization of form manager service.
 */
ErrCode FormMgrService::Init()
{
    runner_ = EventRunner::Create(NAME_FORM_MGR_SERVICE);
    if (!runner_) {
        HILOG_ERROR("%{public}s fail, Failed to init due to create runner error", __func__);
        return ERR_INVALID_OPERATION;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (!handler_) {
        HILOG_ERROR("%{public}s fail, Failed to init due to create handler error", __func__);
        return ERR_INVALID_OPERATION;
    }
    FormTaskMgr::GetInstance().SetEventHandler(handler_);
    FormAmsHelper::GetInstance().SetEventHandler(handler_);
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    bool ret = Publish(DelayedSingleton<FormMgrService>::GetInstance().get());
    if (!ret) {
        HILOG_ERROR("%{public}s fail, FormMgrService::Init Publish failed!", __func__);
        return ERR_INVALID_OPERATION;
    }

    if (formSysEventReceiver_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_UID_REMOVED);
        // init TimerReceiver
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        formSysEventReceiver_ = std::make_shared<FormSysEventReceiver>(subscribeInfo);
        EventFwk::CommonEventManager::SubscribeCommonEvent(formSysEventReceiver_);
    }
    FormDbCache::GetInstance().Start();
    FormInfoMgr::GetInstance().Start();
    FormTimerMgr::GetInstance(); // Init FormTimerMgr
    HILOG_INFO("init success");
    return ERR_OK;
}

ErrCode FormMgrService::CheckFormPermission()
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    // get IBundleMgr
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s error, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    // check if system appint
    auto isSystemApp = iBundleMgr->CheckIsSystemAppByUid(IPCSkeleton::GetCallingUid());
    if (!isSystemApp) {
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY_SYS;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AppExecFwk::Constants::PERMISSION_REQUIRE_FORM);
    if (!isCallingPerm) {
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }
    HILOG_ERROR("Permission verification ok!");
    return ERR_OK;
}

/**
 * @brief  Add forms to storage for st .
 * @param Want The Want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DistributedDataAddForm(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().DistributedDataAddForm(want);
}

/**
 * @brief  Delete form form storage for st.
 * @param formId The formId of the form to delete.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DistributedDataDeleteForm(const std::string &formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().DistributedDataDeleteForm(formId);
}

/**
 * @brief Delete the given invalid forms.
 * @param formIds Indicates the ID of the forms to delete.
 * @param callerToken Caller ability token.
 * @param numFormsDeleted Returns the number of the deleted forms.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::DeleteInvalidForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
                                       int32_t &numFormsDeleted)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, delete form permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().DeleteInvalidForms(formIds, callerToken, numFormsDeleted);
}

/**
  * @brief Acquire form state info by passing a set of parameters (using Want) to the form provider.
  * @param want Indicates a set of parameters to be transparently passed to the form provider.
  * @param callerToken Caller ability token.
  * @param stateInfo Returns the form's state info of the specify.
  * @return Returns ERR_OK on success, others on failure.
  */
int FormMgrService::AcquireFormState(const Want &want, const sptr<IRemoteObject> &callerToken, FormStateInfo &stateInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, acquire form state permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().AcquireFormState(want, callerToken, stateInfo);
}

/**
 * @brief Delete the given invalid forms.
 * @param formIds Indicates the ID of the forms.
 * @param isVisible Visible or not.
 * @param callerToken Host client.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::NotifyFormsVisible(const std::vector<int64_t> &formIds, bool isVisible,
                                       const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, notify form visible permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().NotifyFormsVisible(formIds, isVisible, callerToken);
}

/**
 * @brief Delete the given invalid forms.
 * @param formIds Indicates the ID of the forms.
 * @param isEnableUpdate enable update or not.
 * @param callerToken Host client.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::NotifyFormsEnableUpdate(const std::vector<int64_t> &formIds, bool isEnableUpdate,
                                            const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    ErrCode ret = CheckFormPermission();
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s fail, notify form enable update permission denied", __func__);
        return ret;
    }
    return FormMgrAdapter::GetInstance().NotifyFormsEnableUpdate(formIds, isEnableUpdate, callerToken);
}

/**
 * @brief Get All FormsInfo.
 * @param formInfos Return the forms' information of all forms provided.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().GetAllFormsInfo(formInfos);
}

/**
 * @brief Get forms info by bundle name .
 * @param bundleName Application name.
 * @param formInfos Return the forms' information of the specify application name.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().GetFormsInfoByApp(bundleName, formInfos);
}

/**
 * @brief Get forms info by bundle name and module name.
 * @param bundleName bundle name.
 * @param moduleName Module name of hap.
 * @param formInfos Return the forms' information of the specify bundle name and module name.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::GetFormsInfoByModule(std::string &bundleName, std::string &moduleName,
                                         std::vector<FormInfo> &formInfos)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
}

/**
 * @brief Update action string for router event.
 * @param formId Indicates the unique id of form.
 * @param action Indicates the origin action string.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrService::UpdateRouterAction(const int64_t formId, std::string &action)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormMgrAdapter::GetInstance().UpdateRouterAction(formId, action);
}
}  // namespace AppExecFwk
}  // namespace OHOS
