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
#include "data_ability_predicates.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
DataAbilityOperationBuilder::DataAbilityOperationBuilder(const int type, const std::shared_ptr<Uri> &uri)
{
    type_ = type;
    uri_ = uri;
    expectedCount_ = 0;
    interrupted_ = false;
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
    HILOG_DEBUG("DataAbilityOperationBuilder::Build start");
    if (type_ != DataAbilityOperation::TYPE_UPDATE || (valuesBucket_ != nullptr && !valuesBucket_->IsEmpty())) {
        std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>(shared_from_this());
        HILOG_DEBUG("DataAbilityOperationBuilder::Build end");
        return operation;
    }
    HILOG_ERROR("DataAbilityOperationBuilder::Build return nullptr");
    return nullptr;
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValuesBucket(
    std::shared_ptr<NativeRdb::ValuesBucket> &values)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithValuesBucket start");
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        HILOG_ERROR(
            "DataAbilityOperationBuilder::WithValuesBucket only inserts, updates can have values, type=%{public}d",
            type_);
        return nullptr;
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    values->GetAll(valuesMap);

    valuesBucket_.reset(new (std::nothrow) NativeRdb::ValuesBucket(valuesMap));
    HILOG_DEBUG("DataAbilityOperationBuilder::WithValuesBucket end");
    return shared_from_this();
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicates(
    std::shared_ptr<NativeRdb::DataAbilityPredicates> &predicates)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithPredicates start");
    if (type_ != DataAbilityOperation::TYPE_DELETE && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        HILOG_ERROR(
            "DataAbilityOperationBuilder::withPredicates only deletes and updates can have selections, type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicates_ = predicates;
    HILOG_DEBUG("DataAbilityOperationBuilder::WithPredicates end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithExpectedCount(int count)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithExpectedCount start");
    HILOG_INFO("DataAbilityOperationBuilder::WithExpectedCount expectedCount:%{public}d", count);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        HILOG_ERROR("DataAbilityOperationBuilder::withExpectedCount only updates, deletes can have expected counts, "
            "type=%{public}d",
            type_);
        return nullptr;
    }
    expectedCount_ = count;
    HILOG_DEBUG("DataAbilityOperationBuilder::WithExpectedCount end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicatesBackReference(
    int requestArgIndex, int previousResult)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithPredicatesBackReference start");
    HILOG_INFO("DataAbilityOperationBuilder::WithPredicatesBackReference requestArgIndex:%{public}d, "
        "previousResult:%{public}d",
        requestArgIndex,
        previousResult);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        HILOG_ERROR(
            "DataAbilityOperationBuilder::withPredicatesBackReference only updates, deletes, and asserts can have "
            "select back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicatesBackReferences_.insert(std::make_pair(requestArgIndex, previousResult));
    HILOG_DEBUG("DataAbilityOperationBuilder::WithPredicatesBackReference end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValueBackReferences(
    std::shared_ptr<NativeRdb::ValuesBucket> &backReferences)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithValueBackReferences start");
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        HILOG_ERROR("DataAbilityOperationBuilder::withValueBackReferences only inserts, updates, and asserts can have "
            "value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    valuesBucketReferences_ = backReferences;
    HILOG_DEBUG("DataAbilityOperationBuilder::WithValueBackReferences end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithInterruptionAllowed(bool interrupted)
{
    HILOG_DEBUG("DataAbilityOperationBuilder::WithInterruptionAllowed start");
    HILOG_INFO("DataAbilityOperationBuilder::WithInterruptionAllowed  interrupted=%{public}d", interrupted);
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT && type_ != DataAbilityOperation::TYPE_DELETE) {
        HILOG_ERROR(
            "DataAbilityOperationBuilder::withInterruptionAllowed only inserts, updates, delete, and asserts can "
            "have value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    interrupted_ = interrupted;
    HILOG_DEBUG("DataAbilityOperationBuilder::WithInterruptionAllowed end");
    return shared_from_this();
}
}  // namespace AppExecFwk
}  // namespace OHOS
