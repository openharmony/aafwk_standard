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

#include "mock_bundle_mgr.h"
#include "ability_info.h"
#include "application_info.h"
#include "hilog_wrapper.h"
#include "ability_config.h"
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
BundleMgrService::BundleMgrService()
{
    MakingPackageData();
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

bool BundleMgrService::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    bundleInfo.uid = 0;
    return true;
}

bool BundleMgrService::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    bundleInfos = bundleInfos_;
    return true;
}

bool BundleMgrService::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    ElementName elementTemp = want.GetElement();
    std::string abilityNameTemp = elementTemp.GetAbilityName();
    std::string bundleNameTemp = elementTemp.GetBundleName();

    GTEST_LOG_(INFO) << "QueryAbilityInfosForClone bundleNameTemp : "<<bundleNameTemp;
    GTEST_LOG_(INFO) << "QueryAbilityInfosForClone abilityNameTemp : "<<abilityNameTemp;
    for (auto bundleInfo : bundleInfos_) {
        for (auto abilityInfo : bundleInfo.abilityInfos) {
            if (abilityInfo.name == abilityNameTemp && abilityInfo.bundleName == bundleNameTemp) {
                abilityInfos.emplace_back(abilityInfo);
            }
        }
    }
    GTEST_LOG_(INFO) << "QueryAbilityInfosForClone abilityInfos size : "<<abilityInfos.size();
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

    if (std::string::npos != elementTemp.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = elementTemp.GetBundleName();
    abilityInfo.deviceId = elementTemp.GetDeviceID();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.name = "hello";
    if (want.HasEntity(Want::ENTITY_HOME) && want.GetAction() == Want::ACTION_HOME) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
        abilityInfo.applicationInfo.iconPath = "icon path";
        abilityInfo.applicationInfo.label = "app label";
    }
    return true;
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

bool BundleMgrService::CheckWantEntity(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    auto entityVector = want.GetEntities();
    ElementName element = want.GetElement();

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

bool BundleMgrService::NotifyActivityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    return true;
}

bool BundleMgrService::QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos)
{
    if (abilityUri.find(COM_IX_HIDATA) == std::string::npos) {
        GTEST_LOG_(INFO) << "QueryAbilityInfosByUri false";
        return false;
    }

    for (auto bundleInfo : bundleInfos_) {
        for (auto abilityInfo : bundleInfo.abilityInfos) {
            if (abilityInfo.bundleName == COM_IX_HIDATA) {
                abilityInfos.emplace_back(abilityInfo);
            }
        }
    }
    GTEST_LOG_(INFO) << "QueryAbilityInfosByUri size : "<<abilityInfos.size();
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

void BundleMgrService::MakingPackageData()
{
    PushHiWorldAbility();
    PushHiMusicAbility();
    PushHiRadioAbility();
    PushHiServiceAbility();
    PushHiMusicServiceAbility();
    PushHiDataAbility();
    PushPhoneAbility();
    PushTVAbility();
    PushFilmAbility();
    PushClockAbility();
    PushClockCloneAbility();
    PushEgcbility();
    PushEgcClonebility();
    PushSelectorAbility();
}

void BundleMgrService::PushHiWorldAbility()
{
    AbilityInfo info;
    info.name = "luncher";
    info.bundleName = COM_IX_HIWORLD;
    info.applicationInfo.bundleName = COM_IX_HIWORLD;
    info.applicationName = "Helloworld";
    info.applicationInfo.name = "Helloworld";
    info.type = AbilityType::PAGE;
    info.applicationInfo.isLauncherApp = true;
    info.applicationInfo.uid = HIWORLD_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.configChanges.push_back("fontSize");

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_HIWORLD;
    bundleInfo.uid = HIWORLD_APP_UID;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushHiMusicAbility()
{
    AbilityInfo info1;
    info1.name = "MusicAbility";
    info1.bundleName = COM_IX_HIMUSIC;
    info1.applicationInfo.bundleName = COM_IX_HIMUSIC;
    info1.applicationName = "hiMusic";
    info1.applicationInfo.name = "hiMusic";
    info1.type = AbilityType::PAGE;
    info1.applicationInfo.isLauncherApp = false;
    info1.applicationInfo.uid = HIMUSIC_APP_UID;
    info1.applicationInfo.isCloned = false;
    info1.deviceId = "deviceId";
    info1.visible = true;
    info1.process = "p1";
    info1.launchMode = LaunchMode::STANDARD;

    AbilityInfo info2;
    info2.name = "MusicTopAbility";
    info2.bundleName = COM_IX_HIMUSIC;
    info2.applicationInfo.bundleName = COM_IX_HIMUSIC;
    info2.applicationName = "hiMusic";
    info2.applicationInfo.name = "hiMusic";
    info2.type = AbilityType::PAGE;
    info2.applicationInfo.isLauncherApp = false;
    info2.applicationInfo.uid = HIMUSIC_APP_UID;
    info2.applicationInfo.isCloned = false;
    info2.deviceId = "deviceId";
    info2.visible = true;
    info2.process = "p1";
    info2.launchMode = LaunchMode::SINGLETOP;

    AbilityInfo info3;
    info3.name = "MusicSAbility";
    info3.bundleName = COM_IX_HIMUSIC;
    info3.applicationInfo.bundleName = COM_IX_HIMUSIC;
    info3.applicationName = "hiMusic";
    info3.applicationInfo.name = "hiMusic";
    info3.type = AbilityType::PAGE;
    info3.applicationInfo.isLauncherApp = false;
    info3.applicationInfo.uid = HIMUSIC_APP_UID;
    info3.applicationInfo.isCloned = false;
    info3.deviceId = "deviceId";
    info3.visible = true;
    info3.process = "p1";
    info3.launchMode = LaunchMode::SINGLETON;

    AbilityInfo info4;
    info4.name = "Music";
    info4.bundleName = COM_IX_HIMUSIC;
    info4.applicationInfo.bundleName = COM_IX_HIMUSIC;
    info4.applicationName = "hiMusic";
    info4.applicationInfo.name = "hiMusic";
    info4.type = AbilityType::PAGE;
    info4.applicationInfo.isLauncherApp = false;
    info4.applicationInfo.uid = HIMUSIC_APP_UID;
    info4.applicationInfo.isCloned = false;
    info4.deviceId = "deviceId";
    info4.visible = true;
    info4.process = "p1";
    info4.launchMode = LaunchMode::STANDARD;
    info4.configChanges.push_back("layout");

    AbilityInfo info5;
    info5.name = "MusicTon";
    info5.bundleName = COM_IX_HIMUSIC;
    info5.applicationInfo.bundleName = COM_IX_HIMUSIC;
    info5.applicationName = "hiMusic";
    info5.applicationInfo.name = "hiMusic";
    info5.type = AbilityType::PAGE;
    info5.applicationInfo.isLauncherApp = false;
    info5.applicationInfo.uid = HIMUSIC_APP_UID;
    info5.applicationInfo.isCloned = false;
    info5.deviceId = "deviceId";
    info5.visible = true;
    info5.process = "p1";
    info5.launchMode = LaunchMode::SINGLETON;
    info5.configChanges.push_back("layout");

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_HIMUSIC;
    bundleInfo.uid = HIMUSIC_APP_UID;
    bundleInfo.abilityInfos.emplace_back(info1);
    bundleInfo.abilityInfos.emplace_back(info2);
    bundleInfo.abilityInfos.emplace_back(info3);
    bundleInfo.abilityInfos.emplace_back(info4);
    bundleInfo.abilityInfos.emplace_back(info5);
    bundleInfo.applicationInfo = info5.applicationInfo;

    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushHiRadioAbility()
{
    AbilityInfo info;
    info.name = "RadioAbility";
    info.bundleName = COM_IX_HIRADIO;
    info.applicationInfo.bundleName = COM_IX_HIRADIO;
    info.applicationName = "hiRadio";
    info.applicationInfo.name = "hiRadio";
    info.type = AbilityType::PAGE;
    info.process = "p3";
    info.applicationInfo.uid = HIRADIO_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.launchMode = LaunchMode::STANDARD;

    AbilityInfo info1;
    info1.name = "RadioTopAbility";
    info1.bundleName = COM_IX_HIRADIO;
    info1.applicationInfo.bundleName = COM_IX_HIRADIO;
    info1.applicationName = "hiRadio";
    info1.applicationInfo.name = "hiRadio";
    info1.type = AbilityType::PAGE;
    info1.process = "p3";
    info1.applicationInfo.uid = HIRADIO_APP_UID;
    info1.applicationInfo.isCloned = false;
    info1.deviceId = "deviceId";
    info1.visible = true;
    info1.launchMode = LaunchMode::SINGLETON;

    AbilityInfo info2;
    info2.name = "Radio";
    info2.bundleName = COM_IX_HIRADIO;
    info2.applicationInfo.bundleName = COM_IX_HIRADIO;
    info2.applicationName = "hiRadio";
    info2.applicationInfo.name = "hiRadio";
    info2.type = AbilityType::PAGE;
    info2.process = "p3";
    info2.applicationInfo.uid = HIRADIO_APP_UID;
    info2.applicationInfo.isCloned = false;
    info2.deviceId = "deviceId";
    info2.visible = true;
    info2.launchMode = LaunchMode::STANDARD;
    info2.configChanges.push_back("orientation");

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_HIRADIO;
    bundleInfo.uid = info2.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.abilityInfos.emplace_back(info1);
    bundleInfo.abilityInfos.emplace_back(info2);
    bundleInfo.applicationInfo = info2.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushHiServiceAbility()
{
    AbilityInfo info;
    info.name = "hiService";
    info.bundleName = COM_IX_HISERVICE;
    info.applicationInfo.bundleName = COM_IX_HISERVICE;
    info.applicationName = "hiService";
    info.applicationInfo.name = "hiService";
    info.type = AbilityType::SERVICE;
    info.process = "p4";
    info.applicationInfo.uid = HISERVICE_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_HISERVICE;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushHiMusicServiceAbility()
{
    AbilityInfo info;
    info.name = "hiService";
    info.bundleName = COM_IX_MUSICSERVICE;
    info.applicationInfo.bundleName = COM_IX_MUSICSERVICE;
    info.applicationName = "musicService";
    info.applicationInfo.name = "musicService";
    info.type = AbilityType::SERVICE;
    info.process = "p5";
    info.applicationInfo.uid = MUSICSERVICE_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_MUSICSERVICE;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushHiDataAbility()
{
    AbilityInfo info;
    info.name = "hiData";
    info.bundleName = COM_IX_HIDATA;
    info.applicationInfo.bundleName = COM_IX_HIDATA;
    info.applicationName = "hiData";
    info.applicationInfo.name = "hiData";
    info.type = AbilityType::DATA;
    info.process = "p6";
    info.applicationInfo.uid = HIDATA_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;

    AbilityInfo info1;
    info1.name = "hiData";
    info1.bundleName = COM_IX_HIDATA;
    info1.applicationInfo.bundleName = COM_IX_HIDATA;
    info1.applicationName = "hiData";
    info1.applicationInfo.name = "hiData";
    info1.type = AbilityType::DATA;
    info1.process = "p6";
    info1.applicationInfo.uid = HIDATA_APP_CLONE_UID;
    info1.applicationInfo.isCloned = true;
    info1.deviceId = "deviceId";
    info1.visible = true;


    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_HIDATA;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;

    BundleInfo bundleInfo1;
    bundleInfo1.name = COM_IX_HIDATA;
    bundleInfo1.uid = info1.applicationInfo.uid;
    bundleInfo1.abilityInfos.emplace_back(info1);
    bundleInfo1.applicationInfo = info1.applicationInfo;

    bundleInfos_.emplace_back(bundleInfo);
    bundleInfos_.emplace_back(bundleInfo1);
}

void BundleMgrService::PushPhoneAbility()
{
    AbilityInfo info;
    info.name = "PhoneAbility1";
    info.bundleName = COM_IX_PHONE;
    info.applicationInfo.bundleName = COM_IX_PHONE;
    info.applicationName = "hiPhone";
    info.applicationInfo.name = "hiPhone";
    info.type = AbilityType::PAGE;
    info.applicationInfo.isLauncherApp = false;
    info.process = "p7";
    info.applicationInfo.uid = PHONE_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.launchMode = LaunchMode::SINGLETON;

    AbilityInfo info1;
    info1.name = "PhoneAbility2";
    info1.bundleName = COM_IX_PHONE;
    info1.applicationInfo.bundleName = COM_IX_PHONE;
    info1.applicationName = "hiPhone";
    info1.applicationInfo.name = "hiPhone";
    info1.type = AbilityType::PAGE;
    info1.applicationInfo.isLauncherApp = false;
    info1.process = "p7";
    info1.applicationInfo.uid = PHONE_APP_UID;
    info1.applicationInfo.isCloned = false;
    info1.deviceId = "deviceId";
    info1.visible = true;
    info1.launchMode = LaunchMode::SINGLETON;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_PHONE;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.abilityInfos.emplace_back(info1);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushTVAbility()
{
    AbilityInfo info;
    info.name = "TVAbility";
    info.bundleName = COM_IX_TV;
    info.applicationInfo.bundleName = COM_IX_TV;
    info.applicationName = "hiTV";
    info.applicationInfo.name = "hiTV";
    info.type = AbilityType::PAGE;
    info.applicationInfo.isLauncherApp = false;
    info.applicationInfo.uid = TV_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.process = "p8";
    info.launchMode = LaunchMode::SINGLETON;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_TV;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushFilmAbility()
{
    AbilityInfo info;
    info.name = "FilmAbility";
    info.bundleName = COM_IX_Film;
    info.applicationInfo.bundleName = COM_IX_Film;
    info.applicationName = "hiFilm";
    info.applicationInfo.name = "hiFilm";
    info.type = AbilityType::PAGE;
    info.applicationInfo.isLauncherApp = false;
    info.applicationInfo.uid = FILM_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.process = "p9";
    info.launchMode = LaunchMode::SINGLETON;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_Film;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushClockAbility()
{
    AbilityInfo info;
    info.name = "ClockService";
    info.bundleName = COM_IX_CLOCK;
    info.applicationInfo.bundleName = COM_IX_CLOCK;
    info.applicationName = "clock";
    info.applicationInfo.name = "clock";
    info.type = AbilityType::SERVICE;
    info.applicationInfo.isLauncherApp = false;
    info.applicationInfo.uid = CLOCK_SERVICE_CLONE_UID;
    info.applicationInfo.isCloned = true;
    info.deviceId = "deviceId";
    info.visible = true;
    info.process = "p10";
    info.launchMode = LaunchMode::SINGLETON;

    AbilityInfo info1;
    info1.name = "ClockService";
    info1.bundleName = COM_IX_CLOCK;
    info1.applicationInfo.bundleName = COM_IX_CLOCK;
    info1.applicationName = "clock";
    info1.applicationInfo.name = "clock";
    info1.type = AbilityType::SERVICE;
    info1.process = "p10";
    info1.applicationInfo.uid = CLOCK_SERVICE_UID;
    info1.applicationInfo.isCloned = false;
    info1.deviceId = "deviceId";
    info1.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_CLOCK;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    BundleInfo bundleInfo1;
    bundleInfo1.name = COM_IX_CLOCK;
    bundleInfo1.uid = info1.applicationInfo.uid;
    bundleInfo1.abilityInfos.emplace_back(info1);
    bundleInfo1.applicationInfo = info1.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
    bundleInfos_.emplace_back(bundleInfo1);
}

void BundleMgrService::PushClockCloneAbility()
{
    AbilityInfo info;
    info.name = "ClockAbility";
    info.bundleName = COM_IX_CLOCK;
    info.applicationInfo.bundleName = COM_IX_CLOCK;
    info.applicationName = "clock";
    info.applicationInfo.name = "clock";
    info.type = AbilityType::PAGE;
    info.applicationInfo.isLauncherApp = false;
    info.applicationInfo.uid = CLOCK_APP_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;
    info.process = "p10";
    info.launchMode = LaunchMode::STANDARD;

    AbilityInfo info1;
    info1.name = "ClockAbility";
    info1.bundleName = COM_IX_CLOCK;
    info1.applicationInfo.bundleName = COM_IX_CLOCK;
    info1.applicationName = "clock";
    info1.applicationInfo.name = "clock";
    info1.type = AbilityType::PAGE;
    info1.process = "p10";
    info1.applicationInfo.uid = CLOCK_APP_CLONE_UID;
    info1.applicationInfo.isCloned = true;
    info1.deviceId = "deviceId";
    info1.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_CLOCK;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    BundleInfo bundleInfo1;
    bundleInfo1.name = COM_IX_CLOCK;
    bundleInfo1.uid = info1.applicationInfo.uid;
    bundleInfo1.abilityInfos.emplace_back(info1);
    bundleInfo1.applicationInfo = info1.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
    bundleInfos_.emplace_back(bundleInfo1);
}

void BundleMgrService::PushEgcbility()
{
    AbilityInfo info;
    info.name = "EgcDataAbility";
    info.bundleName = COM_IX_EGC;
    info.applicationInfo.bundleName = COM_IX_EGC;
    info.applicationName = "egc";
    info.applicationInfo.name = "egc";
    info.type = AbilityType::DATA;
    info.process = "p12";
    info.applicationInfo.uid = EGC_DATA_UID;
    info.applicationInfo.isCloned = false;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_EGC;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushEgcClonebility()
{
    AbilityInfo info;
    info.name = "EgcDataAbility";
    info.bundleName = COM_IX_EGC;
    info.applicationInfo.bundleName = COM_IX_EGC;
    info.applicationName = "egc";
    info.applicationInfo.name = "egc";
    info.type = AbilityType::DATA;
    info.process = "p12";
    info.applicationInfo.uid = EGC_DATA_CLONE_UID;
    info.applicationInfo.isCloned = true;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_EGC;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushSelectorAbility()
{
    AbilityInfo info;
    info.name = "com.ohos.selector.MainAbility";
    info.bundleName = COM_IX_SELECTOR;
    info.applicationInfo.bundleName = COM_IX_SELECTOR;
    info.applicationName = "selector";
    info.applicationInfo.name = "selector";
    info.process = "p12";
    info.applicationInfo.uid = SELECTOR_APP_UID;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_IX_SELECTOR;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}
}  // namespace AppExecFwk
}  // namespace OHOS
