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
#include "napi_common_util.h"

#include <cstring>

#include "hilog_wrapper.h"
#include "napi_common_data.h"
#include "napi_common_error.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {

bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;

    if (param == nullptr) {
        return false;
    }

    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }

    return valueType == expectType;
}

bool IsArrayForNapiValue(napi_env env, napi_value param, uint32_t &arraySize)
{
    bool isArray = false;
    arraySize = 0;

    if (napi_is_array(env, param, &isArray) != napi_ok || isArray == false) {
        return false;
    }

    if (napi_get_array_length(env, param, &arraySize) != napi_ok) {
        return false;
    }
    return true;
}

napi_value WrapVoidToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value WrapUndefinedToJS(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value CreateJSObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

napi_value WrapInt32ToJS(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

int UnwrapInt32FromJS(napi_env env, napi_value param, int defaultValue)
{
    int value = defaultValue;
    if (napi_get_value_int32(env, param, &value) == napi_ok) {
        return value;
    } else {
        return defaultValue;
    }
}

bool UnwrapInt32FromJS2(napi_env env, napi_value param, int &value)
{
    bool result = false;
    if (napi_get_value_int32(env, param, &value) == napi_ok) {
        result = true;
    }
    return result;
}

napi_value WrapLongToJS(napi_env env, long value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

long UnwrapLongFromJS(napi_env env, napi_value param, long defaultValue)
{
    int value = 0;
    if (napi_get_value_int32(env, param, &value) == napi_ok) {
        return value;
    } else {
        return defaultValue;
    }
}

bool UnwrapLongFromJS2(napi_env env, napi_value param, long &value)
{
    bool result = false;
    int natValue = 0;
    if (napi_get_value_int32(env, param, &natValue) == napi_ok) {
        value = natValue;
        result = true;
    }
    return result;
}

napi_value WrapInt64ToJS(napi_env env, int64_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

int64_t UnwrapInt64FromJS(napi_env env, napi_value param, int64_t defaultValue)
{
    int64_t value = defaultValue;
    if (napi_get_value_int64(env, param, &value) == napi_ok) {
        return value;
    } else {
        return defaultValue;
    }
}

bool UnwrapInt64FromJS2(napi_env env, napi_value param, int64_t &value)
{
    bool result = false;
    if (napi_get_value_int64(env, param, &value) == napi_ok) {
        result = true;
    }
    return result;
}

napi_value WrapBoolToJS(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

bool UnwrapBoolFromJS(napi_env env, napi_value param, bool defaultValue)
{
    bool value = defaultValue;
    if (napi_get_value_bool(env, param, &value) == napi_ok) {
        return value;
    } else {
        return defaultValue;
    }
}

bool UnwrapBoolFromJS2(napi_env env, napi_value param, bool &value)
{
    bool result = false;
    if (napi_get_value_bool(env, param, &value) == napi_ok) {
        result = true;
    }
    return result;
}

napi_value WrapDoubleToJS(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

double UnwrapDoubleFromJS(napi_env env, napi_value param, double defaultValue)
{
    double value = defaultValue;
    if (napi_get_value_double(env, param, &value) == napi_ok) {
        return value;
    } else {
        return defaultValue;
    }
}

bool UnwrapDoubleFromJS2(napi_env env, napi_value param, double &value)
{
    bool result = false;
    if (napi_get_value_double(env, param, &value) == napi_ok) {
        result = true;
    }
    return result;
}

napi_value WrapStringToJS(napi_env env, const std::string &value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result));
    return result;
}

std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string &defaultValue)
{
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return defaultValue;
    }

    std::string value("");
    if (size == 0) {
        return defaultValue;
    }

    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return value;
    }
    (void)memset_s(buf, size + 1, 0, size + 1);

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    } else {
        value = defaultValue;
    }

    delete[] buf;
    buf = nullptr;
    return value;
}

bool UnwrapStringFromJS2(napi_env env, napi_value param, std::string &value)
{
    value = "";
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return false;
    }

    if (size == 0) {
        return true;
    }

    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return false;
    }
    (void)memset_s(buf, (size + 1), 0, (size + 1));

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    }
    delete[] buf;
    buf = nullptr;
    return rev;
}

napi_value WrapArrayInt32ToJS(napi_env env, const std::vector<int> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_int32(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayInt32FromJS(napi_env env, napi_value param, std::vector<int> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    int natValue = 0;

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = 0;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapInt32FromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

napi_value WrapArrayLongToJS(napi_env env, const std::vector<long> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_int32(env, (int)(value[i]), &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayLongFromJS(napi_env env, napi_value param, std::vector<long> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    long natValue = 0;

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = 0;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapLongFromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

napi_value WrapArrayInt64ToJS(napi_env env, const std::vector<int64_t> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_int64(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayInt64FromJS(napi_env env, napi_value param, std::vector<int64_t> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    int64_t natValue = 0;

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = 0;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapInt64FromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

napi_value WrapArrayDoubleToJS(napi_env env, const std::vector<double> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_double(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayDoubleFromJS(napi_env env, napi_value param, std::vector<double> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    double natValue = 0;

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = 0;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapDoubleFromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

napi_value WrapArrayBoolToJS(napi_env env, const std::vector<bool> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_get_boolean(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayBoolFromJS(napi_env env, napi_value param, std::vector<bool> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    bool natValue = 0;

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = 0;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapBoolFromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

napi_value WrapArrayStringToJS(napi_env env, const std::vector<std::string> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_string_utf8(env, value[i].c_str(), NAPI_AUTO_LENGTH, &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool UnwrapArrayStringFromJS(napi_env env, napi_value param, std::vector<std::string> &value)
{
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string natValue("");

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        natValue = "";
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            return false;
        }

        if (!UnwrapStringFromJS2(env, jsValue, natValue)) {
            return false;
        }

        value.push_back(natValue);
    }
    return true;
}

bool UnwrapArrayComplexFromJSNumber(napi_env env, ComplexArrayData &value, bool isDouble, napi_value jsValue)
{
    int32_t elementInt32 = 0;
    double elementDouble = 0.0;
    if (isDouble) {
        if (napi_get_value_double(env, jsValue, &elementDouble) == napi_ok) {
            value.doubleList.push_back(elementDouble);
        }
        return isDouble;
    }

    bool isReadValue32 = napi_get_value_int32(env, jsValue, &elementInt32) == napi_ok;
    bool isReadDouble = napi_get_value_double(env, jsValue, &elementDouble) == napi_ok;
    if (isReadValue32 && isReadDouble) {
        if (abs(elementDouble - elementInt32 * 1.0) > 0.0) {
            isDouble = true;
            if (value.intList.size() > 0) {
                for (size_t j = 0; j < value.intList.size(); j++) {
                    value.doubleList.push_back(value.intList[j]);
                }
                value.intList.clear();
            }
            value.doubleList.push_back(elementDouble);
        } else {
            value.intList.push_back(elementInt32);
        }
    } else if (isReadValue32) {
        value.intList.push_back(elementInt32);
    } else if (isReadDouble) {
        isDouble = true;
        if (value.intList.size() > 0) {
            for (size_t j = 0; j < value.intList.size(); j++) {
                value.doubleList.push_back(value.intList[j]);
            }
            value.intList.clear();
        }
        value.doubleList.push_back(elementDouble);
    }
    return isDouble;
}

bool UnwrapArrayComplexFromJS(napi_env env, napi_value param, ComplexArrayData &value)
{
    uint32_t arraySize = 0;
    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_value jsValue = nullptr;
    bool isDouble = false;

    value.intList.clear();
    value.longList.clear();
    value.boolList.clear();
    value.doubleList.clear();
    value.stringList.clear();
    value.objectList.clear();

    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        valueType = napi_undefined;
        NAPI_CALL_BASE(env, napi_get_element(env, param, i, &jsValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsValue, &valueType), false);
        switch (valueType) {
            case napi_string: {
                std::string elementValue("");
                if (UnwrapStringFromJS2(env, jsValue, elementValue)) {
                    value.stringList.push_back(elementValue);
                } else {
                    return false;
                }
                break;
            }
            case napi_boolean: {
                bool elementValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsValue, &elementValue), false);
                value.boolList.push_back(elementValue);
                break;
            }
            case napi_number: {
                isDouble = UnwrapArrayComplexFromJSNumber(env, value, isDouble, jsValue);
                break;
            }
            case napi_object: {
                value.objectList.push_back(jsValue);
                break;
            }
            default:
                break;
        }
    }
    return true;
}

/**
 * @brief Indicates the specified attribute exists in the object passed by JS.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 *
 * @return Returns true if the attribute exists, else returns false.
 */
bool IsExistsByPropertyName(napi_env env, napi_value jsObject, const char *propertyName)
{
    bool result = false;
    if (napi_has_named_property(env, jsObject, propertyName, &result) == napi_ok) {
        return result;
    } else {
        return false;
    }
}

/**
 * @brief Get the JSValue of the specified name from the JS object.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param expectType Indicates expected JS data type.
 *
 * @return Return the property value of the specified property name int jsObject on success, otherwise return nullptr.
 */
napi_value GetPropertyValueByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, napi_valuetype expectType)
{
    napi_value value = nullptr;
    if (IsExistsByPropertyName(env, jsObject, propertyName) == false) {
        return nullptr;
    }

    if (napi_get_named_property(env, jsObject, propertyName, &value) != napi_ok) {
        return nullptr;
    }

    if (!IsTypeForNapiValue(env, value, expectType)) {
        return nullptr;
    }

    return value;
}

bool SetPropertyValueByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, napi_value value)
{
    if (value != nullptr && propertyName != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, propertyName, value), false);
        return true;
    }
    return false;
}

/**
 * @brief Get the native number(int32) from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param value Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool UnwrapInt32ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int32_t &value)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_number);
    if (jsValue != nullptr) {
        return UnwrapInt32FromJS2(env, jsValue, value);
    } else {
        return false;
    }
}

/**
 * @brief Get the native number(double) from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param value Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool UnwrapDoubleByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, double &value)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_number);
    if (jsValue != nullptr) {
        return UnwrapDoubleFromJS2(env, jsValue, value);
    } else {
        return false;
    }
}

/**
 * @brief Get the native boolean from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param value Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool UnwrapBooleanByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, bool &value)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_boolean);
    if (jsValue != nullptr) {
        return UnwrapBoolFromJS2(env, jsValue, value);
    } else {
        return false;
    }
}

bool UnwrapBooleanArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<bool> &value)
{
    napi_value jsArray = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_object);
    if (jsArray == nullptr) {
        return false;
    }

    return UnwrapArrayBoolFromJS(env, jsArray, value);
}

/**
 * @brief Get the native string from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param value Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool UnwrapStringByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, std::string &value)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_string);
    if (jsValue != nullptr) {
        return UnwrapStringFromJS2(env, jsValue, value);
    } else {
        return false;
    }
}

bool UnwrapStringArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<std::string> &value)
{
    napi_value jsArray = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_object);
    if (jsArray == nullptr) {
        return false;
    }

    return UnwrapArrayStringFromJS(env, jsArray, value);
}

bool UnwrapComplexArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, ComplexArrayData &value)
{
    napi_value jsArray = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_object);
    if (jsArray == nullptr) {
        return false;
    }

    return UnwrapArrayComplexFromJS(env, jsArray, value);
}

void ClearThreadReturnData(ThreadReturnData *data)
{
    if (data != nullptr) {
        data->data_type = NVT_NONE;
        data->int32_value = 0;
        data->bool_value = false;
        data->str_value = "";
        data->double_value = 0.0;
    }
}

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &jsValue));
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "code", jsValue));
    return jsObject;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncPermissionCallbackInfo on success, nullptr on failure
 */
AsyncJSCallbackInfo *CreateAsyncJSCallbackInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);

    napi_value global = 0;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = 0;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AsyncJSCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncJSCallbackInfo;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }
    asyncCallbackInfo->cbInfo.env = env;
    asyncCallbackInfo->cbInfo.callback = nullptr;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->deferred = nullptr;
    asyncCallbackInfo->ability = ability;
    asyncCallbackInfo->abilityType = AbilityType::UNKNOWN;
    asyncCallbackInfo->aceCallback = nullptr;

    if (asyncCallbackInfo != nullptr) {
        ClearThreadReturnData(&asyncCallbackInfo->native_data);
    }
    return asyncCallbackInfo;
}

void FreeAsyncJSCallbackInfo(AsyncJSCallbackInfo **asyncCallbackInfo)
{
    if (asyncCallbackInfo == nullptr) {
        return;
    }
    if (*asyncCallbackInfo == nullptr) {
        return;
    }

    if ((*asyncCallbackInfo)->cbInfo.callback != nullptr && (*asyncCallbackInfo)->cbInfo.env != nullptr) {
        napi_delete_reference((*asyncCallbackInfo)->cbInfo.env, (*asyncCallbackInfo)->cbInfo.callback);
        (*asyncCallbackInfo)->cbInfo.callback = nullptr;
        (*asyncCallbackInfo)->cbInfo.env = nullptr;
    }

    delete (*asyncCallbackInfo);
    *asyncCallbackInfo = nullptr;
}

/**
 * @brief Convert local data to JS data.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data The local data.
 * @param value the JS data.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
bool WrapThreadReturnData(napi_env env, const ThreadReturnData *data, napi_value *value)
{
    if (data == nullptr || value == nullptr) {
        return false;
    }

    switch (data->data_type) {
        case NVT_UNDEFINED:
            NAPI_CALL_BASE(env, napi_get_undefined(env, value), false);
            break;
        case NVT_INT32:
            NAPI_CALL_BASE(env, napi_create_int32(env, data->int32_value, value), false);
            break;
        case NVT_BOOL:
            NAPI_CALL_BASE(env, napi_get_boolean(env, data->bool_value, value), false);
            break;
        case NVT_STRING:
            NAPI_CALL_BASE(env, napi_create_string_utf8(env, data->str_value.c_str(), NAPI_AUTO_LENGTH, value), false);
            break;
        default:
            NAPI_CALL_BASE(env, napi_get_null(env, value), false);
            break;
    }
    return true;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param Parameter list.
 * @param callback Point to asynchronous processing of data.
 *
 * @return Return true successfully, otherwise return false.
 */
bool CreateAsyncCallback(napi_env env, napi_value param, AsyncJSCallbackInfo *callback)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (param == nullptr || callback == nullptr) {
        HILOG_INFO("%{public}s called, param or callback is null.", __func__);
        return false;
    }

    callback->cbInfo.callback = CreateCallbackRefFromJS(env, param);
    if (callback->cbInfo.callback == nullptr) {
        HILOG_INFO("%{public}s, create ref failed.", __func__);
        return false;
    }

    return true;
}

napi_ref CreateCallbackRefFromJS(napi_env env, napi_value param)
{
    if (env == nullptr || param == nullptr) {
        HILOG_INFO("%{public}s called, env or param is null  ", __func__);
        return nullptr;
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, param, &valueType));

    if (valueType != napi_function) {
        HILOG_INFO("%{public}s called, Param is invalid.", __func__);
        return nullptr;
    }

    napi_ref callbackRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, param, 1, &callbackRef));
    return callbackRef;
}

/**
 * @brief Asynchronous callback processing.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 * @param param other param.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecuteAsyncCallbackWork(napi_env env, AsyncJSCallbackInfo *asyncCallbackInfo, const AsyncParamEx *param)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr || param == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo or param is null", __func__);
        return nullptr;
    }

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, param->resource.c_str(), NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            param->execute,
            param->complete,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    return WrapVoidToJS(env);
}

/**
 * @brief Asynchronous promise processing.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 * @param param other param.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecutePromiseCallbackWork(napi_env env, AsyncJSCallbackInfo *asyncCallbackInfo, const AsyncParamEx *param)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr || param == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo or param is null", __func__);
        return nullptr;
    }

    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, param->resource.c_str(), NAPI_AUTO_LENGTH, &resourceName));

    napi_deferred deferred = 0;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));

    asyncCallbackInfo->deferred = deferred;
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            param->execute,
            param->complete,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    return promise;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompleteAsyncCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }

    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(env, &undefined);
    napi_value callResult = 0;
    napi_value revParam[ARGS_TWO] = {nullptr};

    revParam[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->error_code);
    WrapThreadReturnData(env, &asyncCallbackInfo->native_data, &revParam[PARAM1]);

    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
        napi_call_function(env, undefined, callback, ARGS_TWO, revParam, &callResult);
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

void CompleteAsyncVoidCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(env, &undefined);
    napi_value callResult = 0;
    napi_value result[ARGS_TWO] = {nullptr};

    if (asyncCallbackInfo->error_code) {
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncCallbackInfo->error_code, &result[PARAM0]));
    }

    if (asyncCallbackInfo->error_code == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapVoidToJS(env);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
        napi_call_function(env, undefined, callback, ARGS_TWO, result, &callResult);
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompletePromiseCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }

    napi_value result = 0;
    if (asyncCallbackInfo->error_code == NAPI_ERR_NO_ERROR) {
        WrapThreadReturnData(env, &asyncCallbackInfo->native_data, &result);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->error_code);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

void CompletePromiseVoidCallbackWork(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);

    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called, asyncCallbackInfo is null", __func__);
        return;
    }
    napi_value result = 0;
    if (asyncCallbackInfo->error_code == NAPI_ERR_NO_ERROR) {
        result = WrapVoidToJS(env);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->error_code);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

std::vector<uint8_t> ConvertU8Vector(napi_env env, napi_value jsValue)
{
    bool isTypedArray = false;
    if (napi_is_typedarray(env, jsValue, &isTypedArray) != napi_ok || !isTypedArray) {
        return {};
    }

    napi_typedarray_type type;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    NAPI_CALL_BASE(env, napi_get_typedarray_info(env, jsValue, &type, &length, nullptr, &buffer, &offset), {});
    if (type != napi_uint8_array) {
        return {};
    }
    uint8_t *data = nullptr;
    size_t total = 0;
    NAPI_CALL_BASE(env, napi_get_arraybuffer_info(env, buffer, reinterpret_cast<void **>(&data), &total), {});
    length = std::min<size_t>(length, total - offset);
    std::vector<uint8_t> result(sizeof(uint8_t) + length);
    int retCode = memcpy_s(result.data(), result.size(), &data[offset], length);
    if (retCode != 0) {
        return {};
    }
    return result;
}

std::vector<std::string> ConvertStringVector(napi_env env, napi_value jsValue)
{
    bool isTypedArray = false;
    napi_status status = napi_is_typedarray(env, jsValue, &isTypedArray);
    if (status != napi_ok || !isTypedArray) {
        HILOG_ERROR("%{public}s called, napi_is_typedarray error", __func__);
        return {};
    }

    napi_typedarray_type type;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    NAPI_CALL_BASE(env, napi_get_typedarray_info(env, jsValue, &type, &length, nullptr, &buffer, &offset), {});
    if (type != napi_uint8_array) {
        HILOG_ERROR("%{public}s called, napi_uint8_array is null", __func__);
        return {};
    }
    std::string *data = nullptr;
    size_t total = 0;
    NAPI_CALL_BASE(env, napi_get_arraybuffer_info(env, buffer, reinterpret_cast<void **>(&data), &total), {});
    length = std::min<size_t>(length, total - offset);
    std::vector<std::string> result(sizeof(std::string) + length);
    int retCode = memcpy_s(result.data(), result.size(), &data[offset], length);
    if (retCode != 0) {
        return {};
    }
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
