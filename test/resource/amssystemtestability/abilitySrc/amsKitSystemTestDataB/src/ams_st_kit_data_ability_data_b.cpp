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
#include "ams_st_kit_data_ability_data_b.h"

#include <condition_variable>
#include <mutex>
#include <cstdio>

#include "hilog_wrapper.h"
#include "data_ability_helper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static const int ABILITY_DATA_CODE = 270;
static const int LIFECYCLE_CALLBACKS = 271;
static const int LIFECYCLE_OBSERVER = 272;
static const std::string OPERATOR_INSERT = "Insert";
static const std::string OPERATOR_DELETE = "Delete";
static const std::string OPERATOR_UPDATE = "Update";
static const std::string OPERATOR_QUERY = "Query";
static const std::string OPERATOR_GETFILETYPES = "GetFileTypes";
static const std::string OPERATOR_OPENFILE = "OpenFile";
static const std::string OPERATOR_GETTYPE = "GetType";
static const int DEFAULT_INSERT_RESULT = 1111;
static const int DEFAULT_DELETE_RESULT = 2222;
static const int DEFAULT_UPDATE_RESULT = 3333;
static const std::string ABILITY_TYPE_PAGE = "0";
static const std::string ABILITY_TYPE_DATA = "2";
constexpr int charCnt = 5;
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilityStart");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnStart");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilityInactive");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnInactive");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilityBackground");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnBackground");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataA1LifecycleCallbacks  OnAbilityForeground");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnForeground");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilityActive");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnActive");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilityStop");
    std::string abilityName = ability->GetAbilityName();
    if (abilityName == mainAbility_->GetAbilityName()) {
        mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnStop");
    }
}

void AmsStKitDataAbilityDataBLifecycleCallbacks::OnAbilitySaveState(const PacMap &outState)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleCallbacks  OnAbilitySaveState");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_CALLBACKS, "OnSaveState");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnActive()
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnActive");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnActive");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnBackground()
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnBackground");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnBackground");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnForeground(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnForeground");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnForeground");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnInactive()
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnInactive");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnInactive");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnStart(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnStart");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnStart");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnStop()
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnStop");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnStop");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnStateChanged(LifeCycle::Event event, const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnStateChanged");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnStateChanged");
}

void AmsStKitDataAbilityDataBLifecycleObserver::OnStateChanged(LifeCycle::Event event)
{
    HILOG_INFO("AmsStKitDataAbilityDataBLifecycleObserver  OnStateChanged");
    mainAbility_->PublishEvent(abilityEventName, LIFECYCLE_OBSERVER, "OnStateChanged");
}

bool AmsStKitDataAbilityDataB::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}

void KitTestDataBEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    HILOG_INFO("KitTestDataBEventSubscriber::OnReceiveEvent:event=%{public}s", data.GetWant().GetAction().c_str());
    HILOG_INFO("KitTestDataBEventSubscriber::OnReceiveEvent:data=%{public}s", data.GetData().c_str());
    HILOG_INFO("KitTestDataBEventSubscriber::OnReceiveEvent:code=%{public}d", data.GetCode());
    auto eventName = data.GetWant().GetAction();
    if (eventName.compare(testEventName) == 0 && ABILITY_DATA_CODE == data.GetCode()) {
        std::string target = data.GetData();
        STtools::Completed(mainAbility_->event, target, ABILITY_DATA_CODE);
    }
}

void AmsStKitDataAbilityDataB::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("AmsStKitDataAbilityDataB::Init called.");
    Ability::Init(abilityInfo, application, handler, token);
    auto callback = std::make_shared<AmsStKitDataAbilityDataBLifecycleCallbacks>();
    callback->mainAbility_ = this;
    Ability::GetApplication()->RegisterAbilityLifecycleCallbacks(callback);
    auto observer = std::make_shared<AmsStKitDataAbilityDataBLifecycleObserver>();
    observer->mainAbility_ = this;
    Ability::GetLifecycle()->AddObserver(observer);
}

AmsStKitDataAbilityDataB::~AmsStKitDataAbilityDataB()
{
    if (fd1 != nullptr) {
        fclose(fd1);
        fd1 = nullptr;
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

void AmsStKitDataAbilityDataB::SubscribeEvent(const Want &want)
{
    std::vector<std::string> eventList = {
        testEventName,
    };
    MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<KitTestDataBEventSubscriber>(subscribeInfo, this);

    CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AmsStKitDataAbilityDataB::OnStart(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnStart");
    SubscribeEvent(want);
    originWant_ = want;
    Ability::OnStart(want);
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnStart");
}

void AmsStKitDataAbilityDataB::OnStop()
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnStop");
    Ability::OnStop();
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnStop");
}

void AmsStKitDataAbilityDataB::OnActive()
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnActive");
    Ability::OnActive();
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnActive");
}

void AmsStKitDataAbilityDataB::OnInactive()
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnInactive");
    Ability::OnInactive();
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnInactive");
}

void AmsStKitDataAbilityDataB::OnForeground(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnForeground");
    Ability::OnForeground(want);
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnForeground");
}

void AmsStKitDataAbilityDataB::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStKitDataAbilityDataB::OnNewWant");
    originWant_ = want;
    Ability::OnNewWant(want);
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnNewWant");
}

void AmsStKitDataAbilityDataB::OnBackground()
{
    HILOG_INFO("AmsStKitDataAbilityDataB OnBackground");
    Ability::OnBackground();
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OnBackground");
}

int AmsStKitDataAbilityDataB::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<Insert>>>>");
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "Insert");
    return DEFAULT_INSERT_RESULT;
}

int AmsStKitDataAbilityDataB::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<Delete>>>>");
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "Delete");
    return DEFAULT_DELETE_RESULT;
}

int AmsStKitDataAbilityDataB::Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<Update>>>>");
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "Update");
    return DEFAULT_UPDATE_RESULT;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AmsStKitDataAbilityDataB::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    subscriber_->vectorOperator_ = columns;
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<Query>>>>");
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, OPERATOR_QUERY);

    STtools::WaitCompleted(event, OPERATOR_QUERY, ABILITY_DATA_CODE);
    subscriber_->TestPost();

    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultValue = std::make_shared<NativeRdb::AbsSharedResultSet>(OPERATOR_QUERY);
    AppDataFwk::SharedBlock *pSharedBlock = resultValue->GetBlock();
    if (pSharedBlock) {
        pSharedBlock->PutString(0, 0, OPERATOR_QUERY.c_str(), OPERATOR_QUERY.size() + 1);
    }
    return resultValue;
}

std::vector<std::string> AmsStKitDataAbilityDataB::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<GetFileTypes>>>>");
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "GetFileTypes");
    std::vector<std::string> fileType {"filetypes"};
    return fileType;
}

int AmsStKitDataAbilityDataB::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("AmsStKitDataAbilityDataB <<<<OpenFile>>>>");
    if (fd1 != nullptr) {
        fclose(fd1);
        fd1 = nullptr;
    }
    fd1 = fopen("/system/vendor/test.txt", "r");
    if (fd1 == nullptr)
        return -1;
    int fd = fileno(fd1);
    HILOG_INFO("AmsStKitDataAbilityDataB fd: %{public}d", fd);
    PublishEvent(abilityEventName, ABILITY_DATA_CODE, "OpenFile");

    return fd;
}

static void GetResult(std::shared_ptr<STtools::StOperator> child, std::shared_ptr<DataAbilityHelper> helper,
    AmsStKitDataAbilityDataB &mainAbility, Uri dataAbilityUri, string &result)
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

void KitTestDataBEventSubscriber::TestPost(const std::string funName)
{
    HILOG_INFO("KitTestDataBEventSubscriber::TestPost %{public}s", funName.c_str());
    STtools::StOperator allOperator {};
    STtools::DeserializationStOperatorFromVector(allOperator, vectorOperator_);
    std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(mainAbility_->GetContext());
    for (auto child : allOperator.GetChildOperator()) {
        /// data ability
        if (child->GetAbilityType() == ABILITY_TYPE_DATA) {
            HILOG_INFO("---------------------targetAbility_--------------------");
            Uri dataAbilityUri("dataability:///" + child->GetBundleName() + "." + child->GetAbilityName());
            std::string result;
            if (helper != nullptr) {
                HILOG_INFO("---------------------helper--------------------");
                GetResult(child, helper, *mainAbility_, dataAbilityUri, result);
            }
            mainAbility_->PublishEvent(abilityEventName, ABILITY_DATA_CODE, child->GetOperatorName() + " " + result);
        } else if (child->GetAbilityType() == ABILITY_TYPE_PAGE) {
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
            mainAbility_->PublishEvent(abilityEventName, ABILITY_DATA_CODE, child->GetOperatorName());
        }
    }
}
REGISTER_AA(AmsStKitDataAbilityDataB);
}  // namespace AppExecFwk
}  // namespace OHOS