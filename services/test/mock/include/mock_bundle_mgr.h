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
const std::string COM_IX_CLOCK = "com.ix.clock";
const std::string COM_IX_EGC = "com.ix.egc";
const std::string COM_IX_SELECTOR = "com.ohos.selector";
constexpr int32_t MAX_SYS_UID = 2899;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t HIWORLD_APP_UID = 10001;
constexpr int32_t HIMUSIC_APP_UID = 10002;
constexpr int32_t HIRADIO_APP_UID = 10003;
constexpr int32_t HISERVICE_APP_UID = 10004;
constexpr int32_t MUSICSERVICE_APP_UID = 10005;
constexpr int32_t HIDATA_APP_UID = 10006;
constexpr int32_t HIDATA_APP_CLONE_UID = 100061;
constexpr int32_t PHONE_APP_UID = 10007;
constexpr int32_t TV_APP_UID = 10008;
constexpr int32_t FILM_APP_UID = 10009;
constexpr int32_t CLOCK_SERVICE_CLONE_UID = 10010;
constexpr int32_t CLOCK_SERVICE_UID = 10011;
constexpr int32_t CLOCK_APP_UID = 10012;
constexpr int32_t CLOCK_APP_CLONE_UID = 10013;
constexpr int32_t EGC_DATA_UID = 10014;
constexpr int32_t EGC_DATA_CLONE_UID = 10015;
constexpr int32_t SELECTOR_APP_UID = 10016;
}  // namespace

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
    MOCK_METHOD3(
        CheckPermissionByUid, int(const std::string &bundleName, const std::string &permission, const int userId));
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
    MOCK_METHOD2(GetAllCommonEventInfo, bool(const std::string &eventKey,
        std::vector<CommonEventInfo> &commonEventInfos));
    MOCK_METHOD2(GetUidByBundleName, int(const std::string &bundleName, const int userId));
    MOCK_METHOD2(GetModuleUsageRecords, bool(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords));
    MOCK_METHOD2(RemoveClonedBundle, bool(const std::string &bundleName, const int32_t uid));
    MOCK_METHOD1(BundleClone, bool(const std::string &bundleName));
    MOCK_METHOD3(GetBundleGidsByUid, bool(const std::string &bundleName, const int &uid, std::vector<int> &gids));

    bool QueryAbilityInfosByUri(const std::string &abilityUri, std::vector<AbilityInfo> &abilityInfos) override;
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;
    bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId) override;
    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfo(const AAFwk::Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    };
    bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool NotifyAbilityLifeStatus(const std::string &bundleName,
        const std::string &abilityName, const int64_t launchTime, const int uid) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;

    BundleMgrService();
    virtual ~BundleMgrService() {}

    virtual bool CheckBundleNameInAllowList(const std::string &bundleName);

    bool CheckWantEntity(const AAFwk::Want &, AbilityInfo &);

    void MakingPackageData();
    void PushHiWorldAbility();
    void PushHiMusicAbility();
    void PushHiRadioAbility();
    void PushHiServiceAbility();
    void PushHiMusicServiceAbility();
    void PushHiDataAbility();
    void PushPhoneAbility();
    void PushTVAbility();
    void PushFilmAbility();
    void PushClockAbility();
    void PushClockCloneAbility();
    void PushEgcbility();
    void PushEgcClonebility();
    void PushSelectorAbility();

public:
    std::vector<BundleInfo> bundleInfos_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
