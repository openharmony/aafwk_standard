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

#include "ams_st_service_ability_b2.h"
#include "hilog_wrapper.h"
using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {
int AmsStServiceAbilityB2::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
std::map<std::string, AmsStServiceAbilityB2::func> AmsStServiceAbilityB2::funcMap_ = {
    {"StartOtherAbility", &AmsStServiceAbilityB2::StartOtherAbility},
    {"ConnectOtherAbility", &AmsStServiceAbilityB2::ConnectOtherAbility},
    {"StopSelfAbility", &AmsStServiceAbilityB2::StopSelfAbility},
    {"DisConnectOtherAbility", &AmsStServiceAbilityB2::DisConnectOtherAbility},
};

AmsStServiceAbilityB2::~AmsStServiceAbilityB2()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

std::vector<std::string> AmsStServiceAbilityB2::Split(std::string str, const std::string &token)
{
    HILOG_INFO("AmsStServiceAbilityB2::Split");

    std::vector<std::string> splitString;
    while (str.size()) {
        size_t index = str.find(token);
        if (index != std::string::npos) {
            splitString.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0) {
                splitString.push_back(str);
            }
        } else {
            splitString.push_back(str);
            str = "";
        }
    }
    return splitString;
}
void AmsStServiceAbilityB2::StartOtherAbility()
{
    HILOG_INFO("AmsStServiceAbilityB2::StartOtherAbility begin targetBundle=%{public}s, targetAbility=%{public}s",
        targetBundle_.c_str(),
        targetAbility_.c_str());
    HILOG_INFO("AmsStServiceAbilityB2::StartOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    if (!targetBundle_.empty() && !targetAbility_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundle_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbility_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("nextTargetBundleConn", nextTargetBundleConn_);
            want.SetParam("nextTargetAbilityConn", nextTargetAbilityConn_);
            StartAbility(want);
            sleep(1);
        }
    }
}
void AmsStServiceAbilityB2::ConnectOtherAbility()
{
    HILOG_INFO(
        "AmsStServiceAbilityB2::ConnectOtherAbility begin targetBundleConn=%{public}s, targetAbilityConn=%{public}s",
        targetBundleConn_.c_str(),
        targetAbilityConn_.c_str());
    HILOG_INFO("AmsStServiceAbilityB2::ConnectOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    // connect service ability
    if (!targetBundleConn_.empty() && !targetAbilityConn_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundleConn_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbilityConn_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("nextTargetBundleConn", nextTargetBundleConn_);
            want.SetParam("nextTargetAbilityConn", nextTargetAbilityConn_);
            stub_.push_back(new (std::nothrow) AbilityConnectCallback());
            connCallback_.push_back(new (std::nothrow) AbilityConnectionProxy(stub_[i]));
            HILOG_INFO("AmsStAbilitySvcD1::ConnectOtherAbility->ConnectAbility");
            bool ret = ConnectAbility(want, connCallback_[i]);
            sleep(1);
            if (!ret) {
                HILOG_ERROR("AmsStServiceAbilityB2::ConnectAbility failed!");
            }
        }
    }
    HILOG_INFO("AmsStServiceAbilityB2::ConnectOtherAbility end");
}
void AmsStServiceAbilityB2::StopSelfAbility()
{
    HILOG_INFO("AmsStServiceAbilityB2::StopSelfAbility");

    TerminateAbility();
}

void AmsStServiceAbilityB2::DisConnectOtherAbility()
{
    HILOG_INFO("AmsStServiceAbilityB2::DisConnectOtherAbility begin");
    for (auto callBack : connCallback_) {
        DisconnectAbility(callBack);
        sleep(1);
    }
    HILOG_INFO("AmsStServiceAbilityB2::DisConnectOtherAbility end");
}

void AmsStServiceAbilityB2::OnStart(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityB2::OnStart");

    GetWantInfo(want);
    Ability::OnStart(want);
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnStart");
    SubscribeEvent();
}
void AmsStServiceAbilityB2::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityB2::OnNewWant");

    GetWantInfo(want);
    Ability::OnNewWant(want);
}
void AmsStServiceAbilityB2::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HILOG_INFO("AmsStServiceAbilityB2::OnCommand");

    GetWantInfo(want);
    Ability::OnCommand(want, restart, startId);
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnCommand");
}
void AmsStServiceAbilityB2::OnStop()
{
    HILOG_INFO("AmsStServiceAbilityB2::OnStop");

    Ability::OnStop();
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INITIAL, "OnStop");
    Clear();
}
void AmsStServiceAbilityB2::OnActive()
{
    HILOG_INFO("AmsStServiceAbilityB2::OnActive");

    Ability::OnActive();
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnActive");
}
void AmsStServiceAbilityB2::OnInactive()
{
    HILOG_INFO("AmsStServiceAbilityB2::OnInactive");

    Ability::OnInactive();
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnInactive");
}
void AmsStServiceAbilityB2::OnBackground()
{
    HILOG_INFO("AmsStServiceAbilityB2::OnBackground");

    Ability::OnBackground();
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnBackground");
}

sptr<IRemoteObject> AmsStServiceAbilityB2::OnConnect(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityB2::OnConnect");

    sptr<IRemoteObject> remote = Ability::OnConnect(want);
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnConnect");
    return remote;
}
void AmsStServiceAbilityB2::OnDisconnect(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityB2::OnDisconnect");

    Ability::OnDisconnect(want);
    PublishEvent(APP_B2_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnDisconnect");
}

void AmsStServiceAbilityB2::Clear()
{
    shouldReturn_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
    targetBundleConn_ = "";
    targetAbilityConn_ = "";
    nextTargetBundle_ = "";
    nextTargetAbility_ = "";
    nextTargetBundleConn_ = "";
    nextTargetAbilityConn_ = "";
    AmsStServiceAbilityB2::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}
void AmsStServiceAbilityB2::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn_ = mWant.GetStringParam("shouldReturn");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
    targetBundleConn_ = mWant.GetStringParam("targetBundleConn");
    targetAbilityConn_ = mWant.GetStringParam("targetAbilityConn");
    nextTargetBundle_ = mWant.GetStringParam("nextTargetBundle");
    nextTargetAbility_ = mWant.GetStringParam("nextTargetAbility");
    nextTargetBundleConn_ = mWant.GetStringParam("nextTargetBundleConn");
    nextTargetAbilityConn_ = mWant.GetStringParam("nextTargetAbilityConn");
    AmsStServiceAbilityB2::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}
bool AmsStServiceAbilityB2::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    HILOG_INFO("AmsStServiceAbilityB2::PublishEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        code,
        data.c_str());

    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}
bool AmsStServiceAbilityB2::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(APP_B1_REQ_EVENT_NAME);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    auto subscriber = std::make_shared<AppEventSubscriber>(subscribeInfo);
    subscriber->mainAbility_ = this;
    return CommonEventManager::SubscribeCommonEvent(subscriber);
}
void AmsStServiceAbilityB2::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    auto dataContent = data.GetData();
    HILOG_INFO(
        "AmsStServiceAbilityB2::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s is exist",
        eventName.c_str(),
        data.GetCode(),
        dataContent.c_str());
    if (APP_B1_REQ_EVENT_NAME.compare(eventName) == 0) {
        if (funcMap_.find(dataContent) == funcMap_.end()) {
            HILOG_INFO("AmsStServiceAbilityB2::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = "
                     "%{public}s not find",
                eventName.c_str(),
                data.GetCode(),
                dataContent.c_str());
        } else {
            if (mainAbility_ != nullptr) {
                (mainAbility_->*funcMap_[dataContent])();
            }
        }
    }
}
REGISTER_AA(AmsStServiceAbilityB2);
}  // namespace AppExecFwk
}  // namespace OHOS