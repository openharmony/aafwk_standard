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

#include "amsstabilityq2.h"

namespace OHOS {
namespace AppExecFwk {
void AmsStAbilityQ2::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("AmsStAbilityQ2::Init");
    Ability::Init(abilityInfo, application, handler, token);
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this());
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateInit;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
    std::this_thread::sleep_for(std::chrono::milliseconds(STEventName::LOAD_TIMEOUT));
}

void AmsStAbilityQ2::OnStart(const Want &want)
{
    GetWantInfo(want);
    HILOG_INFO("AmsStAbilityQ2::onStart");
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsStAbilityQ2::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStAbilityQ2::OnNewWant");
    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsStAbilityQ2::OnForeground(const Want &want)
{
    HILOG_INFO("AmsStAbilityQ2::OnForeground");
    Ability::OnForeground(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnForeground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnForegroundCount(), eventData);
}

void AmsStAbilityQ2::OnStop()
{
    HILOG_INFO("AmsStAbilityQ2::onStop");
    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsStAbilityQ2::OnActive()
{
    HILOG_INFO("AmsStAbilityQ2::OnActive");
    Ability::OnActive();
    std::string startBundleName = this->Split(targetBundle, ",");
    std::string startAbilityName = this->Split(targetAbility, ",");
    if (!startBundleName.empty() && !startAbilityName.empty()) {
        Want want;
        want.SetElementName(startBundleName, startAbilityName);
        want.SetParam("shouldReturn", shouldReturn);
        if (!targetBundle.empty() && !targetAbility.empty()) {
            want.SetParam("targetBundle", targetBundle);
            want.SetParam("targetAbility", targetAbility);
        }
        StartAbility(want);
    }
    if (std::string::npos != shouldReturn.find(GetAbilityName())) {
        TerminateAbility();
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsStAbilityQ2::OnInactive()
{
    HILOG_INFO("AmsStAbilityQ2::OnInactive");
    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsStAbilityQ2::OnBackground()
{
    HILOG_INFO("AmsStAbilityQ2::OnBackground");
    Ability::OnBackground();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

void AmsStAbilityQ2::Clear()
{
    shouldReturn = "";
    targetBundle = "";
    targetAbility = "";
}

void AmsStAbilityQ2::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn = mWant.GetStringParam("shouldReturn");
    targetBundle = mWant.GetStringParam("targetBundle");
    targetAbility = mWant.GetStringParam("targetAbility");
}

std::string AmsStAbilityQ2::Split(std::string &str, std::string delim)
{
    std::string result;
    if (!str.empty()) {
        size_t index = str.find(delim);
        if (index != std::string::npos) {
            result = str.substr(0, index);
            str = str.substr(index + delim.size());
        } else {
            result = str;
            str = "";
        }
    }
    return result;
}

REGISTER_AA(AmsStAbilityQ2);
}  // namespace AppExecFwk
}  // namespace OHOS