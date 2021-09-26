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

#include "data_ability_operation.h"
#include "app_log_wrapper.h"
#include "hilog_wrapper.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
DataAbilityOperation::DataAbilityOperation(
    const std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, const std::shared_ptr<Uri> &withUri)
{
    uri_ = withUri;
    if (dataAbilityOperation != nullptr) {
        type_ = dataAbilityOperation->type_;
        valuesBucket_ = dataAbilityOperation->valuesBucket_;
        expectedCount_ = dataAbilityOperation->expectedCount_;
        dataAbilityPredicates_ = dataAbilityOperation->dataAbilityPredicates_;
        valuesBucketReferences_ = dataAbilityOperation->valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = dataAbilityOperation->dataAbilityPredicatesBackReferences_;
        interrupted_ = dataAbilityOperation->interrupted_;
    } else {
        type_ = 0;
        expectedCount_ = 0;
        valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>();
        dataAbilityPredicates_ = std::make_shared<NativeRdb::DataAbilityPredicates>();
        valuesBucketReferences_ = std::make_shared<NativeRdb::ValuesBucket>();
        dataAbilityPredicatesBackReferences_.clear();
        interrupted_ = false;
    }
}
DataAbilityOperation::DataAbilityOperation(Parcel &in)
{
    ReadFromParcel(in);
}
DataAbilityOperation::DataAbilityOperation(const std::shared_ptr<DataAbilityOperationBuilder> &builder)
{
    if (builder != nullptr) {
        type_ = builder->type_;
        uri_ = builder->uri_;
        valuesBucket_ = builder->valuesBucket_;
        expectedCount_ = builder->expectedCount_;
        dataAbilityPredicates_ = builder->dataAbilityPredicates_;
        valuesBucketReferences_ = builder->valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = builder->dataAbilityPredicatesBackReferences_;
        interrupted_ = builder->interrupted_;
    }
}

DataAbilityOperation::DataAbilityOperation()
{
    type_ = 0;
    uri_ = nullptr;
    expectedCount_ = 0;
    valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>();
    dataAbilityPredicates_ = std::make_shared<NativeRdb::DataAbilityPredicates>();
    valuesBucketReferences_ = std::make_shared<NativeRdb::ValuesBucket>();
    dataAbilityPredicatesBackReferences_.clear();
    interrupted_ = false;
}

DataAbilityOperation::~DataAbilityOperation()
{
    dataAbilityPredicatesBackReferences_.clear();
}

bool DataAbilityOperation::operator==(const DataAbilityOperation &other) const
{
    if (type_ != other.type_) {
        return false;
    }
    if ((uri_ != nullptr) && (other.uri_ != nullptr) && (uri_->ToString() != other.uri_->ToString())) {
        return false;
    }
    if (expectedCount_ != other.expectedCount_) {
        return false;
    }
    if (valuesBucket_ != other.valuesBucket_) {
        return false;
    }
    if (dataAbilityPredicates_ != other.dataAbilityPredicates_) {
        return false;
    }
    if (valuesBucketReferences_ != other.valuesBucketReferences_) {
        return false;
    }
    int backReferencesCount = dataAbilityPredicatesBackReferences_.size();
    int otherBackReferencesCount = other.dataAbilityPredicatesBackReferences_.size();
    if (backReferencesCount != otherBackReferencesCount) {
        return false;
    }

    std::map<int, int>::const_iterator it = dataAbilityPredicatesBackReferences_.begin();
    while (it != dataAbilityPredicatesBackReferences_.end()) {
        std::map<int, int>::const_iterator otherIt = other.dataAbilityPredicatesBackReferences_.find(it->first);
        if (otherIt != other.dataAbilityPredicatesBackReferences_.end()) {
            if (otherIt->second != it->second) {
                return false;
            }
        } else {
            return false;
        }
        it++;
    }

    if (interrupted_ != other.interrupted_) {
        return false;
    }
    return true;
}

DataAbilityOperation &DataAbilityOperation::operator=(const DataAbilityOperation &other)
{
    if (this != &other) {
        type_ = other.type_;
        uri_ = other.uri_;
        expectedCount_ = other.expectedCount_;
        valuesBucket_ = other.valuesBucket_;
        dataAbilityPredicates_ = other.dataAbilityPredicates_;
        valuesBucketReferences_ = other.valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = other.dataAbilityPredicatesBackReferences_;
        interrupted_ = other.interrupted_;
    }
    return *this;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewInsertBuilder(const std::shared_ptr<Uri> &uri)
{
    APP_LOGD("DataAbilityOperation::NewInsertBuilder start");
    if (uri == nullptr) {
        APP_LOGE("DataAbilityOperation::NewInsertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_INSERT, uri);
    APP_LOGD("DataAbilityOperation::NewInsertBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewUpdateBuilder(const std::shared_ptr<Uri> &uri)
{
    APP_LOGD("DataAbilityOperation::NewUpdateBuilder start");
    if (uri == nullptr) {
        APP_LOGE("DataAbilityOperation::NewUpdateBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_UPDATE, uri);
    APP_LOGD("DataAbilityOperation::NewUpdateBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewDeleteBuilder(const std::shared_ptr<Uri> &uri)
{
    APP_LOGD("DataAbilityOperation::NewDeleteBuilder start");
    if (uri == nullptr) {
        APP_LOGE("DataAbilityOperation::NewDeleteBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_DELETE, uri);
    APP_LOGD("DataAbilityOperation::NewDeleteBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewAssertBuilder(const std::shared_ptr<Uri> &uri)
{
    APP_LOGD("DataAbilityOperation::NewAssertBuilder start");
    if (uri == nullptr) {
        APP_LOGE("DataAbilityOperation::NewAssertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_ASSERT, uri);
    APP_LOGD("DataAbilityOperation::NewAssertBuilder end");
    return builder;
}

int DataAbilityOperation::GetType() const
{
    APP_LOGD("DataAbilityOperation::GetType");
    return type_;
}

std::shared_ptr<Uri> DataAbilityOperation::GetUri() const
{
    APP_LOGD("DataAbilityOperation::GetUri");
    return uri_;
}

std::shared_ptr<NativeRdb::ValuesBucket> DataAbilityOperation::GetValuesBucket() const
{
    APP_LOGD("DataAbilityOperation::GetValuesBucket");
    return valuesBucket_;
}

int DataAbilityOperation::GetExpectedCount() const
{
    APP_LOGD("DataAbilityOperation::GetExpectedCount");
    return expectedCount_;
}

std::shared_ptr<NativeRdb::DataAbilityPredicates> DataAbilityOperation::GetDataAbilityPredicates() const
{
    APP_LOGD("DataAbilityOperation::GetDataAbilityPredicates");
    return dataAbilityPredicates_;
}

std::shared_ptr<NativeRdb::ValuesBucket> DataAbilityOperation::GetValuesBucketReferences() const
{
    APP_LOGD("DataAbilityOperation::GetValuesBucketReferences");
    return valuesBucketReferences_;
}
std::map<int, int> DataAbilityOperation::GetDataAbilityPredicatesBackReferences() const
{
    APP_LOGD("DataAbilityOperation::GetDataAbilityPredicatesBackReferences");
    return dataAbilityPredicatesBackReferences_;
}
bool DataAbilityOperation::IsInsertOperation() const
{
    APP_LOGD("DataAbilityOperation::IsInsertOperation：%d", type_ == TYPE_INSERT);
    return type_ == TYPE_INSERT;
}
bool DataAbilityOperation::IsUpdateOperation() const
{
    APP_LOGD("DataAbilityOperation::IsUpdateOperation：%d", type_ == TYPE_UPDATE);
    return type_ == TYPE_UPDATE;
}
bool DataAbilityOperation::IsDeleteOperation() const
{
    APP_LOGD("DataAbilityOperation::IsDeleteOperation：%d", type_ == TYPE_DELETE);
    return type_ == TYPE_DELETE;
}
bool DataAbilityOperation::IsAssertOperation() const
{
    APP_LOGD("DataAbilityOperation::IsAssertOperation：%d", type_ == TYPE_ASSERT);
    return type_ == TYPE_ASSERT;
}
bool DataAbilityOperation::IsInterruptionAllowed() const
{
    APP_LOGD("DataAbilityOperation::IsInterruptionAllowed：%d", interrupted_);
    return interrupted_;
}
bool DataAbilityOperation::Marshalling(Parcel &out) const
{
    APP_LOGD("DataAbilityOperation::Marshalling start");
    if (!out.WriteInt32(type_)) {
        APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(type_) error");
        return false;
    }
    if (!out.WriteInt32(expectedCount_)) {
        APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
        return false;
    }
    
    if (!out.WriteBool(interrupted_)) {
        APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
        return false;
    }

    if (uri_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }

        if (!out.WriteParcelable(uri_.get())) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    if (valuesBucket_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }

        if (!out.WriteParcelable(valuesBucket_.get())) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    }

    if (dataAbilityPredicates_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (!out.WriteParcelable(dataAbilityPredicates_.get())) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    if (valuesBucketReferences_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (!out.WriteParcelable(valuesBucketReferences_.get())) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    int referenceSize = 0;
    if (!dataAbilityPredicatesBackReferences_.empty()) {
        referenceSize = dataAbilityPredicatesBackReferences_.size();
        if (!out.WriteInt32(referenceSize)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (referenceSize >= REFERENCE_THRESHOLD) {
            APP_LOGI("DataAbilityOperation::Marshalling referenceSize >= REFERENCE_THRESHOLD");
            return true;
        }
        for (auto it = dataAbilityPredicatesBackReferences_.begin(); it != dataAbilityPredicatesBackReferences_.end();
             it++) {

            if (!out.WriteInt32(it->first)) {
                APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
                return false;
            }
            if (!out.WriteInt32(it->second)) {
                APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
                return false;
            }
        }
    } else {
        APP_LOGD("DataAbilityOperation::Marshalling dataAbilityPredicatesBackReferences_ is empty");
        if (!out.WriteInt32(referenceSize)) {
            APP_LOGE("DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    }

    APP_LOGD("DataAbilityOperation::Marshalling end");
    return true;
}
DataAbilityOperation *DataAbilityOperation::Unmarshalling(Parcel &in)
{
    APP_LOGD("DataAbilityOperation::Unmarshalling start");
    DataAbilityOperation *dataAbilityOperation = new (std::nothrow) DataAbilityOperation();
    if (dataAbilityOperation != nullptr && !dataAbilityOperation->ReadFromParcel(in)) {
        APP_LOGE("DataAbilityOperation::Unmarshalling dataAbilityOperation(%p) error", dataAbilityOperation);
        delete dataAbilityOperation;
        dataAbilityOperation = nullptr;
    }
    APP_LOGD("DataAbilityOperation::Unmarshalling end");
    return dataAbilityOperation;
}
bool DataAbilityOperation::ReadFromParcel(Parcel &in)
{
    APP_LOGD("DataAbilityOperation::ReadFromParcel start");
    if (!in.ReadInt32(type_)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(type_) error");
        return false;
    }
    if (!in.ReadInt32(expectedCount_)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    interrupted_ = in.ReadBool();

    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        uri_.reset(in.ReadParcelable<Uri>());
    } else {
        uri_.reset();
    }

    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    APP_LOGD("DataAbilityOperation::ReadFromParcel empty is %{public}s", empty == VALUE_OBJECT ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        valuesBucket_.reset(in.ReadParcelable<NativeRdb::ValuesBucket>());
    } else {
        valuesBucket_.reset();
    }

    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    APP_LOGD("DataAbilityOperation::ReadFromParcel empty is %{public}s", empty == VALUE_OBJECT ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        dataAbilityPredicates_.reset(in.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    } else {
        dataAbilityPredicates_.reset();
    }

    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    APP_LOGD("DataAbilityOperation::ReadFromParcel empty is %{public}s", empty == VALUE_OBJECT ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        valuesBucketReferences_.reset(in.ReadParcelable<NativeRdb::ValuesBucket>());
    } else {
        valuesBucketReferences_.reset();
    }

    int referenceSize = 0;
    if (!in.ReadInt32(referenceSize)) {
        APP_LOGE("DataAbilityOperation::ReadFromParcel end");
        return false;
    }
    if (referenceSize >= REFERENCE_THRESHOLD) {
        APP_LOGI("DataAbilityOperation::ReadFromParcel referenceSize:%{public}d >= REFERENCE_THRESHOLD:%{public}d", referenceSize, REFERENCE_THRESHOLD);
        return true;
    }

    for (int i = 0; i < REFERENCE_THRESHOLD && i < referenceSize; ++i) {
        int first = 0;
        int second = 0;
        if (!in.ReadInt32(first)) {
            APP_LOGE("DataAbilityOperation::ReadFromParcel end");
            return false;
        }
        if (!in.ReadInt32(second)) {
            APP_LOGE("DataAbilityOperation::ReadFromParcel end");
            return false;
        }
        dataAbilityPredicatesBackReferences_.insert(std::make_pair(first, second));
    }

    APP_LOGD("DataAbilityOperation::ReadFromParcel end");
    return true;
}
std::shared_ptr<DataAbilityOperation> DataAbilityOperation::CreateFromParcel(Parcel &in)
{
    APP_LOGD("DataAbilityOperation::CreateFromParcel start");
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>(in);
    APP_LOGD("DataAbilityOperation::CreateFromParcel end");
    return operation;
}
void DataAbilityOperation::PutMap(Parcel &in)
{
    APP_LOGD("DataAbilityOperation::PutMap start");
    int count = in.ReadInt32();
    if (count > 0 && count < REFERENCE_THRESHOLD) {
        for (int i = 0; i < count; ++i) {
            dataAbilityPredicatesBackReferences_.insert(std::make_pair(in.ReadInt32(), in.ReadInt32()));
        }
    }
    APP_LOGD("DataAbilityOperation::PutMap end");
}
}  // namespace AppExecFwk
}  // namespace OHOS