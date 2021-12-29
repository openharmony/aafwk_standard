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

bool BundleMgrProxy::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
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
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
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
    abilityInfoMap_.emplace(SYSTEM_UI_BUNDLE_NAME, SystemUiInfo);
    abilityInfoMap_.emplace(COM_IX_HIPHONE, HiPhoneInfo);
    abilityInfoMap_.emplace(COM_IX_HISELECTOR, HiSelectorInfo);
    abilityInfoMap_.emplace(COM_IX_TEST1, HiTest1Info);
    abilityInfoMap_.emplace(COM_IX_TEST2, HiTest2Info);
    abilityInfoMap_.emplace(COM_IX_TEST3, HiTest3Info);
    abilityInfoMap_.emplace(COM_IX_TEST4, HiTest4Info);
    GTEST_LOG_(INFO) << "BundleMgrService()";
}

BundleMgrService::~BundleMgrService()
{
    GTEST_LOG_(INFO) << "~BundleMgrService()";
}

bool BundleMgrService::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}

bool BundleMgrService::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    BundleInfo worldInfo;
    worldInfo.name = COM_IX_HIWORLD;
    worldInfo.uid = HIWORLD_APP_UID;
    BundleInfo musicInfo;
    musicInfo.name = COM_IX_HIMUSIC;
    musicInfo.uid = HIMUSIC_APP_UID;
    BundleInfo radioInfo;
    radioInfo.name = COM_IX_HIRADIO;
    radioInfo.uid = HIRADIO_APP_UID;
    BundleInfo serviceInfo;
    serviceInfo.name = COM_IX_HISERVICE;
    serviceInfo.uid = HISERVICE_UID;
    BundleInfo musicServiceInfo;
    musicServiceInfo.name = COM_IX_MUSICSERVICE;
    musicServiceInfo.uid = MUSICSERVICE_UID;
    BundleInfo systemUiInfo;
    systemUiInfo.name = SYSTEM_UI_BUNDLE_NAME;
    systemUiInfo.uid = SYSTEM_UI_UID;
    BundleInfo phoneInfo;
    phoneInfo.name = COM_IX_HIPHONE;
    phoneInfo.uid = HIPHONE_UID;
    BundleInfo selectorInfo;
    selectorInfo.name = COM_IX_HISELECTOR;
    selectorInfo.uid = HISELECTOR_UID;
    BundleInfo test1Info;
    test1Info.name = COM_IX_TEST1;
    test1Info.uid = TEST1_UID;
    BundleInfo test2Info;
    test2Info.name = COM_IX_TEST2;
    test2Info.uid = TEST2_UID;
    BundleInfo test3Info;
    test3Info.name = COM_IX_TEST3;
    test3Info.uid = TEST3_UID;
    BundleInfo test4Info;
    test4Info.name = COM_IX_TEST4;
    test4Info.uid = TEST4_UID;

    bundleInfos.emplace_back(worldInfo);
    bundleInfos.emplace_back(musicInfo);
    bundleInfos.emplace_back(radioInfo);
    bundleInfos.emplace_back(serviceInfo);
    bundleInfos.emplace_back(musicServiceInfo);
    bundleInfos.emplace_back(systemUiInfo);
    bundleInfos.emplace_back(phoneInfo);
    bundleInfos.emplace_back(selectorInfo);
    bundleInfos.emplace_back(test1Info);
    bundleInfos.emplace_back(test2Info);
    bundleInfos.emplace_back(test3Info);
    bundleInfos.emplace_back(test4Info);
    return true;
}

bool BundleMgrService::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    ElementName element = want.GetElement();
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();

    AbilityInfo info;
    info.name = abilityName;
    info.isLauncherAbility = false;
    info.applicationName = bundleName;
    info.type = AbilityType::PAGE;

    ApplicationInfo appInfo;
    appInfo.name = bundleName;
    appInfo.bundleName = bundleName;
    appInfo.isLauncherApp = false;

    info.applicationInfo = appInfo;
    abilityInfos.emplace_back(info);
    return true;
}

bool BundleMgrService::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    ElementName element = want.GetElement();
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();

    AbilityInfo info;
    auto fun = abilityInfoMap_.find(bundleName);
    if (fun != abilityInfoMap_.end()) {
        auto call = fun->second;
        if (call) {
            call(bundleName, info, element);
            abilityInfos.emplace_back(info);
            return true;
        }
    }
    return false;
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

    return false;
}

bool BundleMgrService::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    AbilityInfo info;
    auto bundleName = COM_IX_HIDATA;
    ElementName element("device", "com.ix.hiData", "DataAbility");

    auto fun = abilityInfoMap_.find(bundleName);
    if (fun != abilityInfoMap_.end()) {
        auto call = fun->second;
        if (call) {
            call(bundleName, info, element);
            abilityInfos.emplace_back(info);
            return true;
        }
    }
    return false;
}

bool BundleMgrService::QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo)
{
    return false;
}

bool BundleMgrService::CheckBundleNameInAllowList(const std::string &bundleName)
{
    return true;
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
    int baseSysUid {ROOT_UID};
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
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    GTEST_LOG_(INFO) << "BundleMgrService::NotifyAbilityLifeStatus()";
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
