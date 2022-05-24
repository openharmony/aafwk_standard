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

#include <cinttypes>
#include <regex>

#include "ability_manager_errors.h"
#include "appexecfwk_errors.h"
#include "bundle_active_client.h"
#include "form_acquire_connection.h"
#include "form_acquire_state_connection.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "form_cache_mgr.h"
#include "form_cast_temp_connection.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_db_cache.h"
#include "form_db_info.h"
#include "form_delete_connection.h"
#include "form_dump_mgr.h"
#include "form_event_notify_connection.h"
#include "form_info_mgr.h"
#include "form_mgr_adapter.h"
#include "form_mgr_errors.h"
#include "form_provider_info.h"
#include "form_provider_interface.h"
#include "form_provider_mgr.h"
#include "form_refresh_connection.h"
#include "form_supply_callback.h"
#include "form_timer_mgr.h"
#include "form_util.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "ohos_account_kits.h"
#include "power_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
const int HUNDRED = 100;
constexpr int32_t UID_CALLINGUID_TRANSFORM_DIVISOR = 200000;

FormMgrAdapter::FormMgrAdapter()
{
};
FormMgrAdapter::~FormMgrAdapter()
{
};
/**
 * @brief Add form with want, send want to form manager service.
 * @param formId The Id of the forms to add.
 * @param want The want of the form to add.
 * @param callerToken Caller ability token.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::AddForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken, FormJsInfo &formInfo)
{
    if (formId < 0 || callerToken == nullptr) {
        HILOG_ERROR("%{public}s fail, callerToken can not be NULL", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // check form count limit
    bool tempFormFlag = want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false);
    int callingUid = IPCSkeleton::GetCallingUid();
    int checkCode = 0;
    if (tempFormFlag) {
        if (formId > 0) {
            HILOG_ERROR("%{public}s fail, temp form id is invalid, formId:%{public}" PRId64 "", __func__, formId);
            return ERR_APPEXECFWK_FORM_INVALID_PARAM;
        }
        checkCode = FormDataMgr::GetInstance().CheckTempEnoughForm();
    } else {
        if (formId == 0) {
            // get current userId
            int32_t currentUserId = GetCurrentUserId(callingUid);
            checkCode = FormDataMgr::GetInstance().CheckEnoughForm(callingUid, currentUserId);
        }
    }
    if (checkCode != 0) {
        HILOG_ERROR("%{public}s fail, too much forms in system", __func__);
        return checkCode;
    }

    // get from config info
    FormItemInfo formItemInfo;
    int32_t errCode = GetFormConfigInfo(want, formItemInfo);
    formItemInfo.SetFormId(formId);
    if (errCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, get form config info failed.", __func__);
        return errCode;
    }
    if (!formItemInfo.IsValidItem()) {
        HILOG_ERROR("%{public}s fail, input param itemInfo is invalid", __func__);
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }
    if (!FormDataMgr::GetInstance().GenerateUdidHash()) {
        HILOG_ERROR("%{public}s fail, generate udid hash failed", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    formItemInfo.SetDeviceId(want.GetElement().GetDeviceID());
    WantParams wantParams = want.GetParams();
    if (formId > 0) {
        return AllotFormById(formItemInfo, callerToken, wantParams, formInfo);
    } else {
        return AllotFormByInfo(formItemInfo, callerToken, wantParams, formInfo);
    }
}

/**
 * @brief Delete forms with formIds, send formIds to form manager service.
 * @param formId The Id of the forms to delete.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (formId <= 0 || callerToken == nullptr) {
        HILOG_ERROR("%{public}s, deleteForm invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        // delete temp form if receive delete form call
        return HandleDeleteTempForm(matchedFormId, callerToken);
    }
    return HandleDeleteForm(matchedFormId, callerToken);
}

/**
 * @brief Release forms with formIds, send formIds to form Mgr service.
 * @param formId The Id of the forms to release.
 * @param callerToken Caller ability token.
 * @param delCache Delete Cache or not.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::ReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const bool delCache)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (formId <= 0 || callerToken == nullptr) {
        HILOG_ERROR("%{public}s, releaseForm invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        // delete temp form if receive release form call
        return HandleDeleteTempForm(matchedFormId, callerToken);
    }
    FormRecord dbRecord;
    if (FormDbCache::GetInstance().GetDBRecord(matchedFormId, dbRecord) != ERR_OK) {
        HILOG_ERROR("%{public}s, not exist such db form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    bool isSelfDbFormId = (userId == dbRecord.userId) && ((std::find(dbRecord.formUserUids.begin(),
    dbRecord.formUserUids.end(), callingUid) != dbRecord.formUserUids.end()) ? true : false);
    if (!isSelfDbFormId) {
        HILOG_ERROR("%{public}s, not self form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }
    if (delCache) {
        if (ErrCode result = HandleReleaseForm(matchedFormId, callerToken); result != ERR_OK) {
            HILOG_ERROR("%{public}s, release form error.", __func__);
            return result;
        }
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, matchedFormId)) {
        HILOG_ERROR("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    if (!FormTimerMgr::GetInstance().RemoveFormTimer(matchedFormId)) {
        HILOG_ERROR("%{public}s, remove timer error", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    return ERR_OK;
}

/**
 * @brief Handle release form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleReleaseForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!FormDataMgr::GetInstance().ExistFormRecord(formId)) {
        HILOG_ERROR("%{public}s, not exist such db or temp form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormHostRecord hostRecord;
    bool hasHostRec = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, hostRecord);
    bool isSelfId = hasHostRec && hostRecord.Contains(formId);
    if (!isSelfId) {
        HILOG_ERROR("%{public}s, not self form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    HILOG_DEBUG("%{public}s, release formRecords, formId: %{public}" PRId64 "", __func__, formId);
    FormDataMgr::GetInstance().DeleteFormUserUid(formId, IPCSkeleton::GetCallingUid());
    if (!FormDataMgr::GetInstance().HasFormUserUids(formId)) {
        FormDataMgr::GetInstance().DeleteFormRecord(formId);
        if (!FormTimerMgr::GetInstance().RemoveFormTimer(formId)) {
            HILOG_ERROR("%{public}s, remove timer error", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }
    return ERR_OK;
}

/**
 * @brief Handle delete form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    FormRecord dbRecord;
    if (FormDbCache::GetInstance().GetDBRecord(formId, dbRecord) != ERR_OK) {
        HILOG_ERROR("%{public}s, not exist such db form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormRecord record;
    FormDataMgr::GetInstance().GetFormRecord(formId, record);
    DeviceUsageStats::BundleActiveEvent event(record.bundleName, record.moduleName, record.formName,
        record.specification, record.formId, DeviceUsageStats::BundleActiveEvent::FORM_IS_REMOVED);

    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    bool isSelfDbFormId = (userId == dbRecord.userId) && ((std::find(dbRecord.formUserUids.begin(),
    dbRecord.formUserUids.end(), callingUid) != dbRecord.formUserUids.end()) ? true : false);
    if (!isSelfDbFormId) {
        HILOG_ERROR(
            "%{public}s, not self form:%{public}" PRId64 ", callingUid:%{public}d", __func__, formId, callingUid);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    if (ErrCode result = HandleDeleteFormCache(dbRecord, callingUid, formId); result != ERR_OK) {
        return result;
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, formId)) {
        HILOG_ERROR("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    DeviceUsageStats::BundleActiveClient::GetInstance().ReportEvent(event, userId);
    return ERR_OK;
}

/**
 * @brief Handle delete temp form.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);

    int uid = IPCSkeleton::GetCallingUid();
    FormRecord record;
    bool isFormRecExist = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    bool isSelfTempFormId = false;
    if (isFormRecExist && record.formTempFlg) {
        int32_t userId = GetCurrentUserId(uid);
        isSelfTempFormId = (userId == record.userId) && ((std::find(record.formUserUids.begin(),
        record.formUserUids.end(), uid) != record.formUserUids.end()) ? true : false);
    }
    if (!isSelfTempFormId) {
        HILOG_ERROR("%{public}s, not self form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    FormDataMgr::GetInstance().DeleteFormUserUid(formId, uid);
    if (!FormDataMgr::GetInstance().HasFormUserUids(formId)) {
        int result = FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, record);
        if (result != ERR_OK) {
            HILOG_ERROR("%{public}s, failed!", __func__);
            FormDataMgr::GetInstance().AddFormUserUid(formId, uid);
            return result;
        }
        FormDataMgr::GetInstance().DeleteTempForm(formId);
        FormDataMgr::GetInstance().DeleteFormRecord(formId);
        if (!FormCacheMgr::GetInstance().DeleteData(formId)) {
            HILOG_ERROR("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
    }

    if (!FormDataMgr::GetInstance().DeleteHostRecord(callerToken, formId)) {
        HILOG_ERROR("%{public}s, failed to remove host record", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    HILOG_DEBUG("%{public}s, record.formUserUids size: %{public}zu", __func__, record.formUserUids.size());
    return ERR_OK;
}

/**
 * @brief Handle delete form cache.
 * @param dbRecord Form storage information.
 * @param uid calling user id.
 * @param formId The form id.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleDeleteFormCache(FormRecord &dbRecord, const int uid, const int64_t formId)
{
    HILOG_DEBUG("%{public}s, delete formDBRecords, formId: %{public}" PRId64 "", __func__, formId);
    auto iter = std::find(dbRecord.formUserUids.begin(), dbRecord.formUserUids.end(), uid);
    if (iter != dbRecord.formUserUids.end()) {
        dbRecord.formUserUids.erase(iter);
    }

    ErrCode result = ERR_OK;
    if (dbRecord.formUserUids.empty()) {
        result = FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, dbRecord);
        if (result != ERR_OK) {
            HILOG_ERROR("%{public}s, failed to notify provider form delete", __func__);
            return result;
        }
        if (!FormDataMgr::GetInstance().DeleteFormRecord(formId)) {
            HILOG_ERROR("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        if (result = FormDbCache::GetInstance().DeleteFormInfo(formId); result != ERR_OK) {
            HILOG_ERROR("%{public}s, failed to remove db data", __func__);
            return result;
        }

        int32_t matchCount = FormDbCache::GetInstance().GetMatchCount(dbRecord.bundleName, dbRecord.moduleName);
        if (matchCount == 0) {
            FormBmsHelper::GetInstance().NotifyModuleRemovable(dbRecord.bundleName, dbRecord.moduleName);
        }

        if (!FormCacheMgr::GetInstance().DeleteData(formId)) {
            HILOG_ERROR("%{public}s, failed to remove cache data", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        if (!FormTimerMgr::GetInstance().RemoveFormTimer(formId)) {
            HILOG_ERROR("%{public}s, remove timer error", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }
        return ERR_OK;
    }

    if (result = FormDbCache::GetInstance().UpdateDBRecord(formId, dbRecord); result != ERR_OK) {
        return result;
    }

    HILOG_DEBUG("%{public}s, dbRecord.formUserUids size: %{public}zu", __func__, dbRecord.formUserUids.size());
    FormBmsHelper::GetInstance().NotifyModuleNotRemovable(dbRecord.bundleName, dbRecord.moduleName);
    FormDataMgr::GetInstance().DeleteFormUserUid(formId, uid);
    return result;
}

/**
 * @brief Update form with formId, send formId to form manager service.
 * @param formId The Id of the form to update.
 * @param bundleName Provider ability bundleName.
 * @param formProviderData form provider data.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::UpdateForm(const int64_t formId,
    const std::string &bundleName, const FormProviderData &formProviderData)
{
    HILOG_INFO("%{public}s start.", __func__);

    // check formId and bundleName
    if (formId <= 0 || bundleName.empty()) {
        HILOG_ERROR("%{public}s error, the passed in formId can't be negative or zero, bundleName is not empty.",
            __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // get IBundleMgr
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s error, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    // check bundle uid for permission
    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    int32_t bundleUid = IN_PROCESS_CALL(iBundleMgr->GetUidByBundleName(bundleName, userId));
    if (bundleUid != callingUid) {
        HILOG_ERROR("%{public}s error, permission denied, the updated form is not your own.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // find matched formId
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);

    // check exist and get the formRecord
    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        HILOG_ERROR("%{public}s error, not exist such form:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    if (userId != formRecord.userId) {
        HILOG_ERROR("%{public}s error, not under current user, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    // check bundleName match
    if (formRecord.bundleName.compare(bundleName) != 0) {
        HILOG_ERROR("%{public}s error, not match bundleName:%{public}s.", __func__, bundleName.c_str());
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // update Form
    return FormProviderMgr::GetInstance().UpdateForm(matchedFormId, formRecord, formProviderData);
}

/**
 * @brief Request form with formId and want, send formId and want to form manager service.
 * @param formId The Id of the form to update.
 * @param callerToken Caller ability token.
 * @param want The want of the form to request.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::RequestForm(const int64_t formId, const sptr<IRemoteObject> &callerToken, const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (formId <= 0 || callerToken == nullptr) {
        HILOG_ERROR("%{public}s fail, callerToken can not be NULL.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (!FormDataMgr::GetInstance().ExistFormRecord(matchedFormId)) {
        HILOG_ERROR("%{public}s fail, not exist such formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormHostRecord formHostRecord;
    bool isHostExist = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
    if (!isHostExist) {
        HILOG_ERROR("%{public}s fail, cannot find target client.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (!formHostRecord.Contains(matchedFormId)) {
        HILOG_ERROR("%{public}s fail, form is not self-owned.", __func__);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    HILOG_INFO("%{public}s, find target client.", __func__);
    Want reqWant(want);
    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    reqWant.SetParam(Constants::PARAM_FORM_USER_ID, userId);
    return FormProviderMgr::GetInstance().RefreshForm(matchedFormId, reqWant, true);
}

/**
 * @brief Form visible/invisible notify, send formIds to form manager service.
 * @param formIds The vector of form Ids.
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds,
    const sptr<IRemoteObject> &callerToken, const int32_t formVisibleType)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s fail, callerToken can not be NULL.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s fail, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    int64_t matchedFormId;
    std::map<std::string, std::vector<int64_t>> eventMaps;
    for (int64_t formId : formIds) {
        if (formId <= 0) {
            HILOG_WARN("%{public}s, formId %{public}" PRId64 " is less than 0", __func__, formId);
            continue;
        }
        matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
        FormRecord formRecord;
        // Update provider info to host
        if (!UpdateProviderInfoToHost(matchedFormId, callerToken, formVisibleType, formRecord)) {
            continue;
        }

        // Check if the form provider is system app
        if (!CheckIsSystemAppByBundleName(iBundleMgr, formRecord.bundleName)) {
            continue;
        }

        // Create eventMaps
        if (!CreateHandleEventMap(matchedFormId, formRecord, eventMaps)) {
            continue;
        }
    }

    for (auto iter = eventMaps.begin(); iter != eventMaps.end(); iter++) {
        if (HandleEventNotify(iter->first, iter->second, formVisibleType) != ERR_OK) {
            HILOG_WARN("%{public}s fail, HandleEventNotify error, key is %{public}s.", __func__, iter->first.c_str());
        }
    }

    return ERR_OK;
}

/**
 * @brief Temp form to normal form.
 * @param formId The Id of the form.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::CastTempForm(const int64_t formId, const sptr<IRemoteObject> &callerToken)
{
    if (formId <= 0 || callerToken == nullptr) {
        HILOG_ERROR("%{public}s, invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (!FormDataMgr::GetInstance().ExistFormRecord(matchedFormId) ||
        !FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        HILOG_ERROR("%{public}s, not exist such temp form:%{public}" PRId64 "", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormHostRecord record;
    bool hasHostRec = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, record);
    if (!hasHostRec || !record.Contains(matchedFormId)) {
        HILOG_ERROR("%{public}s, not self form:%{public}" PRId64 "", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    int checkCode = FormDataMgr::GetInstance().CheckEnoughForm(callingUid, userId);
    if (checkCode != 0) {
        HILOG_ERROR("%{public}s, %{public}" PRId64 " failed,because if too mush forms", __func__, matchedFormId);
        return checkCode;
    }

    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    int bindSupplierCheckCode = HandleCastTempForm(matchedFormId, formRecord);
    if (bindSupplierCheckCode != 0) {
        HILOG_ERROR("%{public}s, cast temp form bindSupplier failed", __func__);
        return bindSupplierCheckCode;
    }

    if (!FormDataMgr::GetInstance().DeleteTempForm(matchedFormId)) {
        HILOG_ERROR("%{public}s fail, delete temp form error, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    if (!FormDataMgr::GetInstance().ModifyFormTempFlg(matchedFormId, false)) {
        HILOG_ERROR(
            "%{public}s fail, modify form temp flag error, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    if (!FormDataMgr::GetInstance().AddFormUserUid(matchedFormId, callingUid)) {
        HILOG_ERROR("%{public}s fail, add form user uid error, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 ".", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    if (ErrCode errorCode = FormDbCache::GetInstance().UpdateDBRecord(matchedFormId, formRecord); errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, update db record error, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return errorCode;
    }

    // start timer
    return AddFormTimer(formRecord);
}
/**
 * @brief Handle cast temp form.
 * @param formId The form id.
 * @param record Form information.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleCastTempForm(const int64_t formId, const FormRecord &record)
{
    HILOG_DEBUG("%{public}s, cast temp form to normal form, notify supplier, package:%{public}s, class:%{public}s",
        __func__, record.bundleName.c_str(), record.abilityName.c_str());
    sptr<IAbilityConnection> castTempConnection = new FormCastTempConnection(formId,
        record.bundleName, record.abilityName);

    Want want;
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    want.SetElementName(record.bundleName, record.abilityName);
    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, castTempConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ERR_OK;
}
/**
 * @brief Dump all of form storage infos.
 * @param formInfos All of form storage infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpStorageFormInfos(std::string &formInfos) const
{
    std::vector<FormDBInfo> formDBInfos;
    FormDbCache::GetInstance().GetAllFormInfo(formDBInfos);
    if (formDBInfos.size() > 0) {
        std::sort(formDBInfos.begin(), formDBInfos.end(),
        [] (FormDBInfo &formDBInfoA, FormDBInfo &formDBInfoB) -> bool {
            return formDBInfoA.formId < formDBInfoB.formId;
        });
        FormDumpMgr::GetInstance().DumpStorageFormInfos(formDBInfos, formInfos);
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
}
/**
 * @brief Dump form info by a bundle name.
 * @param bundleName The bundle name of form provider.
 * @param formInfos Form infos.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpFormInfoByBundleName(const std::string &bundleName, std::string &formInfos) const
{
    HILOG_INFO("%{public}s called.", __func__);
    std::vector<FormRecord> formRecordInfos;
    if (FormDataMgr::GetInstance().GetFormRecord(bundleName, formRecordInfos)) {
        FormDumpMgr::GetInstance().DumpFormInfos(formRecordInfos, formInfos);
        return ERR_OK;
    } else {
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
}
/**
 * @brief Dump form info by a bundle name.
 * @param formId The id of the form.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpFormInfoByFormId(const std::int64_t formId, std::string &formInfo) const
{
    HILOG_INFO("%{public}s called.", __func__);
    int reply = ERR_APPEXECFWK_FORM_NOT_EXIST_ID;

    FormRecord formRecord;
    if (FormDataMgr::GetInstance().GetFormRecord(formId, formRecord)) {
        FormDumpMgr::GetInstance().DumpFormInfo(formRecord, formInfo);
        reply = ERR_OK;
    }

    FormHostRecord formHostRecord;
    if (FormDataMgr::GetInstance().GetFormHostRecord(formId, formHostRecord)) {
        FormDumpMgr::GetInstance().DumpFormHostInfo(formHostRecord, formInfo);
        reply = ERR_OK;
    }

    return reply;
}
/**
 * @brief Dump form timer by form id.
 * @param formId The id of the form.
 * @param formInfo Form timer.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DumpFormTimerByFormId(const std::int64_t formId, std::string &isTimingService) const
{
    HILOG_INFO("%{public}s called.", __func__);
    FormTimer formTimer;
    UpdateAtItem updateAtItem;
    DynamicRefreshItem dynamicItem;
    bool result_inter = FormTimerMgr::GetInstance().GetIntervalTimer(formId, formTimer);
    bool result_update = FormTimerMgr::GetInstance().GetUpdateAtTimer(formId, updateAtItem);
    bool result_dynamic = FormTimerMgr::GetInstance().GetDynamicItem(formId, dynamicItem);
    HILOG_INFO("%{public}s result_inter:%{public}d,result_update:%{public}d,result_dynamic:%{public}d",
        __func__, result_inter, result_update, result_dynamic);
    if (result_inter || result_update || result_dynamic) {
        isTimingService = "true";
    } else {
        isTimingService = "false";
    }
    return ERR_OK;
}
/**
 * @brief Get form configure info.
 * @param want The want of the request.
 * @param formItemInfo Form configure info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormConfigInfo(const Want &want, FormItemInfo &formConfigInfo)
{
    HILOG_DEBUG("GetFormConfigInfo start.");
    BundleInfo bundleInfo;
    std::string packageName;

    ErrCode errCode = GetBundleInfo(want, bundleInfo, packageName);
    if (errCode != ERR_OK) {
        HILOG_ERROR("addForm find bundle info failed");
        return errCode;
    }

    FormInfo formInfo;
    errCode = GetFormInfo(want, formInfo);
    if (errCode != ERR_OK) {
        HILOG_ERROR("addForm can not find target form info");
        return errCode;
    }
    if (!formInfo.IsValid()) {
        HILOG_ERROR("The form info is invalid");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    errCode = GetFormItemInfo(want, bundleInfo, formInfo, formConfigInfo);
    if (errCode != ERR_OK) {
        HILOG_ERROR("get form item info failed.");
        return errCode;
    }
    formConfigInfo.SetPackageName(packageName);

    HILOG_DEBUG("GetFormConfigInfo end.");
    return ERR_OK;
}
/**
 * @brief Allocate form by formId.
 * @param info Form configure info.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AllotFormById(const FormItemInfo &info,
    const sptr<IRemoteObject> &callerToken, const WantParams &wantParams, FormJsInfo &formInfo)
{
    int64_t formId = PaddingUDIDHash(info.GetFormId());
    FormRecord record;
    bool hasRecord = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    if (hasRecord && record.formTempFlg) {
        HILOG_ERROR("%{public}s, addForm can not acquire temp form when select form id", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    // get current userId
    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t currentUserId = GetCurrentUserId(callingUid);
    if (hasRecord && (record.userId == currentUserId
        || FormDataMgr::GetInstance().IsCallingUidValid(record.formUserUids))) {
        if (!info.IsMatch(record)) {
            HILOG_ERROR("%{public}s, formId and item info not match:%{public}" PRId64 "", __func__, formId);
            return ERR_APPEXECFWK_FORM_CFG_NOT_MATCH_ID;
        }
        return AddExistFormRecord(info, callerToken, record, formId, wantParams, formInfo);
    }

    // find in db but not in cache
    FormRecord dbRecord;
    ErrCode getDbRet = FormDbCache::GetInstance().GetDBRecord(formId, dbRecord);
    if (getDbRet == ERR_OK && (dbRecord.userId == currentUserId
        || FormDataMgr::GetInstance().IsCallingUidValid(dbRecord.formUserUids))) {
        return AddNewFormRecord(info, formId, callerToken, wantParams, formInfo);
    }

    HILOG_INFO("%{public}s, addForm no such form %{public}" PRId64 "", __func__, formId);

    // delete form data in provider
    FormRecord delRecord;
    delRecord.bundleName = info.GetProviderBundleName();
    delRecord.abilityName = info.GetAbilityName();
    FormProviderMgr::GetInstance().NotifyProviderFormDelete(formId, delRecord);

    return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
}
int64_t FormMgrAdapter::PaddingUDIDHash(const int64_t formId) const
{
    // Compatible with int form id.
    uint64_t unsignedFormId = static_cast<uint64_t>(formId);
    if ((unsignedFormId & 0xffffffff00000000L) == 0) {
        int64_t udidHash = FormDataMgr::GetInstance().GetUdidHash();
        uint64_t unsignUdidHash  = static_cast<uint64_t>(udidHash);
        uint64_t unsignUdidHashFormId = unsignUdidHash | unsignedFormId;
        int64_t udidHashFormId = static_cast<int64_t>(unsignUdidHashFormId);
        return udidHashFormId;
    }
    return formId;
}
ErrCode FormMgrAdapter::AddExistFormRecord(const FormItemInfo &info, const sptr<IRemoteObject> &callerToken,
                                           const FormRecord &record, const int64_t formId, const WantParams &wantParams,
                                           FormJsInfo &formInfo)
{
    HILOG_INFO("%{public}s call, formId:%{public}" PRId64 "", __func__, formId);
    // allot form host record
    int callingUid = IPCSkeleton::GetCallingUid();
    bool isCreated = FormDataMgr::GetInstance().AllotFormHostRecord(info, callerToken, formId, callingUid);
    if (!isCreated) {
        HILOG_ERROR("%{public}s fail, AllotFormHostRecord failed when no matched formRecord", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    FormRecord newRecord(record);

    if (newRecord.needRefresh || !FormCacheMgr::GetInstance().IsExist(newRecord.formId)) {
        newRecord.isInited = false;
        FormDataMgr::GetInstance().SetFormCacheInited(formId, false);
        // acquire formInfo from provider
        ErrCode errorCode = AcquireProviderFormInfoAsync(formId, info, wantParams);
        if (errorCode != ERR_OK) {
            HILOG_ERROR("%{public}s fail, AcquireProviderFormInfoAsync failed", __func__);
            return errorCode;
        }
    }

    // Add new form user uid.
    FormDataMgr::GetInstance().AddFormUserUid(formId, callingUid);
    if (std::find(newRecord.formUserUids.begin(), newRecord.formUserUids.end(), callingUid) ==
        newRecord.formUserUids.end()) {
        newRecord.formUserUids.emplace_back(callingUid);
    }

    // create form info for js
    std::string cacheData;
    std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageMap;
    if (FormCacheMgr::GetInstance().GetData(formId, cacheData, imageMap)) {
        formInfo.formData = cacheData;
        formInfo.formProviderData.SetImageDataMap(imageMap);
    }
    FormDataMgr::GetInstance().CreateFormInfo(formId, record, formInfo);

    // start update timer
    if (ErrCode errorCode = AddFormTimer(newRecord); errorCode != ERR_OK) {
        return errorCode;
    }

    if (!newRecord.formTempFlg) {
        return FormDbCache::GetInstance().UpdateDBRecord(formId, newRecord);
    }
    return ERR_OK;
}
/**
 * @brief Allocate form by form configure info.
 * @param info Form configure info.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AllotFormByInfo(const FormItemInfo &info,
    const sptr<IRemoteObject> &callerToken, const WantParams &wantParams, FormJsInfo &formInfo)
{
    // generate formId
    int64_t newFormId = FormDataMgr::GetInstance().GenerateFormId();
    if (newFormId < 0) {
        HILOG_ERROR("%{public}s fail, generateFormId no invalid formId", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    HILOG_DEBUG("newFormId:%{public}" PRId64 "", newFormId);

    return AddNewFormRecord(info, newFormId, callerToken, wantParams, formInfo);
}

/**
 * @brief Add new form record.
 * @param info Form configure info.
 * @param formId The form id.
 * @param callerToken Caller ability token.
 * @param wantParams WantParams of the request.
 * @param formInfo Form info for form host.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AddNewFormRecord(const FormItemInfo &info, const int64_t formId,
    const sptr<IRemoteObject> &callerToken, const WantParams &wantParams, FormJsInfo &formInfo)
{
    HILOG_INFO("%{public}s start", __func__);
    FormItemInfo newInfo(info);
    newInfo.SetFormId(formId);
    // allot form host record
    int callingUid = IPCSkeleton::GetCallingUid();
    if (!FormDataMgr::GetInstance().AllotFormHostRecord(newInfo, callerToken, formId, callingUid)) {
        HILOG_ERROR("%{public}s fail, AllotFormHostRecord failed when no matched formRecord", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    // get current userId
    int32_t currentUserId = GetCurrentUserId(callingUid);
    // allot form record
    FormRecord formRecord = FormDataMgr::GetInstance().AllotFormRecord(newInfo, callingUid, currentUserId);

    // acquire formInfo from provider
    if (ErrCode errorCode = AcquireProviderFormInfoAsync(formId, newInfo, wantParams); errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, AcquireProviderFormInfoAsync failed", __func__);
        return errorCode;
    }

    // create form info for js
    FormDataMgr::GetInstance().CreateFormInfo(formId, formRecord, formInfo);

    // storage info
    if (!newInfo.IsTemporaryForm()) {
        if (ErrCode errorCode = FormDbCache::GetInstance().UpdateDBRecord(formId, formRecord); errorCode != ERR_OK) {
            HILOG_ERROR("%{public}s fail, UpdateDBRecord failed", __func__);
            return errorCode;
        }
    }

    // start update timer
    return AddFormTimer(formRecord);
}

/**
 * @brief Add form timer.
 * @param formRecord Form information.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AddFormTimer(const FormRecord &formRecord)
{
    HILOG_INFO("%{public}s start", __func__);
    if (!formRecord.isEnableUpdate || formRecord.formTempFlg) {
        HILOG_INFO("%{public}s isEnableUpdate:%{public}d formTempFlg:%{public}d.",
            __func__, formRecord.isEnableUpdate, formRecord.formTempFlg);
        return ERR_OK;
    }
    if (formRecord.updateDuration > 0) {
        bool ret = FormTimerMgr::GetInstance().AddFormTimer(formRecord.formId,
            formRecord.updateDuration, formRecord.userId);
        return ret ? ERR_OK : ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    if (formRecord.updateAtHour >= 0 && formRecord.updateAtMin >= 0) {
        bool ret = FormTimerMgr::GetInstance().AddFormTimer(formRecord.formId,
            formRecord.updateAtHour, formRecord.updateAtMin, formRecord.userId);
        return ret ? ERR_OK : ERR_APPEXECFWK_FORM_COMMON_CODE;
    }
    HILOG_INFO("%{public}s no need add form timer.", __func__);
    return ERR_OK;
}

/**
 * @brief Send event notify to form provider. The event notify type include FORM_VISIBLE and FORM_INVISIBLE.
 *
 * @param providerKey The provider key string which consists of the provider bundle name and ability name.
 * @param formIdsByProvider The vector of form Ids which have the same provider.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::HandleEventNotify(const std::string &providerKey, const std::vector<int64_t> &formIdsByProvider,
    const int32_t formVisibleType)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t position = providerKey.find(Constants::NAME_DELIMITER);
    std::string bundleName = providerKey.substr(0, position);
    std::string abilityName = providerKey.substr(position + Constants::NAME_DELIMITER.size());
    sptr<IAbilityConnection> formEventNotifyConnection = new FormEventNotifyConnection(formIdsByProvider,
        formVisibleType, bundleName, abilityName);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);

    connectWant.SetElementName(bundleName, abilityName);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formEventNotifyConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    return ERR_OK;
}

/**
 * @brief Acquire form data from form provider.
 * @param formId The Id of the form.
 * @param info Form configure info.
 * @param wantParams WantParams of the request.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AcquireProviderFormInfoAsync(const int64_t formId,
    const FormItemInfo &info, const WantParams &wantParams)
{
    if (formId <= 0) {
        HILOG_ERROR("%{public}s fail, formId should be greater than 0", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IAbilityConnection> formAcquireConnection
        = new FormAcquireConnection(formId, info, wantParams);
    Want want;
    want.SetElementName(info.GetProviderBundleName(), info.GetAbilityName());
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, formAcquireConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ERR_OK;
}

/**
 * @brief Get bundle info.
 * @param want The want of the request.
 * @param bundleInfo Bundle info.
 * @param packageName Package name.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetBundleInfo(const AAFwk::Want &want, BundleInfo &bundleInfo, std::string &packageName)
{
    HILOG_DEBUG("GetBundleMgr start.");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);
    if (bundleName.empty() || abilityName.empty() || moduleName.empty()) {
        HILOG_ERROR("GetBundleInfo bundleName or abilityName or moduleName is invalid");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("GetBundleMgr, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    if (!IN_PROCESS_CALL(iBundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, FormUtil::GetCurrentAccountId()))) {
        HILOG_ERROR("GetBundleInfo, failed to get bundle info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    bool moduleExist = false;
    for (const auto &moduleInfo : bundleInfo.moduleNames) {
        HILOG_DEBUG("bundleInfo.moduleNames, module name:%{public}s", moduleInfo.c_str());
        if (moduleInfo.compare(moduleName) == 0) {
            moduleExist = true;
            break;
        }
    }
    if (!moduleExist) {
        HILOG_ERROR("GetBundleInfo no such module, name:%{public}s", moduleName.c_str());
        return ERR_APPEXECFWK_FORM_NO_SUCH_MODULE;
    }

    packageName = bundleName + moduleName;
    HILOG_DEBUG("GetBundleMgr end.");
    return ERR_OK;
}
/**
 * @brief Get form info.
 * @param want The want of the request.
 * @param packageName Package name.
 * @param formInfo Form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormInfo(const AAFwk::Want &want, FormInfo &formInfo)
{
    HILOG_DEBUG("GetFormInfo start.");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    std::string moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);
    if (bundleName.empty() || abilityName.empty() || moduleName.empty()) {
        HILOG_ERROR("addForm bundleName or abilityName or moduleName is invalid");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::vector<FormInfo> formInfos {};
    ErrCode errCode = FormInfoMgr::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
    if (errCode != ERR_OK) {
        HILOG_ERROR("GetFormsInfoByModule, failed to get form config info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    std::string formName = want.GetStringParam(Constants::PARAM_FORM_NAME_KEY);
    if (formName.empty()) {
        for (const auto &form : formInfos) {
            if (form.defaultFlag) {
                formInfo = form;
                formInfo.moduleName = moduleName;
                HILOG_DEBUG("GetFormInfo end.");
                return ERR_OK;
            }
        }
    } else  {
        for (const auto &form : formInfos) {
            if (form.name == formName) {
                formInfo = form;
                formInfo.moduleName = moduleName;
                HILOG_DEBUG("GetFormInfo end.");
                return ERR_OK;
            }
        }
    }
    HILOG_ERROR("failed to get form info failed.");
    return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
}
/**
 * @brief Get form configure info.
 * @param want The want of the request.
 * @param bundleInfo Bundle info.
 * @param formInfo Form info.
 * @param formItemInfo Form configure info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::GetFormItemInfo(const AAFwk::Want &want, const BundleInfo &bundleInfo,
    const FormInfo &formInfo, FormItemInfo &formItemInfo)
{
    HILOG_DEBUG("GetFormItemInfo start.");
    int32_t dimensionId = want.GetIntParam(Constants::PARAM_FORM_DIMENSION_KEY, formInfo.defaultDimension);
    if (!IsDimensionValid(formInfo, dimensionId)) {
        HILOG_ERROR("GetFormItemInfo failed, dimension is not valid.");
        return ERR_APPEXECFWK_FORM_NO_SUCH_DIMENSION;
    }

    if (ErrCode ret = CreateFormItemInfo(bundleInfo, formInfo, formItemInfo); ret != ERR_OK) {
        HILOG_ERROR("GetFormItemInfo failed, CreateFormItemInfo failed.");
        return ret;
    }
    formItemInfo.SetSpecificationId(dimensionId);
    formItemInfo.SetTemporaryFlag(want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false));

    HILOG_DEBUG("GetFormItemInfo end.");
    return ERR_OK;
}
/**
 * @brief Dimension valid check.
 * @param formInfo Form info.
 * @param dimensionId Dimension id.
 * @return Returns true on success, false on failure.
 */
bool FormMgrAdapter::IsDimensionValid(const FormInfo &formInfo, int dimensionId) const
{
    if (formInfo.supportDimensions.empty()) {
        HILOG_ERROR("Js form, no support dimension.");
        return false;
    }

    for (size_t i = 0; i < formInfo.supportDimensions.size() && i < Constants::MAX_LAYOUT; i++) {
        int supportDimensionId = formInfo.supportDimensions[i];
        if (supportDimensionId == dimensionId) {
            return true;
        }
    }

    HILOG_ERROR("No matched dimension found.");
    return false;
}
/**
 * @brief Create form configure info.
 * @param bundleInfo Bundle info.
 * @param formInfo Form info.
 * @param itemInfo Form configure info.
 */
ErrCode FormMgrAdapter::CreateFormItemInfo(const BundleInfo &bundleInfo,
    const FormInfo &formInfo, FormItemInfo &itemInfo)
{
    itemInfo.SetProviderBundleName(bundleInfo.name);
    itemInfo.SetVersionCode(bundleInfo.versionCode);
    itemInfo.SetVersionName(bundleInfo.versionName);
    itemInfo.SetCompatibleVersion(bundleInfo.compatibleVersion);

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("GetFormInfo, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }
    std::string hostBundleName {};
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!IN_PROCESS_CALL(iBundleMgr->GetBundleNameForUid(callingUid, hostBundleName))) {
        HILOG_ERROR("GetFormsInfoByModule, failed to get form config info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }
    itemInfo.SetHostBundleName(hostBundleName);

    itemInfo.SetAbilityName(formInfo.abilityName);
    itemInfo.SetModuleName(formInfo.moduleName); // formInfo.moduleName: bundleMagr do not set
    itemInfo.SetFormName(formInfo.name);
    itemInfo.SetEnableUpdateFlag(formInfo.updateEnabled);
    itemInfo.SetUpdateDuration(formInfo.updateDuration);
    itemInfo.SetScheduledUpdateTime(formInfo.scheduledUpdateTime);
    itemInfo.SetJsComponentName(formInfo.jsComponentName);
    itemInfo.SetFormVisibleNotify(formInfo.formVisibleNotify);
    itemInfo.SetFormSrc(formInfo.src);
    itemInfo.SetFormWindow(formInfo.window);

    for (const auto &abilityInfo : bundleInfo.abilityInfos) {
        if (abilityInfo.name == formInfo.abilityName) {
            itemInfo.SetAbilityModuleName(abilityInfo.moduleName);
            if (!abilityInfo.isModuleJson) {
                itemInfo.SetFormSrc("");
            }
        }
    }

    HILOG_INFO("%{public}s moduleInfos size: %{public}zu", __func__, bundleInfo.applicationInfo.moduleInfos.size());
    for (const auto &item : bundleInfo.applicationInfo.moduleInfos) {
        HILOG_INFO("%{public}s moduleInfos,  moduleName: %{public}s, moduleSourceDir: %{public}s", __func__,
            item.moduleName.c_str(), item.moduleSourceDir.c_str());
        if (formInfo.moduleName == item.moduleName) {
            itemInfo.AddHapSourceDirs(item.moduleSourceDir);
        }
        auto moduleSourceDir = std::regex_replace(item.moduleSourceDir, std::regex(Constants::ABS_CODE_PATH),
            Constants::LOCAL_BUNDLES);
        itemInfo.AddModuleInfo(item.moduleName, moduleSourceDir);
    }
    return ERR_OK;
}

/**
 * @brief set next refresh time.
 * @param formId The id of the form.
 * @param nextTime next refresh time.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::SetNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    HILOG_INFO(
        "%{public}s begin here, formId:%{public}" PRId64 ",nextTime:%{public}" PRId64 "", __func__, formId, nextTime);
    if (formId <= 0) {
        HILOG_ERROR("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::string bundleName;
    if (!GetBundleName(bundleName)) {
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    // get IBundleMgr
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s error, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    int callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = GetCurrentUserId(callingUid);
    HILOG_INFO("%{public}s, userId:%{public}d, callingUid:%{public}d.", __func__, userId, callingUid);

    int32_t bundleUid = IN_PROCESS_CALL(iBundleMgr->GetUidByBundleName(bundleName, userId));
    if (bundleUid != callingUid) {
        HILOG_ERROR("%{public}s error, permission denied, the form is not your own.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    FormRecord formRecord;
    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        HILOG_ERROR("%{public}s, not found in formrecord.", __func__);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    bool isSelfFormId = (userId == formRecord.userId);
    if (!isSelfFormId) {
        HILOG_ERROR("%{public}s, not self form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    // check bundleName
    if (bundleName != formRecord.bundleName) {
        HILOG_ERROR("%{public}s, not match bundleName:%{public}s", __func__, bundleName.c_str());
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    return SetNextRefreshtTimeLocked(matchedFormId, nextTime, userId);
}

/**
 * @brief get bundleName.
 * @param bundleName for output.
 * @return Returns true on success, others on failure.
 */
bool FormMgrAdapter::GetBundleName(std::string &bundleName)
{
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s, failed to get IBundleMgr.", __func__);
        return false;
    }

    int uid = IPCSkeleton::GetCallingUid();
    if (!IN_PROCESS_CALL(iBundleMgr->CheckIsSystemAppByUid(uid))) {
        HILOG_ERROR("%{public}s fail, form is not system app. uid:%{public}d", __func__, uid);
        return false;
    }

    bool result = IN_PROCESS_CALL(iBundleMgr->GetBundleNameForUid(uid, bundleName));
    if (!result || bundleName.empty()) {
        HILOG_ERROR("%{public}s failed, cannot get bundle name by uid:%{public}d", __func__, uid);
        return false;
    }
    return true;
}

/**
 * @brief set next refresht time locked.
 * @param formId The form's id.
 * @param nextTime next refresh time.
 * @param userId User ID.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::SetNextRefreshtTimeLocked(const int64_t formId, const int64_t nextTime, const int32_t userId)
{
    HILOG_ERROR("SetNextRefreshtTimeLocked.");
    int32_t timerRefreshedCount = FormTimerMgr::GetInstance().GetRefreshCount(formId);
    if (timerRefreshedCount >= Constants::LIMIT_COUNT) {
        HILOG_ERROR("%{public}s, already refresh times:%{public}d", __func__, timerRefreshedCount);
        FormTimerMgr::GetInstance().MarkRemind(formId);
        return ERR_APPEXECFWK_FORM_MAX_REFRESH;
    }

    if (!FormTimerMgr::GetInstance().SetNextRefreshTime(formId, nextTime * Constants::SEC_PER_MIN, userId)) {
        HILOG_ERROR("%{public}s failed", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    return ERR_OK;
}

/**
 * @brief set next refresht time locked.
 * @param formId The form's id.
 * @param bundleName Provider ability bundleName.
 * @return Returns true or false.
 */
bool FormMgrAdapter::IsUpdateValid(const int64_t formId, const std::string &bundleName)
{
    if (formId <= 0 || bundleName.empty()) {
        return false;
    }
    return true;
}

/**
 * @brief enable update form.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::EnableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("enableUpdateForm");
    return HandleUpdateFormFlag(formIDs, callerToken, true, false);
}

/**
 * @brief disable update form.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DisableUpdateForm(const std::vector<int64_t> formIDs, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("disableUpdateForm");
    return HandleUpdateFormFlag(formIDs, callerToken, false, false);
}

/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param want information passed to supplier.
 * @param callerToken Caller ability token.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrAdapter::MessageEvent(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0) {
        HILOG_ERROR("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s failed, callerToken can not be NULL", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (!want.HasParameter(Constants::PARAM_MESSAGE_KEY)) {
        HILOG_ERROR("%{public}s failed, message info is not exist", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(matchedFormId, record);
    if (!bGetRecord) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 "", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormHostRecord formHostRecord;
    bool isHostExist = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
    if (!isHostExist) {
        HILOG_ERROR("%{public}s failed, cannot find target client.", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    if (!formHostRecord.Contains(matchedFormId)) {
        HILOG_ERROR("%{public}s failed, form is not self-owned.", __func__);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    auto errCode = FormProviderMgr::GetInstance().MessageEvent(matchedFormId, record, want);
    if (errCode != ERR_OK) {
        return errCode;
    }
    HILOG_INFO("%{public}s, find target client.", __func__);

    if (!FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        int callingUid = IPCSkeleton::GetCallingUid();
        int32_t userId = GetCurrentUserId(callingUid);
        DeviceUsageStats::BundleActiveEvent event(record.bundleName, record.moduleName, record.formName,
            record.specification, record.formId, DeviceUsageStats::BundleActiveEvent::FORM_IS_CLICKED);
        DeviceUsageStats::BundleActiveClient::GetInstance().ReportEvent(event, userId);
    }
    return ERR_OK;
}

/**
 * @brief Process js router event.
 * @param formId Indicates the unique id of form.
 * @param want the want of the ability to start.
 * @return Returns true if execute success, false otherwise.
 */
int FormMgrAdapter::RouterEvent(const int64_t formId, Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0) {
        HILOG_ERROR("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(matchedFormId, record);
    if (!bGetRecord) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 "", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s fail, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    if (record.bundleName != want.GetBundle()) {
        if (!CheckIsSystemAppByBundleName(iBundleMgr, record.bundleName)) {
            HILOG_WARN("Only system apps can launch the ability of the other apps.");
            want.SetBundle(record.bundleName);
        }
    }

    int32_t result = FormAmsHelper::GetInstance().GetAbilityManager()->StartAbility(want);
    if (result != ERR_OK && result != START_ABILITY_WAITING) {
        HILOG_ERROR("Failed to StartAbility, result: %{public}d.", result);
        return result;
    }

    if (!FormDataMgr::GetInstance().ExistTempForm(matchedFormId)) {
        int32_t callingUid = IPCSkeleton::GetCallingUid();
        int32_t userId = GetCurrentUserId(callingUid);
        DeviceUsageStats::BundleActiveEvent event(record.bundleName, record.moduleName, record.formName,
            record.specification, record.formId, DeviceUsageStats::BundleActiveEvent::FORM_IS_CLICKED);
        DeviceUsageStats::BundleActiveClient::GetInstance().ReportEvent(event, userId);
    }
    return ERR_OK;
}

ErrCode FormMgrAdapter::HandleUpdateFormFlag(const std::vector<int64_t> &formIds,
                                             const sptr<IRemoteObject> &callerToken, bool flag, bool isOnlyEnableUpdate)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formIds.empty() || callerToken == nullptr) {
        HILOG_ERROR("%{public}s, invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    std::vector<int64_t> refreshForms;
    int errCode = FormDataMgr::GetInstance().UpdateHostFormFlag(formIds, callerToken, flag, isOnlyEnableUpdate,
        refreshForms);
    if (errCode == ERR_OK && !refreshForms.empty()) {
        for (const int64_t id : refreshForms) {
            HILOG_INFO("%{public}s, formRecord need refresh: %{public}" PRId64 "", __func__, id);
            Want want;
            int callingUid = IPCSkeleton::GetCallingUid();
            int32_t userId = GetCurrentUserId(callingUid);
            want.SetParam(Constants::PARAM_FORM_USER_ID, userId);
            FormProviderMgr::GetInstance().RefreshForm(id, want, false);
        }
    }
    return errCode;
}

/**
 * @brief handle update form flag.
 * @param formIDs The id of the forms.
 * @param callerToken Caller ability token.
 * @param flag form flag.
 * @return Returns true on success, false on failure.
 */
bool FormMgrAdapter::IsFormCached(const FormRecord record)
{
    if (record.versionUpgrade) {
        return false;
    }
    return true;
}

/**
 * @brief Acquire form data from form provider.
 * @param formId The Id of the from.
 * @param want The want of the request.
 * @param remoteObject Form provider proxy object.
 */
void FormMgrAdapter::AcquireProviderFormInfo(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("%{public}s called.", __func__);

    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        HILOG_ERROR("%{public}s fail, Failed to get formProviderProxy", __func__);
        return;
    }

    int error = formProviderProxy->AcquireProviderFormInfo(formId, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        HILOG_ERROR("%{public}s fail, Failed to get acquire provider form info", __func__);
    }
}

/**
 * @brief Notify form provider for delete form.
 *
 * @param formId The Id of the from.
 * @param want The want of the form.
 * @param remoteObject Form provider proxy object.
 * @return none.
 */
void FormMgrAdapter::NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("%{public}s called.", __func__);

    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);
    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        HILOG_ERROR("%{public}s fail, Failed to get formProviderProxy", __func__);
        return;
    }
    int error = formProviderProxy->NotifyFormDelete(formId, want, FormSupplyCallback::GetInstance());
    if (error != ERR_OK) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        HILOG_ERROR("%{public}s fail, Failed to get acquire provider form info", __func__);
    }
}

/**
 * @brief Batch add forms to form records for st limit value test.
 * @param want The want of the form to add.
 * @return Returns forms count to add.
 */
int FormMgrAdapter::BatchAddFormRecords(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    ElementName elementName = want.GetElement();
    std::string bundleName = elementName.GetBundleName();
    std::string abilityName = elementName.GetAbilityName();
    int formCount = want.GetIntParam(Constants::PARAM_FORM_ADD_COUNT, 0);
    HILOG_INFO("%{public}s, batch add form, bundleName: %{public}s, abilityName: %{public}s, count: %{public}d.",
        __func__,
        bundleName.c_str(),
        abilityName.c_str(),
        formCount);

    for (int count = 0; count < formCount; count++) {
        // get from comfig info
        FormItemInfo formItemInfo;
        int32_t errCode = GetFormConfigInfo(want, formItemInfo);
        if (errCode != ERR_OK) {
            HILOG_ERROR("%{public}s fail, get form config info failed.", __func__);
            return errCode;
        }
        if (!formItemInfo.IsValidItem()) {
            HILOG_ERROR("%{public}s fail, input param itemInfo is invalid", __func__);
            return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
        }
        if (!FormDataMgr::GetInstance().GenerateUdidHash()) {
            HILOG_ERROR("%{public}s fail, generate udid hash failed", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }

        // generate formId
        int64_t newFormId = FormDataMgr::GetInstance().GenerateFormId();
        if (newFormId < 0) {
            HILOG_ERROR("%{public}s fail, generateFormId no invalid formId", __func__);
            return ERR_APPEXECFWK_FORM_COMMON_CODE;
        }

        formItemInfo.SetFormId(newFormId);
        // allot form host record
        int callingUid = IPCSkeleton::GetCallingUid();
        int32_t currentUserId = GetCurrentUserId(callingUid);
        // allot form record
        FormRecord formRecord = FormDataMgr::GetInstance().AllotFormRecord(formItemInfo, callingUid, currentUserId);
        HILOG_INFO("%{public}s, batch add form, formId:" "%{public}" PRId64 ".", __func__, formRecord.formId);

        HILOG_INFO("%{public}s, count: %{public}d", __func__, count + 1);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ERR_OK;
}
/**
 * @brief Clear form records for st limit value test.
 * @return Returns forms count to delete.
 */
int FormMgrAdapter::ClearFormRecords()
{
    HILOG_INFO("%{public}s called.", __func__);
    FormDataMgr::GetInstance().ClearFormRecords();
    HILOG_INFO("%{public}s end.", __func__);
    return ERR_OK;
}
/**
 * @brief Create eventMaps for event notify.
 *
 * @param matchedFormId The Id of the form
 * @param formRecord Form storage information
 * @param eventMaps eventMaps for event notify
 * @return Returns true on success, false on failure.
 */
bool FormMgrAdapter::CreateHandleEventMap(const int64_t matchedFormId, const FormRecord &formRecord,
    std::map<std::string, std::vector<int64_t>> &eventMaps)
{
    if (!formRecord.formVisibleNotify) {
        HILOG_WARN("%{public}s fail, the config item 'formVisibleNotify' is false, formId:%{public}" PRId64 ".",
            __func__, matchedFormId);
        return false;
    }

    std::string providerKey = formRecord.bundleName + Constants::NAME_DELIMITER + formRecord.abilityName;
    auto iter = eventMaps.find(providerKey);
    if (iter == eventMaps.end()) {
        std::vector<int64_t> formEventsByProvider {matchedFormId};
        eventMaps.insert(std::make_pair(providerKey, formEventsByProvider));
    } else {
        iter->second.emplace_back(matchedFormId);
    }
    return true;
}
/**
 * @brief Update provider info to host
 *
 * @param matchedFormId The Id of the form
 * @param callerToken Caller ability token.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param formRecord Form storage information
 * @return Returns true on success, false on failure.
 */
bool FormMgrAdapter::UpdateProviderInfoToHost(const int64_t matchedFormId, const sptr<IRemoteObject> &callerToken,
    const int32_t formVisibleType, FormRecord &formRecord)
{
    if (!FormDataMgr::GetInstance().GetFormRecord(matchedFormId, formRecord)) {
        HILOG_WARN("%{public}s fail, not exist such form, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return false;
    }

    FormHostRecord formHostRecord;
    bool hasFormHostRecord = FormDataMgr::GetInstance().GetMatchedHostClient(callerToken, formHostRecord);
    if (!(hasFormHostRecord && formHostRecord.Contains(matchedFormId))) {
        HILOG_WARN("%{public}s fail, form is not belong to self, formId:%{public}" PRId64 ".", __func__, matchedFormId);
        return false;
    }

    formRecord.formVisibleNotifyState = formVisibleType;
    if (!FormDataMgr::GetInstance().UpdateFormRecord(matchedFormId, formRecord)) {
        HILOG_WARN("%{public}s fail, set formVisibleNotifyState error, formId:%{public}" PRId64 ".",
        __func__, matchedFormId);
        return false;
    }

    // If the form need refrsh flag is true and form visibleType is FORM_VISIBLE, refresh the form host.
    if (formRecord.needRefresh && formVisibleType == Constants::FORM_VISIBLE) {
        std::string cacheData;
        std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageMap;
        // If the form has business cache, refresh the form host.
        if (FormCacheMgr::GetInstance().GetData(matchedFormId, cacheData, imageMap)) {
            formRecord.formProviderInfo.SetFormDataString(cacheData);
            formRecord.formProviderInfo.SetImageDataMap(imageMap);
            formHostRecord.OnUpdate(matchedFormId, formRecord);
        }
    }
    return true;
}
/**
 * @brief If the form provider is system app and the config item 'formVisibleNotify' is true,
 *        notify the form provider that the current form is visible.
 *
 * @param bundleName BundleName
 * @return Returns true if the form provider is system app, false if not.
 */
bool FormMgrAdapter::CheckIsSystemAppByBundleName(const sptr<IBundleMgr> &iBundleMgr, const std::string &bundleName)
{
    BundleInfo bundleInfo;
    if (IN_PROCESS_CALL(iBundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT,
        bundleInfo, FormUtil::GetCurrentAccountId()))) {
        HILOG_DEBUG("%{public}s, get bundle uid success", __func__);
        if (!IN_PROCESS_CALL(iBundleMgr->CheckIsSystemAppByUid(bundleInfo.uid))) {
            HILOG_WARN("%{public}s fail, form provider is not system app, bundleName: %{public}s",
                __func__, bundleName.c_str());
            return false;
        }
    } else {
        HILOG_WARN("%{public}s fail, can not get bundleInfo's uid", __func__);
        return false;
    }

    return true;
}

/**
 * @brief  Add forms to storage for st .
 * @param Want The Want of the form to add.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DistributedDataAddForm(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);

    FormDBInfo formDBInfo;
    ElementName elementName = want.GetElement();
    formDBInfo.formId = want.GetIntParam(Constants::PARAM_FORM_ADD_COUNT, 0);
    formDBInfo.formName = want.GetStringParam(Constants::PARAM_FORM_NAME_KEY);
    formDBInfo.bundleName = elementName.GetBundleName();
    formDBInfo.moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);
    formDBInfo.abilityName = elementName.GetAbilityName();
    formDBInfo.formUserUids.push_back(HUNDRED);
    return FormDbCache::GetInstance().SaveFormInfo(formDBInfo);
}

/**
 * @brief  Delete form form storage for st.
 * @param formId The formId of the form to delete.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DistributedDataDeleteForm(const std::string &formId)
{
    return FormDbCache::GetInstance().DeleteFormInfo(std::stoll(formId));
}
/**
 * @brief Get current user ID.
 * @param callingUid calling Uid.
 * @return Returns user ID.
 */
int32_t FormMgrAdapter::GetCurrentUserId(const int callingUid)
{
    // get current userId
    int32_t userId = callingUid / UID_CALLINGUID_TRANSFORM_DIVISOR;
    return userId;
}

/**
 * @brief Delete the invalid forms.
 * @param formIds Indicates the ID of the valid forms.
 * @param callerToken Caller ability token.
 * @param numFormsDeleted Returns the number of the deleted forms.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::DeleteInvalidForms(const std::vector<int64_t> &formIds, const sptr<IRemoteObject> &callerToken,
                                       int32_t &numFormsDeleted)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s, deleteForm invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::set<int64_t> matchedFormIds {};
    for (int64_t formId: formIds) {
        int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
        matchedFormIds.emplace(matchedFormId);
        HILOG_DEBUG("valid formId, matchedFormIds: %{public}" PRId64 "", formId);
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();

    std::map<int64_t, bool> removedFormsMap {};
    int32_t userId = GetCurrentUserId(callingUid);

    // delete invalid DB form record
    FormDbCache::GetInstance().DeleteInvalidDBForms(userId, callingUid, matchedFormIds, removedFormsMap);
    // delete invalid temp form record
    FormDataMgr::GetInstance().DeleteInvalidTempForms(userId, callingUid, matchedFormIds, removedFormsMap);

    if (!removedFormsMap.empty()) {
        FormDataMgr::GetInstance().ClearHostDataByInvalidForms(callingUid, removedFormsMap);
        // delete forms timer
        for (const auto &removedForm: removedFormsMap) {
            if (removedForm.second) {
                FormTimerMgr::GetInstance().RemoveFormTimer(removedForm.first);
            }
        }
    }

    numFormsDeleted = (int32_t) removedFormsMap.size();
    HILOG_INFO("%{public}s done, %{public}d forms deleted.", __func__, numFormsDeleted);
    return ERR_OK;
}

/**
 * @brief AcquireFormState want check.
 * @param bundleName The bundle name of the form.
 * @param abilityName The ability name of the form.
 * @param want The want of the form.
 * @param provider the provider info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormMgrAdapter::AcquireFormStateCheck(const std::string &bundleName, const std::string &abilityName,
                                              const Want &want, std::string &provider)
{
    if (bundleName.empty()) {
        HILOG_ERROR("%{public}s error, bundleName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    if (abilityName.empty()) {
        HILOG_ERROR("%{public}s error, abilityName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::string moduleName = want.GetStringParam(AppExecFwk::Constants::PARAM_MODULE_NAME_KEY);
    std::string formName = want.GetStringParam(AppExecFwk::Constants::PARAM_FORM_NAME_KEY);
    int32_t dimensionId = want.GetIntParam(AppExecFwk::Constants::PARAM_FORM_DIMENSION_KEY, 1);

    if (moduleName.empty()) {
        HILOG_ERROR("%{public}s error, moduleName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    if (formName.empty()) {
        HILOG_ERROR("%{public}s error, formName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::vector<FormInfo> formInfos {};
    ErrCode errCode = FormInfoMgr::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
    if (errCode != ERR_OK) {
        HILOG_ERROR("%{public}s error, failed to get forms info.", __func__);
        return errCode;
    }

    bool found = false;
    for (auto &formInfo: formInfos) {
        if ((formInfo.abilityName == abilityName) && (formInfo.name == formName) &&
            (IsDimensionValid(formInfo, dimensionId))) {
            found = true;
            HILOG_INFO("%{public}s form info found.", __func__);
            break;
        }
    }
    if (!found) {
        HILOG_INFO("failed to find match form info.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    const std::string doubleColon = "::";
    provider.append(bundleName).append(doubleColon).append(abilityName).append(doubleColon)
        .append(moduleName).append(doubleColon).append(formName).append(doubleColon)
        .append(std::to_string(dimensionId)).append(doubleColon).append(std::to_string(callingUid));
    return ERR_OK;
}

/**
 * @brief Acquire form state info by passing a set of parameters (using Want) to the form provider.
 * @param want Indicates a set of parameters to be transparently passed to the form provider.
 * @param callerToken Caller ability token.
 * @param stateInfo Returns the form's state info of the specify.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::AcquireFormState(const Want &want, const sptr<IRemoteObject> &callerToken,
                                     FormStateInfo &stateInfo)
{
    if (callerToken == nullptr) {
        HILOG_ERROR("%{public}s, deleteForm invalid param", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();

    std::string provider;
    ErrCode errCode = AcquireFormStateCheck(bundleName, abilityName, want, provider);
    if (errCode != ERR_OK) {
        return errCode;
    }

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    FormItemInfo info;
    FormDataMgr::GetInstance().CreateFormStateRecord(provider, info, callerToken, callingUid);

    HILOG_DEBUG("bundleName:%{public}s, abilityName:%{public}s", bundleName.c_str(), abilityName.c_str());
    sptr<IAbilityConnection> acquireFormStateConnection = new FormAcquireStateConnection(bundleName, abilityName, want,
        provider);

    Want targetWant;
    targetWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    targetWant.SetElementName(bundleName, abilityName);
    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(targetWant, acquireFormStateConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    stateInfo.state = FormState::DEFAULT;
    return ERR_OK;
}

/**
 * @brief Notify the form is visible or not.
 * @param formIds Indicates the ID of the forms.
 * @param isVisible Visible or not.
 * @param callerToken Host client.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::NotifyFormsVisible(const std::vector<int64_t> &formIds, bool isVisible,
                                       const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, isVisible: %{public}d.", __func__, isVisible);
    return FormDataMgr::GetInstance().NotifyFormsVisible(formIds, isVisible, callerToken);
}

/**
 * @brief Notify the form is enable to be updated or not.
 * @param formIds Indicates the ID of the forms.
 * @param isEnableUpdate enable update or not.
 * @param callerToken Host client.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::NotifyFormsEnableUpdate(const std::vector<int64_t> &formIds, bool isEnableUpdate,
                                            const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, isEnableUpdate: %{public}d.", __func__, isEnableUpdate);
    return HandleUpdateFormFlag(formIds, callerToken, isEnableUpdate, true);
}

/**
 * @brief Get All FormsInfo.
 * @param formInfos Return the forms' information of all forms provided.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    return FormInfoMgr::GetInstance().GetAllFormsInfo(formInfos);
}

/**
 * @brief Get forms info by bundle name .
 * @param bundleName Application name.
 * @param formInfos Return the forms' information of the specify application name.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    return FormInfoMgr::GetInstance().GetFormsInfoByBundle(bundleName, formInfos);
}

/**
 * @brief Get forms info by bundle name and module name.
 * @param bundleName bundle name.
 * @param moduleName Module name of hap.
 * @param formInfos Return the forms' information of the specify bundle name and module name.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::GetFormsInfoByModule(std::string &bundleName, std::string &moduleName,
                                         std::vector<FormInfo> &formInfos)
{
    return FormInfoMgr::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
}

/**
 * @brief Update action string for router event.
 * @param formId Indicates the unique id of form.
 * @param action Indicates the origin action string.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormMgrAdapter::UpdateRouterAction(const int64_t formId, std::string &action)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0) {
        HILOG_ERROR("%{public}s form formId or bundleName is invalid", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    int64_t matchedFormId = FormDataMgr::GetInstance().FindMatchedFormId(formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(matchedFormId, record);
    if (!bGetRecord) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 "", __func__, matchedFormId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("%{public}s fail, failed to get IBundleMgr.", __func__);
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    if (CheckIsSystemAppByBundleName(iBundleMgr, record.bundleName)) {
        return ERR_OK;
    }

    if (action.empty()) {
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    nlohmann::json actionObject = nlohmann::json::parse(action, nullptr, false);
    if (actionObject.is_discarded()) {
        HILOG_ERROR("failed to parse jsonDataString: %{public}s.", action.c_str());
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    actionObject["bundleName"] = record.bundleName;
    action = actionObject.dump();
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
