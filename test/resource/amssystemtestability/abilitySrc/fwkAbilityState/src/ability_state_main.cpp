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

#include "ability_state_main.h"
#include <string>
#include "test_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::EventFwk;
namespace {
std::string FwkAbilityState_Event_Resp_A = "resp_com_ohos_amsst_FwkAbilityStateA";
std::string FwkAbilityState_Event_Requ_A = "requ_com_ohos_amsst_FwkAbilityStateA";
const std::string FwkAbilityState_SaveData_Int = "int_data_1";
int OnStopCode = 3;
int OnActiveCode = 1;
int OnBackgroundCode = 2;
int OnRestoreCheckCode = 4;
constexpr int iBlockTime = 12;
const int iSaveData = 12345;
}  // namespace

FwkAbilityStateMain::~FwkAbilityStateMain()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void FwkAbilityStateMain::OnStart(const Want &want)
{
    HILOG_INFO("FwkAbilityStateMain::onStart");
    SubscribeEvent();
    bIsBlockRestore = want.HasParameter("StartType1");
    bIsBlockSave = false;
    Ability::OnStart(want);
    callback_seq += "OnStart";
}

void FwkAbilityStateMain::OnNewWant(const Want &want)
{
    HILOG_INFO("FwkAbilityStateMain::OnNewWant");
    Ability::OnNewWant(want);
    callback_seq += "OnNewWant";
}

void FwkAbilityStateMain::OnForeground(const Want &want)
{
    HILOG_INFO("FwkAbilityStateMain::OnForeground");
    Ability::OnForeground(want);
    callback_seq += "OnForeground";
}

void FwkAbilityStateMain::OnStop()
{
    HILOG_INFO("FwkAbilityStateMain::onStop");
    Ability::OnStop();
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    callback_seq += "OnStop";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnStopCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnActive()
{
    HILOG_INFO("FwkAbilityStateMain::OnActive");
    Ability::OnActive();
    callback_seq += "OnActive";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnActiveCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnInactive()
{
    HILOG_INFO("FwkAbilityStateMain::OnInactive");
    Ability::OnInactive();
    callback_seq += "OnInactive";
}

void FwkAbilityStateMain::OnBackground()
{
    HILOG_INFO("FwkAbilityStateMain::OnBackground");
    Ability::OnBackground();
    callback_seq += "OnBackground";
    TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnBackgroundCode, callback_seq);
    callback_seq = "";
}

void FwkAbilityStateMain::OnBlockProcess(bool &bIsBlockFlag)
{
    int i = iBlockTime;

    if (bIsBlockFlag) {
        while (i-- > 0) {
            HILOG_INFO("FwkAbilityStateMain::OnBlockProcess time left %{public}d", i);
            sleep(1);
        }
        bIsBlockFlag = false;
    }
}

void FwkAbilityStateMain::OnSaveAbilityState(PacMap &outState)
{
    HILOG_INFO("FwkAbilityStateMain::OnSaveAbilityState");
    OnBlockProcess(bIsBlockSave);
    outState.PutIntValue(FwkAbilityState_SaveData_Int, iSaveData);
    Ability::OnSaveAbilityState(outState);
    callback_seq += "OnSaveAbilityState";
}

void FwkAbilityStateMain::OnRestoreAbilityState(const PacMap &inState)
{
    int iGetSaveData;
    PacMap tmpPacMap;

    HILOG_INFO("FwkAbilityStateMain::OnRestoreAbilityState");
    OnBlockProcess(bIsBlockRestore);
    Ability::OnRestoreAbilityState(inState);
    tmpPacMap = (PacMap)inState;
    iGetSaveData = tmpPacMap.GetIntValue(FwkAbilityState_SaveData_Int);
    if (iSaveData != iGetSaveData) {
        TestUtils::PublishEvent(FwkAbilityState_Event_Resp_A, OnRestoreCheckCode, "NotEqual");
        HILOG_INFO("FwkAbilityStateMain::restore not equal %{public}d", iGetSaveData);
    } else {
        HILOG_INFO("FwkAbilityStateMain::restore equal %{public}d", iGetSaveData);
    }
    callback_seq += "OnRestoreAbilityState";
}

void FwkAbilityStateMain::SubscribeEvent()
{
    std::vector<std::string> eventList = {
        FwkAbilityState_Event_Requ_A,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<FwkAbilityStateMainSubscriber>(subscribeInfo);
    subscriber_->mainAbility = this;
    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void FwkAbilityStateMainSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    if (strcmp(eventName.c_str(), FwkAbilityState_Event_Requ_A.c_str()) == 0) {
        auto target = data.GetData();
        if (mapAction_.find(target) != mapAction_.end()) {
            mapAction_[target](target, data.GetCode());
        } else {
            HILOG_INFO("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

void FwkAbilityStateMain::Action(std::string action, int code)
{
    HILOG_INFO("FwkAbilityStateMain::Action Called");
    if (mapAction_.find(action) != mapAction_.end()) {
        mapAction_[action](code);
    }
}

void FwkAbilityStateMain::DoCrash(std::string action, int code)
{
    HILOG_INFO("FwkAbilityStateMain::DoCrash Called");
    CrashMaker *pcCrashMaker = nullptr;
    int a = pcCrashMaker->CrashTry();
    HILOG_INFO("FwkAbilityStateMain::DoCrash Process %{public}d", a);
}

void FwkAbilityStateMain::StartNextAbility(int code)
{
    std::string targetBundle = "com.ohos.amsst.fwkAbilityState";
    std::string targetAbility = "FwkAbilityStateSecond";
    Want want;
    want.SetElementName(targetBundle, targetAbility);
    StartAbility(want);
}

void FwkAbilityStateMain::BlockAndStart(std::string action, int code)
{
    bIsBlockSave = true;
    StartNextAbility(code);
}

REGISTER_AA(FwkAbilityStateMain);
}  // namespace AppExecFwk
}  // namespace OHOS