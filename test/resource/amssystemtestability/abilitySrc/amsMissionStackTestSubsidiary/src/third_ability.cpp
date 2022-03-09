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

#include "third_ability.h"
#include "hilog_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;

void ThirdAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("ThirdAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

ThirdAbility::~ThirdAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void ThirdAbility::OnStart(const Want &want)
{
    HILOG_INFO("ThirdAbility::OnStart");
    SubscribeEvent();
    Ability::OnStart(want);
    callbackSeq += "OnStart";
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnStart");
}

void ThirdAbility::OnStop()
{
    HILOG_INFO("ThirdAbility::OnStop");
    Ability::OnStop();
    callbackSeq += "OnStop";  // OnInactiveOnBackgroundOnStop
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void ThirdAbility::OnActive()
{
    HILOG_INFO("ThirdAbility::OnActive====<");
    Ability::OnActive();
    callbackSeq += "OnActive";  // OnStartOnActive
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, callbackSeq);
    callbackSeq = "";
}

void ThirdAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    HILOG_INFO("ThirdAbility::OnConfigurationUpdated====<");
    Ability::OnConfigurationUpdated(configuration);
    callbackUpdated += "Updated";  // UpdatedUpdated
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, callbackUpdated);
}

void ThirdAbility::OnInactive()
{
    HILOG_INFO("ThirdAbility::OnInactive");
    Ability::OnInactive();
    callbackSeq += "OnInactive";
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnInactive");
}

void ThirdAbility::OnBackground()
{
    HILOG_INFO("ThirdAbility::OnBackground");
    Ability::OnBackground();
    callbackSeq += "OnBackground";
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnBackground");
}

void ThirdAbility::OnForeground(const Want &want)
{
    HILOG_INFO("ThirdAbility::OnForeground");
    Ability::OnForeground(want);
    callbackSeq += "OnForeground";
    TestUtils::PublishEvent(g_EVENT_RESP_THIRD_LIFECYCLE, THIRD_ABILITY_CODE, "OnForeground");
}

void ThirdAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        // g_EVENT_REQU_THIRD,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<ThirdAbilityEventSubscriber>(subscribeInfo);
    subscriber_->thirdAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void ThirdAbilityEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("ThirdAbilityEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("ThirdAbilityEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("ThirdAbilityEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
}

void ThirdAbility::TestAbility(int apiIndex, int caseIndex, int code)
{
    HILOG_INFO("ThirdAbility::TestAbility");
    if (mapCase_.find(apiIndex) != mapCase_.end()) {
        if (caseIndex < (int)mapCase_[apiIndex].size()) {
            mapCase_[apiIndex][caseIndex](code);
        }
    }
}

REGISTER_AA(ThirdAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
