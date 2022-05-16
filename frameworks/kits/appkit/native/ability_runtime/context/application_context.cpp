/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "application_context.h"

#include <algorithm>

#include "configuration.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const int EL2 = 1;
std::map<int64_t, std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::shared_ptr<ApplicationContext> Context::applicationContext_ = nullptr;

std::shared_ptr<ApplicationContext> Context::GetJsApplicationContext()
{
    std::lock_guard<std::mutex> lock(Context::contextMutex_);
    return applicationContext_;
}

void ApplicationContext::InitApplicationContext()
{
    std::lock_guard<std::mutex> lock(Context::contextMutex_);
    applicationContext_ = shared_from_this();
}

void ApplicationContext::AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl)
{
    contextImpl_ = contextImpl;
}

void ApplicationContext::RegisterAbilityLifecycleCallback(
    const int64_t callbackId, const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    HILOG_INFO("ApplicationContext RegisterAbilityLifecycleCallback");
    callbacks_.emplace(callbackId, abilityLifecycleCallback);
}

void ApplicationContext::UnregisterAbilityLifecycleCallback(const int64_t callbackId)
{
    HILOG_INFO("ApplicationContext UnregisterAbilityLifecycleCallback");
    auto iter = callbacks_.find(callbackId);
    if (iter != callbacks_.end()) {
        HILOG_INFO("callbackId exist");
        callbacks_.erase(iter);
    }
}

void ApplicationContext::DispatchOnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityCreate(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityWindowStageCreate(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityWindowStageDestroy(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityDestroy(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityForeground(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityBackground(abilityObj);
    }
}
void ApplicationContext::DispatchOnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj)
{
    if (abilityObj.expired()) {
        HILOG_WARN("abilityObj is nullptr");
        return;
    }
    for (auto callback : callbacks_) {
        callback.second->OnAbilityContinue(abilityObj);
    }
}

std::string ApplicationContext::GetBundleName() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleName() : "";
}

std::shared_ptr<Context> ApplicationContext::CreateBundleContext(const std::string &bundleName)
{
    return (contextImpl_ != nullptr) ? contextImpl_->CreateBundleContext(bundleName) : nullptr;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ApplicationContext::GetApplicationInfo() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetApplicationInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::GetResourceManager() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceManager() : nullptr;
}

std::string ApplicationContext::GetBundleCodePath() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetHapModuleInfo() : nullptr;
}

std::string ApplicationContext::GetBundleCodeDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodeDir() : "";
}

std::string ApplicationContext::GetCacheDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetCacheDir() : "";
}

std::string ApplicationContext::GetTempDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetTempDir() : "";
}

std::string ApplicationContext::GetFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetFilesDir() : "";
}

bool ApplicationContext::IsUpdatingConfigurations()
{
    return (contextImpl_ != nullptr) ? contextImpl_->IsUpdatingConfigurations() : false;
}

bool ApplicationContext::PrintDrawnCompleted()
{
    return (contextImpl_ != nullptr) ? contextImpl_->PrintDrawnCompleted() : false;
}

std::string ApplicationContext::GetDatabaseDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDatabaseDir() : "";
}

std::string ApplicationContext::GetPreferencesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetPreferencesDir() : "";
}

std::string ApplicationContext::GetDistributedFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDistributedFilesDir() : "";
}

sptr<IRemoteObject> ApplicationContext::GetToken()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetToken() : nullptr;
}

void ApplicationContext::SetToken(const sptr<IRemoteObject> &token)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetToken(token);
    }
}

void ApplicationContext::SwitchArea(int mode)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SwitchArea(mode);
    }
}

int ApplicationContext::GetArea()
{
    if (contextImpl_ != nullptr) {
        return contextImpl_->GetArea();
    }
    HILOG_ERROR("AbilityContext::contextImpl is nullptr.");
    return EL2;
}


std::shared_ptr<AppExecFwk::Configuration> ApplicationContext::GetConfiguration() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetConfiguration() : nullptr;
}

std::string ApplicationContext::GetBaseDir() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBaseDir() : nullptr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS