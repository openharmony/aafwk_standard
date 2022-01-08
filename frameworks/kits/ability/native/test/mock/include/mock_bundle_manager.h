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
#include <gtest/gtest.h>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_interface.h"
#include "ohos/aafwk/content/want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    ~BundleMgrProxy() = default;

    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override;
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override;
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override;
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override;
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override;
    virtual bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) override;
    virtual bool GetNameForUid(const int uid, std::string &name) override;
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    virtual std::string GetAppType(const std::string &bundleName) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override;
    virtual bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) override;
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) override;
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override;
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) override;
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override;
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override;
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override;
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission) override;
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override;
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) override;
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) override;
    virtual bool HasSystemCapability(const std::string &capName) override;
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override;
    virtual bool IsSafeMode() override;
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) override;
    virtual bool CleanBundleDataFiles(const std::string &bundleName, const int userId) override;
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    virtual bool UnregisterBundleStatusCallback() override;
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override;
    virtual bool IsApplicationEnabled(const std::string &bundleName) override;
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) override;
    virtual bool IsAbilityEnabled(const AbilityInfo &abilityInfo) override;
    virtual bool SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled) override;
    virtual std::string GetAbilityIcon(const std::string &bundleName, const std::string &className) override;
    virtual bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int userId) override;
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId) override;
    virtual bool RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool RegisterPermissionsChanged(
        const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override;
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override;
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override;
    virtual bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override;
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override;
    virtual sptr<IBundleInstaller> GetBundleInstaller() override;
    virtual bool NotifyAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid) override;
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IBundleMgr");
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override;
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override;
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override;
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override;
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override;
    virtual bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) override;
    virtual bool GetNameForUid(const int uid, std::string &name) override;
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override;
    virtual std::string GetAppType(const std::string &bundleName) override;
    virtual bool CheckIsSystemAppByUid(const int uid) override;
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override;
    virtual bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) override;
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos) override;
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) override;
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override;
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) override;
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override;
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override;
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override;
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override;
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission) override;
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override;
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) override;
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) override;
    virtual bool HasSystemCapability(const std::string &capName) override;
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override;
    virtual bool IsSafeMode() override;
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) override;
    virtual bool CleanBundleDataFiles(const std::string &bundleName, const int userId) override;
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override;
    virtual bool UnregisterBundleStatusCallback() override;
    virtual bool DumpInfos(
        const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result) override;
    virtual bool IsApplicationEnabled(const std::string &bundleName) override;
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) override;
    virtual bool IsAbilityEnabled(const AbilityInfo &abilityInfo) override;
    virtual bool SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled) override;
    virtual std::string GetAbilityIcon(const std::string &bundleName, const std::string &className) override;
    virtual bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int userId) override;
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId) override;
    virtual bool RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool RegisterPermissionsChanged(
        const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override;
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override;
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override;
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override;
    virtual bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override;
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override;
    virtual sptr<IBundleInstaller> GetBundleInstaller() override;
    virtual bool NotifyAbilityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid) override;
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
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MOCK_BUNDLE_MANAGER_H
