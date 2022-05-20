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

#include "main_thread.h"

#include <new>
#include <regex>

#include "ability_constants.h"
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "ability_loader.h"
#include "ability_thread.h"
#include "app_loader.h"
#include "application_env_impl.h"
#include "hitrace_meter.h"
#include "configuration_convertor.h"
#include "context_deal.h"
#include "context_impl.h"
#include "extension_module_loader.h"
#include "hilog_wrapper.h"
#ifdef SUPPORT_GRAPHICS
#include "form_extension.h"
#include "locale_config.h"
#endif
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "js_runtime.h"
#include "ohos_application.h"
#include "resource_manager.h"
#include "runtime.h"
#include "service_extension.h"
#include "static_subscriber_extension.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "faultloggerd_client.h"
#include "dfx_dump_catcher.h"
#include "hisysevent.h"
#include "js_runtime_utils.h"
#include "context/application_context.h"

#include "hdc_register.h"

#if defined(ABILITY_LIBRARY_LOADER) || defined(APPLICATION_LIBRARY_LOADER)
#include <dirent.h>
#include <dlfcn.h>
#endif

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime::Constants;
std::shared_ptr<OHOSApplication> MainThread::applicationForAnr_ = nullptr;
std::shared_ptr<std::thread> MainThread::handleANRThread_ = nullptr;
namespace {
constexpr int32_t DELIVERY_TIME = 200;
constexpr int32_t DISTRIBUTE_TIME = 100;
constexpr int32_t UNSPECIFIED_USERID = -2;

constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_VERSION[] = "VERSION";
constexpr char EVENT_KEY_TYPE[] = "TYPE";
constexpr char EVENT_KEY_HAPPEN_TIME[] = "HAPPEN_TIME";
constexpr char EVENT_KEY_REASON[] = "REASON";
constexpr char EVENT_KEY_JSVM[] = "JSVM";
constexpr char EVENT_KEY_SUMMARY[] = "SUMMARY";

const std::string JSCRASH_TYPE = "3";
const std::string JSVM_TYPE = "ARK";
constexpr char EXTENSION_PARAMS_TYPE[] = "type";
constexpr char EXTENSION_PARAMS_NAME[] = "name";
}

#define ACEABILITY_LIBRARY_LOADER
#ifdef ABILITY_LIBRARY_LOADER
#endif

/**
 *
 * @brief Notify the AppMgrDeathRecipient that the remote is dead.
 *
 * @param remote The remote which is dead.
 */
void AppMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("MainThread::AppMgrDeathRecipient remote died receive");
}

MainThread::MainThread()
{
#ifdef ABILITY_LIBRARY_LOADER
    fileEntries_.clear();
    handleAbilityLib_.clear();
#endif  // ABILITY_LIBRARY_LOADER
}

MainThread::~MainThread()
{
#ifdef ABILITY_LIBRARY_LOADER
    CloseAbilityLibrary();
#endif  // ABILITY_LIBRARY_LOADER
}

/**
 *
 * @brief Get the current MainThreadState.
 *
 * @return Returns the current MainThreadState.
 */
MainThreadState MainThread::GetMainThreadState() const
{
    return mainThreadState_;
}

/**
 *
 * @brief Set the runner state of mainthread.
 *
 * @param runnerStart whether the runner is started.
 */
void MainThread::SetRunnerStarted(bool runnerStart)
{
    isRunnerStarted_ = runnerStart;
}

/**
 *
 * @brief Get the runner state of mainthread.
 *
 * @return Returns the runner state of mainthread.
 */
bool MainThread::GetRunnerStarted() const
{
    return isRunnerStarted_;
}

/**
 *
 * @brief Get the newThreadId.
 *
 * @return Returns the newThreadId.
 */
int MainThread::GetNewThreadId()
{
    return newThreadId_++;
}

/**
 *
 * @brief Get the application.
 *
 * @return Returns the application.
 */
std::shared_ptr<OHOSApplication> MainThread::GetApplication() const
{
    return application_;
}

/**
 *
 * @brief Get the applicationInfo.
 *
 * @return Returns the applicationInfo.
 */
std::shared_ptr<ApplicationInfo> MainThread::GetApplicationInfo() const
{
    return applicationInfo_;
}

/**
 *
 * @brief Get the applicationImpl.
 *
 * @return Returns the applicationImpl.
 */
std::shared_ptr<ApplicationImpl> MainThread::GetApplicationImpl()
{
    return applicationImpl_;
}

/**
 *
 * @brief Connect the mainthread to the AppMgr.
 *
 */
bool MainThread::ConnectToAppMgr()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread ConnectToAppMgr start.");
    auto object = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        HILOG_ERROR("failed to get app manager service");
        return false;
    }
    deathRecipient_ = new (std::nothrow) AppMgrDeathRecipient();
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("failed to new AppMgrDeathRecipient");
        return false;
    }

    if (!object->AddDeathRecipient(deathRecipient_)) {
        HILOG_ERROR("failed to AddDeathRecipient");
        return false;
    }

    appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        HILOG_ERROR("failed to iface_cast object to appMgr_");
        return false;
    }
    appMgr_->AttachApplication(this);
    HILOG_INFO("MainThread::connectToAppMgr end");
    return true;
}

/**
 *
 * @brief Attach the mainthread to the AppMgr.
 *
 */
void MainThread::Attach()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread attach called.");
    if (!ConnectToAppMgr()) {
        HILOG_ERROR("attachApplication failed");
        return;
    }
    mainThreadState_ = MainThreadState::ATTACH;
    HILOG_INFO("MainThread::attach mainThreadState: %{public}d", mainThreadState_);
}

/**
 *
 * @brief remove the deathRecipient from appMgr.
 *
 */
void MainThread::RemoveAppMgrDeathRecipient()
{
    HILOG_INFO("MainThread::RemoveAppMgrDeathRecipient called begin");
    if (appMgr_ == nullptr) {
        HILOG_ERROR("MainThread::RemoveAppMgrDeathRecipient failed");
        return;
    }

    sptr<IRemoteObject> object = appMgr_->AsObject();
    if (object != nullptr) {
        object->RemoveDeathRecipient(deathRecipient_);
    } else {
        HILOG_ERROR("appMgr_->AsObject() failed");
    }
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 *
 * @brief Get the eventHandler of mainthread.
 *
 * @return Returns the eventHandler of mainthread.
 */
std::shared_ptr<EventHandler> MainThread::GetMainHandler() const
{
    return mainHandler_;
}

/**
 *
 * @brief Schedule the foreground lifecycle of application.
 *
 */
void MainThread::ScheduleForegroundApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule the application to foreground begin.");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleForegroundApplication failed.");
            return;
        }
        appThread->HandleForegroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("PostTask task failed");
    }
    HILOG_INFO("Schedule the application to foreground end.");
}

/**
 *
 * @brief Schedule the background lifecycle of application.
 *
 */
void MainThread::ScheduleBackgroundApplication()
{
    HILOG_INFO("MainThread::scheduleBackgroundApplication called begin");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleBackgroundApplication failed.");
            return;
        }
        appThread->HandleBackgroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleBackgroundApplication PostTask task failed");
    }
    HILOG_INFO("MainThread::scheduleBackgroundApplication called end.");
}

/**
 *
 * @brief Schedule the terminate lifecycle of application.
 *
 */
void MainThread::ScheduleTerminateApplication()
{
    HILOG_INFO("MainThread::scheduleTerminateApplication called begin");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleTerminateApplication failed.");
            return;
        }
        appThread->HandleTerminateApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleTerminateApplication PostTask task failed");
    }
    HILOG_INFO("MainThread::scheduleTerminateApplication called.");
}

/**
 *
 * @brief Shrink the memory which used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void MainThread::ScheduleShrinkMemory(const int level)
{
    HILOG_INFO("MainThread::scheduleShrinkMemory level: %{public}d", level);
    wptr<MainThread> weak = this;
    auto task = [weak, level]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleShrinkMemory(level);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleShrinkMemory PostTask task failed");
    }
    HILOG_INFO("MainThread::scheduleShrinkMemory level: %{public}d end.", level);
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::ScheduleProcessSecurityExit()
{
    HILOG_INFO("MainThread::ScheduleProcessSecurityExit called start");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleProcessSecurityExit();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleProcessSecurityExit PostTask task failed");
    }
    HILOG_INFO("MainThread::ScheduleProcessSecurityExit called end");
}

/**
 *
 * @brief Low the memory which used by application.
 *
 */
void MainThread::ScheduleLowMemory()
{
    HILOG_INFO("MainThread::scheduleLowMemory called");
}

/**
 *
 * @brief Launch the application.
 *
 * @param data The launchdata of the application witch launced.
 *
 */
void MainThread::ScheduleLaunchApplication(const AppLaunchData &data, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread schedule launch application start.");
    wptr<MainThread> weak = this;
    auto task = [weak, data, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleLaunchApplication failed.");
            return;
        }
        appThread->HandleLaunchApplication(data, config);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleLaunchApplication PostTask task failed");
    }
}

void MainThread::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_INFO("MainThread::ScheduleAbilityStageInfo start");
    wptr<MainThread> weak = this;
    auto task = [weak, abilityStage]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleAbilityStage(abilityStage);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleAbilityStageInfo PostTask task failed");
    }
    HILOG_INFO("MainThread::ScheduleAbilityStageInfo end.");
}

void MainThread::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want)
{
    HILOG_INFO("MainThread schedule launch ability, name is %{public}s, type is %{public}d.",
        info.name.c_str(), info.type);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>(info);
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    abilityRecord->SetWant(want);

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    sptr<IBundleMgr> bundleMgr = contextDeal->GetBundleManager();
    if (bundleMgr) {
        BundleInfo bundleInfo;
        bundleMgr->GetBundleInfo(abilityInfo->bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    }

    wptr<MainThread> weak = this;
    auto task = [weak, abilityRecord]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleLaunchAbility failed.");
            return;
        }
        appThread->HandleLaunchAbility(abilityRecord);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleLaunchAbility PostTask task failed");
    }
}

/**
 *
 * @brief clean the ability by token.
 *
 * @param token The token belong to the ability which want to be cleaned.
 *
 */
void MainThread::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Schedule clean ability called, app is %{public}s.", applicationInfo_->name.c_str());
    wptr<MainThread> weak = this;
    auto task = [weak, token]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleCleanAbility failed.");
            return;
        }
        appThread->HandleCleanAbility(token);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleCleanAbility PostTask task failed");
    }
}

/**
 *
 * @brief send the new profile.
 *
 * @param profile The updated profile.
 *
 */
void MainThread::ScheduleProfileChanged(const Profile &profile)
{
    HILOG_INFO("MainThread::scheduleProfileChanged profile name: %{public}s", profile.GetName().c_str());
}

/**
 *
 * @brief send the new config to the application.
 *
 * @param config The updated config.
 *
 */
void MainThread::ScheduleConfigurationUpdated(const Configuration &config)
{
    HILOG_INFO("MainThread::ScheduleConfigurationUpdated called start.");
    wptr<MainThread> weak = this;
    auto task = [weak, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleConfigurationUpdated failed.");
            return;
        }
        appThread->HandleConfigurationUpdated(config);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleConfigurationUpdated PostTask task failed");
    }
    HILOG_INFO("MainThread::ScheduleConfigurationUpdated called end.");
}

bool MainThread::CheckLaunchApplicationParam(const AppLaunchData &appLaunchData) const
{
    ApplicationInfo appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();

    if (appInfo.name.empty()) {
        HILOG_ERROR("MainThread::checkLaunchApplicationParam applicationName is empty");
        return false;
    }

    if (processInfo.GetProcessName().empty()) {
        HILOG_ERROR("MainThread::checkLaunchApplicationParam processName is empty");
        return false;
    }
    return true;
}

/**
 *
 * @brief Check whether the record is legal.
 *
 * @param record The record should be checked.
 *
 * @return if the record is legal, return true. else return false.
 */
bool MainThread::CheckAbilityItem(const std::shared_ptr<AbilityLocalRecord> &record) const
{
    if (record == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem record is null");
        return false;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    sptr<IRemoteObject> token = record->GetToken();

    if (abilityInfo == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem abilityInfo is null");
        return false;
    }

    if (token == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem token is null");
        return false;
    }
    return true;
}

/**
 *
 * @brief Terminate the application but don't notify ams.
 *
 */
void MainThread::HandleTerminateApplicationLocal()
{
    HILOG_INFO("MainThread::HandleTerminateApplicationLocal called start.");
    if (application_ == nullptr) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal error!");
        return;
    }
    applicationImpl_->PerformTerminateStrong();
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal get manHandler error");
        return;
    }

    if (watchDogHandler_ != nullptr) {
        watchDogHandler_->Stop();
    }
    if (handleANRThread_ != nullptr && handleANRThread_->joinable()) {
        handleANRThread_->join();
        handleANRThread_ = nullptr;
    }

    int ret = runner->Stop();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal failed. runner->Run failed ret = %{public}d", ret);
    }
    HILOG_INFO("runner is stopped");
    SetRunnerStarted(false);

#ifdef ABILITY_LIBRARY_LOADER
    CloseAbilityLibrary();
#endif  // ABILITY_LIBRARY_LOADER
#ifdef APPLICATION_LIBRARY_LOADER
    if (handleAppLib_ != nullptr) {
        dlclose(handleAppLib_);
        handleAppLib_ = nullptr;
    }
#endif  // APPLICATION_LIBRARY_LOADER
    HILOG_INFO("MainThread::HandleTerminateApplicationLocal called end.");
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::HandleProcessSecurityExit()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::HandleProcessSecurityExit called start.");
    if (abilityRecordMgr_ == nullptr) {
        HILOG_ERROR("MainThread::HandleProcessSecurityExit abilityRecordMgr_ is null");
        return;
    }

    std::vector<sptr<IRemoteObject>> tokens = (abilityRecordMgr_->GetAllTokens());

    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        HandleCleanAbilityLocal(*iter);
    }

    HandleTerminateApplicationLocal();
    HILOG_INFO("MainThread::HandleProcessSecurityExit called end.");
}

bool MainThread::InitCreate(
    std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, ProcessInfo &processInfo, Profile &appProfile)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    applicationInfo_ = std::make_shared<ApplicationInfo>(appInfo);
    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create applicationInfo_ failed");
        return false;
    }

    processInfo_ = std::make_shared<ProcessInfo>(processInfo);
    if (processInfo_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create processInfo_ failed");
        return false;
    }

    appProfile_ = std::make_shared<Profile>(appProfile);
    if (appProfile_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create appProfile_ failed");
        return false;
    }

    applicationImpl_ = std::make_shared<ApplicationImpl>();
    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create applicationImpl_ failed");
        return false;
    }

    abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    if (abilityRecordMgr_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create AbilityRecordMgr failed");
        return false;
    }

    contextDeal = std::make_shared<ContextDeal>();
    if (contextDeal == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create contextDeal failed");
        return false;
    }

    if (watchDogHandler_ != nullptr) {
        watchDogHandler_->SetApplicationInfo(applicationInfo_);
    }

    contextDeal->SetProcessInfo(processInfo_);
    contextDeal->SetApplicationInfo(applicationInfo_);
    contextDeal->SetProfile(appProfile_);
    contextDeal->SetBundleCodePath(applicationInfo_->codePath);  // BMS need to add cpath

    return true;
}

bool MainThread::CheckForHandleLaunchApplication(const AppLaunchData &appLaunchData)
{
    if (application_ != nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication already create application");
        return false;
    }

    if (!CheckLaunchApplicationParam(appLaunchData)) {
        HILOG_ERROR("MainThread::handleLaunchApplication appLaunchData invalid");
        return false;
    }
    return true;
}

bool MainThread::InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, BundleInfo& bundleInfo,
    const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::vector<std::string> resPaths;
    ChangeToLocalPath(bundleInfo.name, bundleInfo.moduleResPaths, resPaths);
    for (auto moduleResPath : resPaths) {
        if (!moduleResPath.empty()) {
            HILOG_INFO("MainThread::handleLaunchApplication length: %{public}zu, moduleResPath: %{public}s",
                moduleResPath.length(),
                moduleResPath.c_str());
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                HILOG_ERROR("MainThread::handleLaunchApplication AddResource failed");
            }
        }
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    const icu::Locale *localeInfo = resConfig->GetLocaleInfo();
    if (localeInfo != nullptr) {
        HILOG_INFO("MainThread::handleLaunchApplication language: %{public}s, script: %{public}s, region: %{public}s,",
            localeInfo->getLanguage(),
            localeInfo->getScript(),
            localeInfo->getCountry());
    } else {
        HILOG_INFO("MainThread::handleLaunchApplication localeInfo is nullptr.");
    }

    std::string colormode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    HILOG_INFO("colormode is %{public}s.", colormode.c_str());
    resConfig->SetColorMode(ConvertColorMode(colormode));
#endif
    resourceManager->UpdateResConfig(*resConfig);
    return true;
}

static std::string GetNativeStrFromJsTaggedObj(NativeObject* obj, const char* key)
{
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get value from key:%{public}s, Null NativeObject", key);
        return "";
    }

    NativeValue* value = obj->GetProperty(key);
    NativeString* valueStr = AbilityRuntime::ConvertNativeValueTo<NativeString>(value);
    if (valueStr == nullptr) {
        HILOG_ERROR("Failed to convert value from key:%{public}s", key);
        return "";
    }
    size_t valueStrBufLength = valueStr->GetLength();
    size_t valueStrLength = 0;
    char* valueCStr = new (std::nothrow) char[valueStrBufLength + 1];
    if (valueCStr == nullptr) {
        HILOG_ERROR("Failed to new valueCStr");
        return "";
    }
    valueStr->GetCString(valueCStr, valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr, valueStrLength);
    delete []valueCStr;
    HILOG_DEBUG("GetNativeStrFromJsTaggedObj Success %{public}s:%{public}s", key, ret.c_str());
    return ret;
}

/**
 *
 * @brief Launch the application.
 *
 * @param appLaunchData The launchdata of the application witch launced.
 *
 */
void MainThread::HandleLaunchApplication(const AppLaunchData &appLaunchData, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread handle launch application start.");
    if (!CheckForHandleLaunchApplication(appLaunchData)) {
        HILOG_ERROR("MainThread::handleLaunchApplication CheckForHandleLaunchApplication failed");
        return;
    }
    std::vector<std::string> localPaths;
    ChangeToLocalPath(appLaunchData.GetApplicationInfo().bundleName,
        appLaunchData.GetApplicationInfo().moduleSourceDirs, localPaths);
    LoadAbilityLibrary(localPaths);
    LoadAppLibrary();

    ApplicationInfo appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();
    Profile appProfile = appLaunchData.GetProfile();

    std::shared_ptr<ContextDeal> contextDeal = nullptr;
    if (!InitCreate(contextDeal, appInfo, processInfo, appProfile)) {
        HILOG_ERROR("MainThread::handleLaunchApplication InitCreate failed");
        return;
    }

    // get application shared point
    application_ = std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName());
    if (application_ == nullptr) {
        HILOG_ERROR("HandleLaunchApplication::application launch failed");
        return;
    }
    applicationForAnr_ = application_;

    // init resourceManager.
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication create resourceManager failed");
        return;
    }

    sptr<IBundleMgr> bundleMgr = contextDeal->GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication GetBundleManager is nullptr");
        return;
    }

    BundleInfo bundleInfo;
    if (!bundleMgr->GetBundleInfo(appInfo.bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, UNSPECIFIED_USERID)) {
        HILOG_DEBUG("MainThread::handleLaunchApplication GetBundleInfo fail.");
    }

    if (!InitResourceManager(resourceManager, contextDeal, appInfo, bundleInfo, config)) {
        HILOG_ERROR("MainThread::handleLaunchApplication InitResourceManager failed");
        return;
    }

    // create contextImpl
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetResourceManager(resourceManager);
    contextImpl->SetApplicationInfo(std::make_shared<ApplicationInfo>(appInfo));
    contextImpl->InitAppContext();
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->AttachContextImpl(contextImpl);
    applicationContext->InitApplicationContext();
    application_->SetApplicationContext(applicationContext);

    bool moduelJson = false;
    bool isStageBased = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduelJson = bundleInfo.hapModuleInfos.back().isModuleJson;
        isStageBased = bundleInfo.hapModuleInfos.back().isStageBasedModel;
    }
    HILOG_INFO("stageBased:%{public}d moduleJson:%{public}d size:%{public}d",
        isStageBased, moduelJson, (int32_t)bundleInfo.hapModuleInfos.size());

    if (isStageBased) {
        // Create runtime
        AbilityRuntime::Runtime::Options options;
        options.codePath = LOCAL_CODE_PATH;
        options.eventRunner = mainHandler_->GetEventRunner();
        options.loadAce = true;
        std::string nativeLibraryPath = appInfo.nativeLibraryPath;
        if (!nativeLibraryPath.empty()) {
            if (nativeLibraryPath.back() == '/') {
                nativeLibraryPath.pop_back();
            }
            std::string libPath = LOCAL_CODE_PATH;
            libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
            HILOG_INFO("napi lib path = %{private}s", libPath.c_str());
            options.packagePath = libPath;
        }
        auto runtime = AbilityRuntime::Runtime::Create(options);
        if (!runtime) {
            HILOG_ERROR("OHOSApplication::OHOSApplication: Failed to create runtime");
            return;
        }
        auto& jsEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNativeEngine();
        auto bundleName = appInfo.bundleName;
        auto versionCode = appInfo.versionCode;
        wptr<MainThread> weak = this;
        auto uncaughtTask = [weak, bundleName, versionCode](NativeValue* v) {
            NativeObject* obj = AbilityRuntime::ConvertNativeValueTo<NativeObject>(v);
            std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
            std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
            std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
            std::string summary = "Error message:" + errorMsg + "\nStacktrace:\n" + errorStack;
            time_t timet;
            struct tm localUTC;
            struct timeval gtime;
            time(&timet);
            gmtime_r(&timet, &localUTC);
            gettimeofday(&gtime, NULL);
            std::string loacalUTCTime = std::to_string(localUTC.tm_year + 1900)
                + "/" + std::to_string(localUTC.tm_mon + 1)
                + "/" + std::to_string(localUTC.tm_mday)
                + " " + std::to_string(localUTC.tm_hour)
                + "-" + std::to_string(localUTC.tm_min)
                + "-" + std::to_string(localUTC.tm_sec)
                + "." + std::to_string(gtime.tv_usec/1000);
            OHOS::HiviewDFX::HiSysEvent::Write(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, "JS_ERROR",
                OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                EVENT_KEY_PACKAGE_NAME, bundleName,
                EVENT_KEY_VERSION, std::to_string(versionCode),
                EVENT_KEY_TYPE, JSCRASH_TYPE,
                EVENT_KEY_HAPPEN_TIME, loacalUTCTime,
                EVENT_KEY_REASON, errorName,
                EVENT_KEY_JSVM, JSVM_TYPE,
                EVENT_KEY_SUMMARY, summary);
            auto appThread = weak.promote();
            if (appThread == nullptr) {
                HILOG_ERROR("appThread is nullptr, HandleLaunchApplication failed.");
                return;
            }
            // if app's callback has been registered, let app decide whether exit or not.
            HILOG_ERROR("\n%{public}s is about to exit due to RuntimeError\nError type:%{public}s\n%{public}s",
                bundleName.c_str(), errorName.c_str(), summary.c_str());
            appThread->ScheduleProcessSecurityExit();
        };
        jsEngine.RegisterUncaughtExceptionHandler(uncaughtTask);
        application_->SetRuntime(std::move(runtime));

        AbilityLoader::GetInstance().RegisterAbility("Ability", [application = application_]() {
            return Ability::Create(application->GetRuntime());
        });
#ifdef SUPPORT_GRAPHICS
        AbilityLoader::GetInstance().RegisterExtension("FormExtension", [application = application_]() {
            return AbilityRuntime::FormExtension::Create(application->GetRuntime());
        });
#endif
        AbilityLoader::GetInstance().RegisterExtension("StaticSubscriberExtension", [application = application_]() {
            return AbilityRuntime::StaticSubscriberExtension::Create(application->GetRuntime());
        });
#ifdef __aarch64__
        LoadAllExtensions("system/lib64/extensionability");
        LoadAndRegisterExtension("system/lib64/libdatashare_ext_ability_module.z.so", "DataShareExtAbility",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib64/libworkschedextension.z.so", "WorkSchedulerExtension",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib64/libaccessibility_extension_module.z.so", "AccessibilityExtension",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib64/libwallpaper_extension_module.z.so", "WallpaperExtension",
            application_->GetRuntime());
#else
        LoadAllExtensions("system/lib/extensionability");
        LoadAndRegisterExtension("system/lib/libdatashare_ext_ability_module.z.so", "DataShareExtAbility",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib/libworkschedextension.z.so", "WorkSchedulerExtension",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib/libaccessibility_extension_module.z.so", "AccessibilityExtension",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib/libwallpaper_extension_module.z.so", "WallpaperExtension",
            application_->GetRuntime());
#endif
    }

    auto usertestInfo = appLaunchData.GetUserTestInfo();
    if (usertestInfo) {
        if (!PrepareAbilityDelegator(usertestInfo, isStageBased, bundleInfo)) {
            HILOG_ERROR("Failed to prepare ability delegator");
            return;
        }
    }

    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetApplicationContext(application_);
    application_->AttachBaseContext(contextDeal);
    application_->SetAbilityRecordMgr(abilityRecordMgr_);
    application_->SetConfiguration(config);
    contextImpl->SetConfiguration(application_->GetConfiguration());

    applicationImpl_->SetRecordId(appLaunchData.GetRecordId());
    applicationImpl_->SetApplication(application_);
    mainThreadState_ = MainThreadState::READY;
    if (!applicationImpl_->PerformAppReady()) {
        HILOG_ERROR("HandleLaunchApplication::application applicationImpl_->PerformAppReady failed");
        return;
    }
    // L1 needs to add corresponding interface
    ApplicationEnvImpl *pAppEvnIml = ApplicationEnvImpl::GetInstance();

    if (pAppEvnIml) {
        pAppEvnIml->SetAppInfo(*applicationInfo_.get());
    } else {
        HILOG_ERROR("HandleLaunchApplication::application pAppEvnIml is null");
    }

    HILOG_INFO("MainThread::handleLaunchApplication called end.");
}

void MainThread::ChangeToLocalPath(const std::string &bundleName,
    const std::vector<std::string> &sourceDirs, std::vector<std::string> &localPath)
{
    for (auto item : sourceDirs) {
        if (item.empty()) {
            continue;
        }
        std::regex pattern(ABS_CODE_PATH + FILE_SEPARATOR + bundleName + FILE_SEPARATOR);
        localPath.emplace_back(
            std::regex_replace(item, pattern, LOCAL_CODE_PATH + FILE_SEPARATOR));
    }
}

void MainThread::HandleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_INFO("MainThread::HandleAbilityStageInfo");
    if (!application_) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->AddAbilityStage(abilityStage);

    if (!appMgr_ || !applicationImpl_) {
        HILOG_ERROR("appMgr_ is nullptr");
        return;
    }

    appMgr_->AddAbilityStageDone(applicationImpl_->GetRecordId());
}

void MainThread::LoadAndRegisterExtension(const std::string &libName,
    const std::string &extensionName, const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    HILOG_INFO("MainThread::LoadAndRegisterExtension.libName:%{public}s,extensionName:%{public}s,",
        libName.c_str(), extensionName.c_str());
    if (application_ == nullptr) {
        HILOG_ERROR("LoadAndRegisterExtension::application launch failed");
        return;
    }
    HILOG_INFO("MainThread::LoadAndRegisterExtension load success.");
    AbilityLoader::GetInstance().RegisterExtension(extensionName, [application = application_, libName]() {
        return AbilityRuntime::ExtensionModuleLoader::GetLoader(libName.c_str()).Create(application->GetRuntime());
    });
}

void MainThread::LoadAllExtensions(const std::string &filePath)
{
    HILOG_INFO("LoadAllExtensions.filePath:%{public}s", filePath.c_str());
    if (application_ == nullptr) {
        HILOG_ERROR("application launch failed");
        return;
    }
    // scan all extensions in path
    std::vector<std::string> extensionFiles;
    ScanDir(filePath, extensionFiles);
    if (extensionFiles.empty()) {
        HILOG_ERROR("no extension files.");
        return;
    }
    std::map<int32_t, std::string> extensionTypeMap;
    for (auto file : extensionFiles) {
        HILOG_INFO("Begin load extension file:%{public}s", file.c_str());
        std::map<std::string, std::string> params =
            AbilityRuntime::ExtensionModuleLoader::GetLoader(file.c_str()).GetParams();
        if (params.empty()) {
            HILOG_ERROR("no extension params.");
            continue;
        }
        // get extension name and type
        std::map<std::string, std::string>::iterator it = params.find(EXTENSION_PARAMS_TYPE);
        if (it == params.end()) {
            HILOG_ERROR("no extension type.");
            continue;
        }
        int32_t type = std::stoi(it->second);

        it = params.find(EXTENSION_PARAMS_NAME);
        if (it == params.end()) {
            HILOG_ERROR("no extension name.");
            continue;
        }
        std::string extensionName = it->second;

        extensionTypeMap.insert(std::pair<int32_t, std::string>(type, extensionName));
        HILOG_INFO("Success load, extension type: %{public}d, extension name:%{public}s", type, extensionName.c_str());
        AbilityLoader::GetInstance().RegisterExtension(extensionName, [application = application_, file]() {
            return AbilityRuntime::ExtensionModuleLoader::GetLoader(file.c_str()).Create(application->GetRuntime());
        });
    }
    application_->SetExtensionTypeMap(extensionTypeMap);
}

bool MainThread::PrepareAbilityDelegator(const std::shared_ptr<UserTestRecord> &record, bool isStageBased,
    BundleInfo& bundleInfo)
{
    HILOG_INFO("enter");
    if (!record) {
        HILOG_ERROR("Invalid UserTestRecord");
        return false;
    }
    auto args = std::make_shared<AbilityDelegatorArgs>(record->want);
    if (!args) {
        HILOG_ERROR("args is null");
        return false;
    }
    if (isStageBased) { // Stage model
        HILOG_INFO("PrepareAbilityDelegator for Stage model.");
        auto testRunner = TestRunner::Create(application_->GetRuntime(), args, false);
        auto delegator = std::make_shared<AbilityDelegator>(
            application_->GetAppContext(), std::move(testRunner), record->observer);
        AbilityDelegatorRegistry::RegisterInstance(delegator, args);
        delegator->Prepare();
    } else { // FA model
        HILOG_INFO("PrepareAbilityDelegator for FA model.");
        AbilityRuntime::Runtime::Options options;
        options.codePath = LOCAL_CODE_PATH;
        options.eventRunner = mainHandler_->GetEventRunner();
        options.loadAce = false;
        if (bundleInfo.hapModuleInfos.empty() || bundleInfo.hapModuleInfos.front().abilityInfos.empty()) {
            HILOG_ERROR("Failed to abilityInfos");
            return false;
        }
        bool isFaJsModel = bundleInfo.hapModuleInfos.front().abilityInfos.front().srcLanguage == "js" ? true : false;
        static auto runtime = AbilityRuntime::Runtime::Create(options);
        auto testRunner = TestRunner::Create(runtime, args, isFaJsModel);
        if (!testRunner->Initialize()) {
            HILOG_ERROR("Failed to Initialize testRunner");
            return false;
        }
        auto delegator = std::make_shared<AbilityDelegator>(
            application_->GetAppContext(), std::move(testRunner), record->observer);
        AbilityDelegatorRegistry::RegisterInstance(delegator, args);
        delegator->Prepare();
    }
    return true;
}

/**
 *
 * @brief launch the ability.
 *
 * @param abilityRecord The abilityRecord which belongs to the ability launched.
 *
 */
void MainThread::HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::handleLaunchAbility called start.");

    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::HandleLaunchAbility applicationImpl_ is null");
        return;
    }

    if (abilityRecordMgr_ == nullptr) {
        HILOG_ERROR("MainThread::HandleLaunchAbility abilityRecordMgr_ is null");
        return;
    }

    if (abilityRecord == nullptr) {
        HILOG_ERROR("MainThread::HandleLaunchAbility parameter(abilityRecord) is null");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    if (abilityToken == nullptr) {
        HILOG_ERROR("MainThread::HandleLaunchAbility failed. abilityRecord->GetToken failed");
        return;
    }

    abilityRecordMgr_->SetToken(abilityToken);
    abilityRecordMgr_->AddAbilityRecord(abilityToken, abilityRecord);

    if (!IsApplicationReady()) {
        HILOG_ERROR("MainThread::handleLaunchAbility not init OHOSApplication, should launch application first");
        return;
    }

    if (!CheckAbilityItem(abilityRecord)) {
        HILOG_ERROR("MainThread::handleLaunchAbility record is invalid");
        return;
    }

    auto& runtime = application_->GetRuntime();
    auto appInfo = application_->GetApplicationInfo();
    auto want = abilityRecord->GetWant();
    if (runtime && appInfo && want && appInfo->debug) {
        HdcRegister::Get().StartHdcRegister(appInfo->bundleName);
        runtime->StartDebugMode(want->GetBoolParam("debugApp", false));
    }

    mainThreadState_ = MainThreadState::RUNNING;
    std::shared_ptr<AbilityRuntime::Context> stageContext = application_->AddAbilityStage(abilityRecord);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    AbilityThread::AbilityThreadMain(application_, abilityRecord, stageContext);
#else
    AbilityThread::AbilityThreadMain(application_, abilityRecord, mainHandler_->GetEventRunner(), stageContext);
#endif
}

/**
 *
 * @brief Clean the ability but don't notify ams.
 *
 * @param token The token which belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbilityLocal(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("MainThread::HandleCleanAbilityLocal called start.");
    if (!IsApplicationReady()) {
        HILOG_ERROR("MainThread::HandleCleanAbilityLocal not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        HILOG_ERROR("MainThread::HandleCleanAbilityLocal token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        HILOG_ERROR("MainThread::HandleCleanAbilityLocal abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("MainThread::HandleCleanAbilityLocal record->GetAbilityInfo() failed");
        return;
    }
    HILOG_INFO("MainThread::HandleCleanAbilityLocal ability name: %{public}s", abilityInfo->name.c_str());

    abilityRecordMgr_->RemoveAbilityRecord(token);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            HILOG_ERROR("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
    } else {
        HILOG_WARN("MainThread::HandleCleanAbilityLocal runner not found");
    }
#endif
    HILOG_INFO("MainThread::HandleCleanAbilityLocal called end.");
}

/**
 *
 * @brief Clean the ability.
 *
 * @param token The token which belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbility(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("Handle clean ability start, app is %{public}s.", applicationInfo_->name.c_str());
    if (!IsApplicationReady()) {
        HILOG_ERROR("MainThread::handleCleanAbility not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        HILOG_ERROR("MainThread::handleCleanAbility token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        HILOG_ERROR("MainThread::handleCleanAbility abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("MainThread::handleCleanAbility record->GetAbilityInfo() failed");
        return;
    }
    HILOG_INFO("Handle clean ability, ability: %{public}s.", abilityInfo->name.c_str());

    abilityRecordMgr_->RemoveAbilityRecord(token);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            HILOG_ERROR("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
    } else {
        HILOG_WARN("MainThread::handleCleanAbility runner not found");
    }
#endif
    appMgr_->AbilityCleaned(token);
    HILOG_INFO("Handle clean ability end, app is %{public}s.", applicationInfo_->name.c_str());
}

/**
 *
 * @brief Foreground the application.
 *
 */
void MainThread::HandleForegroundApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread handle application to foreground called.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!");
        return;
    }

    if (!applicationImpl_->PerformForeground()) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!, applicationImpl_->PerformForeground() failed");
        return;
    }

    HILOG_INFO("MainThread handle application to foreground success, recordId is %{public}d",
        applicationImpl_->GetRecordId());
    appMgr_->ApplicationForegrounded(applicationImpl_->GetRecordId());
}

/**
 *
 * @brief Background the application.
 *
 */
void MainThread::HandleBackgroundApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::handleBackgroundApplication called start.");

    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleBackgroundApplication error!");
        return;
    }

    if (!applicationImpl_->PerformBackground()) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!, applicationImpl_->PerformBackground() failed");
        return;
    }
    appMgr_->ApplicationBackgrounded(applicationImpl_->GetRecordId());

    HILOG_INFO("MainThread::handleBackgroundApplication called end");
}

/**
 *
 * @brief Terminate the application.
 *
 */
void MainThread::HandleTerminateApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::handleTerminateApplication called start.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleTerminateApplication error!");
        return;
    }

    if (!applicationImpl_->PerformTerminate()) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!, applicationImpl_->PerformTerminate() failed");
        return;
    }
    appMgr_->ApplicationTerminated(applicationImpl_->GetRecordId());
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::handleTerminateApplication get manHandler error");
        return;
    }

    if (watchDogHandler_ != nullptr) {
        watchDogHandler_->Stop();
    }
    if (handleANRThread_ != nullptr && handleANRThread_->joinable()) {
        handleANRThread_->join();
        handleANRThread_ = nullptr;
    }

    int ret = runner->Stop();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::handleTerminateApplication failed. runner->Run failed ret = %{public}d", ret);
    }
    SetRunnerStarted(false);

#ifdef ABILITY_LIBRARY_LOADER
    CloseAbilityLibrary();
#endif  // ABILITY_LIBRARY_LOADER
#ifdef APPLICATION_LIBRARY_LOADER
    if (handleAppLib_ != nullptr) {
        dlclose(handleAppLib_);
        handleAppLib_ = nullptr;
    }
#endif  // APPLICATION_LIBRARY_LOADER
    HILOG_INFO("MainThread::handleTerminateApplication called end.");
}

/**
 *
 * @brief Shrink the memory which used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void MainThread::HandleShrinkMemory(const int level)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::HandleShrinkMemory called start.");

    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::HandleShrinkMemory error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformMemoryLevel(level);
    HILOG_INFO("MainThread::HandleShrinkMemory called end.");
}

/**
 *
 * @brief send the new config to the application.
 *
 * @param config The updated config.
 *
 */
void MainThread::HandleConfigurationUpdated(const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::HandleConfigurationUpdated called start.");

    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::HandleConfigurationUpdated error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformConfigurationUpdated(config);
    HILOG_INFO("MainThread::HandleConfigurationUpdated called end.");
}

void MainThread::TaskTimeoutDetected(const std::shared_ptr<EventRunner> &runner)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread::TaskTimeoutDetected called start.");

    auto deliveryTimeoutCallback = []() {
        HILOG_INFO("MainThread::TaskTimeoutDetected delivery timeout");
    };
    auto distributeTimeoutCallback = []() {
        HILOG_INFO("MainThread::TaskTimeoutDetected distribute timeout");
    };

    if (runner !=nullptr && mainHandler_ != nullptr) {
        runner->SetDeliveryTimeout(DELIVERY_TIME);
        mainHandler_->SetDeliveryTimeoutCallback(deliveryTimeoutCallback);

        runner->SetDistributeTimeout(DISTRIBUTE_TIME);
        mainHandler_->SetDistributeTimeoutCallback(distributeTimeoutCallback);
    }
    HILOG_INFO("MainThread::TaskTimeoutDetected called end.");
}

void MainThread::Init(const std::shared_ptr<EventRunner> &runner, const std::shared_ptr<EventRunner> &watchDogRunner)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread:Init Start");
    mainHandler_ = std::make_shared<MainHandler>(runner, this);
    watchDogHandler_ = std::make_shared<WatchDog>(watchDogRunner);
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, SetRunnerStarted failed.");
            return;
        }
        appThread->SetRunnerStarted(true);
    };
    auto taskWatchDog = []() {
        HILOG_INFO("MainThread:WatchDogHandler Start");
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::Init PostTask task failed");
    }
    if (!watchDogHandler_->PostTask(taskWatchDog)) {
        HILOG_ERROR("MainThread::Init WatchDog postTask task failed");
    }
    TaskTimeoutDetected(runner);

    watchDogHandler_->Init(mainHandler_, watchDogHandler_);
    TaskHandlerClient::GetInstance()->CreateRunner();
    HILOG_INFO("MainThread:Init end.");
}

void MainThread::HandleSignal(int signal)
{
    switch (signal) {
        case SIGUSR1:
            HILOG_INFO("MainThread::ScheduleANRProcess called begin");
            if (handleANRThread_ == nullptr) {
                handleANRThread_ = std::make_shared<std::thread>(&MainThread::HandleScheduleANRProcess);
            }
            break;
        default:
            HILOG_INFO("Signal:%{public}d need not to handle.", signal);
            break;
    }
}

void MainThread::HandleScheduleANRProcess()
{
    HILOG_INFO("MainThread:HandleScheduleANRProcess start.");
    int rFD = -1;
    std::string mainThreadStackInfo;
    if ((rFD = RequestFileDescriptor(int32_t(FaultLoggerType::JS_STACKTRACE))) < 0) {
        HILOG_ERROR("MainThread::HandleScheduleANRProcess request file eescriptor failed");
        return;
    }
    if (applicationForAnr_ != nullptr && applicationForAnr_->GetRuntime() != nullptr) {
        mainThreadStackInfo = applicationForAnr_->GetRuntime()->BuildNativeAndJsBackStackTrace();
        if (write(rFD, mainThreadStackInfo.c_str(), mainThreadStackInfo.size()) !=
          (ssize_t)mainThreadStackInfo.size()) {
            HILOG_ERROR("MainThread::HandleScheduleANRProcess write main thread stack info failed");
        }
    }
    OHOS::HiviewDFX::DfxDumpCatcher dumplog;
    std::string proStackInfo;
    if (dumplog.DumpCatch(getpid(), 0, proStackInfo) == false) {
        HILOG_ERROR("MainThread::HandleScheduleANRProcess get process stack info failed");
    }
    if (write(rFD, proStackInfo.c_str(), proStackInfo.size()) != (ssize_t)proStackInfo.size()) {
        HILOG_ERROR("MainThread::HandleScheduleANRProcess write process stack info failed");
    }
    if (rFD != -1) {
        close(rFD);
    }
}

void MainThread::Start()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("MainThread start come.");
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::main failed, runner is nullptr");
        return;
    }
    std::shared_ptr<EventRunner> runnerWatchDog = EventRunner::Create("WatchDogRunner");
    if (runnerWatchDog == nullptr) {
        HILOG_ERROR("MainThread::Start runnerWatchDog is nullptr");
        return;
    }
    sptr<MainThread> thread = sptr<MainThread>(new (std::nothrow) MainThread());
    if (thread == nullptr) {
        HILOG_ERROR("MainThread::static failed. new MainThread failed");
        return;
    }

    struct sigaction sigAct;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = 0;
    sigAct.sa_handler = &MainThread::HandleSignal;
    sigaction(SIGUSR1, &sigAct, NULL);

    thread->Init(runner, runnerWatchDog);

    thread->Attach();

    int ret = runner->Run();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::main failed. runner->Run failed ret = %{public}d", ret);
    }

    thread->RemoveAppMgrDeathRecipient();
    HILOG_INFO("MainThread::main runner stopped");
}

MainThread::MainHandler::MainHandler(const std::shared_ptr<EventRunner> &runner, const sptr<MainThread> &thread)
    : AppExecFwk::EventHandler(runner), mainThreadObj_(thread)
{}

/**
 *
 * @brief Process the event.
 *
 * @param event the event want to be processed.
 *
 */
void MainThread::MainHandler::ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event)
{
    auto eventId = event->GetInnerEventId();
    if (eventId == MAIN_THREAD_IS_ALIVE) {
        auto watchDogHanlder = WatchDog::GetCurrentHandler();
        if (watchDogHanlder != nullptr) {
            watchDogHanlder->SendEvent(MAIN_THREAD_IS_ALIVE);
        }
    }
}

/**
 *
 * @brief Check whether the OHOSApplication is ready.
 *
 * @return if the record is legal, return true. else return false.
 *
 */
bool MainThread::IsApplicationReady() const
{
    HILOG_INFO("MainThread::IsApplicationReady called start");
    if (application_ == nullptr || applicationImpl_ == nullptr) {
        HILOG_WARN("MainThread::IsApplicationReady called. application_=null or applicationImpl_=null");
        return false;
    }

    return true;
}

#ifdef ABILITY_LIBRARY_LOADER
/**
 *
 * @brief Load the ability library.
 *
 * @param libraryPaths the library paths.
 *
 */
void MainThread::LoadAbilityLibrary(const std::vector<std::string> &libraryPaths)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef ABILITY_LIBRARY_LOADER
    HILOG_INFO("MainThread load ability library start.");
#ifdef ACEABILITY_LIBRARY_LOADER
#ifdef _ARM64_
    std::string acelibdir("/system/lib64/libace.z.so");
#else
    std::string acelibdir("/system/lib/libace.z.so");
#endif
    void *AceAbilityLib = nullptr;
    AceAbilityLib = dlopen(acelibdir.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (AceAbilityLib == nullptr) {
        HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]", acelibdir.c_str(), dlerror());
    } else {
        HILOG_INFO("Success to dlopen %{public}s", acelibdir.c_str());
        handleAbilityLib_.emplace_back(AceAbilityLib);
    }
#endif  // ACEABILITY_LIBRARY_LOADER
    size_t size = libraryPaths.size();
    for (size_t index = 0; index < size; index++) {
        std::string libraryPath = libraryPaths[index];
        HILOG_INFO("MainThread::LoadAbilityLibrary Try to scanDir %{public}s", libraryPath.c_str());
        if (!ScanDir(libraryPath, fileEntries_)) {
            HILOG_INFO("MainThread::LoadAbilityLibrary scanDir %{public}s not exits", libraryPath.c_str());
        }
        libraryPath = libraryPath + "/libs";
        if (!ScanDir(libraryPath, fileEntries_)) {
            HILOG_INFO("MainThread::LoadAbilityLibrary scanDir %{public}s not exits", libraryPath.c_str());
        }
    }

    if (fileEntries_.empty()) {
        HILOG_INFO("No ability library");
        return;
    }

    void *handleAbilityLib = nullptr;
    for (auto fileEntry : fileEntries_) {
        if (!fileEntry.empty()) {
            handleAbilityLib = dlopen(fileEntry.c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (handleAbilityLib == nullptr) {
                HILOG_ERROR("MainThread::LoadAbilityLibrary Fail to dlopen %{public}s, [%{public}s]",
                    fileEntry.c_str(),
                    dlerror());
                exit(-1);
            }
            HILOG_INFO("MainThread::LoadAbilityLibrary Success to dlopen %{public}s", fileEntry.c_str());
            handleAbilityLib_.emplace_back(handleAbilityLib);
        }
    }
    HILOG_INFO("MainThread::LoadAbilityLibrary called end.");
#endif  // ABILITY_LIBRARY_LOADER
}

void MainThread::LoadAppLibrary()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef APPLICATION_LIBRARY_LOADER
    std::string appPath = applicationLibraryPath;
    HILOG_INFO("MainThread::handleLaunchApplication Start calling dlopen. appPath=%{public}s", appPath.c_str());
    handleAppLib_ = dlopen(appPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handleAppLib_ == nullptr) {
        HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]", appPath.c_str(), dlerror());
        exit(-1);
    }
    HILOG_INFO("MainThread::handleLaunchApplication End calling dlopen.");
#endif  // APPLICATION_LIBRARY_LOADER
}

/**
 *
 * @brief Close the ability library loaded.
 *
 */
void MainThread::CloseAbilityLibrary()
{
    HILOG_INFO("MainThread::CloseAbilityLibrary called start");
    for (auto iter : handleAbilityLib_) {
        if (iter != nullptr) {
            dlclose(iter);
            iter = nullptr;
        }
    }
    handleAbilityLib_.clear();
    fileEntries_.clear();
}

bool MainThread::ScanDir(const std::string &dirPath, std::vector<std::string> &files)
{
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == nullptr) {
        HILOG_ERROR("MainThread::ScanDir open dir:%{public}s fail", dirPath.c_str());
        return false;
    }
    struct dirent *df = nullptr;
    for (;;) {
        df = readdir(dirp);
        if (df == nullptr) {
            break;
        }

        std::string currentName(df->d_name);
        if (currentName.compare(".") == 0 || currentName.compare("..") == 0) {
            continue;
        }

        if (CheckFileType(currentName, abilityLibraryType_)) {
            files.emplace_back(dirPath + pathSeparator_ + currentName);
        }
    }

    if (closedir(dirp) == -1) {
        HILOG_WARN("close dir fail");
    }
    return true;
}

/**
 *
 * @brief Check the fileType.
 *
 * @param fileName The fileName of the lib.
 * @param extensionName The extensionName of the lib.
 *
 * @return if the FileType is legal, return true. else return false.
 *
 */
bool MainThread::CheckFileType(const std::string &fileName, const std::string &extensionName)
{
    HILOG_DEBUG("MainThread::CheckFileType path is %{public}s, support suffix is %{public}s",
        fileName.c_str(),
        extensionName.c_str());

    if (fileName.empty()) {
        HILOG_ERROR("the file name is empty");
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        HILOG_WARN("filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    return LowerStr(suffixStr) == extensionName;
}

void MainThread::HandleScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    HILOG_INFO("MainThread::HandleScheduleAcceptWant");
    if (!application_) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    std::string specifiedFlag;
    application_->ScheduleAcceptWant(want, moduleName, specifiedFlag);

    if (!appMgr_ || !applicationImpl_) {
        HILOG_ERROR("appMgr_ is nullptr");
        return;
    }

    appMgr_->ScheduleAcceptWantDone(applicationImpl_->GetRecordId(), want, specifiedFlag);
}

void MainThread::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    HILOG_INFO("MainThread::ScheduleAcceptWant start");
    wptr<MainThread> weak = this;
    auto task = [weak, want, moduleName]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, HandleScheduleAcceptWant failed.");
            return;
        }
        appThread->HandleScheduleAcceptWant(want, moduleName);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleAcceptWant PostTask task failed");
    }
    HILOG_INFO("MainThread::ScheduleAcceptWant end.");
}
#endif  // ABILITY_LIBRARY_LOADER
}  // namespace AppExecFwk
}  // namespace OHOS
