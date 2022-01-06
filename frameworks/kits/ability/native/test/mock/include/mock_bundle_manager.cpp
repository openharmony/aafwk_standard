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
#include <gtest/gtest.h>
#include "ability_info.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}
bool BundleMgrProxy::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
{
    return true;
}
bool BundleMgrProxy::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}
bool BundleMgrProxy::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
int BundleMgrProxy::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return 0;
}
std::string BundleMgrProxy::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    return "";
}
bool BundleMgrProxy::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    return true;
}
bool BundleMgrProxy::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    return true;
}
bool BundleMgrProxy::GetNameForUid(const int uid, std::string &name)
{
    return true;
}
bool BundleMgrProxy::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return true;
}
std::string BundleMgrProxy::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetAppTyp";
    return "system";
}
bool BundleMgrProxy::CheckIsSystemAppByUid(const int uid)
{
    return true;
}
bool BundleMgrProxy::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
bool BundleMgrProxy::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    ElementName eleName = want.GetElement();
    if (eleName.GetBundleName().empty()) {
        return false;
    }
    abilityInfo.visible = true;
    abilityInfo.name = eleName.GetAbilityName();
    abilityInfo.bundleName = eleName.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    return true;
}
bool BundleMgrProxy::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return true;
}
bool BundleMgrProxy::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return true;
}
bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    return false;
}
bool BundleMgrProxy::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
std::string BundleMgrProxy::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    return "";
}
bool BundleMgrProxy::GetBundleArchiveInfo(
    const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}
bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrProxy::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}
bool BundleMgrProxy::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    return true;
}
int BundleMgrProxy::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    return 0;
}
int BundleMgrProxy::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    return 0;
}
bool BundleMgrProxy::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    return true;
}
bool BundleMgrProxy::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return true;
}
bool BundleMgrProxy::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return true;
}
bool BundleMgrProxy::HasSystemCapability(const std::string &capName)
{
    return true;
}
bool BundleMgrProxy::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return true;
}
bool BundleMgrProxy::IsSafeMode()
{
    return true;
}
bool BundleMgrProxy::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
{
    return true;
}
bool BundleMgrProxy::CleanBundleDataFiles(const std::string &bundleName)
{
    return true;
}
bool BundleMgrProxy::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrProxy::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrProxy::UnregisterBundleStatusCallback()
{
    return true;
}
bool BundleMgrProxy::DumpInfos(
    const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
{
    return true;
}
bool BundleMgrProxy::IsApplicationEnabled(const std::string &bundleName)
{
    return true;
}
bool BundleMgrProxy::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    return true;
}
bool BundleMgrProxy::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    return true;
}
bool BundleMgrProxy::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    return true;
}
std::string BundleMgrProxy::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    return "";
}
bool BundleMgrProxy::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    return true;
}
bool BundleMgrProxy::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    return true;
}
bool BundleMgrProxy::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrProxy::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrProxy::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrProxy::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrProxy::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrProxy::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrProxy::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    return true;
}
bool BundleMgrProxy::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
{
    return true;
}
sptr<IBundleInstaller> BundleMgrProxy::GetBundleInstaller()
{
    return nullptr;
}
bool BundleMgrProxy::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
{
    return true;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    GTEST_LOG_(INFO) << " BundleMgrStub::OnRemoteRequest";
    return 0;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.foobar.hiworld";
    return true;
}
bool BundleMgrService::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
{
    return true;
}
bool BundleMgrService::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}
bool BundleMgrService::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
int BundleMgrService::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return 0;
}
std::string BundleMgrService::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    return "";
}
bool BundleMgrService::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    return true;
}
bool BundleMgrService::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    return true;
}
bool BundleMgrService::GetNameForUid(const int uid, std::string &name)
{
    return true;
}
bool BundleMgrService::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return true;
}
std::string BundleMgrService::GetAppType(const std::string &bundleName)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetAppType";
    return "system";
}
bool BundleMgrService::CheckIsSystemAppByUid(const int uid)
{
    return true;
}
bool BundleMgrService::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
bool BundleMgrService::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    ElementName elementName = want.GetElement();
    if (elementName.GetBundleName().empty()) {
        return false;
    }
    if (std::string::npos != elementName.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.visible = true;
    abilityInfo.name = elementName.GetAbilityName();
    abilityInfo.bundleName = elementName.GetBundleName();
    abilityInfo.applicationName = elementName.GetBundleName();
    if (want.HasEntity(Want::ENTITY_HOME) && want.GetAction() == Want::ACTION_HOME) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
    }
    return true;
}
bool BundleMgrService::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return true;
}
bool BundleMgrService::QueryAbilityInfosForClone(const Want &want, std::vector<AbilityInfo> &abilityInfos)
{
    return true;
}
bool BundleMgrService::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    return false;
}
bool BundleMgrService::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
std::string BundleMgrService::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    return "";
}
bool BundleMgrService::GetBundleArchiveInfo(
    const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}
bool BundleMgrService::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    GTEST_LOG_(INFO) << " BundleMgrService::GetHapModuleInfo";
    hapModuleInfo.name = abilityInfo.package;
    return true;
}
bool BundleMgrService::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    return true;
}
int BundleMgrService::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    return 0;
}
int BundleMgrService::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    return 0;
}
bool BundleMgrService::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    return true;
}
bool BundleMgrService::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return true;
}
bool BundleMgrService::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return true;
}
bool BundleMgrService::HasSystemCapability(const std::string &capName)
{
    return true;
}
bool BundleMgrService::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return true;
}
bool BundleMgrService::IsSafeMode()
{
    return true;
}
bool BundleMgrService::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
{
    return true;
}
bool BundleMgrService::CleanBundleDataFiles(const std::string &bundleName)
{
    return true;
}
bool BundleMgrService::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrService::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrService::UnregisterBundleStatusCallback()
{
    return true;
}
bool BundleMgrService::DumpInfos(
    const DumpFlag flag, const std::string &bundleName, int32_t userId, std::string &result)
{
    return true;
}
bool BundleMgrService::IsApplicationEnabled(const std::string &bundleName)
{
    return true;
}
bool BundleMgrService::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    return true;
}
bool BundleMgrService::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    return true;
}
bool BundleMgrService::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    return true;
}
std::string BundleMgrService::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    return "";
}
bool BundleMgrService::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    return true;
}
bool BundleMgrService::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    return true;
}
bool BundleMgrService::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrService::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrService::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrService::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    return true;
}
bool BundleMgrService::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords)
{
    return true;
}
sptr<IBundleInstaller> BundleMgrService::GetBundleInstaller()
{
    return nullptr;
}
bool BundleMgrService::NotifyAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
{
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
