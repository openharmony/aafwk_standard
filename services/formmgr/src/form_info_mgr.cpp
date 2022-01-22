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

#include "app_log_wrapper.h"
#include "bundle_mgr_client.h"
#include "extension_form_profile.h"
#include "form_bms_helper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string FORM_METADATA_NAME = "ohos.extension.form";
} // namespace

ErrCode FormInfoMgr::LoadFormConfigInfoByBundleName(const std::string &bundleName, std::vector<FormInfo> &formInfos)
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

ErrCode FormInfoMgr::LoadStageFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos)
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

        std::vector<std::string> profileInfos{};
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

ErrCode FormInfoMgr::LoadAbilityFormConfigInfo(const BundleInfo &bundleInfo, std::vector<FormInfo> &formInfos)
{
    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        APP_LOGE("failed to get IBundleMgr.");
        return ERR_APPEXECFWK_FORM_GET_BMS_FAILED;
    }

    const std::string &bundleName = bundleInfo.name;
    for (const auto &modelInfo: bundleInfo.hapModuleInfos) {
        const std::string &moduleName = modelInfo.moduleName;
        std::vector<FormInfo> formInfoVec{};
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

}  // namespace AppExecFwk
}  // namespace OHOS
