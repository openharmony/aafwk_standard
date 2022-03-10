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

#include "main_ability.h"
#include "hilog_wrapper.h"
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
namespace {
constexpr int numZero = 0;
constexpr int numOne = 1;
constexpr int numTwo = 2;
constexpr int numThree = 3;
}

void MainAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("MainAbility::Init");
    Ability::Init(abilityInfo, application, handler, token);
}

void MainAbility::OnStart(const Want &want)
{
    HILOG_INFO("MainAbility::onStart");
    SubscribeEvent();
    Ability::OnStart(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "onStart");
}

void MainAbility::OnStop()
{
    HILOG_INFO("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "OnStop");
}

void MainAbility::OnActive()
{
    HILOG_INFO("MainAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "OnActive");
}

void MainAbility::OnInactive()
{
    HILOG_INFO("MainAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "OnInactive");
}

void MainAbility::OnBackground()
{
    HILOG_INFO("MainAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    HILOG_INFO("MainAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_FIRSTB_LIFECYCLE, MAIN_ABILITY_B_CODE, "OnForeground");
}

MainAbility::~MainAbility()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void MainAbility::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        g_EVENT_REQU_FIRSTB,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FirstEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FirstEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("FirstEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("FirstEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("FirstEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_FIRSTB.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        if (caseInfo.size() < numThree) {
            return;
        }
        if (mapTestFunc_.find(caseInfo[numZero]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[numZero]](std::stoi(caseInfo[numOne]), std::stoi(caseInfo[numTwo]), data.GetCode());
        } else {
            HILOG_INFO("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
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
