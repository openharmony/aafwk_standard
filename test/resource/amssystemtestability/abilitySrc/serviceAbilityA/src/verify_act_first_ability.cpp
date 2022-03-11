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

#include "verify_act_first_ability.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
void AbilityConnectionActFirst::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("AbilityConnectionActFirst OnAbilityConnectDone resultCode: %{public}d", resultCode);
    HILOG_INFO("AbilityConnectionActFirst OnAbilityConnectDone %{public}s", element.GetAbilityName().c_str());
    HILOG_INFO("AbilityConnectionActFirst OnAbilityConnectDone %{public}p", remoteObject.GetRefPtr());
}

void AbilityConnectionActFirst::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("AbilityConnectionActFirst OnAbilityDisconnectDone resultCode: %{public}d", resultCode);
    HILOG_INFO("AbilityConnectionActFirst OnAbilityDisconnectDone : %{public}s", element.GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityStart called. AbilityName is %{public}s",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityInactive called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityBackground called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO(
        "----------VerifyIOAbilityLifecycleCallbacks::OnAbilityForeground called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityActive called. AbilityName is %{public}s",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("----------VerifyIOAbilityLifecycleCallbacks::OnAbilityStop called. AbilityName is %{public}s----------",
        ability->GetAbilityName().c_str());
}

void VerifyIOAbilityLifecycleCallbacks::OnAbilitySaveState(const PacMap &outState)
{
    HILOG_INFO("----------VerifyIOAbilityLifecycleCallbacks::OnAbilitySaveState called.----------");
}

void VerifyActFirstAbility::OnStart(const Want &want)
{
    HILOG_INFO("----------VerifyActFirstAbility::onStart begin----------");
    std::shared_ptr<VerifyIOAbilityLifecycleCallbacks> callback = std::make_shared<VerifyIOAbilityLifecycleCallbacks>();
    Ability::GetApplication()->RegisterAbilityLifecycleCallbacks(callback);

    EventFwk::MatchingSkills MatchingSkillsFirst;
    MatchingSkillsFirst.AddEvent("Test_AbilityContextStartAbility");
    EventFwk::CommonEventSubscribeInfo subscriberInfoFirst(MatchingSkillsFirst);
    std::shared_ptr<AbilityContextStartAbilityTest> subscriberFirst =
        std::make_shared<AbilityContextStartAbilityTest>(subscriberInfoFirst);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberFirst);

    EventFwk::MatchingSkills MatchingSkillsService;
    MatchingSkillsService.AddEvent("Test_ServiceAbility");
    EventFwk::CommonEventSubscribeInfo subscriberInfoService(MatchingSkillsService);
    sptr<OHOS::AAFwk::IAbilityConnection> conne = new AbilityConnectionActFirst();
    std::shared_ptr<ConnectServiceAbilityTest> serviceSubscriber =
        std::make_shared<ConnectServiceAbilityTest>(subscriberInfoService, Ability::GetContext(), conne);
    EventFwk::CommonEventManager::SubscribeCommonEvent(serviceSubscriber);
    HILOG_INFO("----------VerifyActFirstAbility::onStart end----------");

    Ability::OnStart(want);
}

void VerifyActFirstAbility::OnStop()
{
    HILOG_INFO("----------VerifyActFirstAbility::onStop begin----------");
    HILOG_INFO("----------VerifyActFirstAbility::onStop end----------");
    Ability::OnStop();
}

void VerifyActFirstAbility::OnActive()
{
    HILOG_INFO("----------VerifyActFirstAbility::OnActive begin----------");
    HILOG_INFO("----------VerifyActFirstAbility::OnActive end----------");
    Ability::OnActive();
}

void VerifyActFirstAbility::OnInactive()
{
    HILOG_INFO("----------VerifyActFirstAbility::OnInactive begin----------");
    HILOG_INFO("----------VerifyActFirstAbility::OnInactive end----------");
    Ability::OnInactive();
}

void VerifyActFirstAbility::OnBackground()
{
    HILOG_INFO("----------VerifyActFirstAbility::OnBackground begin----------");
    HILOG_INFO("----------VerifyActFirstAbility::OnBackground end----------");
    Ability::OnBackground();
}

void AbilityContextStartAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    HILOG_INFO("---------- VerifyActFirstAbility AbilityContextStartAbilityTest begin----------");
    HILOG_INFO("---------- VerifyActFirstAbility AbilityContextStartAbilityTest start ability----------");
    auto abilityContext = std::make_shared<AbilityContext>();
    int requestCode = -1;
    Want want;
    std::string targetBundle = "com.ix.ServiceaAbility";
    std::string targetAbility = "MainServiceAbilityDemo";
    want.SetElementName(targetBundle, targetAbility);
    abilityContext->StartAbility(want, requestCode);
    HILOG_INFO("---------- VerifyActFirstAbility AbilityContextStartAbilityTest start ability end----------");
    HILOG_INFO("---------- VerifyActFirstAbility AbilityContextStartAbilityTest end----------");
}

void ConnectServiceAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    if (abilityContext_ == nullptr) {
        HILOG_INFO("----------ConnectServiceAbilityTest::OnReceiveEvent abilityContext_ == nullptr----------");
        return;
    }

    Want want_service;
    std::string targetBundle = "com.ix.ServiceaAbility";
    std::string targetAbility = "MainServiceAbilityDemo";
    want_service.SetElementName(targetBundle, targetAbility);
    switch (data.GetCode()) {
        case CONNECT_SERVICE_ABILITY: {
            auto ret = abilityContext_->ConnectAbility(want_service, conne_);
            HILOG_INFO("connect ability return %{public}d", ret);
            break;
        }
        case DISCONNECT_SERVICE_ABILITY: {
            abilityContext_->DisconnectAbility(conne_);
            break;
        }
        case START_SERVICE_ABILITY: {
            abilityContext_->StartAbility(want_service, 0);
            break;
        }
        case STOP_SERVICE_ABILITY: {
            auto ret = abilityContext_->StopAbility(want_service);
            HILOG_INFO("stop ability return %{public}d", ret);
            break;
        }
        default:
            HILOG_INFO("---------- ConnectServiceAbilityTest OnReceiveEvent default----------");
            break;
    }
}
REGISTER_AA(VerifyActFirstAbility)
}  // namespace AppExecFwk
}  // namespace OHOS