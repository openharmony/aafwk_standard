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
#ifndef OHOS_APPEXECFWK_DATA_ABILITY_OPERATION_H
#define OHOS_APPEXECFWK_DATA_ABILITY_OPERATION_H

#include "data_ability_operation.h"
#include "feature_ability_common.h"

namespace OHOS {
namespace NativeRdb {
class DataAbilityPredicates;
class ValuesBucket;
}  // namespace NativeRdb
namespace AppExecFwk {
/**
 * @brief DataAbilityOperation NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value DataAbilityOperationInit(napi_env env, napi_value exports);
void SetNamedProperty(napi_env env, napi_value obj, const char *propName, int propValue);
/**
 * @brief Parse the dataAbilityOperation parameters.
 *
 * @param param Indicates the dataAbilityOperation parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapDataAbilityOperation(std::shared_ptr<DataAbilityOperation> &param, napi_env env, napi_value args);
napi_value BuildDataAbilityOperation(
    std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, napi_env env, napi_value param);
bool GetDataAbilityOperationBuilder(
    std::shared_ptr<DataAbilityOperationBuilder> &builder, int type, const std::shared_ptr<Uri> &uri);
/**
 * @brief Parse the ValuesBucket parameters.
 *
 * @param param Indicates the valuesBucket parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapValuesBucket(std::shared_ptr<NativeRdb::ValuesBucket> &param, napi_env env, napi_value args);
/**
 * @brief Parse the DataAbilityPredicatesBackReferences parameters.
 *
 * @param builder Indicates the dataAbilityPredicatesBackReferences parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapDataAbilityPredicatesBackReferences(
    std::shared_ptr<DataAbilityOperationBuilder> &builder, napi_env env, napi_value args);

}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_WANT_WRAPPER_H */
