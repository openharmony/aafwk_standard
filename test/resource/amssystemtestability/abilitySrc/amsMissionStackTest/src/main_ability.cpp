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
constexpr int index_f = 0;
constexpr int index_s = 1;
constexpr int index_t = 2;
constexpr int paramCnt = 3;

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
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnStart");
}

void MainAbility::OnStop()
{
    HILOG_INFO("MainAbility::OnStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnStop");
}

void MainAbility::OnActive()
{
    HILOG_INFO("MainAbility::OnActive");
    Ability::OnActive();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnActive");
}

void MainAbility::OnInactive()
{
    HILOG_INFO("MainAbility::OnInactive");
    Ability::OnInactive();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnInactive");
}

void MainAbility::OnBackground()
{
    HILOG_INFO("MainAbility::OnBackground");
    Ability::OnBackground();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnBackground");
}

void MainAbility::OnForeground(const Want &want)
{
    HILOG_INFO("MainAbility::OnForeground");
    Ability::OnForeground(want);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnForeground");
}

void MainAbility::OnRestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("MainAbility::OnRestoreAbilityState");
    Ability::OnRestoreAbilityState(inState);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnRestoreAbilityState");
}
void MainAbility::OnSaveAbilityState(PacMap &outState)
{
    HILOG_INFO("MainAbility::OnSaveAbilityState");
    Ability::OnSaveAbilityState(outState);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN_LIFECYCLE, MAIN_ABILITY_CODE, "OnSaveAbilityState");
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
    if (std::strcmp(eventName.c_str(), g_EVENT_REQU_MAIN.c_str()) == 0) {
        auto target = data.GetData();
        auto caseInfo = TestUtils::split(target, "_");
        if (caseInfo.size() < paramCnt) {
            return;
        }
        if (mapTestFunc_.find(caseInfo[index_f]) != mapTestFunc_.end()) {
            mapTestFunc_[caseInfo[index_f]](std::stoi(caseInfo[index_s]), std::stoi(caseInfo[index_t]), data.GetCode());
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

void MainAbility::MissionStackCase1(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase1");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase2(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase2");
    bool result = true;
    LockMission();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
    TerminateAbility();
}
void MainAbility::MissionStackCase3(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase3");
    bool result = true;
    LockMission();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase4(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase5(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase6(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase6");
    bool result = true;
    LockMission();
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "ThirdAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase7(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase7");
    bool result = true;
    LockMission();
    UnlockMission();
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase8(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase9(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase9");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase10(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase10");
    bool result = true;
    int missionId = GetMissionId();
    result = missionId >= 0;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(missionId));
}
void MainAbility::MissionStackCase11(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase11");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase12(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase12");
    bool result = true;
    result = MoveMissionToEnd(true);
    if (result) {
        HILOG_INFO("MainAbility::MissionStackCase12 MoveMissionToEnd(true)true ====> %{public}d", result);
    } else {
        HILOG_INFO("MainAbility::MissionStackCase12 MoveMissionToEnd(true)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase13(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase13");
    bool result = true;
    result = MoveMissionToEnd(false);
    if (result) {
        HILOG_INFO("MainAbility::MissionStackCase13 MoveMissionToEnd(false)true ====> %{public}d", result);
    } else {
        HILOG_INFO("MainAbility::MissionStackCase13 MoveMissionToEnd(false)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase14(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase14");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase15(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase15");
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase16(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase16");
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase17(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase17");
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::MissionStackCase18(int code)
{
    HILOG_INFO("MainAbility::MissionStackCase18");
    bool result = true;
    LockMission();
    result = MoveMissionToEnd(false);
    if (result) {
        HILOG_INFO("MainAbility::MissionStackCase18 MoveMissionToEnd(false)true ====> %{public}d", result);
    } else {
        HILOG_INFO("MainAbility::MissionStackCase18 MoveMissionToEnd(false)false ====> %{public}d", result);
    }
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::SaveAbilityStateCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::SaveAbilityStateCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::SaveAbilityStateCase3(int code)
{
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}

void MainAbility::RestoreAbilityStateCase1(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::RestoreAbilityStateCase2(int code)
{
    bool result = true;
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}
void MainAbility::RestoreAbilityStateCase3(int code)
{
    bool result = true;
    std::map<std::string, std::string> params;
    Want want = TestUtils::MakeWant("", "SecondAbility", "com.ohos.amsst.MissionStack", params);
    AbilityContext::StartAbility(want, 1);
    TestUtils::PublishEvent(g_EVENT_RESP_MAIN, code, std::to_string(result));
}

REGISTER_AA(MainAbility)
}  // namespace AppExecFwk
}  // namespace OHOS
