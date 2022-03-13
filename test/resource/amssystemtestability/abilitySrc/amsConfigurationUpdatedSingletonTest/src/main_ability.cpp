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

#include "main_ability.h"
#include "hilog_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void MainAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("MainAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

MainAbility::~MainAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void MainAbility::OnStart(const Want &want)
{
    HILOG_INFO("MainAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, "OnStart");
}

void MainAbility::OnStop()
{
    HILOG_INFO("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void MainAbility::OnActive()
{
    HILOG_INFO("MainAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void MainAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    std::string languageValue;
    std::string orientationValue;

    HILOG_INFO("MainAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    languageValue = configuration.GetItem(GlobalConfigurationKey::SYSTEM_LANGUAGE);
    orientationValue = configuration.GetItem(GlobalConfigurationKey::SYSTEM_ORIENTATION);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, languageValue);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, orientationValue);
    callbackUpdated += "Updated";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, callbackUpdated);
    callbackUpdated = "";
}

void MainAbility::OnInactive()
{
    HILOG_INFO("MainAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, "OnInactive");
}

void MainAbility::OnBackground()
{
    HILOG_INFO("MainAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    HILOG_INFO("MainAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, THIRD_ABILITY_CODE, "OnForeground");
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_MAIN,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<MainAbilityEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void MainAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("MainAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("MainAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("MainAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void MainAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    HILOG_INFO("MainAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
