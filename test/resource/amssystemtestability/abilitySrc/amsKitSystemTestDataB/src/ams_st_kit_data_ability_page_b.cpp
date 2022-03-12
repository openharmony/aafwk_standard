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
#include "ams_st_kit_data_ability_page_b.h"

#include <condition_variable>
#include <cstring>
#include <mutex>

#include "data_ability_helper.h"
#include "hilog_wrapper.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static const int ABILITY_PAGE_CODE = 140;
static const std::string OPERATOR_INSERT = "Insert";
static const std::string OPERATOR_DELETE = "Delete";
static const std::string OPERATOR_UPDATE = "Update";
static const std::string OPERATOR_QUERY = "Query";
static const std::string OPERATOR_GETFILETYPES = "GetFileTypes";
static const std::string OPERATOR_OPENFILE = "OpenFile";
static const std::string OPERATOR_GETTYPE = "GetType";
constexpr int charCnt = 5;
}

bool AmsStKitDataAbilityPageB::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}

void KitTestPageBEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("KitTestPageBEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("KitTestPageBEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("KitTestPageBEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (eventName.compare("event_data_test_action") == 0 && ABILITY_PAGE_CODE == data.GetCode()) {
        auto target = data.GetData();
        auto func = mapTestFunc_.find(target);
        if (func != mapTestFunc_.end()) {
            func->second();
        } else {
            HILOG_INFO("OnReceiveEvent: CommonEventData error(%{public}s)", target.c_str());
        }
    }
}

AmsStKitDataAbilityPageB::~AmsStKitDataAbilityPageB()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsStKitDataAbilityPageB::SubscribeEvent(const Want &want)
{
    Want mwant {want};
    std::vector<std::string> eventList = {
        "event_data_test_action",
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<KitTestPageBEventSubscriber>(subscribeInfo, this);
    subscriber_->vectorOperator_ = mwant.GetStringArrayParam("operator");

    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AmsStKitDataAbilityPageB::OnStart(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityPageB::onStart");
    originWant_ = want;
    SubscribeEvent(want);
    Ability::OnStart(want);
    PublishEvent(abilityEventName, ABILITY_PAGE_CODE, "onStart");
}

void AmsStKitDataAbilityPageB::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityPageB::OnNewWant");
    originWant_ = want;
    Ability::OnNewWant(want);
}

void AmsStKitDataAbilityPageB::OnStop()
{
    HILOG_INFO("AmsStKitDataAbilityPageB::onStop");
    Ability::OnStop();
    PublishEvent(abilityEventName, ABILITY_PAGE_CODE, "OnStop");
}

void AmsStKitDataAbilityPageB::OnActive()
{
    HILOG_INFO("AmsStKitDataAbilityPageB::OnActive");
    Ability::OnActive();
    PublishEvent(abilityEventName, ABILITY_PAGE_CODE, "OnActive");
}

void AmsStKitDataAbilityPageB::OnInactive()
{
    HILOG_INFO("AmsStKitDataAbilityPageB::OnInactive");
    Ability::OnInactive();
    PublishEvent(abilityEventName, ABILITY_PAGE_CODE, "OnInactive");
}

void AmsStKitDataAbilityPageB::OnBackground()
{
    HILOG_INFO("AmsStKitDataAbilityPageB::OnBackground");
    Ability::OnBackground();
    PublishEvent(abilityEventName, ABILITY_PAGE_CODE, "OnBackground");
}

void AmsStKitDataAbilityPageB::GetWantInfo(const Want &want)
{
    Want mWant(want);
    STtools::StOperator allOperator {};
    std::vector<std::string> vectorOperator = mWant.GetStringArrayParam("operator");
    STtools::DeserializationStOperatorFromVector(allOperator, vectorOperator);

    for (auto child : allOperator.GetChildOperator()) {
        HILOG_INFO("-----------------targetBundle:%{public}s", child->GetBundleName().c_str());
        HILOG_INFO("-----------------targetAbility:%{public}s", child->GetAbilityName().c_str());
        HILOG_INFO("-----------------targetAbilityType:%{public}s", child->GetAbilityType().c_str());
        HILOG_INFO("-----------------operatorName:%{public}s", child->GetOperatorName().c_str());
        HILOG_INFO("-----------------childOperatorNum:%{public}zu", child->GetChildOperator().size());
    }
}

static void GetResult(std::shared_ptr<STtools::StOperator> child, std::shared_ptr<DataAbilityHelper> helper,
    AmsStKitDataAbilityPageB &mainAbility, Uri dataAbilityUri, string &result)
{
    NativeRdb::DataAbilityPredicates predicates;
    NativeRdb::ValuesBucket bucket;
    result = "failed";
    if (child->GetOperatorName() == OPERATOR_INSERT) {
        result = std::to_string(helper->Insert(dataAbilityUri, bucket));
    } else if (child->GetOperatorName() == OPERATOR_DELETE) {
        result = std::to_string(helper->Delete(dataAbilityUri, predicates));
    } else if (child->GetOperatorName() == OPERATOR_UPDATE) {
        result = std::to_string(helper->Update(dataAbilityUri, bucket, predicates));
    } else if (child->GetOperatorName() == OPERATOR_QUERY) {
        std::vector<std::string> columns = STtools::SerializationStOperatorToVector(*child);
        std::shared_ptr<NativeRdb::AbsSharedResultSet> resultValue = helper->Query(dataAbilityUri, columns, predicates);
        result = OPERATOR_QUERY;
        if (resultValue != nullptr) {
            resultValue->GoToRow(0);
            resultValue->GetString(0, result);
        }
    } else if (child->GetOperatorName() == OPERATOR_GETFILETYPES) {
        std::vector<std::string> types = helper->GetFileTypes(dataAbilityUri, child->GetMessage());
        result = (types.size() > 0) ? types[0] : "failed";
    } else if (child->GetOperatorName() == OPERATOR_OPENFILE) {
        int fd = helper->OpenFile(dataAbilityUri, child->GetMessage());
        if (fd < 0) {
            return;
        }
        FILE *file = fdopen(fd, "r");
        if (file == nullptr) {
            return;
        }
        result = std::to_string(fd);
        char str[charCnt];
        if (!feof(file)) {
            fgets(str, charCnt, file);
        }
        result = str;
        fclose(file);
    } else if (child->GetOperatorName() == OPERATOR_GETTYPE) {
        result = helper->GetType(dataAbilityUri);
        result = (result != "") ? OPERATOR_GETTYPE : result;
    }
}

void KitTestPageBEventSubscriber::TestPost(const std::string funName)
{
    HILOG_INFO("KitTestPageBEventSubscriber::TestPost %{public}s", funName.c_str());
    STtools::StOperator allOperator {};
    STtools::DeserializationStOperatorFromVector(allOperator, vectorOperator_);
    std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(mainAbility_->GetContext());
    for (auto child : allOperator.GetChildOperator()) {
        /// data ability
        if (child->GetAbilityType() == "2") {
            HILOG_INFO("---------------------targetAbility_--------------------");

            Uri dataAbilityUri("dataability:///" + child->GetBundleName() + "." + child->GetAbilityName());
            std::string result;
            if (helper != nullptr) {
                HILOG_INFO("---------------------helper--------------------");
                GetResult(child, helper, *mainAbility_, dataAbilityUri, result);
            }
            mainAbility_->PublishEvent(abilityEventName, ABILITY_PAGE_CODE, child->GetOperatorName() + " " + result);
        } else if (child->GetAbilityType() == "0") {
            HILOG_INFO("---------------------StartPageAbility--------------------");
            std::vector<std::string> vectoroperator;
            if (child->GetChildOperator().size() != 0) {
                vectoroperator = STtools::SerializationStOperatorToVector(*child);
            }
            std::string targetBundle = child->GetBundleName();
            std::string targetAbility = child->GetAbilityName();
            Want want;
            want.SetElementName(targetBundle, targetAbility);
            want.SetParam("operator", vectoroperator);
            mainAbility_->StartAbility(want);
            mainAbility_->PublishEvent(abilityEventName, ABILITY_PAGE_CODE, child->GetOperatorName());
        }
    }
}
REGISTER_AA(AmsStKitDataAbilityPageB);
}  // namespace AppExecFwk
}  // namespace OHOS