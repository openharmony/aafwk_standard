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

#include "ability.h"
#include <gtest/gtest.h>
#include "ability_loader.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
REGISTER_AA(Ability)

void Ability::Init(std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Ability::Init called.");

    abilityInfo_ = abilityInfo;
    handler_ = handler;
    AbilityContext::token_ = token;
#ifdef SUPPORT_GRAPHICS
    // page ability only.
    if (abilityInfo_->type == AbilityType::PAGE) {
        abilityWindow_ = std::make_shared<AbilityWindow>();
    }
#endif
    lifecycle_ = std::make_shared<LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AbilityLifecycleExecutor>();
    application_ = application;
}

std::shared_ptr<Global::Resource::ResourceManager> Ability::GetResourceManager() const
{
    std::shared_ptr<Global::Resource::ResourceManager> remoteObject(Global::CreateResourceManager());
    return remoteObject;
}

void Ability::OnStart(const Want &want)
{
    return;
}

void Ability::OnStop()
{
    return;
}

void Ability::OnActive()
{
    return;
}

void Ability::OnInactive()
{}

#ifdef SUPPORT_GRAPHICS
void Ability::OnForeground(const Want &want)
{
    return;
}

void Ability::OnBackground()
{
    return;
}
#endif

sptr<IRemoteObject> Ability::OnConnect(const Want &want)
{
    return nullptr;
}

void Ability::OnDisconnect(const Want &want)
{}

void Ability::StartAbilityForResult(const Want &want, int requestCode)
{
    return;
}

void Ability::StartAbilityForResult(const Want &want, int requestCode, AbilityStartSetting abilityStartSetting)
{
    return;
}

void Ability::StartAbility(const Want &want, AbilityStartSetting abilityStartSetting)
{
    return;
}

#ifdef SUPPORT_GRAPHICS
bool Ability::OnKeyDown(int keyCode, const KeyEvent &keyEvent)
{
    return false;
}

bool Ability::OnKeyUp(int keyCode, const KeyEvent &keyEvent)
{
    return false;
}

bool Ability::OnTouchEvent(const TouchEvent &touchEvent)
{
    return false;
}

void Ability::SetUIContent(const ComponentContainer &componentContainer)
{
    return;
}

void Ability::SetUIContent(int layoutRes)
{
    return;
}  // namespace AppExecFwk

void Ability::SetUIContent(
    const ComponentContainer &componentContainer, std::shared_ptr<Context> &context, int typeFlag)
{
    return;
}

void Ability::SetUIContent(int layoutRes, std::shared_ptr<Context> &context, int typeFlag)
{
    return;
}

/**
 * @brief Inflates UI controls by using WindowConfig.
 *
 * @param config Indicates the window config defined by the user.
 */
void Ability::SetUIContent(const WindowConfig &config)
{
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::SetUIContent abilityWindow_ is nullptr");
        return;
    }

    HILOG_INFO("Ability::SetUIContent called");
    abilityWindow_->SetWindowConfig(config);
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a IWindowsManager object pointer.
 */
std::unique_ptr<Window> &Ability::GetWindow(int windowID)
{
    HILOG_INFO("Ability::GetWindow called windowID = %d.", windowID);

    return abilityWindow_->GetWindow(windowID);
}

int Ability::GetVolumeTypeAdjustedByKey()
{
    return 0;
}

bool Ability::HasWindowFocus()
{
    return false;
}

bool Ability::OnKeyPressAndHold(int keyCode, const std::shared_ptr<KeyEvent> &keyEvent)
{
    return false;
}
#endif

void Ability::OnRequestPermissionsFromUserResult(
    int requestCode, const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{
    return;
}

#ifdef SUPPORT_GRAPHICS
void Ability::OnLeaveForeground()
{
    return;
}
#endif

std::string Ability::GetType(const Uri &uri)
{
    std::string value("\nullptr");
    return value;
}

int Ability::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    GTEST_LOG_(INFO) << "Mock Ability::Insert called";
    return 1;
}

void Ability::OnConfigurationUpdated(const Configuration &configuration)
{
    return;
}

void Ability::OnMemoryLevel(int level)
{
    return;
}

std::shared_ptr<RawFileDescriptor> Ability::OpenRawFile(const Uri &uri, const std::string &mode)
{
    return nullptr;
}

int Ability::Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    GTEST_LOG_(INFO) << "Mock Ability::Update called";
    return 1;
}

std::shared_ptr<OHOSApplication> Ability::GetApplication()
{
    return application_;
}

std::string Ability::GetAbilityName()
{
    return abilityInfo_->name;
}

bool Ability::IsTerminating()
{
    return false;
}

void Ability::OnAbilityResult(int requestCode, int resultCode, const Want &want)
{}

void Ability::OnBackPressed()
{
    return;
}

void Ability::OnNewWant(const Want &want)
{
    HILOG_INFO("Ability::OnNewWant called");
}

void Ability::OnRestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("Ability::OnRestoreAbilityState called");
}

void Ability::OnSaveAbilityState(const PacMap &outState)
{
    HILOG_INFO("Ability::OnSaveAbilityState called");
}

void Ability::OnEventDispatch()
{
    return;
}

#ifdef SUPPORT_GRAPHICS
void Ability::OnWindowFocusChanged(bool hasFocus)
{
    return;
}
#endif

void Ability::SetWant(const AAFwk::Want &want)
{
    setWant_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> Ability::GetWant()
{
    return setWant_;
}

void Ability::SetResult(int resultCode, const Want &resultData)
{
    if (abilityInfo_ == nullptr) {
        HILOG_INFO("Ability::SetResult nullptr == abilityInfo_");
        return;
    }
    HILOG_INFO("Ability::SetResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        AbilityContext::resultWant_ = resultData;
        AbilityContext::resultCode_ = resultCode;
    }
}

#ifdef SUPPORT_GRAPHICS
void Ability::SetVolumeTypeAdjustedByKey(int volumeType)
{
    return;
}
#endif

void Ability::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    return;
}

void Ability::Dump(const std::string &extra)
{
    return;
}

void Ability::KeepBackgroundRunning(int id, const NotificationRequest &notificationRequest)
{
    return;
}

void Ability::CancelBackgroundRunning()
{
    return;
}

const std::shared_ptr<Uri> Ability::NormalizeUri(const Uri &uri)
{
    return nullptr;
}

int Ability::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    GTEST_LOG_(INFO) << "Mock Ability::Delete called";
    return 1;
}

std::vector<std::string> Ability::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> value;
    value.push_back(mimeTypeFilter);
    GTEST_LOG_(INFO) << "Mock Ability::GetFileTypes called";
    return value;
}

int Ability::OpenFile(const Uri &uri, const std::string &mode)
{
    GTEST_LOG_(INFO) << "Mock Ability::OpenFile called";
    return 1;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> Ability::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    GTEST_LOG_(INFO) << "Mock Ability::Query called";
    return nullptr;
}

bool Ability::Reload(const Uri &uri, const PacMap &extras)
{
    return true;
}

void Ability::ContinueAbilityReversibly(const std::string &deviceId)
{
    return;
}

void Ability::ContinueAbilityReversibly()
{
    return;
}

std::string Ability::GetOriginalDeviceId()
{
    return "";
}

std::shared_ptr<ContinuationState> Ability::GetContinuationState()
{
    return nullptr;
}

std::shared_ptr<AbilityPackage> Ability::GetAbilityPackage()
{
    return nullptr;
}

std::shared_ptr<Uri> Ability::DenormalizeUri(const Uri &uri)
{
    return nullptr;
}

std::shared_ptr<LifeCycle> Ability::GetLifecycle()
{
    HILOG_INFO("Ability::GetLifecycle called");
    return lifecycle_;
}

AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    HILOG_INFO("Ability::GetState called");

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_INFO("Ability::GetState error. abilityLifecycleExecutor_ == nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

void Ability::StartAbility(const Want &want)
{
    HILOG_INFO("Ability::StartAbility called");
    AbilityContext::StartAbility(want, -1);
}

void Ability::TerminateAbility()
{
    HILOG_INFO("Ability::TerminateAbility called");
    AbilityContext::TerminateAbility();
}

int Ability::TerminateAbility(Want &want)
{
    return -1;
}

void Ability::SetMainRoute(const std::string &entry)
{
    return;
}

void Ability::AddActionRoute(const std::string &action, const std::string &entry)
{
    return;
}

#ifdef SUPPORT_GRAPHICS
int Ability::SetWindowBackgroundColor(int red, int green, int blue)
{
    return -1;
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
