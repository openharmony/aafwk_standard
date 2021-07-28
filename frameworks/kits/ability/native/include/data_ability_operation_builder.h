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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_BUILDER_H
#define FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_BUILDER_H

#include <map>
#include <memory>
#include "dummy_data_ability_predicates.h"
#include "dummy_values_bucket.h"
#include "data_ability_operation.h"
#include "uri.h"
#include "parcel.h"

using Uri = OHOS::Uri;
namespace OHOS {
namespace AppExecFwk {
class DataAbilityOperation;
class DataAbilityOperationBuilder final : public std::enable_shared_from_this<DataAbilityOperationBuilder> {
    friend class DataAbilityOperation;

public:
    DataAbilityOperationBuilder(const int type, const std::shared_ptr<Uri> &uri);
    ~DataAbilityOperationBuilder();
    /**
     * @brief Creates a DataAbilityOperation object.
     * @return Returns the DataAbilityOperation object.
     */
    std::shared_ptr<DataAbilityOperation> Build();
    /**
     * @brief Sets the data records to be inserted or updated.
     * @param values Indicates the data values to be set.
     * @return Returns a DataAbilityOperationBuilder object containing the given values parameter.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithValuesBucket(std::shared_ptr<ValuesBucket> &values);
    /**
     * @brief Sets filter criteria used for deleting updating or assert query data.
     * @param predicates Indicates the filter criteria to set. If this parameter is null, all data records will be
     * operated by default.
     * @return Returns an object containing the given filter criteria.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithPredicates(std::shared_ptr<DataAbilityPredicates> &predicates);
    /**
     * @brief Sets the expected number of rows to update ,delete or assert query.
     * @param count Indicates the expected number of rows to update or delete.
     * @return Returns a DataAbilityOperationBuilder object containing the given count parameter.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithExpectedCount(int count);
    /**
     * @brief Adds a back reference to be used as a filter criterion in withPredicates(DataAbilityPredicates).
     * @param requestArgIndex Indicates the index referencing the predicate parameter whose value is to be replaced.
     * @param previousResult Indicates the index referencing the historical DataAbilityResult used to replace the value
     * of the specified predicate parameter.
     * @return Returns a DataAbilityOperationBuilder object containing the given requestArgIndex and previousResult
     * parameters.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithPredicatesBackReference(int requestArgIndex, int previousResult);
    /**
     * @brief Adds a back reference to be used in withValuesBucket(ValuesBucket).
     * @param backReferences Indicates the ValuesBucket object containing a set of key-value pairs. In each pair, the
     * key specifies the value to be updated and the value specifies the index referencing the DataAbilityResult used to
     * replace the specified value. This parameter cannot be null.
     * @return Returns a DataAbilityOperationBuilder object containing the given backReferences parameter.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithValueBackReferences(std::shared_ptr<ValuesBucket> &backReferences);
    /**
     * @brief Sets an interrupt flag bit for a batch operation, which can be insert, update, delete, or assert.
     * @param interrupted Specifies whether a batch operation can be interrupted. The value true indicates that the
     * operation can be interrupted, and false indicates the opposite.
     * @return Returns a DataAbilityOperationBuilder object containing the given interrupted parameter.
     */
    std::shared_ptr<DataAbilityOperationBuilder> WithInterruptionAllowed(bool interrupted);

private:
    int type_;
    int expectedCount_;
    bool interrupted_;
    std::shared_ptr<Uri> uri_;
    std::shared_ptr<ValuesBucket> valuesBucket_;
    std::shared_ptr<DataAbilityPredicates> dataAbilityPredicates_;
    std::shared_ptr<ValuesBucket> valuesBucketReferences_;
    std::map<int, int> dataAbilityPredicatesBackReferences_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_BUILDER_H