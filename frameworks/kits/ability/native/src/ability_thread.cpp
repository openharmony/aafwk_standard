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

#include "ability_thread.h"

#include <chrono>
#include <functional>
#include <thread>

#include "ability_context_impl.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "ability_loader.h"
#include "abs_shared_result_set.h"
#include "application_impl.h"
#include "hitrace_meter.h"
#include "context_deal.h"
#include "data_ability_predicates.h"
#include "dataobs_mgr_client.h"
#include "hilog_wrapper.h"
#include "ohos_application.h"
#ifdef SUPPORT_GRAPHICS
#include "page_ability_impl.h"
#endif
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::chrono_literals;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
#ifdef ABILITY_COMMAND_FOR_TEST
const int32_t BLOCK_ABILITY_TIME = 20;
#endif
constexpr static char ACE_SERVICE_ABILITY_NAME[] = "AceServiceAbility";
constexpr static char ACE_DATA_ABILITY_NAME[] = "AceDataAbility";
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "Ability";
constexpr static char ACE_ABILITY_NAME[] = "AceAbility";
constexpr static char ACE_FORM_ABILITY_NAME[] = "AceFormAbility";
constexpr static char FORM_EXTENSION[] = "FormExtension";
#endif
constexpr static char BASE_SERVICE_EXTENSION[] = "ServiceExtension";
constexpr static char STATIC_SUBSCRIBER_EXTENSION[] = "StaticSubscriberExtension";
constexpr static char DATA_SHARE_EXT_ABILITY[] = "DataShareExtAbility";
constexpr static char WORK_SCHEDULER_EXTENSION[] = "WorkSchedulerExtension";
constexpr static char ACCESSIBILITY_EXTENSION[] = "AccessibilityExtension";
constexpr static char WALLPAPER_EXTENSION[] = "WallpaperExtension";

/**
 * @brief Default constructor used to create a AbilityThread instance.
 */
AbilityThread::AbilityThread()
    : abilityImpl_(nullptr), token_(nullptr), currentAbility_(nullptr), abilityHandler_(nullptr), runner_(nullptr)
{}

AbilityThread::~AbilityThread()
{
    DelayedSingleton<AbilityImplFactory>::DestroyInstance();
}

std::string AbilityThread::CreateAbilityName(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    std::shared_ptr<OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        HILOG_ERROR("AbilityThread::CreateAbilityName failed,abilityRecord or app is nullptr");
        return abilityName;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,abilityInfo is nullptr");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        HILOG_INFO("Create ability name success, name is %{public}s.", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AbilityType::PAGE) {
        if (abilityInfo->isStageBasedModel) {
            abilityName = ABILITY_NAME;
        } else {
            abilityName = ACE_ABILITY_NAME;
        }
    } else if (abilityInfo->type == AbilityType::SERVICE) {
#else
    if (abilityInfo->type == AbilityType::SERVICE) {
#endif
#ifdef SUPPORT_GRAPHICS
        if (abilityInfo->formEnabled == true) {
            abilityName = ACE_FORM_ABILITY_NAME;
        } else {
#endif
            abilityName = ACE_SERVICE_ABILITY_NAME;
#ifdef SUPPORT_GRAPHICS
        }
#endif
    } else if (abilityInfo->type == AbilityType::DATA) {
        abilityName = ACE_DATA_ABILITY_NAME;
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
        if (abilityName.length() > 0) {
            HILOG_INFO("Get extension name by plugin success, name: %{public}s", abilityName.c_str());
            return abilityName;
        }
        abilityName = BASE_SERVICE_EXTENSION;
#ifdef SUPPORT_GRAPHICS
        if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == ExtensionAbilityType::FORM) {
            abilityName = FORM_EXTENSION;
        }
#endif
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::STATICSUBSCRIBER) {
            abilityName = STATIC_SUBSCRIBER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::DATASHARE) {
            abilityName = DATA_SHARE_EXT_ABILITY;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::WORK_SCHEDULER) {
            abilityName = WORK_SCHEDULER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::ACCESSIBILITY) {
            abilityName = ACCESSIBILITY_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::WALLPAPER) {
            abilityName = WALLPAPER_EXTENSION;
        }
        HILOG_INFO("CreateAbilityName extension type, abilityName:%{public}s", abilityName.c_str());
    } else {
        abilityName = abilityInfo->name;
    }

    HILOG_INFO("Create ability name success, name is %{public}s.", abilityName.c_str());
    return abilityName;
}

/**
 * @description: Create and init contextDeal.
 *
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param abilityObject Indicates the abilityObject.
 *
 * @return Returns the contextDeal.
 *
 */
std::shared_ptr<ContextDeal> AbilityThread::CreateAndInitContextDeal(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<Context> &abilityObject)
{
    HILOG_INFO("AbilityThread::CreateAndInitContextDeal.");
    std::shared_ptr<ContextDeal> contextDeal = nullptr;
    if ((application == nullptr) || (abilityRecord == nullptr) || (abilityObject == nullptr)) {
        HILOG_ERROR("AbilityThread::ability attach failed,context or record or abilityObject is nullptr");
        return contextDeal;
    }

    contextDeal = std::make_shared<ContextDeal>();
    if (contextDeal == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,contextDeal  is nullptr");
        return contextDeal;
    }

    contextDeal->SetAbilityInfo(abilityRecord->GetAbilityInfo());
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    contextDeal->SetProcessInfo(application->GetProcessInfo());

    std::shared_ptr<Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);

    contextDeal->SetBundleCodePath(abilityRecord->GetAbilityInfo()->codePath);
    contextDeal->SetContext(abilityObject);
    contextDeal->SetRunner(abilityHandler_->GetEventRunner());
    return contextDeal;
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param mainRunner The runner which main_thread holds.
 */
void AbilityThread::Attach(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("Attach ability failed, context or record is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName == "") {
        HILOG_ERROR("Attach ability failed, abilityInfo is nullptr.");
        return;
    }
    HILOG_INFO("Attach ability begin, ability:%{public}s.", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AbilityHandler>(mainRunner, this);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Attach ability failed, abilityHandler_ is nullptr.");
        return;
    }

    // 2.new ability
    auto ability = AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("Attach ability failed, load ability failed.");
        return;
    }

    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<Context> abilityObject = currentAbility_;
    std::shared_ptr<ContextDeal> contextDeal = CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_,
        stageContext));

    // 3.new abilityImpl
    abilityImpl_ =
        DelayedSingleton<AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Attach ability failed, abilityImpl_ == nullptr.");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_, contextDeal);
    // 4. ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Attach ability failed, err = %{public}d.", err);
        return;
    }
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param mainRunner The runner which main_thread holds.
 */
void AbilityThread::AttachExtension(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("Attach extension failed, context or record is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName == "") {
        HILOG_ERROR("Attach ability failed, abilityInfo is nullptr.");
        return;
    }
    HILOG_INFO("Attach extension begin, extension:%{public}s.", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AbilityHandler>(mainRunner, this);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Attach extension failed, abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("Attach extension failed, load ability failed");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("Attach extension failed, extensionImpl_ == nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Attach extension failed, err = %{public}d", err);
        return;
    }
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param mainRunner The runner which main_thread holds.
 */
void AbilityThread::AttachExtension(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::AttachExtension begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,context or record is nullptr");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,create runner failed");
        return;
    }
    abilityHandler_ = std::make_shared<AbilityHandler>(runner_, this);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,load extension failed");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::extension extensionImpl_ == nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: AttachExtension failed err = %{public}d", err);
        return;
    }
    HILOG_INFO("AbilityThread::AttachExtension end");
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 */
void AbilityThread::Attach(
    std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::Attach begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("AbilityThread::ability attach failed,context or record is nullptr");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,create runner failed");
        return;
    }
    abilityHandler_ = std::make_shared<AbilityHandler>(runner_, this);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,load ability failed");
        return;
    }

    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<Context> abilityObject = currentAbility_;
    std::shared_ptr<ContextDeal> contextDeal = CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_,
        stageContext));

    // 3.new abilityImpl
    abilityImpl_ =
        DelayedSingleton<AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_, contextDeal);
    // 4. ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: attach success failed err = %{public}d", err);
        return;
    }

    HILOG_INFO("AbilityThread::Attach end");
}

/**
 * @description:  Handle the life cycle of Ability.
 * @param want  Indicates the structure containing lifecycle information about the ability.
 * @param lifeCycleStateInfo  Indicates the lifeCycleStateInfo.
 */
void AbilityThread::HandleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Handle ability transaction begin, name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Handle ability transaction error, abilityImpl_ == nullptr.");
        return;
    }

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId,
        lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName,
        lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo);
    HILOG_INFO("Handle ability transaction end.");
}

/**
 * @brief Handle the life cycle of Extension.
 *
 * @param want  Indicates the structure containing lifecycle information about the extension.
 * @param lifeCycleStateInfo  Indicates the lifeCycleStateInfo.
 */
void AbilityThread::HandleExtensionTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleExtensionTransaction begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleExtensionTransaction extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo);
    HILOG_INFO("AbilityThread::HandleAbilityTransaction end");
}

/**
 * @description:  Handle the current connection of Ability.
 * @param want  Indicates the structure containing connection information about the ability.
 */
void AbilityThread::HandleConnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleConnectAbility begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleConnectAbility abilityImpl_ == nullptr");
        return;
    }

    sptr<IRemoteObject> service = abilityImpl_->ConnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: HandleConnectAbility failed err = %{public}d", err);
    }
    HILOG_INFO("AbilityThread::HandleConnectAbility end");
}

/**
 * @description:  Handle the current disconnection of Ability.
 */
void AbilityThread::HandleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Handle disconnect ability begin.");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Handle disconnect ability error, abilityImpl_ == nullptr.");
        return;
    }

    abilityImpl_->DisconnectAbility(want);
    HILOG_INFO("Handle disconnect ability done, notify ability manager service.");
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Handle disconnect ability error, err = %{public}d.", err);
    }
}

/**
 * @brief Handle the current command of Ability.
 *
 * @param want The Want object to command to.
 *
 * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 *
 * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by 1
 * every time the ability is started. For example, if the ability has been started for six times, the value of startId
 * is 6.
 */
void AbilityThread::HandleCommandAbility(const Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleCommandAbility begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleCommandAbility failed. abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->CommandAbility(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: HandleCommandAbility  failed err = %{public}d", err);
    }
    HILOG_INFO("AbilityThread::HandleCommandAbility end");
}

/**
 * @brief Handle the current connection of Extension.
 *
 * @param want  Indicates the structure containing connection information about the extension.
 */
void AbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleConnectExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleConnectExtension extensionImpl_ == nullptr");
        return;
    }
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread::HandleConnectExtension failed err = %{public}d", err);
    }
    HILOG_INFO("AbilityThread::HandleConnectExtension end");
}

/**
 * @brief Handle the current disconnection of Extension.
 */
void AbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleDisconnectExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleDisconnectExtension extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->DisconnectExtension(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: HandleDisconnectExtension failed err = %{public}d", err);
    }
    HILOG_INFO("AbilityThread::HandleDisconnectExtension end");
}

/**
 * @brief Handle the current command of Extension.
 *
 * @param want The Want object to command to.
 * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 * @param startId Indicates the number of times the Service Extension has been started. The startId is incremented by 1
 * every time the Extension is started. For example, if the Extension has been started for six times,
 * the value of startId is 6.
 */
void AbilityThread::HandleCommandExtension(const Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread::HandleCommandExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleCommandExtension extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread::HandleCommandExtension failed err = %{public}d", err);
    }
    HILOG_INFO("AbilityThread::HandleCommandExtension end");
}

/**
 * @description: Handle the restoreAbility state.
 * @param state  Indicates save ability state used to dispatchRestoreAbilityState.
 */
void AbilityThread::HandleRestoreAbilityState(const PacMap &state)
{
    HILOG_INFO("AbilityThread::HandleRestoreAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleRestoreAbilityState abilityImpl_ == nullptr");
        return;
    }

    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_INFO("AbilityThread::HandleRestoreAbilityState end");
}

/**
 * @description: Provide operating system SaveabilityState information to the observer
 */
void AbilityThread::ScheduleSaveAbilityState()
{
    HILOG_INFO("AbilityThread::ScheduleSaveAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleSaveAbilityState abilityImpl_ == nullptr");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    HILOG_INFO("AbilityThread::ScheduleSaveAbilityState end");
}

/**
 * @description:  Provide operating system RestoreAbilityState information to the observer
 * @param state Indicates resotre ability state used to dispatchRestoreAbilityState.
 */
void AbilityThread::ScheduleRestoreAbilityState(const PacMap &state)
{
    HILOG_INFO("AbilityThread::ScheduleRestoreAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleRestoreAbilityState abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_INFO("AbilityThread::ScheduleRestoreAbilityState end");
}

/*
 * @brief ScheduleUpdateConfiguration, scheduling update configuration.
 */
void AbilityThread::ScheduleUpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("AbilityThread::ScheduleUpdateConfiguration begin");
    wptr<AbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleUpdateConfiguration failed.");
            return;
        }

        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionUpdateConfiguration(config);
        } else {
            abilityThread->HandleUpdateConfiguration(config);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleUpdateConfiguration abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleUpdateConfiguration PostTask error");
    }
    HILOG_INFO("AbilityThread::ScheduleUpdateConfiguration end");
}

/*
 * @brief Handle the scheduling update configuration.
 */
void AbilityThread::HandleUpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("AbilityThread::HandleUpdateConfiguration begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleUpdateConfiguration abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    HILOG_INFO("AbilityThread::HandleUpdateConfiguration end");
}

void AbilityThread::HandleExtensionUpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("AbilityThread::HandleExtensionUpdateConfiguration begin");
    if (!extensionImpl_) {
        HILOG_ERROR("AbilityThread::HandleExtensionUpdateConfiguration extensionImpl_ is nullptr");
        return;
    }

    extensionImpl_->ScheduleUpdateConfiguration(config);
    HILOG_INFO("AbilityThread::HandleExtensionUpdateConfiguration end");
}

/**
 * @description:  Provide operating system AbilityTransaction information to the observer
 * @param want Indicates the structure containing Transaction information about the ability.
 * @param lifeCycleStateInfo Indicates the lifecycle state.
 */
void AbilityThread::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule ability transaction, name is %{public}s, targeState is %{public}d, isNewWant is %{public}d.",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);

    if (token_ == nullptr) {
        HILOG_ERROR("ScheduleAbilityTransaction::failed, token_  nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleAbilityTransaction failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo);
        } else {
            abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction PostTask error");
    }
}

/**
 * @description:  Provide operating system ConnectAbility information to the observer
 * @param  want Indicates the structure containing connect information about the ability.
 */
void AbilityThread::ScheduleConnectAbility(const Want &want)
{
    HILOG_INFO("AbilityThread::ScheduleConnectAbility begin, isExtension_:%{public}d", isExtension_);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleConnectAbility failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleConnectExtension(want);
        } else {
            abilityThread->HandleConnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleConnectAbility abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleConnectAbility PostTask error");
    }
    HILOG_INFO("AbilityThread::ScheduleConnectAbility end");
}

/**
 * @description: Provide operating system ConnectAbility information to the observer
 * @return  None
 */
void AbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule disconnect ability begin, isExtension:%{public}d.", isExtension_);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("Schedule disconnect ability error, abilityThread is nullptr.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleDisconnectExtension(want);
        } else {
            abilityThread->HandleDisconnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Schedule disconnect ability error, abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("Schedule disconnect ability error, PostTask error");
    }
}

/**
 * @description: Provide operating system CommandAbility information to the observer
 *
 * @param want The Want object to command to.
 *
 * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 *
 * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by 1
 * every time the ability is started. For example, if the ability has been started for six times, the value of startId
 * is 6.
 */
void AbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_INFO("AbilityThread::ScheduleCommandAbility begin. startId:%{public}d", startId);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleCommandAbility failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleCommandExtension(want, restart, startId);
        } else {
            abilityThread->HandleCommandAbility(want, restart, startId);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleCommandAbility abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleCommandAbility PostTask error");
    }
    HILOG_INFO("AbilityThread::ScheduleCommandAbility end");
}

/**
 * @brief Send the result code and data to be returned by this Page ability to the caller.
 * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
 * receive the result set in the current method. This method can be called only after the ability has been initialized.
 *
 * @param requestCode Indicates the request code for send.
 * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result code
 * to identify an error.
 * @param want Indicates the data returned after the ability is destroyed. You can define the data returned. This
 * parameter can be null.
 */
void AbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_INFO("AbilityThread::SendResult begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::SendResult abilityImpl_ == nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr || abilityThread->abilityImpl_ == nullptr) {
            HILOG_ERROR("abilityThread or abilityImpl is nullptr, SendResult failed.");
            return;
        }
        if (requestCode != -1) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::SendResult abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::SendResult PostTask error");
    }
    HILOG_INFO("AbilityThread::SendResult end");
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> AbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("AbilityThread::GetFileTypes begin");
    std::vector<std::string> types;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::GetFileTypes abilityImpl_ is nullptr");
        return types;
    }

    types = abilityImpl_->GetFileTypes(uri, mimeTypeFilter);
    HILOG_INFO("AbilityThread::GetFileTypes end");
    return types;
}

/**
 * @brief Opens a file in a specified remote path.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the file descriptor.
 */
int AbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("AbilityThread::OpenFile begin");
    int fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::OpenFile abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenFile(uri, mode);
    HILOG_INFO("AbilityThread::OpenFile end");
    return fd;
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int AbilityThread::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("AbilityThread::OpenRawFile begin");
    int fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::OpenRawFile abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenRawFile(uri, mode);
    HILOG_INFO("AbilityThread::OpenRawFile end");
    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int AbilityThread::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("AbilityThread::Insert begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Insert abilityImpl_ is nullptr");
        return index;
    }

    index = abilityImpl_->Insert(uri, value);
    HILOG_INFO("AbilityThread::Insert end");
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityThread::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    HILOG_INFO("AbilityThread::Call begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Insert abilityImpl_ is nullptr");
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::PacMap> result = abilityImpl_->Call(uri, method, arg, pacMap);
    HILOG_INFO("AbilityThread::Call end");
    return result;
}

/**
 * @brief Updates data records in the database.
 *
 * @param uri Indicates the path of data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records updated.
 */
int AbilityThread::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityThread::Update begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Update abilityImpl_ is nullptr");
        return index;
    }

    index = abilityImpl_->Update(uri, value, predicates);
    HILOG_INFO("AbilityThread::Update end");
    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records deleted.
 */
int AbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityThread::Delete begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Delete abilityImpl_ is nullptr");
        return index;
    }
    index = abilityImpl_->Delete(uri, predicates);
    HILOG_INFO("AbilityThread::Delete end");
    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of data to query.
 * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the query result.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityThread::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityThread::Query begin");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Query abilityImpl_ is nullptr");
        return resultSet;
    }

    resultSet = abilityImpl_->Query(uri, columns, predicates);
    HILOG_INFO("AbilityThread::Query end");
    return resultSet;
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string AbilityThread::GetType(const Uri &uri)
{
    HILOG_INFO("AbilityThread::GetType begin");
    std::string type;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::GetType abilityImpl_ is nullptr");
        return type;
    }

    type = abilityImpl_->GetType(uri);
    HILOG_INFO("AbilityThread::GetType end");
    return type;
}

/**
 * @brief Reloads data in the database.
 *
 * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
 * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
 * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
 * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool AbilityThread::Reload(const Uri &uri, const PacMap &extras)
{
    HILOG_INFO("AbilityThread::Reload begin");
    bool ret = false;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Reload abilityImpl_ is nullptr");
        return ret;
    }
    ret = abilityImpl_->Reload(uri, extras);
    HILOG_INFO("AbilityThread::Reload end");
    return ret;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int AbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("AbilityThread::BatchInsert begin");
    int ret = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::BatchInsert abilityImpl_ is nullptr");
        return ret;
    }

    ret = abilityImpl_->BatchInsert(uri, values);
    HILOG_INFO("AbilityThread::BatchInsert end");
    return ret;
}

void AbilityThread::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    HILOG_INFO("ContinueAbility");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ContinueAbility abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
}

void AbilityThread::NotifyContinuationResult(int32_t result)
{
    HILOG_INFO("NotifyContinuationResult, result:%{public}d", result);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::NotifyContinuationResult abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param mainRunner The runner which main_thread holds.
 * @param stageContext the AbilityStage context
 */
void AbilityThread::AbilityThreadMain(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread main start.");
    sptr<AbilityThread> thread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    if (thread == nullptr) {
        HILOG_ERROR("AbilityThread::AbilityThreadMain failed,thread  is nullptr");
        return;
    }
    thread->InitExtensionFlag(abilityRecord);
    if (thread->isExtension_) {
        thread->AttachExtension(application, abilityRecord, mainRunner);
    } else {
        thread->Attach(application, abilityRecord, mainRunner, stageContext);
    }
    HILOG_INFO("AbilityThread main end.");
}

/**
 * @description: Attach The ability thread to the main process.
 * @param application Indicates the main process.
 * @param abilityRecord Indicates the abilityRecord.
 * @param stageContext the AbilityStage context
 */
void AbilityThread::AbilityThreadMain(
    std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityThread main start.");
    sptr<AbilityThread> thread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    if (thread == nullptr || abilityRecord == nullptr) {
        HILOG_ERROR("AbilityThread::AbilityThreadMain failed, thread is nullptr");
        return;
    }
    thread->InitExtensionFlag(abilityRecord);
    if (thread->isExtension_) {
        thread->AttachExtension(application, abilityRecord);
    } else {
        thread->Attach(application, abilityRecord, stageContext);
    }
    HILOG_INFO("AbilityThread main end.");
}

void AbilityThread::InitExtensionFlag(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HILOG_INFO("AbilityThread::InitExtensionFlag start");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("AbilityThread::InitExtensionFlag abilityRecord null");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityThread::InitExtensionFlag abilityInfo null");
        return;
    }
    if (abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        HILOG_INFO("AbilityThread::InitExtensionFlag true");
        isExtension_ = true;
    } else {
        isExtension_ = false;
    }
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri AbilityThread::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("AbilityThread::NormalizeUri begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("DataAbilityHelper::normalizeUri failed dataAbility == nullptr");
        return urivalue;
    }

    urivalue = abilityImpl_->NormalizeUri(uri);
    HILOG_INFO("AbilityThread::NormalizeUri end");
    return urivalue;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed to
 * this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found in the
 * current environment.
 */
Uri AbilityThread::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("AbilityThread::DenormalizeUri begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("DataAbilityHelper::denormalizeUri failed dataAbility == nullptr");
        return urivalue;
    }

    urivalue = abilityImpl_->DenormalizeUri(uri);
    HILOG_INFO("AbilityThread::DenormalizeUri end");
    return urivalue;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilityThread::HandleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->RegisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilityThread::HandleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->UnregisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
bool AbilityThread::HandleNotifyChange(const Uri &uri)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

/**
 * @brief Access authority verification.
 *
 * @return Returns true on success, others on failure.
 */
bool AbilityThread::CheckObsPermission()
{
    HILOG_INFO("%{public}s CheckObsPermission() run Permission Checkout", __func__);
    return true;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleRegisterObserver failed.");
            return;
        }
        abilityThread->HandleRegisterObserver(uri, dataObserver);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleRegisterObserver abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleRegisterObserver PostTask error");
    }
    return ret;
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleUnregisterObserver failed.");
            return;
        }
        abilityThread->HandleUnregisterObserver(uri, dataObserver);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleUnregisterObserver abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostSyncTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleUnregisterObserver PostTask error");
    }
    return ret;
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
bool AbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    HILOG_INFO("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleNotifyChange failed.");
            return;
        }
        abilityThread->HandleNotifyChange(uri);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleNotifyChange abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleNotifyChange PostTask error");
    }
    return ret;
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_INFO("AbilityThread::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ExecuteBatch abilityImpl_ is nullptr");
        results.clear();
        return results;
    }
    results = abilityImpl_->ExecuteBatch(operations);
    HILOG_INFO("AbilityThread::ExecuteBatch end");
    return results;
}

std::shared_ptr<AbilityRuntime::AbilityContext> AbilityThread::BuildAbilityContext(
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
    const sptr<IRemoteObject> &token, const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    return abilityContextImpl;
}

void AbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (token_ == nullptr) {
        HILOG_ERROR("DumpAbilityInfo::failed, token_  nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleAbilityTransaction failed.");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            HILOG_ERROR("AbilityThread:: DumpAbilityInfo failed err = %{public}d", err);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction abilityHandler_ == nullptr");
        return;
    }

    abilityHandler_->PostTask(task);
}

#ifdef SUPPORT_GRAPHICS
void AbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (currentAbility_ == nullptr && currentExtension_ == nullptr) {
        HILOG_INFO("currentAbility and currentExtension_ is nullptr.");
        return;
    }
    if (currentAbility_ != nullptr) {
        if (abilityImpl_->IsStageBasedModel()) {
            auto scene = currentAbility_->GetScene();
            if (scene == nullptr) {
                HILOG_ERROR("DumpAbilityInfo scene == nullptr");
                return;
            }
            auto window = scene->GetMainWindow();
            if (window == nullptr) {
                HILOG_ERROR("DumpAbilityInfo window == nullptr");
                return;
            }
            window->DumpInfo(params, info);
        }
        currentAbility_->Dump(params, info);
    }
    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
    HILOG_INFO("%{public}s end.", __func__);
}
#else
void AbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (currentAbility_ != nullptr) {
        currentAbility_->Dump(params, info);
    }

    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    DumpOtherInfo(info);
}
#endif

void AbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (!abilityHandler_) {
        HILOG_INFO("abilityHandler_ is nullptr.");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (!runner) {
        HILOG_INFO("runner_ is nullptr.");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (!ablityContext) {
            HILOG_INFO("current ability context is nullptr.");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (!localCallContainer) {
            HILOG_INFO("current ability context locall call container is nullptr.");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

sptr<IRemoteObject> AbilityThread::CallRequest()
{
    HILOG_INFO("AbilityThread::CallRequest begin");

    if (!currentAbility_) {
        HILOG_ERROR("ability is nullptr.");
        return nullptr;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<OHOS::AppExecFwk::Ability> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval] () {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            HILOG_ERROR("ability is nullptr.");
            return;
        }

        retval = currentAbility->CallRequest();
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("ability Handler is nullptr.");
        return nullptr;
    }

    abilityHandler_->PostSyncTask(syncTask);

    HILOG_INFO("AbilityThread::CallRequest end");
    return retval;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityThread::BlockAbility()
{
    HILOG_INFO("AbilityThread::BlockAblity begin");
    if (abilityHandler_) {
        auto task = []() {
            while (1) {
                std::this_thread::sleep_for(BLOCK_ABILITY_TIME*1s);
            }
        };
        abilityHandler_->PostTask(task);
        HILOG_INFO("AbilityThread::BlockAblity end");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
