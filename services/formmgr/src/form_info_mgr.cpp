/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")_;
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

#include "form_info_mgr.h"

#include <utility>

#include "app_log_wrapper.h"
#include "bundle_mgr_client.h"
#include "extension_form_profile.h"
#include "form_bms_helper.h"
#include "form_info_storage_mgr.h"
#include "form_util.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "json_serializer.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string FORM_METADATA_NAME = "ohos.extension.form";
} // namespace

ErrCode FormInfoHelper::LoadFormConfigInfoByBundleName(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName is invalid");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("GetBundleMgr, failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(iBundleMgr->GetBundleInfo(bundleName, GET_BUNDLE_WITH_EXTENSION_INFO,
        bundleInfo, FormUtil::GetCurrentAccountId()))) {
        APP_LOGE("failed to get bundle info.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    ErrCode errCode = LoadAbilityFormConfigInfo(bundleInfo, formInfos);
    if (errCode != ERR_OK) {
        APP_LOGE("failed to load FA form config info, error code=%{public}d.", errCode);
    }

    errCode = LoadStageFormConfigInfo(bundleInfo, formInfos);
    if (errCode != ERR_OK) {
        APP_LOGE("failed to load stage form config info, error code=%{public}d.", errCode);
    }

    return ERR_OK;
}

ErrCode FormInfoHelper::LoadStageFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos)
{
    std::shared_ptr<BundleMgrClient> client = DelayedSingleton<BundleMgrClient>::GetInstance();
    if (client == nullptr) {
        APP_LOGE("failed to get BundleMgrClient.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    for (auto const &extensionInfo: bundleInfo.extensionInfos) {
        if (extensionInfo.type != ExtensionAbilityType::FORM) {
            continue;
        }

        std::vector<std::string> profileInfos {};
        if (!client->GetResConfigFile(extensionInfo, FORM_METADATA_NAME, profileInfos)) {
            APP_LOGE("failed to get form metadata.");
            continue;
        }

        for (const auto &profileInfo: profileInfos) {
            std::vector<ExtensionFormInfo> extensionFormInfos;
            ErrCode errCode = ExtensionFormProfile::TransformTo(profileInfo, extensionFormInfos);
            if (errCode != ERR_OK) {
                APP_LOGW("failed to transform profile to extension form info");
                continue;
            }
            for (const auto &extensionFormInfo: extensionFormInfos) {
                formInfos.emplace_back(extensionInfo, extensionFormInfo);
            }
        }
    }
    return ERR_OK;
}

ErrCode FormInfoHelper::LoadAbilityFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos)
{
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    const std::string &bundleName = bundleInfo.name;
    for (const auto &modelInfo: bundleInfo.hapModuleInfos) {
        const std::string &moduleName = modelInfo.moduleName;
        std::vector<FormInfo> formInfoVec {};
        if (!IN_PROCESS_CALL(iBundleMgr->GetFormsInfoByModule(bundleName, moduleName, formInfoVec))) {
            continue;
        }
        for (const auto &formInfo: formInfoVec) {
            // check form info
            formInfos.push_back(formInfo);
        }
    }

    return ERR_OK;
}

BundleFormInfo::BundleFormInfo(std::string bundleName) : bundleName_(std::move(bundleName))
{
}

ErrCode BundleFormInfo::InitFromJson(const std::string &formInfosJson)
{
    nlohmann::json jsonObject = nlohmann::json::parse(formInfosJson, nullptr, false);
    if (jsonObject.is_discarded()) {
        APP_LOGE("bad profile");
        return ERR_APPEXECFWK_PARSE_BAD_PROFILE;
    }
    std::unique_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    formInfos_ = jsonObject.get<std::vector<FormInfo>>();
    return ERR_OK;
}

ErrCode BundleFormInfo::Update()
{
    std::unique_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    ErrCode errCode = FormInfoHelper::LoadFormConfigInfoByBundleName(bundleName_, formInfos_);
    if (errCode != ERR_OK) {
        return errCode;
    }
    if (formInfos_.empty()) {
        return ERR_OK;
    }

    nlohmann::json jsonObject = formInfos_;
    if (jsonObject.is_discarded()) {
        APP_LOGE("bad form infos.");
        return ERR_APPEXECFWK_PARSE_BAD_PROFILE;
    }
    std::string formInfoStr = jsonObject.dump(Constants::DUMP_INDENT);
    errCode = FormInfoStorageMgr::GetInstance().UpdateBundleFormInfos(bundleName_, formInfoStr);
    return errCode;
}

ErrCode BundleFormInfo::Remove()
{
    std::unique_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    formInfos_.clear();
    ErrCode errCode = FormInfoStorageMgr::GetInstance().RemoveBundleFormInfos(bundleName_);
    return errCode;
}

bool BundleFormInfo::Empty()
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    return formInfos_.empty();
}

ErrCode BundleFormInfo::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    for (const auto &formInfo: formInfos_) {
        formInfos.push_back(formInfo);
    }
    return ERR_OK;
}

ErrCode BundleFormInfo::GetFormsInfoByModule(const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    for (const auto &formInfo: formInfos_) {
        if (formInfo.moduleName == moduleName) {
            formInfos.push_back(formInfo);
        }
    }
    return ERR_OK;
}

FormInfoMgr::FormInfoMgr()
{
    APP_LOGI("FormInfoMgr is created");
}

FormInfoMgr::~FormInfoMgr() = default;

ErrCode FormInfoMgr::Start()
{
    std::vector<std::pair<std::string, std::string>> formInfos;
    ErrCode errCode = FormInfoStorageMgr::GetInstance().LoadFormInfos(formInfos);
    if (errCode != ERR_OK) {
        APP_LOGE("LoadFormData failed.");
        return errCode;
    }

    std::unique_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    for (const auto &item: formInfos) {
        const std::string &bundleName = item.first;
        const std::string &formInfosJson = item.second;
        auto bundleFormInfoPtr = std::make_shared<BundleFormInfo>(bundleName);
        errCode = bundleFormInfoPtr->InitFromJson(formInfosJson);
        if (errCode != ERR_OK) {
            continue;
        }
        APP_LOGE("load bundle %{public}s form infos success.", bundleName.c_str());
        bundleFormInfoMap_[bundleName] = bundleFormInfoPtr;
    }
    APP_LOGI("load bundle form infos from db done.");
    return ERR_OK;
}

ErrCode FormInfoMgr::Update(const std::string &bundleName)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    auto bundleFormInfoPtr = std::make_shared<BundleFormInfo>(bundleName);
    ErrCode errCode = bundleFormInfoPtr->Update();
    if (errCode != ERR_OK) {
        return errCode;
    }

    if (bundleFormInfoPtr->Empty()) {
        // no forms found, no need to be inserted into the map
        return ERR_OK;
    }

    std::unique_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    bundleFormInfoMap_[bundleName] = bundleFormInfoPtr;
    APP_LOGE("update forms info success, bundleName=%{public}s.", bundleName.c_str());
    return ERR_OK;
}

ErrCode FormInfoMgr::Remove(const std::string &bundleName)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    std::unique_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
    if (bundleFormInfoIter == bundleFormInfoMap_.end()) {
        // BundleFormInfo not found, no need to remove
        return ERR_OK;
    }

    ErrCode errCode = ERR_OK;
    if (bundleFormInfoIter->second != nullptr) {
        errCode = bundleFormInfoIter->second->Remove();
    }
    bundleFormInfoMap_.erase(bundleFormInfoIter);
    APP_LOGE("remove forms info success, bundleName=%{public}s.", bundleName.c_str());
    return errCode;
}

ErrCode FormInfoMgr::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    bool hasPermission = CheckBundlePermission();
    std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    if (hasPermission) {
        for (const auto &bundleFormInfo: bundleFormInfoMap_) {
            if (bundleFormInfo.second != nullptr) {
                bundleFormInfo.second->GetAllFormsInfo(formInfos);
            }
        }
    } else {
        for (const auto &bundleFormInfo: bundleFormInfoMap_) {
            if (IsCaller(bundleFormInfo.first)) {
                if (bundleFormInfo.second != nullptr) {
                    bundleFormInfo.second->GetAllFormsInfo(formInfos);
                }
                return ERR_OK;
            }
        }
    }
    return ERR_OK;
}

ErrCode FormInfoMgr::GetFormsInfoByBundle(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (!CheckBundlePermission() && !IsCaller(bundleName)) {
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
    if (bundleFormInfoIter == bundleFormInfoMap_.end()) {
        APP_LOGE("no forms found.");
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    if (bundleFormInfoIter->second != nullptr) {
        bundleFormInfoIter->second->GetAllFormsInfo(formInfos);
    }
    return ERR_OK;
}

ErrCode FormInfoMgr::GetFormsInfoByModule(const std::string &bundleName, const std::string &moduleName,
                                          std::vector<FormInfo> &formInfos)
{
    if (bundleName.empty()) {
        APP_LOGE("bundleName is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    if (!CheckBundlePermission() && !IsCaller(bundleName)) {
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
    if (bundleFormInfoIter == bundleFormInfoMap_.end()) {
        APP_LOGE("no forms found for %{public}s.", bundleName.c_str());
        return ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED;
    }

    if (bundleFormInfoIter->second != nullptr) {
        bundleFormInfoIter->second->GetFormsInfoByModule(moduleName, formInfos);
    }
    return ERR_OK;
}

std::shared_ptr<BundleFormInfo> FormInfoMgr::GetOrCreateBundleFromInfo(const std::string &bundleName)
{
    {
        std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
        auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
        if (bundleFormInfoIter != bundleFormInfoMap_.end()) {
            // found
            return bundleFormInfoIter->second;
        }
    }

    // not found
    std::unique_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
    // try to find again
    if (bundleFormInfoIter != bundleFormInfoMap_.end()) {
        // found
        return bundleFormInfoIter->second;
    }
    auto bundleFormInfoPtr = std::make_shared<BundleFormInfo>(bundleName);
    bundleFormInfoMap_[bundleName] = bundleFormInfoPtr;
    return bundleFormInfoPtr;
}

bool FormInfoMgr::IsCaller(std::string bundleName)
{
    auto bms = FormBmsHelper::GetInstance().GetBundleMgr();
    if (!bms) {
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    bool ret = IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, GET_BUNDLE_DEFAULT, bundleInfo, FormUtil::GetCurrentAccountId()));
    if (!ret) {
        APP_LOGE("Failed to get bundle info.");
        return false;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (bundleInfo.applicationInfo.accessTokenId == callerToken) {
        return true;
    }
    return false;
}

bool FormInfoMgr::CheckBundlePermission()
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return true;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AppExecFwk::Constants::PERMISSION_GET_BUNDLE_INFO_PRIVILEGED);
    if (isCallingPerm) {
        return true;
    }
    APP_LOGE("Permission verification failed");
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
