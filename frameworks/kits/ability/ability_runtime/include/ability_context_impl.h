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

#ifndef ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
#define ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H

#include "ability_context.h"

#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContextImpl : public AbilityContext {
public:
    AbilityContextImpl() = default;
    virtual ~AbilityContextImpl() = default;

    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetStorageDir() override;
    std::string GetDistributedFilesDir() override;
    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;

    std::string GetBundleCodePath() const override;
    ErrCode StartAbility(const AAFwk::Want &want, int requestCode) override;
    ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode, RuntimeTask &&task) override;
    ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode) override;
    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) override;
    bool ConnectAbility(const AAFwk::Want &want,
                        const std::shared_ptr<AbilityConnectCallback> &connectCallback) override;
    void DisconnectAbility(const AAFwk::Want &want,
                           const std::shared_ptr<AbilityConnectCallback> &connectCallback) override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const override;
    void MinimizeAbility() override;

    ErrCode TerminateSelf() override;
    sptr<IRemoteObject> GetAbilityToken() override;
    void RequestPermissionsFromUser(const std::vector<std::string> &permissions, int requestCode) override;
    ErrCode RestoreWindowStage(void* contentStorage) override;

    void SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext);

    /**
     * @brief Set the Ability Info object
     *
     * set ability info to ability context
     */
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Attachs ability's token.
     *
     * @param token The token represents ability.
     */
    inline void SetToken(const sptr<IRemoteObject> &token)
    {
        token_ = token;
    }

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    void* GetContentStorage() override
    {
        return contentStorage_;
    }

private:
    sptr<IRemoteObject> token_;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<AbilityRuntime::Context> stageContext_ = nullptr;
    std::map<int, RuntimeTask> resultCallbacks_;
    void* contentStorage_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H