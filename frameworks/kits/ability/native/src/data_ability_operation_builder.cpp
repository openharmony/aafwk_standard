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

#include "data_ability_operation_builder.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
DataAbilityOperationBuilder::DataAbilityOperationBuilder(const int type, const std::shared_ptr<Uri> &uri)
{
    type_ = type;
    uri_ = uri;
    expectedCount_ = 0;
    valuesBucket_ = nullptr;
    dataAbilityPredicates_ = nullptr;
    valuesBucketReferences_ = nullptr;
    dataAbilityPredicatesBackReferences_.clear();
}
DataAbilityOperationBuilder::~DataAbilityOperationBuilder()
{
    dataAbilityPredicatesBackReferences_.clear();
}

std::shared_ptr<DataAbilityOperation> DataAbilityOperationBuilder::Build()
{
    APP_LOGD("DataAbilityOperationBuilder::Build start");
    if (type_ != DataAbilityOperation::TYPE_UPDATE || (valuesBucket_ != nullptr && !valuesBucket_->IsEmpty())) {
        std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>(shared_from_this());
        APP_LOGD("DataAbilityOperationBuilder::Build end");
        return operation;
    }
    APP_LOGE("DataAbilityOperationBuilder::Build return nullptr");
    return nullptr;
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValuesBucket(
    std::shared_ptr<ValuesBucket> &values)
{
    APP_LOGD("DataAbilityOperationBuilder::WithValuesBucket start");
    // APP_LOGI("DataAbilityOperationBuilder::WithValuesBucket valuesBucket:%{public}s", values->ToString());
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        APP_LOGE("DataAbilityOperationBuilder::WithValuesBucket only inserts, updates can have values, type=%{public}d",
            type_);
        return nullptr;
    }
    if (valuesBucket_ == nullptr) {
        valuesBucket_ = std::make_shared<ValuesBucket>();
    }

    valuesBucket_->PutValues(values);
    APP_LOGD("DataAbilityOperationBuilder::WithValuesBucket end");
    return shared_from_this();
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicates(
    std::shared_ptr<DataAbilityPredicates> &predicates)
{
    APP_LOGD("DataAbilityOperationBuilder::WithPredicates start");
    // APP_LOGI("DataAbilityOperationBuilder::WithPredicates order:%{public}s, group:%{public}s, index:%{public}s, "
    //          "whereClause:%{public}s, limit:%{public}d, offset:%{public}d, distinct:%{public}d",
    //     predicates->GetOrder(),
    //     predicates->GetGroup(),
    //     predicates->GetIndex(),
    //     predicates->GetWhereClause(),
    //     predicates->GetLimit(),
    //     predicates->GetOffset(),
    //     predicates->isDistinct());
    if (type_ != DataAbilityOperation::TYPE_DELETE && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        APP_LOGE(
            "DataAbilityOperationBuilder::withPredicates only deletes and updates can have selections, type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicates_ = predicates;
    APP_LOGD("DataAbilityOperationBuilder::WithPredicates end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithExpectedCount(int count)
{
    APP_LOGD("DataAbilityOperationBuilder::WithExpectedCount start");
    APP_LOGI("DataAbilityOperationBuilder::WithExpectedCount expectedCount:%{public}d", count);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        APP_LOGE("DataAbilityOperationBuilder::withExpectedCount only updates, deletes can have expected counts, "
                 "type=%{public}d",
            type_);
        return nullptr;
    }
    expectedCount_ = count;
    APP_LOGD("DataAbilityOperationBuilder::WithExpectedCount end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicatesBackReference(
    int requestArgIndex, int previousResult)
{
    APP_LOGD("DataAbilityOperationBuilder::WithPredicatesBackReference start");
    APP_LOGI("DataAbilityOperationBuilder::WithPredicatesBackReference requestArgIndex:%{public}d, "
             "previousResult:%{public}d",
        requestArgIndex,
        previousResult);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        APP_LOGE("DataAbilityOperationBuilder::withPredicatesBackReference only updates, deletes, and asserts can have "
                 "select back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicatesBackReferences_.insert(std::make_pair(requestArgIndex, previousResult));
    APP_LOGD("DataAbilityOperationBuilder::WithPredicatesBackReference end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValueBackReferences(
    std::shared_ptr<ValuesBucket> &backReferences)
{
    APP_LOGD("DataAbilityOperationBuilder::WithValueBackReferences start");
    // APP_LOGI("DataAbilityOperationBuilder::WithValueBackReferences backReferences:%{public}s",
    // backReferences->ToString());
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        APP_LOGE("DataAbilityOperationBuilder::withValueBackReferences only inserts, updates, and asserts can have "
                 "value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    valuesBucketReferences_ = backReferences;
    APP_LOGD("DataAbilityOperationBuilder::WithValueBackReferences end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithInterruptionAllowed(bool interrupted)
{
    APP_LOGD("DataAbilityOperationBuilder::WithInterruptionAllowed start");
    APP_LOGI("DataAbilityOperationBuilder::WithInterruptionAllowed  interrupted=%{public}d", interrupted);
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT && type_ != DataAbilityOperation::TYPE_DELETE) {
        APP_LOGE("DataAbilityOperationBuilder::withInterruptionAllowed only inserts, updates, delete, and asserts can "
                 "have value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    interrupted_ = interrupted;
    APP_LOGD("DataAbilityOperationBuilder::WithInterruptionAllowed end");
    return shared_from_this();
}
}  // namespace AppExecFwk
}  // namespace OHOS