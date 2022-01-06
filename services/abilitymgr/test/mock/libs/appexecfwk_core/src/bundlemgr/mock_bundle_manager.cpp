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

#include "mock_bundle_manager.h"
#include "clean_cache_callback_interface.h"
#include "ability_info.h"
#include "application_info.h"
#include "hilog_wrapper.h"
#include "ability_config.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AAFwk;

int BundleMgrProxy::QueryWantAbility(
    const AAFwk::Want &__attribute__((unused)) want, std::vector<AbilityInfo> &__attribute__((unused)) abilityInfos)
{
    return 0;
}

bool BundleMgrProxy::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    ElementName eleName = want.GetElement();
    if (eleName.GetBundleName().empty()) {
        return false;
    }
    abilityInfo.name = eleName.GetAbilityName();
    abilityInfo.bundleName = eleName.GetBundleName();
    abilityInfo.applicationName = eleName.GetAbilityName() + "App";
    abilityInfo.visible = true;
    if (abilityInfo.bundleName != "com.ix.hiworld") {
        abilityInfo.applicationInfo.isLauncherApp = false;
    }
    return true;
}

bool BundleMgrProxy::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ix.hiworld";
    return true;
}

bool BundleMgrProxy::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
{
    GTEST_LOG_(INFO) << "BundleMgrProxy::NotifyAbilityLifeStatus()";
    return true;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

BundleMgrService::BundleMgrService()
{
    abilityInfoMap_.emplace(COM_IX_HIWORLD, HiWordInfo);
    abilityInfoMap_.emplace(COM_IX_HIMUSIC, HiMusicInfo);
    abilityInfoMap_.emplace(COM_IX_HIRADIO, HiRadioInfo);
    abilityInfoMap_.emplace(COM_IX_HISERVICE, HiServiceInfo);
    abilityInfoMap_.emplace(COM_IX_MUSICSERVICE, MusicServiceInfo);
    abilityInfoMap_.emplace(COM_IX_HIDATA, HiDataInfo);
    GTEST_LOG_(INFO) << "BundleMgrService()";
}

BundleMgrService::~BundleMgrService()
{
    GTEST_LOG_(INFO) << "~BundleMgrService()";
}

bool BundleMgrService::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

bool BundleMgrService::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    if (CheckWantEntity(want, abilityInfo)) {
        return true;
    }
    ElementName elementTemp = want.GetElement();
    std::string abilityNameTemp = elementTemp.GetAbilityName();
    std::string bundleNameTemp = elementTemp.GetBundleName();
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.visible = true;
    if (bundleNameTemp.empty() || abilityNameTemp.empty()) {
        return false;
    }

    auto fun = abilityInfoMap_.find(bundleNameTemp);
    if (fun != abilityInfoMap_.end()) {
        auto call = fun->second;
        if (call) {
            call(bundleNameTemp, abilityInfo, elementTemp);
            return true;
        }
    }
    if (std::string::npos != elementTemp.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hello";
    if (elementTemp.GetAbilityName().find("com.ohos.launcher.MainAbility") != std::string::npos) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
        abilityInfo.applicationInfo.iconPath = "icon path";
        abilityInfo.applicationInfo.label = "app label";
    }
    return true;
}

bool BundleMgrService::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    return true;
}

bool BundleMgrService::CheckIsSystemAppByUid(const int uid)
{
    int maxSysUid {MAX_SYS_UID};
    int baseSysUid{ROOT_UID};
    if (uid >= baseSysUid && uid <= maxSysUid) {
        return true;
    }
    return false;
}

bool BundleMgrService::CheckWantEntity(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    auto entityVector = want.GetEntities();
    ElementName element = want.GetElement();
    if (entityVector.empty()) {
        return false;
    }

    auto find = false;
    // filter ams onstart
    for (const auto &entity : entityVector) {
        if (entity == Want::FLAG_HOME_INTENT_FROM_SYSTEM && element.GetAbilityName().empty() &&
            element.GetBundleName().empty()) {
            find = true;
            break;
        }
    }

    auto bundleName = element.GetBundleName();
    auto abilityName = element.GetAbilityName();
    if (find || (bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME &&
        (abilityName == AbilityConfig::SYSTEM_UI_STATUS_BAR ||
        abilityName == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR))) {
        return true;
    }

    return false;
}

int BundleMgrService::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return 1000;
}

bool BundleMgrService::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
{
    GTEST_LOG_(INFO) << "BundleMgrService::NotifyAbilityLifeStatus()";
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
