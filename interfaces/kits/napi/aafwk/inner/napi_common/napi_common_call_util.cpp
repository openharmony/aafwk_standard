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
#include "napi_common_call_util.h"

#include <cstring>

#include "napi_common_util.h"
#include "hilog_wrapper.h"
#include "napi_common_data.h"
#include "napi_common_error.h"

#ifndef SUPPORT_GRAPHICS
#define DBL_MIN ((double)2.22507385850720138309e-308L)
#define DBL_MAX ((double)2.22507385850720138309e-308L)
#endif

namespace OHOS {
namespace AppExecFwk {
void StringSplit(const string &str, const string &splits, std::vector<std::string> &res)
{
    if (str == "") {
        return;
    }
    string strs = str + splits;
    size_t pos = strs.find(splits);
    size_t step = splits.size();

    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + step, strs.size());
        pos = strs.find(splits);
    }
}

void SetPacMapObject(AppExecFwk::PacMap &pacMap, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        pacMap.PutStringValue(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        pacMap.PutDoubleValue(keyStr, valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        pacMap.PutBooleanValue(keyStr, valueBool);
    } else if (valueType == napi_null) {
        pacMap.PutObject(keyStr, nullptr);
    } else if (valueType == napi_object) {
        pacMap.PutStringValueArray(keyStr, ConvertStringVector(env, value));
    } else {
        HILOG_ERROR("pacMap error");
    }
}

void AnalysisPacMap(AppExecFwk::PacMap &pacMap, const napi_env &env, const napi_value &arg)
{
    napi_value keys = 0;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        HILOG_ERROR("PacMap errr");
        return;
    }
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = 0;
        status = napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = 0;
        napi_get_property(env, arg, key, &value);

        SetPacMapObject(pacMap, env, keyStr, value);
    }
}

void ParseEqualTo(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string equalValue = pacMap.GetStringValue(value_str, "");
    if (equalValue != "") {
        if (equalValue.find(",") == string::npos) {
            predicates.EqualTo(value_str, equalValue);
            pacMap.Remove(value_str);
        } else {
            std::vector<std::string> value_vec;
            StringSplit(equalValue, ",", value_vec);
            predicates.EqualTo(value_str, value_vec[0]);
            pacMap.Remove(value_str);
            pacMap.PutStringValue(value_str, equalValue.substr(value_vec[0].length() + 1, equalValue.length()));
        }
    } else {
        double equalDoubleValue = pacMap.GetDoubleValue(value_str, DBL_MIN);
        predicates.EqualTo(value_str, std::to_string(equalDoubleValue));
        pacMap.Remove(value_str);
    }
}

void ParseNotEqualTo(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string equalValue = pacMap.GetStringValue(value_str, "");
    if (equalValue != "") {
        if (equalValue.find(",") == string::npos) {
            predicates.NotEqualTo(value_str, equalValue);
            pacMap.Remove(value_str);
        } else {
            std::vector<std::string> value_vec;
            StringSplit(equalValue, ",", value_vec);
            predicates.NotEqualTo(value_str, value_vec[0]);
            pacMap.Remove(value_str);
            pacMap.PutStringValue(value_str, equalValue.substr(value_vec[0].length() + 1, equalValue.length()));
        }
    } else {
        predicates.NotEqualTo(value_str, std::to_string(pacMap.GetDoubleValue(value_str, DBL_MIN)));
        pacMap.Remove(value_str);
    }
}

void ParseBetween(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string betweenValue = pacMap.GetStringValue(value_str, "");
    std::vector<std::string> value_vec;
    StringSplit(betweenValue, ",", value_vec);
    predicates.Between(value_str, value_vec[0], value_vec[1]);
    size_t tempLength = value_vec[0].length() + 1 + value_vec[1].length() + 1;
    value_vec.erase(value_vec.begin(), value_vec.begin() + 1);
    pacMap.Remove(value_str);
    if (value_vec.size() == 1) {
        pacMap.PutDoubleValue(value_str, stod(value_vec[0]));
    } else if (value_vec.size() > 1) {
        pacMap.PutStringValue(value_str, betweenValue.substr(tempLength, betweenValue.length()));
    }
}

void ParseNotBetween(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string betweenValue = pacMap.GetStringValue(value_str, "");
    std::vector<std::string> value_vec;
    StringSplit(betweenValue, ",", value_vec);
    predicates.NotBetween(value_str, value_vec[0], value_vec[1]);
    size_t tempLength = value_vec[0].length() + 1 + value_vec[1].length() + 1;
    value_vec.erase(value_vec.begin(), value_vec.begin() + 1);
    pacMap.Remove(value_str);
    if (value_vec.size() == 1) {
        pacMap.PutDoubleValue(value_str, stod(value_vec[0]));
    } else if (value_vec.size() > 1) {
        pacMap.PutStringValue(value_str, betweenValue.substr(tempLength, betweenValue.length()));
    }
}

void ParseIn(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string inValue = pacMap.GetStringValue(value_str, "");
    std::vector<std::string> value_vec;
    StringSplit(inValue, ",", value_vec);
    if (value_vec.size() >= ARGS_TWO) {
        std::vector<std::string> in_vec;
        in_vec.push_back(value_vec[0]);
        in_vec.push_back(value_vec[1]);
        predicates.In(value_str, in_vec);
        size_t tempLength = value_vec[0].length() + 1 + value_vec[1].length() + 1;
        value_vec.erase(value_vec.begin(), value_vec.begin() + 1);
        pacMap.Remove(value_str);
        if (value_vec.size() == 1) {
            pacMap.PutDoubleValue(value_str, stod(value_vec[0]));
        } else if (value_vec.size() > 1) {
            pacMap.PutStringValue(value_str, inValue.substr(tempLength, inValue.length()));
        }
    } else {
        pacMap.Remove(value_str);
    }
}

void ParseNotIn(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates)
{
    std::string inValue = pacMap.GetStringValue(value_str, "");
    std::vector<std::string> value_vec;
    StringSplit(inValue, ",", value_vec);
    if (value_vec.size() >= ARGS_TWO) {
        std::vector<std::string> in_vec;
        in_vec.push_back(value_vec[0]);
        in_vec.push_back(value_vec[1]);
        predicates.NotIn(value_str, in_vec);
        size_t tempLength = value_vec[0].length() + 1 + value_vec[1].length() + 1;
        value_vec.erase(value_vec.begin(), value_vec.begin() + 1);
        pacMap.Remove(value_str);
        if (value_vec.size() == 1) {
            pacMap.PutDoubleValue(value_str, stod(value_vec[0]));
        } else if (value_vec.size() > 1) {
            pacMap.PutStringValue(value_str, inValue.substr(tempLength, inValue.length()));
        }
    } else {
        pacMap.Remove(value_str);
    }
}

void ParseNormal(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates,
    std::string key_str)
{
    if (key_str == "equalTo") {
        ParseEqualTo(pacMap, value_str, predicates);
    } else if (key_str == "notEqualTo") {
        ParseNotEqualTo(pacMap, value_str, predicates);
    } else if (key_str == "contains") {
        predicates.Contains(value_str, pacMap.GetStringValue(value_str, ""));
        pacMap.Remove(value_str);
    } else if (key_str == "beginsWith") {
        predicates.BeginsWith(value_str, pacMap.GetStringValue(value_str, ""));
        pacMap.Remove(value_str);
    } else if (key_str == "endsWith") {
        predicates.EndsWith(value_str, pacMap.GetStringValue(value_str, ""));
        pacMap.Remove(value_str);
    } else if (key_str == "greaterThan") {
        double greateValue = pacMap.GetDoubleValue(value_str, DBL_MIN);
        predicates.GreaterThan(value_str, std::to_string(greateValue));
        pacMap.Remove(value_str);
    } else if (key_str == "lessThan") {
        double lessValue = pacMap.GetDoubleValue(value_str, DBL_MAX);
        predicates.LessThan(value_str, std::to_string(lessValue));
        pacMap.Remove(value_str);
    } else if (key_str == "greaterThanOrEqualTo") {
        double greateValue = pacMap.GetDoubleValue(value_str, DBL_MIN);
        predicates.GreaterThanOrEqualTo(value_str, std::to_string(greateValue));
        pacMap.Remove(value_str);
    } else if (key_str == "lessThanOrEqualTo") {
        double lessValue = pacMap.GetDoubleValue(value_str, DBL_MAX);
        predicates.LessThanOrEqualTo(value_str, std::to_string(lessValue));
        pacMap.Remove(value_str);
    }
}

void ParseUnNormal(AppExecFwk::PacMap &pacMap, std::string &value_str, NativeRdb::DataAbilityPredicates &predicates,
    std::string key_str)
{
    if (key_str == "beginWrap") {
        predicates.BeginWrap();
    } else if (key_str == "endWrap") {
        predicates.EndWrap();
    } else if (key_str == "or") {
        predicates.Or();
    } else if (key_str == "and") {
        predicates.And();
    } else if (key_str == "orderByAsc") {
        predicates.OrderByAsc(value_str);
    } else if (key_str == "orderByDesc") {
        predicates.OrderByDesc(value_str);
    } else if (key_str == "distinct") {
        predicates.Distinct();
    } else if (key_str == "limitAs") {
        predicates.Limit(pacMap.GetIntValue(value_str, 0));
    } else if (key_str == "offsetAs") {
        predicates.Offset(pacMap.GetIntValue(value_str, 0));
    } else if (key_str == "groupBy") {
        std::string groupByValue = pacMap.GetStringValue(value_str, "");
        std::vector<std::string> res;
        StringSplit(groupByValue, ",", res);
        predicates.GroupBy(res);
    } else if (key_str == "indexedBy") {
        predicates.IndexedBy(value_str);
    } else if (key_str == "in") {
        ParseIn(pacMap, value_str, predicates);
    } else if (key_str == "notIn") {
        ParseNotIn(pacMap, value_str, predicates);
    }
}

bool KeyInValue(std::string &key_str)
{
    if (key_str == "equalTo" || key_str == "notEqualTo" || key_str == "contains" || key_str == "beginsWith" ||
        key_str == "endsWith" || key_str == "greaterThan" || key_str == "lessThan" ||
        key_str == "greaterThanOrEqualTo" || key_str == "lessThanOrEqualTo") {
        return true;
    } else {
        return false;
    }
}

void ParseJsonKey(std::string &jsonStr, AppExecFwk::PacMap &pacMap, NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s called.", __func__);
#ifdef SUPPORT_GRAPHICS
    Json::Reader reader;
#else
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
#endif
    Json::Value devJson;
    Json::Value::iterator iter;
    Json::Value::Members members;
#ifdef SUPPORT_GRAPHICS
    bool parseSuc = reader.parse(jsonStr, devJson);
#else
    bool parseSuc = reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &devJson, nullptr);
#endif
    if (parseSuc) {
        members = devJson.getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it) {
            std::string key_str = *it;
            std::string value_str = devJson[key_str].asString();
            if (KeyInValue(key_str)) {
                ParseNormal(pacMap, value_str, predicates, key_str);
            } else {
                ParseUnNormal(pacMap, value_str, predicates, key_str);
            }
        }
    }
    HILOG_INFO("%{public}s called end.", __func__);
}

void GetValue(std::vector<std::string> &columnNames, int &index, Json::Value &data,
    std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet)
{
    std::string typeValue = columnNames[index];
    int columnIndex;
    resultSet->GetColumnIndex(typeValue, columnIndex);
    OHOS::NativeRdb::ColumnType columnType;
    resultSet->GetColumnType(columnIndex, columnType);
    if (columnType == OHOS::NativeRdb::ColumnType::TYPE_INTEGER) {
        int intValue;
        resultSet->GetInt(columnIndex, intValue);
        data[typeValue] = intValue;
    } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_FLOAT) {
        double doubleValue;
        resultSet->GetDouble(columnIndex, doubleValue);
        data[typeValue] = doubleValue;
    } else if (columnType == OHOS::NativeRdb::ColumnType::TYPE_STRING) {
        std::string stringValue;
        resultSet->GetString(columnIndex, stringValue);
        if (!stringValue.empty()) {
            data[typeValue] = stringValue;
        }
    }
}

void ConvertResultSet(Json::Value &arrayValue, std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet)
{
    int resultSetNum = resultSet->GoToFirstRow();
    std::vector<std::string> columnNames;
    resultSet->GetAllColumnNames(columnNames);
    while (resultSetNum == NO_ERROR) {
        Json::Value data;
        int size = (int)columnNames.size();
        for (int i = 0; i < size; i++) {
            GetValue(columnNames, i, data, resultSet);
        }
        arrayValue.append(data);
        resultSetNum = resultSet->GoToNextRow();
    }
}

std::string GetResultData(std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet)
{
    if (resultSet == nullptr) {
        return "";
    }
    Json::Value arrayValue;
    ConvertResultSet(arrayValue, resultSet);
    Json::StreamWriterBuilder builder;
    const std::string personal_ringtone = Json::writeString(builder, arrayValue);
    return personal_ringtone;
}
}  // namespace AppExecFwk
}  // namespace OHOS
