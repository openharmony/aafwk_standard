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

#ifndef OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
#define OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include <gmock/gmock.h>
#include "ability_info.h"
#include "application_info.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "ohos/aafwk/content/want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string COM_IX_HIWORLD = "com.ix.hiworld";
const std::string COM_IX_HIMUSIC = "com.ix.hiMusic";
const std::string COM_IX_HIRADIO = "com.ix.hiRadio";
const std::string COM_IX_HISERVICE = "com.ix.hiService";
const std::string COM_IX_MUSICSERVICE = "com.ix.musicService";
const std::string COM_IX_HIDATA = "com.ix.hiData";
const std::string COM_IX_PHONE = "com.ix.hiPhone";
const std::string COM_IX_TV = "com.ix.hiTV";
const std::string COM_IX_Film = "com.ix.hiFilm";
constexpr int32_t MAX_SYS_UID = 2899;
constexpr int32_t ROOT_UID = 0;

auto HiWordInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    abilityInfo.applicationInfo.name = "Helloworld";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = true;

    if (elementTemp.GetAbilityName() == "luncher") {
        abilityInfo.configChanges.push_back("fontSize");
    }
    return true;
};

auto HiMusicInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiMusic";
    abilityInfo.applicationInfo.name = "hiMusic";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;

    if (elementTemp.GetAbilityName() == "MusicAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "MusicTopAbility") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::SINGLETOP;
    }
    if (elementTemp.GetAbilityName() == "MusicSAbility") {
        abilityInfo.process = "p2";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    if (elementTemp.GetAbilityName() == "Music") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::STANDARD;
        abilityInfo.configChanges.push_back("layout");
    }

    if (elementTemp.GetAbilityName() == "MusicTon") {
        abilityInfo.process = "p1";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
        abilityInfo.configChanges.push_back("layout");
    }
    return true;
};

auto HiRadioInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiRadio";
    abilityInfo.applicationInfo.name = "hiRadio";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.process = "p3";
    if (elementTemp.GetAbilityName() == "RadioAbility") {
        abilityInfo.launchMode = LaunchMode::STANDARD;
    }
    if (elementTemp.GetAbilityName() == "RadioTopAbility") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    if (elementTemp.GetAbilityName() == "Radio") {
        abilityInfo.launchMode = LaunchMode::STANDARD;
        abilityInfo.configChanges.push_back("orientation");
    }
    return true;
};

auto HiServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiService";
    abilityInfo.applicationInfo.name = "hiService";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p4";
    return true;
};

auto MusicServiceInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "musicService";
    abilityInfo.applicationInfo.name = "musicService";
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.process = "p5";
    return true;
};

auto HiDataInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiData";
    abilityInfo.applicationInfo.name = "hiData";
    abilityInfo.type = AbilityType::DATA;
    abilityInfo.process = "p6";
    return true;
};

auto HiPhoneInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiPhone";
    abilityInfo.applicationInfo.name = "hiPhone";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    abilityInfo.process = "p7";
    if (elementTemp.GetAbilityName() == "PhoneAbility1") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    if (elementTemp.GetAbilityName() == "PhoneAbility2") {
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto HiTVInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiTV";
    abilityInfo.applicationInfo.name = "hiTV";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    if (elementTemp.GetAbilityName() == "TVAbility") {
        abilityInfo.process = "p8";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};

auto HiFilmInfo = [](std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp) {
    abilityInfo.name = elementTemp.GetAbilityName();
    abilityInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationInfo.bundleName = elementTemp.GetBundleName();
    abilityInfo.applicationName = "hiFilm";
    abilityInfo.applicationInfo.name = "hiFilm";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.applicationInfo.isLauncherApp = false;
    if (elementTemp.GetAbilityName() == "FilmAbility") {
        abilityInfo.process = "p9";
        abilityInfo.launchMode = LaunchMode::SINGLETON;
    }
    return true;
};
}  // namespace
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}

    int QueryWantAbility(const AAFwk::Want &want, std::vector<AbilityInfo> &abilityInfos);
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string &uri, AbilityInfo &abilityInfo) override;
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;

    virtual bool CheckIsSystemAppByUid(const int uid) override;
    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD3(GetBundleInfos, bool(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId));
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(GetAbilityLabel, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(GetAllPermissionGroupDefs, bool(std::vector<PermissionDef> &permissionDefs));
    MOCK_METHOD2(GetAppsGrantedPermissions,
        bool(const std::vector<std::string> &permissions, std::vector<std::string> &appNames));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleCacheFiles,
        bool(const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback));
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD4(
        DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result));
    MOCK_METHOD1(IsApplicationEnabled, bool(const std::string &bundleName));
    MOCK_METHOD2(SetApplicationEnabled, bool(const std::string &bundleName, bool isEnable));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD0(GetBundleUserMgr, sptr<IBundleUserMgr>());
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &, bool));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));
    MOCK_METHOD2(QueryAbilityInfos, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD2(QueryAbilityInfosForClone, bool(const Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD4(GetDistributedBundleInfo, bool(const std::string &networkId, int32_t userId,
        const std::string &bundleName, DistributedBundleInfo &distributedBundleInfo));
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IBundleMgr");
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    MOCK_METHOD2(QueryAbilityInfoByUri, bool(const std::string &abilityUri, AbilityInfo &abilityInfo));
    MOCK_METHOD2(QueryWantAbility, int(const AAFwk::Want &want, std::vector<AbilityInfo> &abilityInfos));
    MOCK_METHOD3(GetApplicationInfos,
        bool(const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos));
    MOCK_METHOD3(GetBundleInfos, bool(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId));
    MOCK_METHOD2(GetBundleNameForUid, bool(const int uid, std::string &bundleName));
    MOCK_METHOD2(GetBundleGids, bool(const std::string &bundleName, std::vector<int> &gids));
    MOCK_METHOD1(GetAppType, std::string(const std::string &bundleName));
    MOCK_METHOD2(GetBundleInfosByMetaData, bool(const std::string &metaData, std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD1(QueryKeepAliveBundleInfos, bool(std::vector<BundleInfo> &bundleInfos));
    MOCK_METHOD2(GetAbilityLabel, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD3(
        GetBundleArchiveInfo, bool(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo));
    MOCK_METHOD2(GetHapModuleInfo, bool(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo));
    MOCK_METHOD2(GetLaunchWantForBundle, bool(const std::string &bundleName, Want &want));
    MOCK_METHOD2(CheckPublicKeys, int(const std::string &firstBundleName, const std::string &secondBundleName));
    MOCK_METHOD2(CheckPermission, int(const std::string &bundleName, const std::string &permission));
    MOCK_METHOD2(GetPermissionDef, bool(const std::string &permissionName, PermissionDef &permissionDef));
    MOCK_METHOD1(GetAllPermissionGroupDefs, bool(std::vector<PermissionDef> &permissionDefs));
    MOCK_METHOD2(GetAppsGrantedPermissions,
        bool(const std::vector<std::string> &permissions, std::vector<std::string> &appNames));
    MOCK_METHOD1(HasSystemCapability, bool(const std::string &capName));
    MOCK_METHOD1(GetSystemAvailableCapabilities, bool(std::vector<std::string> &systemCaps));
    MOCK_METHOD0(IsSafeMode, bool());
    MOCK_METHOD2(CleanBundleCacheFiles,
        bool(const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback));
    MOCK_METHOD2(CleanBundleDataFiles, bool(const std::string &bundleName, const int userId));
    MOCK_METHOD1(RegisterBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD1(ClearBundleStatusCallback, bool(const sptr<IBundleStatusCallback> &bundleStatusCallback));
    MOCK_METHOD0(UnregisterBundleStatusCallback, bool());
    MOCK_METHOD4(
        DumpInfos, bool(const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result));
    MOCK_METHOD1(IsApplicationEnabled, bool(const std::string &bundleName));
    MOCK_METHOD2(SetApplicationEnabled, bool(const std::string &bundleName, bool isEnable));
    MOCK_METHOD0(GetBundleInstaller, sptr<IBundleInstaller>());
    MOCK_METHOD0(GetBundleUserMgr, sptr<IBundleUserMgr>());
    MOCK_METHOD3(
        CanRequestPermission, bool(const std::string &bundleName, const std::string &permissionName, const int userId));
    MOCK_METHOD3(RequestPermissionFromUser,
        bool(const std::string &bundleName, const std::string &permission, const int userId));
    MOCK_METHOD2(GetNameForUid, bool(const int uid, std::string &name));
    MOCK_METHOD2(GetBundlesForUid, bool(const int uid, std::vector<std::string> &));
    MOCK_METHOD2(SetAbilityEnabled, bool(const AbilityInfo &, bool));
    MOCK_METHOD1(IsAbilityEnabled, bool(const AbilityInfo &));
    MOCK_METHOD2(GetAbilityIcon, std::string(const std::string &bundleName, const std::string &className));
    MOCK_METHOD1(RegisterAllPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(RegisterPermissionsChanged,
        bool(const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD1(UnregisterPermissionsChanged, bool(const sptr<OnPermissionChangedCallback> &callback));
    MOCK_METHOD2(GetAppIdByBundleName, std::string(const std::string &bundleName, const int userId));
    MOCK_METHOD1(GetAllFormsInfo, bool(std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetFormsInfoByApp, bool(const std::string &bundleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD3(GetFormsInfoByModule,
        bool(const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos));
    MOCK_METHOD2(GetShortcutInfos, bool(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos));
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetModuleUsageRecords, bool(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords));
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfo(const AAFwk::Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool NotifyAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;
    virtual bool GetBundleGidsByUid(const std::string &bundleName, const int &uid, std::vector<int> &gids) override
    {
        return true;
    }
    virtual bool QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    }
    virtual int CheckPermissionByUid(
        const std::string &bundleName, const std::string &permission, const int userId) override
    {
        return true;
    }
    virtual bool GetAllCommonEventInfo(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos) override
    {
        return true;
    }
    virtual bool RemoveClonedBundle(const std::string &bundleName, const int32_t uid) override
    {
        return true;
    }
    virtual bool BundleClone(const std::string &bundleName) override
    {
        return true;
    }
    virtual bool CheckBundleNameInAllowList(const std::string &bundleName) override
    {
        return true;
    }
    virtual bool GetDistributedBundleInfo(
        const std::string &networkId, int32_t userId, const std::string &bundleName,
        DistributedBundleInfo &distributedBundleInfo) override
    {
        return true;
    }
    BundleMgrService()
    {
        abilityInfoMap_.emplace(COM_IX_HIWORLD, HiWordInfo);
        abilityInfoMap_.emplace(COM_IX_HIMUSIC, HiMusicInfo);
        abilityInfoMap_.emplace(COM_IX_HIRADIO, HiRadioInfo);
        abilityInfoMap_.emplace(COM_IX_HISERVICE, HiServiceInfo);
        abilityInfoMap_.emplace(COM_IX_MUSICSERVICE, MusicServiceInfo);
        abilityInfoMap_.emplace(COM_IX_HIDATA, HiDataInfo);
        abilityInfoMap_.emplace(COM_IX_PHONE, HiPhoneInfo);
        abilityInfoMap_.emplace(COM_IX_TV, HiTVInfo);
        abilityInfoMap_.emplace(COM_IX_Film, HiFilmInfo);
    }

    virtual ~BundleMgrService()
    {}

    bool CheckWantEntity(const AAFwk::Want &, AbilityInfo &);

public:
    using QueryAbilityInfoFunType =
        std::function<bool(std::string bundleName, AbilityInfo &abilityInfo, ElementName &elementTemp)>;
    std::map<std::string, QueryAbilityInfoFunType> abilityInfoMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
