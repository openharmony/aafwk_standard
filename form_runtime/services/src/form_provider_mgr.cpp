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

#include "appexecfwk_errors.h"
#include "form_ams_helper.h"
#include "form_batch_delete_connection.h"
#include "form_cache_mgr.h"
#include "form_constants.h"
#include "form_data_mgr.h"
#include "form_delete_connection.h"
#include "form_mgr_errors.h"
#include "form_msg_event_connection.h"
#include "form_provider_mgr.h"
#include "form_record.h"
#include "form_refresh_connection.h"
#include "form_timer_mgr.h"
#include "hilog_wrapper.h"
#include "power_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
FormProviderMgr::FormProviderMgr(){}
FormProviderMgr::~FormProviderMgr(){}
/**
 * @brief handle for acquire back from ams.
 * @param formId The id of the form.
 * @param formProviderInfo provider form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::AcquireForm(const int64_t formId, const FormProviderInfo &formProviderInfo)
{
    HILOG_DEBUG("%{public}s start, formId:%{public}" PRId64 "", __func__, formId);

    if (formId <= 0) {
        HILOG_ERROR("%{public}s fail, formId should be greater than 0", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    FormRecord formRecord;
    bool isGetFormRecord = FormDataMgr::GetInstance().GetFormRecord(formId, formRecord);
    if (!isGetFormRecord) {
        HILOG_ERROR("%{public}s fail, not exist such form, formId:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    FormHostRecord clientHost;
    bool isGetFormHostRecord = FormDataMgr::GetInstance().GetFormHostRecord(formId, clientHost);
    if (!isGetFormHostRecord) {
        HILOG_ERROR("%{public}s fail, clientHost is null", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    if (formRecord.isInited) {
        if (IsFormCached(formRecord)) {
            if (clientHost.Contains(formId)) {
                formRecord.formProviderInfo = formProviderInfo;
                clientHost.OnAcquire(formId, formRecord);
            }
        } else {
            Want want;
            RefreshForm(formId, want, true);
        }
        return ERR_OK;
    }
    formRecord.isInited = true;
    formRecord.needRefresh = false;
    FormDataMgr::GetInstance().SetFormCacheInited(formId, true);

    if (clientHost.Contains(formId)) {
        formRecord.formProviderInfo = formProviderInfo;
        clientHost.OnAcquire(formId, formRecord);
    }

    // we do not cache when data size is over 1k
    std::string jsonData = formProviderInfo.GetFormDataString(); // get json data
    HILOG_DEBUG("%{public}s , jsonData is %{public}s.",  __func__, jsonData.c_str());

    std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageDataMap = formProviderInfo.GetImageDataMap();
    if (jsonData.size() <= Constants::MAX_FORM_DATA_SIZE) {
        HILOG_WARN("%{public}s, acquire js card, cache the card", __func__);
        FormCacheMgr::GetInstance().AddData(formId, formProviderInfo.GetFormDataString(),
            formProviderInfo.GetImageDataMap());
    }
    return ERR_OK;
}

/**
 * @brief Refresh form.
 *
 * @param formId The form id.
 * @param want The want of the form to request.
 * @param isVisibleToFresh The form is visible to fresh.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::RefreshForm(const int64_t formId, const Want &want, bool isVisibleToFresh)
{
    HILOG_INFO("%{public}s called, formId:%{public}" PRId64 ".", __func__, formId);
    FormRecord record;
    bool bGetRecord = FormDataMgr::GetInstance().GetFormRecord(formId, record);
    if (!bGetRecord) {
        HILOG_ERROR("%{public}s fail, not exist such form:%{public}" PRId64 "", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }

    // get current userId
    int32_t currentUserId = want.GetIntParam(Constants::PARAM_FORM_USER_ID, Constants::DEFAULT_USER_ID);
    HILOG_INFO("%{public}s, current user, userId:%{public}d", __func__, currentUserId);
    if (currentUserId != record.userId) {
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        HILOG_ERROR("%{public}s, not current user, just set refresh flag, userId:%{public}d", __func__, record.userId);
        return ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF;
    }

    bool isTimerRefresh = want.GetBoolParam(Constants::KEY_IS_TIMER, false);
    Want newWant(want);
    newWant.RemoveParam(Constants::KEY_IS_TIMER);

    if (isTimerRefresh) {
        FormDataMgr::GetInstance().SetCountTimerRefresh(formId, true);
    }

    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (!screenOnFlag) {
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        HILOG_ERROR("%{public}s fail, screen off, set refresh flag, do not refresh now", __func__);
        return ERR_OK;
    }

    bool needRefresh = IsNeedToFresh(record, formId, isVisibleToFresh);
    if (!needRefresh) {
        FormDataMgr::GetInstance().SetNeedRefresh(formId, true);
        HILOG_ERROR("%{public}s fail, no one needReresh, set refresh flag, do not refresh now", __func__);
        return ERR_OK;
    }

    FormRecord refreshRecord = GetFormAbilityInfo(record);
    refreshRecord.isInited = record.isInited;
    refreshRecord.versionUpgrade = record.versionUpgrade;
    refreshRecord.isCountTimerRefresh = isTimerRefresh;
    return ConnectAmsForRefresh(formId, refreshRecord, newWant, isTimerRefresh);
}

/**
 * @brief Connect ams for refresh form
 *
 * @param formId The form id.
 * @param record Form data.
 * @param want The want of the form.
 * @param isTimerRefresh The flag of timer refresh.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::ConnectAmsForRefresh(const int64_t formId,
    const FormRecord &record, const Want &want, const bool isTimerRefresh)
{
    HILOG_DEBUG("%{public}s called, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, record.bundleName.c_str(), record.abilityName.c_str());

    sptr<IAbilityConnection> formRefreshConnection = new FormRefreshConnection(formId, want,
        record.bundleName, record.abilityName);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    connectWant.SetElementName(record.bundleName, record.abilityName);

    if (isTimerRefresh) {
        if (!FormTimerMgr::GetInstance().IsLimiterEnableRefresh(formId)) {
            HILOG_ERROR("%{public}s, timer refresh, already limit.", __func__);
            return ERR_APPEXECFWK_FORM_PROVIDER_DEL_FAIL;
        }
    }

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formRefreshConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    if (record.isCountTimerRefresh) {
        IncreaseTimerRefreshCount(formId);
    }

    return ERR_OK;
}

/**
 * @brief Notify provider form delete.
 * @param formId The form id.
 * @param record Form information.
 * @return Function result and has other host flag.
 */
ErrCode FormProviderMgr::NotifyProviderFormDelete(const int64_t formId, const FormRecord &formRecord)
{
    if (formRecord.abilityName.empty()) {
        HILOG_ERROR("%{public}s, formRecord.abilityName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (formRecord.bundleName.empty()) {
        HILOG_ERROR("%{public}s, formRecord.bundleName is empty.", __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_DEBUG("%{public}s, connectAbility,bundleName:%{public}s, abilityName:%{public}s",
        __func__, formRecord.bundleName.c_str(), formRecord.abilityName.c_str());
    sptr<IAbilityConnection> formDeleteConnection = new FormDeleteConnection(formId,
        formRecord.bundleName, formRecord.abilityName);
    Want want;
    want.SetElementName(formRecord.bundleName, formRecord.abilityName);
    want.SetFlags(Want::FLAG_ABILITY_FORM_ENABLED);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, formDeleteConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ERR_OK;
}

/**
 * @brief Notify provider forms batch delete.
 * @param bundleName BundleName.
 * @param bundleName AbilityName.
 * @param formIds form id list.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::NotifyProviderFormsBatchDelete(const std::string &bundleName,
    const std::string &abilityName, const std::set<int64_t> &formIds)
{
    if (abilityName.empty()) {
        HILOG_ERROR("%{public}s error, abilityName is empty.",  __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (bundleName.empty()) {
        HILOG_ERROR("%{public}s error, bundleName is empty.",  __func__);
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    HILOG_DEBUG("%{public}s, bundleName:%{public}s, abilityName:%{public}s",
        __func__, bundleName.c_str(), abilityName.c_str());
    sptr<IAbilityConnection> batchDeleteConnection = new FormBatchDeleteConnection(formIds, bundleName, abilityName);
    Want want;
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    want.SetElementName(bundleName, abilityName);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, batchDeleteConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    return ERR_OK;
}
/**
 * @brief Update form.
 * @param formId The form's id.

 * @param formProviderData form provider data.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::UpdateForm(const int64_t formId, const FormProviderInfo &formProviderInfo)
{
    // check exist and get the formRecord
    FormRecord formRecord;
    if (!FormDataMgr::GetInstance().GetFormRecord(formId, formRecord)) {
        HILOG_ERROR("%{public}s error, not exist such form:%{public}" PRId64 ".", __func__, formId);
        return ERR_APPEXECFWK_FORM_NOT_EXIST_ID;
    }
    return UpdateForm(formId, formRecord, formProviderInfo.GetFormData());
}
/**
 * handle for update form event from provider.
 *
 * @param formId The id of the form.
 * @param formRecord The form's record.
 * @param formProviderData provider form info.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::UpdateForm(const int64_t formId,
    FormRecord &formRecord, const FormProviderData &formProviderData)
{
    HILOG_INFO("%{public}s start, imageDateState is %{public}d", __func__, formProviderData.GetImageDataState());
    if (formRecord.versionUpgrade) {
        formRecord.formProviderInfo.SetFormData(formProviderData);
        formRecord.formProviderInfo.SetUpgradeFlg(true);
    } else {
        nlohmann::json addJsonData = formProviderData.GetData();
        formRecord.formProviderInfo.MergeData(addJsonData);
        // merge image
        auto formData = formRecord.formProviderInfo.GetFormData();
        formData.SetImageDataState(formProviderData.GetImageDataState());
        formData.SetImageDataMap(formProviderData.GetImageDataMap());
        formRecord.formProviderInfo.SetFormData(formData);
    }

    // formRecord init
    formRecord.isInited = true;
    formRecord.needRefresh = false;
    FormDataMgr::GetInstance().SetFormCacheInited(formId, true);

    // update form for host clients
    FormDataMgr::GetInstance().UpdateHostNeedRefresh(formId, true);

    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (screenOnFlag) {
        if (FormDataMgr::GetInstance().UpdateHostForm(formId, formRecord)) {
            FormDataMgr::GetInstance().SetVersionUpgrade(formId, false);
            formRecord.formProviderInfo.SetUpgradeFlg(false);
        }
    }
    // check if cache data size is less than 1k or not
    std::string jsonData = formRecord.formProviderInfo.GetFormDataString(); // get json data
    HILOG_DEBUG("%{public}s screenOn:%{public}d jsonData:%{public}s.", __func__, screenOnFlag, jsonData.c_str());

    std::map<std::string, std::pair<sptr<FormAshmem>, int32_t>> imageDataMap = formProviderData.GetImageDataMap();
    // check if cache data size is less than 1k or not
    if (jsonData.size() <= Constants::MAX_FORM_DATA_SIZE) {
        HILOG_INFO("%{public}s, updateJsForm, data is less than 1k, cache data.", __func__);
        FormCacheMgr::GetInstance().AddData(formId, jsonData, formProviderData.GetImageDataMap());
    } else {
        FormCacheMgr::GetInstance().DeleteData(formId);
    }

    // the update form is successfully
    return ERR_OK;
}
/**
 * @brief Process js message event.
 * @param formId Indicates the unique id of form.
 * @param record Form record.
 * @param want information passed to supplier.
 * @return Returns true if execute success, false otherwise.
 */
int FormProviderMgr::MessageEvent(const int64_t formId, const FormRecord &record, const Want &want)
{
    HILOG_INFO("%{public}s called, formId:%{public}" PRId64 ".", __func__, formId);

    bool screenOnFlag = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    if (!screenOnFlag) {
        HILOG_WARN("%{public}s fail, screen off now", __func__);
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    sptr<IAbilityConnection> formMsgEventConnection = new FormMsgEventConnection(formId, want,
        record.bundleName, record.abilityName);
    Want connectWant;
    connectWant.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    connectWant.SetElementName(record.bundleName, record.abilityName);

    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(connectWant, formMsgEventConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s, ConnectServiceAbility failed.", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    return ERR_OK;
}

/**
 * @brief Increase the timer refresh count.
 *
 * @param formId The form id.
 */
void FormProviderMgr::IncreaseTimerRefreshCount(const int64_t formId)
{
    FormRecord record;
    if (!FormDataMgr::GetInstance().GetFormRecord(formId, record)) {
        HILOG_ERROR("%{public}s failed, not exist such form:%{public}" PRId64 ".", __func__, formId);
        return;
    }

    if (record.isCountTimerRefresh) {
        FormDataMgr::GetInstance().SetCountTimerRefresh(formId, false);
        FormTimerMgr::GetInstance().IncreaseRefreshCount(formId);
    }
}

/**
 * @brief Acquire form state.
 * @param state form state.
 * @param provider provider info.
 * @param wantArg The want of onAcquireFormState.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode FormProviderMgr::AcquireFormStateBack(FormState state, const std::string& provider, const Want &wantArg)
{
    HILOG_DEBUG("AcquireFormState start: %{public}d, provider: %{public}s", state, provider.c_str());
    FormDataMgr::GetInstance().AcquireFormStateBack(state, provider, wantArg);
    return ERR_OK;
}

bool FormProviderMgr::IsNeedToFresh(FormRecord &record, int64_t formId, bool isVisibleToFresh)
{
    bool isEnableRefresh = FormDataMgr::GetInstance().IsEnableRefresh(formId);
    HILOG_DEBUG("isEnableRefresh is %{public}d", isEnableRefresh);
    if (isEnableRefresh) {
        return true;
    }

    HILOG_DEBUG("isVisibleToFresh is %{public}d, record.isVisible is %{public}d", isVisibleToFresh, record.isVisible);
    if (isVisibleToFresh) {
        return record.isVisible;
    }

    bool isEnableUpdate = FormDataMgr::GetInstance().IsEnableUpdate(formId);
    HILOG_DEBUG("isEnableUpdate is %{public}d", isEnableUpdate);
    return isEnableUpdate;
}

FormRecord FormProviderMgr::GetFormAbilityInfo(const FormRecord &record) const
{
    FormRecord newRecord;
    newRecord.bundleName = record.bundleName;
    newRecord.abilityName = record.abilityName;

    return newRecord;
}

bool FormProviderMgr::IsFormCached(const FormRecord &record)
{
    if (record.versionUpgrade) {
        return false;
    }
    return FormCacheMgr::GetInstance().IsExist(record.formId);
}
}  // namespace AppExecFwk
}  // namespace OHOS