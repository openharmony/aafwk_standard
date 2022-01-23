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

    std::shared_ptr<BundleMgrClient> client = DelayedSingleton<BundleMgrClient>::GetInstance();
    if (client == nullptr) {
        APP_LOGE("failed to get BundleMgrClient.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    BundleInfo bundleInfo;
    if (!client->GetBundleInfo(bundleName, GET_BUNDLE_WITH_EXTENSION_INFO, bundleInfo)) {
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
        if (!iBundleMgr->GetFormsInfoByModule(bundleName, moduleName, formInfoVec)) {
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

ErrCode BundleFormInfo::Update()
{
    std::unique_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    ErrCode errCode = FormInfoHelper::LoadFormConfigInfoByBundleName(bundleName_, formInfos_);
    return errCode;
}

ErrCode BundleFormInfo::Remove()
{
    std::unique_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    formInfos_.clear();
    return ERR_OK;
}

bool BundleFormInfo::Empty()
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    return formInfos_.empty();
}

ErrCode BundleFormInfo::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    for (const auto &formInfo : formInfos_) {
        formInfos.push_back(formInfo);
    }
    return ERR_OK;
}

ErrCode BundleFormInfo::GetFormsInfoByModule(const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    std::shared_lock<std::shared_timed_mutex> guard(formInfosMutex_);
    for (const auto &formInfo : formInfos_) {
        if (formInfo.moduleName == moduleName) {
            formInfos.push_back(formInfo);
        }
    }
    return ERR_OK;
}

FormInfoMgr::FormInfoMgr() = default;

FormInfoMgr::~FormInfoMgr() = default;

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
    std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    for (const auto &bundleFormInfo: bundleFormInfoMap_) {
        if (bundleFormInfo.second != nullptr) {
            bundleFormInfo.second->GetAllFormsInfo(formInfos);
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

    std::shared_lock<std::shared_timed_mutex> guard(bundleFormInfoMapMutex_);
    auto bundleFormInfoIter = bundleFormInfoMap_.find(bundleName);
    if (bundleFormInfoIter == bundleFormInfoMap_.end()) {
        APP_LOGE("no forms found.");
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
}  // namespace AppExecFwk
}  // namespace OHOS
