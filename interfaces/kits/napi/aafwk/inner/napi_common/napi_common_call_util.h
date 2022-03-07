/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_APPEXECFWK_NAPI_COMMON_CALL_UTIL_H
#define OHOS_APPEXECFWK_NAPI_COMMON_CALL_UTIL_H

#include <vector>
#include <string>

#include "napi/native_api.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "pac_map.h"
#include "json.h"

namespace OHOS {
namespace AppExecFwk {
void StringSplit(const std::string &str, const std::string &splits, std::vector<std::string> &res);

void SetPacMapObject(AppExecFwk::PacMap &pacMap, const napi_env &env, std::string keyStr, napi_value value);

void AnalysisPacMap(AppExecFwk::PacMap &pacMap, const napi_env &env, const napi_value &arg);

void ParseNotEqualTo(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseNotEqualTo(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseBetween(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseNotBetween(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseIn(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseNotIn(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates);

void ParseNormal(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates,
    std::string key_str);

void ParseUnNormal(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates,
    std::string key_str);

bool KeyInValue(std::string &key_str);

void ParseJsonKey(std::string &jsonStr, AppExecFwk::PacMap &pacMap, NativeRdb::DataAbilityPredicates &predicates);

void GetValue(std::vector<std::string> &columnNames, int &index, Json::Value &data,
    std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);

void ConvertResultSet(Json::Value &arrayValue, std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);

std::string GetResultData(std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APPEXECFWK_NAPI_COMMON_CALL_UTIL_H
