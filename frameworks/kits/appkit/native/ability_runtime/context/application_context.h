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

#ifndef ABILITY_RUNTIME_APPLICATION_CONTEXT_H
#define ABILITY_RUNTIME_APPLICATION_CONTEXT_H

#include <map>
#include <shared_mutex>

#include "ability_lifecycle_callback.h"
#include "context.h"
#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class ContextImpl;
class ApplicationContext : public Context, public std::enable_shared_from_this<ApplicationContext> {
public:
    ApplicationContext() = default;
    ~ApplicationContext() = default;
    void RegisterAbilityLifecycleCallback(
        const int64_t callbackId, const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback);
    void UnregisterAbilityLifecycleCallback(const int64_t callbackId);
    void DispatchOnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj);
    void DispatchOnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj);

    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config);
    void SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext);

    std::string GetBundleName() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::string GetBundleCodePath() const override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    sptr<IRemoteObject> GetToken() override;
    void SetToken(const sptr<IRemoteObject> &token) override;
    void SwitchArea(int mode) override;
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;
    std::string GetBaseDir() const override;

    void InitApplicationContext();
    void AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl);

private:
    std::shared_ptr<ContextImpl> contextImpl_;
    static std::map<int64_t, std::shared_ptr<AbilityLifecycleCallback>> callbacks_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_APPLICATION_CONTEXT_H