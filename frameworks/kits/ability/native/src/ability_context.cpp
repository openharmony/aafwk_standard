/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#endif // OS_ACCOUNT_PART_ENABLED
#include "resource_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
#ifndef OS_ACCOUNT_PART_ENABLED
namespace {
constexpr int UID_TRANSFORM_DIVISOR = 200000;
static void GetOsAccountIdFromUid(int uid, int &osAccountId)
{
    osAccountId = uid / UID_TRANSFORM_DIVISOR;
}
}
#endif // OS_ACCOUNT_PART_ENABLED

int AbilityContext::ABILITY_CONTEXT_DEFAULT_REQUEST_CODE(0);
const std::string GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string STATE_KEY = "ohos.user.grant.permission.state";

ErrCode AbilityContext::StartAbility(const AAFwk::Want &want, int requestCode)
{
    HILOG_INFO("AbilityContext::StartAbility called, requestCode = %{public}d", requestCode);

    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (type != AppExecFwk::AbilityType::PAGE && type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }

    HILOG_INFO("%{public}s. Start calling ams->StartAbility.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContext::StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting)
{
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (AppExecFwk::AbilityType::PAGE != type && AppExecFwk::AbilityType::SERVICE != type) {
        HILOG_ERROR("AbilityContext::StartAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }

    HILOG_INFO("%{public}s. Start calling ams->StartAbility.", __func__);
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, abilityStartSetting, token_, requestCode);
    HILOG_INFO("%{public}s. End calling ams->StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::StartAbility is failed %{public}d", err);
    }

    return err;
}

ErrCode AbilityContext::TerminateAbility(int requestCode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContext::TerminateAbility()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("AbilityContext::TerminateAbility info == nullptr");
        return ERR_NULL_OBJECT;
    }

    ErrCode err = ERR_OK;

    switch (info->type) {
        case AppExecFwk::AbilityType::PAGE:
            HILOG_INFO("Terminate ability begin, type is page, ability is %{public}s.", info->name.c_str());
            err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode_, &resultWant_);
            break;
        case AppExecFwk::AbilityType::SERVICE:
            HILOG_INFO("Terminate ability begin, type is service, ability is %{public}s.", info->name.c_str());
            err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
            break;
        default:
            HILOG_ERROR("AbilityContext::TerminateAbility info type error is %{public}d", info->type);
            break;
    }

    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

std::string AbilityContext::GetCallingBundle()
{
    return callingBundleName_;
}

std::shared_ptr<ElementName> AbilityContext::GetElementName()
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("AbilityContext::GetElementName info == nullptr");
        return nullptr;
    }

    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();
    if (elementName == nullptr) {
        HILOG_ERROR("AbilityContext::GetElementName elementName == nullptr");
        return nullptr;
    }
    elementName->SetAbilityName(info->name);
    elementName->SetBundleName(info->bundleName);
    elementName->SetDeviceID(info->deviceId);
    elementName->SetModuleName(info->moduleName);
    HILOG_INFO("%{public}s end.", __func__);
    return elementName;
}

std::shared_ptr<ElementName> AbilityContext::GetCallingAbility()
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::shared_ptr<ElementName> elementName = std::make_shared<ElementName>();

    if (elementName == nullptr) {
        HILOG_ERROR("AbilityContext::GetElementName elementName == nullptr");
        return nullptr;
    }
    elementName->SetAbilityName(callingAbilityName_);
    elementName->SetBundleName(callingBundleName_);
    elementName->SetDeviceID(callingDeviceId_);
    elementName->SetModuleName(callingModuleName_);
    HILOG_INFO("%{public}s end.", __func__);
    return elementName;
}

bool AbilityContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();

    std::shared_ptr<AbilityInfo> abilityInfo = GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityContext::ConnectAbility info == nullptr");
        return false;
    }

    HILOG_INFO("Connect ability begin, ability:%{public}s.", abilityInfo->name.c_str());

    if (AppExecFwk::AbilityType::PAGE != type && AppExecFwk::AbilityType::SERVICE != type) {
        HILOG_ERROR("AbilityContext::ConnectAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, conn, token_);
    HILOG_INFO("%{public}s end ConnectAbility, ret=%{public}d", __func__, ret);
    bool value = ((ret == ERR_OK) ? true : false);
    if (!value) {
        HILOG_ERROR("AbilityContext::ConnectAbility ErrorCode = %{public}d", ret);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return value;
}

ErrCode AbilityContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    HILOG_INFO("Disconnect ability begin, caller:%{public}s.", info == nullptr ? "" : info->name.c_str());

    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (AppExecFwk::AbilityType::PAGE != type && AppExecFwk::AbilityType::SERVICE != type) {
        HILOG_ERROR("AbilityContext::DisconnectAbility AbilityType = %{public}d", type);
        return ERR_INVALID_VALUE;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn);
    if (ret != ERR_OK) {
        HILOG_ERROR("AbilityContext::DisconnectAbility error, ret=%{public}d.", ret);
    }
    return ret;
}

bool AbilityContext::StopAbility(const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    AppExecFwk::AbilityType type = GetAbilityInfoType();
    if (AppExecFwk::AbilityType::PAGE != type && AppExecFwk::AbilityType::SERVICE != type) {
        HILOG_ERROR("AbilityContext::StopAbility AbilityType = %{public}d", type);
        return false;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopServiceAbility(want);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::StopAbility is failed %{public}d", err);
        return false;
    }

    return true;
}

sptr<IRemoteObject> AbilityContext::GetToken()
{
    return token_;
}

std::shared_ptr<ApplicationInfo> AbilityContext::GetApplicationInfo() const
{
    return ContextContainer::GetApplicationInfo();
}

std::string AbilityContext::GetCacheDir()
{
    return ContextContainer::GetCacheDir();
}

std::string AbilityContext::GetCodeCacheDir()
{
    return ContextContainer::GetCodeCacheDir();
}

std::string AbilityContext::GetDatabaseDir()
{
    return ContextContainer::GetDatabaseDir();
}

std::string AbilityContext::GetDataDir()
{
    return ContextContainer::GetDataDir();
}

std::string AbilityContext::GetDir(const std::string &name, int mode)
{
    return ContextContainer::GetDir(name, mode);
}

sptr<IBundleMgr> AbilityContext::GetBundleManager() const
{
    return ContextContainer::GetBundleManager();
}

std::string AbilityContext::GetBundleCodePath()
{
    return ContextContainer::GetBundleCodePath();
}

std::string AbilityContext::GetBundleName() const
{
    return ContextContainer::GetBundleName();
}

std::string AbilityContext::GetBundleResourcePath()
{
    return ContextContainer::GetBundleResourcePath();
}

std::shared_ptr<Context> AbilityContext::GetApplicationContext() const
{
    return ContextContainer::GetApplicationContext();
}

std::shared_ptr<Context> AbilityContext::GetContext()
{
    return ContextContainer::GetContext();
}

sptr<AAFwk::IAbilityManager> AbilityContext::GetAbilityManager()
{
    return ContextContainer::GetAbilityManager();
}

std::shared_ptr<ProcessInfo> AbilityContext::GetProcessInfo() const
{
    return ContextContainer::GetProcessInfo();
}

std::string AbilityContext::GetAppType()
{
    return ContextContainer::GetAppType();
}

const std::shared_ptr<AbilityInfo> AbilityContext::GetAbilityInfo()
{
    return ContextContainer::GetAbilityInfo();
}

std::shared_ptr<HapModuleInfo> AbilityContext::GetHapModuleInfo()
{
    return ContextContainer::GetHapModuleInfo();
}

AppExecFwk::AbilityType AbilityContext::GetAbilityInfoType()
{
    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("AbilityContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<Context> AbilityContext::CreateBundleContext(std::string bundleName, int flag, int accountId)
{
    return ContextContainer::CreateBundleContext(bundleName, flag, accountId);
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    std::shared_ptr<Context> appcontext = GetApplicationContext();
    if (appcontext == nullptr) {
        HILOG_ERROR("AbilityContext::GetResourceManager appcontext is nullptr");
        return nullptr;
    }

    HILOG_INFO("%{public}s begin appcontext->GetResourceManager.", __func__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = appcontext->GetResourceManager();
    HILOG_INFO("%{public}s end appcontext->GetResourceManager.", __func__);
    if (resourceManager == nullptr) {
        HILOG_ERROR("AbilityContext::GetResourceManager resourceManager is nullptr");
        return nullptr;
    }
    return resourceManager;
}

int AbilityContext::VerifyPermission(const std::string &permission, int pid, int uid)
{
    HILOG_INFO("%{public}s begin. permission=%{public}s, pid=%{public}d, uid=%{public}d",
        __func__,
        permission.c_str(),
        pid,
        uid);
    if (permission.empty()) {
        HILOG_ERROR("VerifyPermission permission invalid");
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }

    sptr<IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        HILOG_ERROR("VerifyPermission failed to get bundle manager service");
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }

    std::string bundle_name;
    if (!ptr->GetBundleNameForUid(uid, bundle_name)) {
        HILOG_ERROR("VerifyPermission failed to get bundle name by uid");
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }

    int account = -1;
#ifdef OS_ACCOUNT_PART_ENABLED
    if (AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, account) != 0) {
        HILOG_ERROR("VerifyPermission failed to get account by uid");
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }
#else // OS_ACCOUNT_PART_ENABLED
    GetOsAccountIdFromUid(uid, account);
#endif // OS_ACCOUNT_PART_ENABLED

    AppExecFwk::ApplicationInfo appInfo;
    if (!ptr->GetApplicationInfo(bundle_name, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, account, appInfo)) {
        HILOG_ERROR("VerifyPermission failed to get application info");
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }

    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_ERROR("VerifyPermission %{public}d: PERMISSION_DENIED", appInfo.accessTokenId);
        return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
    }

    return 0;
}

void AbilityContext::GetPermissionDes(const std::string &permissionName, std::string &des)
{
    Security::AccessToken::PermissionDef permissionDef;
    int32_t ret = Security::AccessToken::AccessTokenKit::GetDefPermission(permissionName, permissionDef);
    if (ret == Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        HILOG_DEBUG("GetPermissionDes %{public}s: RET_SUCCESS", permissionName.c_str());
        des = permissionDef.description;
    }
    HILOG_DEBUG("%{public}s end GetPermissionDef.", __func__);
}

void AbilityContext::RequestPermissionsFromUser(std::vector<std::string> &permissions,
    std::vector<int> &permissionsState, int requestCode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (permissions.size() == 0) {
        HILOG_ERROR("AbilityContext::RequestPermissionsFromUser permissions is empty");
        return;
    }

    if (requestCode < 0) {
        HILOG_ERROR("AbilityContext::RequestPermissionsFromUser requestCode should be >= 0");
        return;
    }

    AAFwk::Want want;
    want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
    want.SetParam(PERMISSION_KEY, permissions);
    want.SetParam(STATE_KEY, permissionsState);
    StartAbility(want, requestCode);
    HILOG_INFO("%{public}s end.", __func__);
}

bool AbilityContext::DeleteFile(const std::string &fileName)
{
    return ContextContainer::DeleteFile(fileName);
}

void AbilityContext::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    callingDeviceId_ = deviceId;
    callingBundleName_ = bundleName;
    callingAbilityName_ = abilityName;
    callingModuleName_ = moduleName;
}

Uri AbilityContext::GetCaller()
{
    return ContextContainer::GetCaller();
}

void AbilityContext::AttachBaseContext(const std::shared_ptr<Context> &base)
{
    HILOG_INFO("AbilityContext::AttachBaseContext.");
    ContextContainer::AttachBaseContext(base);
}

std::string AbilityContext::GetExternalCacheDir()
{
    return ContextContainer::GetExternalCacheDir();
}

std::string AbilityContext::GetExternalFilesDir(std::string &type)
{
    return ContextContainer::GetExternalFilesDir(type);
}

std::string AbilityContext::GetFilesDir()
{
    return ContextContainer::GetFilesDir();
}

std::string AbilityContext::GetNoBackupFilesDir()
{
    return ContextContainer::GetNoBackupFilesDir();
}

void AbilityContext::UnauthUriPermission(const std::string &permission, const Uri &uri, int uid)
{
    ContextContainer::UnauthUriPermission(permission, uri, uid);
}

std::string AbilityContext::GetDistributedDir()
{
    return ContextContainer::GetDistributedDir();
}

void AbilityContext::SetPattern(int patternId)
{
    ContextContainer::SetPattern(patternId);
}

std::shared_ptr<Context> AbilityContext::GetAbilityPackageContext()
{
    return ContextContainer::GetAbilityPackageContext();
}

std::string AbilityContext::GetProcessName()
{
    return ContextContainer::GetProcessName();
}

void AbilityContext::InitResourceManager(BundleInfo &bundleInfo, std::shared_ptr<ContextDeal> &deal)
{
    ContextContainer::InitResourceManager(bundleInfo, deal);
}

std::string AbilityContext::GetString(int resId)
{
    return ContextContainer::GetString(resId);
}

std::vector<std::string> AbilityContext::GetStringArray(int resId)
{
    return ContextContainer::GetStringArray(resId);
}

std::vector<int> AbilityContext::GetIntArray(int resId)
{
    return ContextContainer::GetIntArray(resId);
}

std::map<std::string, std::string> AbilityContext::GetTheme()
{
    return ContextContainer::GetTheme();
}

void AbilityContext::SetTheme(int themeId)
{
    ContextContainer::SetTheme(themeId);
}

std::map<std::string, std::string> AbilityContext::GetPattern()
{
    return ContextContainer::GetPattern();
}

int AbilityContext::GetColor(int resId)
{
    return ContextContainer::GetColor(resId);
}

int AbilityContext::GetThemeId()
{
    return ContextContainer::GetThemeId();
}

bool AbilityContext::TerminateAbilityResult(int startId)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("AbilityContext::TerminateAbilityResult abilityClient is nullptr");
        return false;
    }

    HILOG_INFO("%{public}s begin ams->TerminateAbilityResult, startId=%{public}d.", __func__, startId);
    ErrCode errval = abilityClient->TerminateAbilityResult(token_, startId);
    HILOG_INFO("%{public}s end ams->TerminateAbilityResult, ret=%{public}d.", __func__, errval);
    if (errval != ERR_OK) {
        HILOG_ERROR("AbilityContext::TerminateAbilityResult TerminateAbilityResult retval is %d", errval);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return (errval == ERR_OK) ? true : false;
}

int AbilityContext::GetDisplayOrientation()
{
    return ContextContainer::GetDisplayOrientation();
}

std::string AbilityContext::GetPreferencesDir()
{
    return ContextContainer::GetPreferencesDir();
}

void AbilityContext::SetColorMode(int mode)
{
    ContextContainer::SetColorMode(mode);
}

int AbilityContext::GetColorMode()
{
    return ContextContainer::GetColorMode();
}

int AbilityContext::GetMissionId()
{
    return ContextContainer::GetMissionId();
}

void AbilityContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{
    HILOG_INFO("%{public}s begin.", __func__);
    for (auto want : wants) {
        StartAbility(want, ABILITY_CONTEXT_DEFAULT_REQUEST_CODE);
    }
    HILOG_INFO("%{public}s end.", __func__);
}

bool AbilityContext::IsUpdatingConfigurations()
{
    return ContextContainer::IsUpdatingConfigurations();
}

bool AbilityContext::PrintDrawnCompleted()
{
    return ContextContainer::PrintDrawnCompleted();
}
}  // namespace AppExecFwk
}  // namespace OHOS
