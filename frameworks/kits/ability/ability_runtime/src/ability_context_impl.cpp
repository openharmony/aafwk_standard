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

#include "ability_context_impl.h"

#include <native_engine/native_engine.h>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "hitrace_meter.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "permission_list_state.h"

using OHOS::Security::AccessToken::AccessTokenKit;
using OHOS::Security::AccessToken::PermissionListState;
using OHOS::Security::AccessToken::TypePermissionOper;

namespace OHOS {
namespace AbilityRuntime {
const size_t AbilityContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AbilityContext"));
const std::string GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string STATE_KEY = "ohos.user.grant.permission.state";


std::string AbilityContextImpl::GetBaseDir() const
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}

std::string AbilityContextImpl::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContextImpl::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContextImpl::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContextImpl::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

std::string AbilityContextImpl::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContextImpl::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string AbilityContextImpl::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

bool AbilityContextImpl::IsUpdatingConfigurations()
{
    return stageContext_ ? stageContext_->IsUpdatingConfigurations() : false;
}

bool AbilityContextImpl::PrintDrawnCompleted()
{
    return stageContext_ ? stageContext_->PrintDrawnCompleted() : false;
}

void AbilityContextImpl::SwitchArea(int mode)
{
    HILOG_DEBUG("AbilityContextImpl::SwitchArea.");
    if (stageContext_ != nullptr) {
        stageContext_->SwitchArea(mode);
    }
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want &want, int requestCode)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityWithAccount(const AAFwk::Want &want, int accountId, int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbilityWithAccount. Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    HILOG_INFO("AbilityContextImpl::StartAbilityWithAccount. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
    int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbility. Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityWithAccount(
    const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions, int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbilityWithAccount. Start calling StartAbility.");
    HILOG_INFO(
        "%{public}s called, bundleName=%{public}s, abilityName=%{public}s, accountId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    HILOG_INFO("AbilityContextImpl::StartAbilityWithAccount. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResult.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("%{public}s. End calling StartAbilityForResult. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want &want, const int accountId, int requestCode, RuntimeTask &&task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResultWithAccount. accountId:%{public}d",
        __func__, accountId);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    HILOG_INFO("%{public}s. End calling StartAbilityForResultWithAccount. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
    int requestCode, RuntimeTask &&task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResult.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    HILOG_INFO("%{public}s. End calling StartAbilityForResult. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions,
    int requestCode, RuntimeTask &&task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResultWithAccount.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    HILOG_INFO("%{public}s. End calling StartAbilityForResultWithAccount. ret=%{public}d", __func__, err);
    return err;
}

ErrCode AbilityContextImpl::TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode)
{
    HILOG_DEBUG("%{public}s. Start calling TerminateAbilityWithResult.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode, &want);
    HILOG_INFO("%{public}s. End calling TerminateAbilityWithResult. ret=%{public}d", __func__, err);
    return err;
}

void AbilityContextImpl::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    HILOG_DEBUG("%{public}s. Start calling OnAbilityResult.", __func__);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_INFO("%{public}s. End calling OnAbilityResult.", __func__);
}

bool AbilityContextImpl::ConnectAbility(const AAFwk::Want &want,
                                        const sptr<AbilityConnectCallback> &connectCallback)
{
    HILOG_DEBUG("Connect ability begin, ability:%{public}s.",
        abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("AbilityContextImpl::ConnectAbility ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

bool AbilityContextImpl::ConnectAbilityWithAccount(const AAFwk::Want &want, int accountId,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    HILOG_INFO("AbilityContextImpl::ConnectAbility ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

void AbilityContextImpl::DisconnectAbility(const AAFwk::Want &want,
                                           const sptr<AbilityConnectCallback> &connectCallback)
{
    HILOG_DEBUG("Disconnect ability begin, caller:%{public}s.",
        abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
}

std::string AbilityContextImpl::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContextImpl::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::string AbilityContextImpl::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContextImpl::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContextImpl::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

std::shared_ptr<Context> AbilityContextImpl::CreateBundleContext(const std::string &bundleName)
{
    return stageContext_ ? stageContext_->CreateBundleContext(bundleName) : nullptr;
}

void AbilityContextImpl::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContextImpl::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContextImpl::SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    stageContext_ = stageContext;
}

void AbilityContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    config_ = config;
}

std::shared_ptr<AppExecFwk::Configuration> AbilityContextImpl::GetConfiguration() const
{
    return config_;
}

void AbilityContextImpl::MinimizeAbility(bool fromUser)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->MinimizeAbility(token_, fromUser);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::MinimizeAbility is failed %{public}d", err);
    }
}

ErrCode AbilityContextImpl::TerminateSelf()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::TerminateSelf is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContextImpl::CloseAbility()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->CloseAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("CloseAbility failed: %{public}d", err);
    }
    return err;
}

sptr<IRemoteObject> AbilityContextImpl::GetToken()
{
    return token_;
}

void AbilityContextImpl::RequestPermissionsFromUser(const std::vector<std::string> &permissions,
    int requestCode, PermissionRequestTask &&task)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (permissions.size() == 0 || requestCode < 0) {
        HILOG_ERROR("%{public}s. The params are invalid.", __func__);
        return;
    }

    std::vector<PermissionListState> permList;
    for (auto permission : permissions) {
        HILOG_DEBUG("%{public}s. permission: %{public}s.", __func__, permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = -1;
        permList.emplace_back(permState);
    }
    HILOG_DEBUG("%{public}s. permList size: %{public}zu, permissions size: %{public}zu.",
        __func__, permList.size(), permissions.size());

    auto ret = AccessTokenKit::GetSelfPermissionsState(permList);
    if (permList.size() != permissions.size()) {
        HILOG_ERROR("%{public}s. Returned permList size: %{public}zu.", __func__, permList.size());
        return;
    }

    std::vector<int> permissionsState;
    for (auto permState : permList) {
        HILOG_DEBUG("%{public}s. permissions: %{public}s. permissionsState: %{public}u",
            __func__, permState.permissionName.c_str(), permState.state);
        permissionsState.emplace_back(permState.state);
    }
    HILOG_DEBUG("%{public}s. permissions size: %{public}zu. permissionsState size: %{public}zu",
        __func__, permissions.size(), permissionsState.size());

    if (ret == TypePermissionOper::DYNAMIC_OPER) {
        AAFwk::Want want;
        want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
        want.SetParam(PERMISSION_KEY, permissions);
        want.SetParam(STATE_KEY, permissionsState);
        permissionRequestCallbacks_.insert(make_pair(requestCode, std::move(task)));
        HILOG_DEBUG("%{public}s. Start calling StartAbility.", __func__);
        ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
        HILOG_DEBUG("%{public}s. End calling StartAbility. ret=%{public}d", __func__, err);
    } else {
        HILOG_DEBUG("%{public}s. No dynamic popup required.", __func__);
        if (task) {
            task(permissions, permissionsState);
        }
    }
}

void AbilityContextImpl::OnRequestPermissionsFromUserResult(
    int requestCode, const std::vector<std::string> &permissions, const std::vector<int> &permissionsState)
{
    HILOG_DEBUG("%{public}s. Start calling OnRequestPermissionsFromUserResult.", __func__);
    auto iter = permissionRequestCallbacks_.find(requestCode);
    if (iter != permissionRequestCallbacks_.end() && iter->second) {
        auto task = iter->second;
        task(permissions, permissionsState);
        permissionRequestCallbacks_.erase(iter);
        HILOG_DEBUG("%{public}s. End calling OnRequestPermissionsFromUserResult.", __func__);
    }
}

ErrCode AbilityContextImpl::RestoreWindowStage(NativeEngine& engine, NativeValue* contentStorage)
{
    HILOG_INFO("%{public}s begin.", __func__);
    contentStorage_ = std::unique_ptr<NativeReference>(engine.CreateReference(contentStorage, 1));
    return ERR_OK;
}

ErrCode AbilityContextImpl::StartAbility(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback)
{
    if (localCallContainer_ == nullptr) {
        localCallContainer_ = new (std::nothrow)LocalCallContainer();
    }

    return localCallContainer_->StartAbilityInner(want, callback, token_);
}

ErrCode AbilityContextImpl::ReleaseAbility(const std::shared_ptr<CallerCallBack> &callback)
{
    HILOG_DEBUG("AbilityContextImpl::Release begain.");
    if (!localCallContainer_) {
        HILOG_ERROR("%{public}s false.", __func__);
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("AbilityContextImpl::Release end.");
    return localCallContainer_->Release(callback);
}

ErrCode AbilityContextImpl::SetMissionLabel(const std::string &label)
{
    HILOG_INFO("%{public}s begin. label = %{public}s", __func__, label.c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionLabel(token_, label);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::SetMissionLabel is failed %{public}d", err);
    }
    return err;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityContextImpl::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionIcon(token_, icon);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::SetMissionIcon is failed %{public}d", err);
    }
    return err;
}
#endif

void AbilityContextImpl::RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback)
{
    HILOG_INFO("%{public}s called.", __func__);
    abilityCallback_ = abilityCallback;
}

#ifdef SUPPORT_GRAPHICS
int AbilityContextImpl::GetCurrentWindowMode()
{
    HILOG_INFO("%{public}s called.", __func__);
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        return AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    }
    return abilityCallback->GetCurrentWindowMode();
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
