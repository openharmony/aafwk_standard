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

#include "ohos_application.h"

#include "ability_record_mgr.h"
#include "app_loader.h"
#include "application_impl.h"
#include "context_impl.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "runtime.h"
#include "system_ability_definition.h"
#include "ability_thread.h"

namespace OHOS {
namespace AppExecFwk {
REGISTER_APPLICATION(OHOSApplication, OHOSApplication)

OHOSApplication::OHOSApplication()
{
    abilityLifecycleCallbacks_.clear();
    elementsCallbacks_.clear();
}

OHOSApplication::~OHOSApplication() = default;

/**
 *
 * @brief Called when Ability#onSaveAbilityState(PacMap) was called on an ability.
 *
 * @param outState Indicates the PacMap object passed to Ability#onSaveAbilityState(PacMap)
 * for storing user data and states. This parameter cannot be null.
 */

void OHOSApplication::DispatchAbilitySavedState(const PacMap &outState)
{
    HILOG_INFO("OHOSApplication::dispatchAbilitySavedState: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilitySaveState(outState);
        }
    }
}

/**
 *
 * @brief Will be called the application foregrounds
 *
 */
void OHOSApplication::OnForeground()
{}

/**
 *
 * @brief Will be called the application backgrounds
 *
 */
void OHOSApplication::OnBackground()
{}

void OHOSApplication::DumpApplication()
{
    HILOG_DEBUG("OHOSApplication::Dump called");
    // create and initialize abilityInfos
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AbilityLocalRecord> record = nullptr;

    if (abilityRecordMgr_) {
        record = abilityRecordMgr_->GetAbilityItem(abilityRecordMgr_->GetToken());
    }

    if (record) {
        abilityInfo = record->GetAbilityInfo();
    }

    if (abilityInfo) {
        HILOG_DEBUG("==============AbilityInfo==============");
        HILOG_DEBUG("abilityInfo: package: %{public}s", abilityInfo->package.c_str());
        HILOG_DEBUG("abilityInfo: name: %{public}s", abilityInfo->name.c_str());
        HILOG_DEBUG("abilityInfo: label: %{public}s", abilityInfo->label.c_str());
        HILOG_DEBUG("abilityInfo: description: %{public}s", abilityInfo->description.c_str());
        HILOG_DEBUG("abilityInfo: iconPath: %{public}s", abilityInfo->iconPath.c_str());
        HILOG_DEBUG("abilityInfo: visible: %{public}d", abilityInfo->visible);
        HILOG_DEBUG("abilityInfo: kind: %{public}s", abilityInfo->kind.c_str());
        HILOG_DEBUG("abilityInfo: type: %{public}d", abilityInfo->type);
        HILOG_DEBUG("abilityInfo: orientation: %{public}d", abilityInfo->orientation);
        HILOG_DEBUG("abilityInfo: launchMode: %{public}d", abilityInfo->launchMode);
        for (auto permission : abilityInfo->permissions) {
            HILOG_DEBUG("abilityInfo: permission: %{public}s", permission.c_str());
        }
        HILOG_DEBUG("abilityInfo: bundleName: %{public}s", abilityInfo->bundleName.c_str());
        HILOG_DEBUG("abilityInfo: applicationName: %{public}s", abilityInfo->applicationName.c_str());
    }

    // create and initialize applicationInfo
    HILOG_DEBUG("==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> applicationInfoPtr = GetApplicationInfo();
    if (applicationInfoPtr != nullptr) {
        HILOG_DEBUG("applicationInfo: name: %{public}s", applicationInfoPtr->name.c_str());
        HILOG_DEBUG("applicationInfo: bundleName: %{public}s", applicationInfoPtr->bundleName.c_str());
        HILOG_DEBUG("applicationInfo: signatureKey: %{public}s", applicationInfoPtr->signatureKey.c_str());
    }
}

/**
 * @brief Set Runtime
 *
 * @param runtime Runtime instance.
 */
void OHOSApplication::SetRuntime(std::unique_ptr<AbilityRuntime::Runtime>&& runtime)
{
    HILOG_INFO("OHOSApplication::SetRuntime begin");
    if (runtime == nullptr) {
        HILOG_ERROR("OHOSApplication::SetRuntime failed, runtime is empty");
        return;
    }
    runtime_ = std::move(runtime);
    HILOG_INFO("OHOSApplication::SetRuntime end");
}

/**
 * @brief Set ApplicationContext
 *
 * @param abilityRuntimeContext ApplicationContext instance.
 */
void OHOSApplication::SetApplicationContext(const std::shared_ptr<AbilityRuntime::Context> &abilityRuntimeContext)
{
    HILOG_INFO("OHOSApplication::SetApplicationContext");
    if (abilityRuntimeContext == nullptr) {
        HILOG_ERROR("OHOSApplication::SetApplicationContext failed, context is empty");
        return;
    }
    abilityRuntimeContext_ = abilityRuntimeContext;
}

/**
 *
 * @brief Set the abilityRecordMgr to the OHOSApplication.
 *
 * @param abilityRecordMgr
 */
void OHOSApplication::SetAbilityRecordMgr(const std::shared_ptr<AbilityRecordMgr> &abilityRecordMgr)
{
    HILOG_INFO("OHOSApplication::SetAbilityRecordMgr");
    if (abilityRecordMgr == nullptr) {
        HILOG_ERROR("ContextDeal::SetAbilityRecordMgr failed, abilityRecordMgr is nullptr");
        return;
    }
    abilityRecordMgr_ = abilityRecordMgr;
}

/**
 *
 * Register AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack callBack When the life cycle of the ability in the application changes,
 */
void OHOSApplication::RegisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    HILOG_INFO("OHOSApplication::RegisterAbilityLifecycleCallbacks: called");

    if (callBack == nullptr) {
        HILOG_INFO("OHOSApplication::RegisterAbilityLifecycleCallbacks: observer is null");
        return;
    }

    abilityLifecycleCallbacks_.emplace_back(callBack);
}

/**
 *
 * Unregister AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack RegisterAbilityLifecycleCallbacks`s callBack
 */
void OHOSApplication::UnregisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    HILOG_INFO("OHOSApplication::UnregisterAbilityLifecycleCallbacks: called");

    if (callBack == nullptr) {
        HILOG_INFO("OHOSApplication::UnregisterAbilityLifecycleCallbacks: observer is null");
        return;
    }

    abilityLifecycleCallbacks_.remove(callBack);
}

/**
 *
 * Will be called when the given ability calls Ability->onStart
 *
 * @param Ability Indicates the ability object that calls the onStart() method.
 */
void OHOSApplication::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityStart failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityStart: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStart(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onInactive
 *
 * @param Ability Indicates the Ability object that calls the onInactive() method.
 */
void OHOSApplication::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityInactive failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityInactive: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityInactive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onBackground
 *
 * @param Ability Indicates the Ability object that calls the onBackground() method.
 */
void OHOSApplication::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityBackground failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityBackground: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onForeground
 *
 * @param Ability Indicates the Ability object that calls the onForeground() method.
 */
void OHOSApplication::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityForeground failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityForeground: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onActive
 *
 * @param Ability Indicates the Ability object that calls the onActive() method.
 */
void OHOSApplication::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityActive failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityActive: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityActive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onStop
 *
 * @param Ability Indicates the Ability object that calls the onStop() method.
 */
void OHOSApplication::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ContextDeal::OnAbilityStop failed, ability is nullptr");
        return;
    }

    HILOG_INFO("OHOSApplication::OnAbilityStop: called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStop(ability);
        }
    }
}

/**
 *
 * @brief Register ElementsCallback with OHOSApplication
 *
 * @param callBack callBack when the system configuration of the device changes.
 */
void OHOSApplication::RegisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    HILOG_INFO("OHOSApplication::RegisterElementsCallbacks: called");

    if (callback == nullptr) {
        HILOG_INFO("OHOSApplication::RegisterElementsCallbacks: observer is null");
        return;
    }

    elementsCallbacks_.emplace_back(callback);
}

/**
 *
 * @brief Unregister ElementsCallback with OHOSApplication
 *
 * @param callback RegisterElementsCallbacks`s callback
 */
void OHOSApplication::UnregisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    HILOG_INFO("OHOSApplication::UnregisterElementsCallbacks: called");

    if (callback == nullptr) {
        HILOG_INFO("OHOSApplication::UnregisterElementsCallbacks: observer is null");
        return;
    }

    elementsCallbacks_.remove(callback);
}

/**
 *
 * @brief Will be Called when the system configuration of the device changes.
 *
 * @param config Indicates the new Configuration object.
 */
void OHOSApplication::OnConfigurationUpdated(const Configuration &config)
{
    HILOG_INFO("OHOSApplication::OnConfigurationUpdated: called");
    if (!abilityRecordMgr_ || !configuration_) {
        HILOG_INFO("abilityRecordMgr_ or configuration_ is null");
        return;
    }

    // Update own object configuration_
    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    configuration_->Merge(changeKeyV, config);

    // Notify all abilities
    HILOG_INFO(
        "Number of ability to be notified : [%{public}d]", static_cast<int>(abilityRecordMgr_->GetRecordCount()));
    for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
        auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
        if (abilityRecord && abilityRecord->GetAbilityThread()) {
            abilityRecord->GetAbilityThread()->ScheduleUpdateConfiguration(config);
        }
    }

    // Notify AbilityStage
    HILOG_INFO("Number of abilityStage to be notified : [%{public}zu]", abilityStages_.size());
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnConfigurationUpdated(config);
        }
    }

    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnConfigurationUpdated(nullptr, config);
        }
    }
}

/**
 *
 * @brief Called when the system has determined to trim the memory, for example,
 * when the ability is running in the background and there is no enough memory for
 * running as many background processes as possible.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void OHOSApplication::OnMemoryLevel(int level)
{
    HILOG_INFO("OHOSApplication::OnMemoryLevel: called");
    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnMemoryLevel(level);
        }
    }
}

/**
 *
 * @brief Will be called the application starts
 *
 */
void OHOSApplication::OnStart()
{
    HILOG_INFO("OnStart called.");
}

/**
 *
 * @brief Will be called the application ends
 *
 */
void OHOSApplication::OnTerminate()
{
    HILOG_INFO("OHOSApplication::OnTerminate: called");
}

/**
 *
 * @brief Called when an ability calls Ability#onSaveAbilityState(PacMap).
 * You can implement your own logic in this method.
 * @param outState IIndicates the {@link PacMap} object passed to the onSaveAbilityState() callback.
 *
 */
void OHOSApplication::OnAbilitySaveState(const PacMap &outState)
{
    DispatchAbilitySavedState(outState);
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::AddAbilityStage(
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        HILOG_ERROR("AddAbilityStage:abilityRecord is nullptr");
        return nullptr;
    }
    const std::shared_ptr<AbilityInfo> &abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AddAbilityStage:abilityInfo is nullptr");
        return nullptr;
    }
    std::string moduleName = abilityInfo->moduleName;
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator == abilityStages_.end()) {
        std::shared_ptr<AbilityRuntime::ContextImpl> stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
        stageContext->SetParentContext(abilityRuntimeContext_);
        stageContext->InitHapModuleInfo(abilityInfo);
        stageContext->SetConfiguration(GetConfiguration());
        std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo = stageContext->GetHapModuleInfo();
        if (hapModuleInfo == nullptr) {
            HILOG_ERROR("AddAbilityStage:hapModuleInfo is nullptr");
            return nullptr;
        }
        abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *hapModuleInfo);
        abilityStage->Init(stageContext);
        Want want;
        if (abilityRecord->GetWant()) {
            HILOG_INFO("want is ok, transport to abilityStage");
            want = *(abilityRecord->GetWant());
        }
        abilityStage->OnCreate(want);
        abilityStages_[moduleName] = abilityStage;
    } else {
        abilityStage = iterator->second;
    }
    const sptr<IRemoteObject> &token = abilityRecord->GetToken();
    if (token == nullptr) {
        HILOG_ERROR("AddAbilityStage:token is null");
        return nullptr;
    }
    abilityStage->AddAbility(token, abilityRecord);
    return abilityStage->GetContext();
}

bool OHOSApplication::AddAbilityStage(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    HILOG_INFO("OHOSApplication::AddAbilityStage");
    if (abilityRuntimeContext_ == nullptr) {
        HILOG_ERROR("OHOSApplication::AddAbilityStage abilityRuntimeContext_ is nullptr.");
        return false;
    }

    if (runtime_ == nullptr) {
        HILOG_ERROR("OHOSApplication::AddAbilityStage abilityRuntimeContext_ is nullptr.");
        return false;
    }

    if (abilityStages_.find(hapModuleInfo.moduleName) != abilityStages_.end()) {
        HILOG_ERROR("OHOSApplication::%{public}s: object already exists ", __func__);
        return false;
    }

    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    stageContext->SetParentContext(abilityRuntimeContext_);
    stageContext->InitHapModuleInfo(hapModuleInfo);
    auto moduleInfo = stageContext->GetHapModuleInfo();
    if (moduleInfo == nullptr) {
        HILOG_ERROR("OHOSApplication::%{public}s: moduleInfo is nullptr", __func__);
        return false;
    }
    auto abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *moduleInfo);
    abilityStage->Init(stageContext);
    Want want;
    abilityStage->OnCreate(want);
    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
    HILOG_ERROR("OHOSApplication::%{public}s: abilityStage insert and initialization", __func__);
    return true;
}

void OHOSApplication::CleanAbilityStage(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("CleanAbilityStage:abilityInfo is nullptr");
        return;
    }
    if (token == nullptr) {
        HILOG_ERROR("CleanAbilityStage:token is nullptr");
        return;
    }
    std::string moduleName = abilityInfo->moduleName;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator != abilityStages_.end()) {
        auto abilityStage = iterator->second;
        abilityStage->RemoveAbility(token);
        if (!abilityStage->ContainsAbility()) {
            abilityStage->OnDestory();
            abilityStages_.erase(moduleName);
        }
    }
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::GetAppContext() const
{
    return abilityRuntimeContext_;
}

const std::unique_ptr<AbilityRuntime::Runtime>& OHOSApplication::GetRuntime()
{
    return runtime_;
}

void OHOSApplication::SetConfiguration(const Configuration &config)
{
    if (!configuration_) {
        configuration_ = std::make_shared<Configuration>(config);
    }
}

void OHOSApplication::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName, std::string &flag)
{
    HILOG_INFO("OHOSApplication::ScheduleAcceptWant: called");
    auto iter = abilityStages_.find(moduleName);
    if (iter != abilityStages_.end()) {
        auto abilityStage = iter->second;
        if (abilityStage) {
            flag = abilityStage->OnAcceptWant(want);
        }
    }
}

std::shared_ptr<Configuration> OHOSApplication::GetConfiguration()
{
    return configuration_;
}

void OHOSApplication::SetExtensionTypeMap(std::map<int32_t, std::string> map)
{
    extensionTypeMap_ = map;
}
}  // namespace AppExecFwk
}  // namespace OHOS
