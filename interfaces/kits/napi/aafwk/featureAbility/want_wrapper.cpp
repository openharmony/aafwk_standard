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
#include <cstring>
#include <vector>
#include <map>
#include "want_wrapper.h"
#include "want_params.h"
#include "securec.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace AppExecFwk {

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
 * @return Return value of JSObject if successful, else return nullptr.
 */
napi_value GetJSValueByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    napi_value value = nullptr;
    if (IsExistsByPropertyName(env, jsObject, propertyName) == false) {
        return nullptr;
    }

    if (napi_get_named_property(env, jsObject, propertyName, &value) != napi_ok) {
        return nullptr;
    }

    if (napi_typeof(env, value, &valueType) != napi_ok) {
        return nullptr;
    }

    if (expectType != napi_undefined && valueType != expectType) {
        return nullptr;
    }
    return value;
}

/**
 * @brief Get the native string from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param nativeValue Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool GetStringByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, std::string &nativeValue)
{
    napi_value value = GetJSValueByPropertyName(env, jsObject, propertyName, napi_string);
    if (value != nullptr) {
        nativeValue = NapiValueToStringUtf8(env, value);
        HILOG_INFO("%{public}s,called, nativeValue=%{public}s", __func__, nativeValue.c_str());
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Get the native number(int32) from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param nativeValue Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool GetInt32ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int32_t &nativeValue)
{
    napi_value value = GetJSValueByPropertyName(env, jsObject, propertyName, napi_number);
    if (value != nullptr) {
        return (napi_get_value_int32(env, value, &nativeValue) == napi_ok);
    } else {
        return false;
    }
}

/**
 * @brief Get the native number(int64) from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param nativeValue Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool GetInt64ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int64_t &nativeValue)
{
    napi_value value = GetJSValueByPropertyName(env, jsObject, propertyName, napi_number);
    if (value != nullptr) {
        return (napi_get_value_int64(env, value, &nativeValue) == napi_ok);
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
 * @param nativeValue Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool GetDoubleByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, double &nativeValue)
{
    napi_value value = GetJSValueByPropertyName(env, jsObject, propertyName, napi_number);
    if (value != nullptr) {
        return (napi_get_value_double(env, value, &nativeValue) == napi_ok);
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
 * @param nativeValue Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool GetBooleanByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, bool &nativeValue)
{
    napi_value value = GetJSValueByPropertyName(env, jsObject, propertyName, napi_boolean);
    if (value != nullptr) {
        return (napi_get_value_bool(env, value, &nativeValue) == napi_ok);
    } else {
        return false;
    }
}

/**
 * @brief Get the JSObject from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 *
 * @return Return value of JSObject if successful, else return nullptr.
 */
napi_value GetObjectByPropertyName(napi_env env, napi_value jsObject, const char *propertyName)
{
    return GetJSValueByPropertyName(env, jsObject, propertyName, napi_object);
}

/**
 * @brief Get the native array of string from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param nativeValue Indicates the returned native value.
 *
 */
void GetStringArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<std::string> &nativeValue)
{
    nativeValue.clear();
    napi_value jsArrayObject = GetObjectByPropertyName(env, jsObject, propertyName);
    if (jsArrayObject == nullptr) {
        return;
    }

    bool isArray = false;
    if (napi_is_array(env, jsArrayObject, &isArray) != napi_ok) {
        return;
    }
    if (isArray == false) {
        return;
    }

    bool hasElement = false;
    size_t index = 0;
    napi_value elementValue = nullptr;
    napi_valuetype valueType = napi_undefined;
    while (napi_has_element(env, jsArrayObject, index, &hasElement) == napi_ok) {
        if (hasElement == false) {
            break;
        }

        if (napi_get_element(env, jsArrayObject, index, &elementValue) == napi_ok) {
            if (napi_typeof(env, elementValue, &valueType) == napi_ok) {
                if (valueType == napi_string) {
                    nativeValue.emplace_back(NapiValueToStringUtf8(env, elementValue));
                }
            }
        }
        index++;
    }
}

/**
 * @brief Parse and set WantOptions from the JSObject of the given property name.
 * Get the native array of string from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param nativeValue Indicates the returned native value.
 *
 */
void GetWantOptions(napi_env env, napi_value jsObject, const char *propertyName, Want &want)
{
    napi_value jsOptionsObject = GetObjectByPropertyName(env, jsObject, propertyName);
    if (jsOptionsObject == nullptr) {
        return;
    }

    bool nativeValue = false;
    unsigned int flags = 0;
    std::map<std::string, unsigned int> flagMap;
    InitWantOptionsData(flagMap);
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        nativeValue = false;
        if (GetBooleanByPropertyName(env, jsOptionsObject, iter->first.c_str(), nativeValue)) {
            if (nativeValue) {
                flags |= iter->second;
            }
        }
    }

    want.SetFlags(flags);
}

/**
 * @brief Parse the want parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWant(Want &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // unwrap the param
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    if (valueType != napi_object) {
        return nullptr;
    }

    napi_value jsWant = nullptr;
    // unwrap the param : want object
    jsWant = GetObjectByPropertyName(env, args, "want");
    if (jsWant == nullptr) {
        return nullptr;
    }

    std::string strValue;
    // get want action property
    if (GetStringByPropertyName(env, jsWant, "action", strValue)) {
        param.SetAction(strValue);
    }
    // get want entities property
    std::vector<std::string> nativeStringList;
    GetStringArrayByPropertyName(env, jsWant, "entities", nativeStringList);
    for (size_t i = 0; i < nativeStringList.size(); i++) {
        param.AddEntity(nativeStringList[i]);
    }
    // get want type property
    if (GetStringByPropertyName(env, jsWant, "type", strValue)) {
        param.SetType(strValue);
    }
    // get want flags property(WantOptions in want.d.ts)
    GetWantOptions(env, jsWant, "options", param);
    // get want uri property
    if (GetStringByPropertyName(env, jsWant, "uri", strValue)) {
        param.SetUri(strValue);
    }
    // get elementName property
    UnwrapElementName(param, env, jsWant);

    // get want param (optional)
    UnwrapWantParam(param, env, jsWant);

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

napi_value WrapWant(const Want &param, napi_env env)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_value jsWant = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsWant));

    WrapElementName(param, env, jsWant);
    proValue = nullptr;
    HILOG_INFO("%{public}s, uri=%{public}s", __func__, param.GetUriString().c_str());
    if (napi_create_string_utf8(env, param.GetUriString().c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
        napi_set_named_property(env, jsWant, "uri", proValue);
    }

    proValue = nullptr;
    HILOG_INFO("%{public}s, type=%{public}s", __func__, param.GetType().c_str());
    if (napi_create_string_utf8(env, param.GetType().c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
        napi_set_named_property(env, jsWant, "type", proValue);
    }

    proValue = WrapWantOptions(param, env);
    if (proValue != nullptr) {
        napi_set_named_property(env, jsWant, "options", proValue);
    }

    proValue = nullptr;
    HILOG_INFO("%{public}s, action=%{public}s", __func__, param.GetAction().c_str());
    if (napi_create_string_utf8(env, param.GetAction().c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
        napi_set_named_property(env, jsWant, "action", proValue);
    }

    const AAFwk::WantParams nativeParams = param.GetParams();
    if (!nativeParams.IsEmpty()) {
        proValue = WrapWantParam(param, env);
        if (proValue != nullptr) {
            napi_set_named_property(env, jsWant, "parameters", proValue);
        }
    }

    proValue = WrapEntries(param, env);
    if (proValue != nullptr) {
        napi_set_named_property(env, jsWant, "entities", proValue);
    }
    return jsWant;
}

napi_value WrapEntries(const Want &want, napi_env env)
{
    napi_value jsEntries = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsEntries));
    const std::vector<std::string> nativeEntries = want.GetEntities();
    for (uint32_t i = 0; i < nativeEntries.size(); i++) {
        proValue = nullptr;
        if (napi_create_string_utf8(env, nativeEntries[i].c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
            napi_set_element(env, jsEntries, i, proValue);
        }
    }
    return jsEntries;
}

/**
 * @brief Parse the elementName parameters.
 *
 * @param param Indicates the elementName parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
void UnwrapElementName(Want &param, napi_env env, napi_value args)
{
    HILOG_INFO("%{public}s,called", __func__);
    // get elementName property
    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";

    GetStringByPropertyName(env, args, "deviceId", deviceId);
    GetStringByPropertyName(env, args, "bundleName", bundleName);
    GetStringByPropertyName(env, args, "abilityName", abilityName);
    param.SetElementName(deviceId, bundleName, abilityName);
}

napi_value WrapElementName(const Want &param, napi_env env, napi_value jsWant)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_value proValue = nullptr;
    OHOS::AppExecFwk::ElementName nativeElement = param.GetElement();

    proValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, nativeElement.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, jsWant, "deviceId", proValue));

    proValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, nativeElement.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, jsWant, "bundleName", proValue));

    proValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, nativeElement.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, jsWant, "abilityName", proValue));

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Parse the wantParam parameters.
 *
 * @param param Indicates the wantParam parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWantParam(Want &want, napi_env env, napi_value wantParam)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_value proNameList = 0;
    uint32_t proCount = 0;

    napi_value jsObject = GetJSValueByPropertyName(env, wantParam, "parameters", napi_object);
    if (jsObject == nullptr) {
        return nullptr;
    }

    NAPI_CALL(env, napi_get_property_names(env, jsObject, &proNameList));
    NAPI_CALL(env, napi_get_array_length(env, proNameList, &proCount));
    HILOG_INFO("UnwrapWantParam property size=%{public}d", proCount);

    napi_value proName = 0;
    napi_value proValue = 0;
    for (uint32_t index = 0; index < proCount; index++) {
        NAPI_CALL(env, napi_get_element(env, proNameList, index, &proName));
        std::string strProName = NapiValueToStringUtf8(env, proName);
        HILOG_INFO("UnwrapWantParam proName=%{public}s", strProName.c_str());
        NAPI_CALL(env, napi_get_named_property(env, jsObject, strProName.c_str(), &proValue));
        NAPI_CALL(env, napi_typeof(env, proValue, &valueType));

        switch (valueType) {
            case napi_string: {
                std::string str_pro_value = NapiValueToStringUtf8(env, proValue);
                HILOG_INFO("UnwrapWantParam proValue=%{public}s", str_pro_value.c_str());
                want.SetParam(strProName, str_pro_value);
                break;
            }
            case napi_boolean: {
                bool c_pro_value = false;
                NAPI_CALL(env, napi_get_value_bool(env, proValue, &c_pro_value));
                HILOG_INFO("UnwrapWantParam proValue=%{public}d", c_pro_value);
                want.SetParam(strProName, c_pro_value);
                break;
            }
            case napi_number: {
                int32_t c_pro_value32 = 0;
                double c_pro_value_double = 0.0;

                if (napi_get_value_int32(env, proValue, &c_pro_value32) == napi_ok) {
                    HILOG_INFO("UnwrapWantParam proValue=%{public}d", c_pro_value32);
                    want.SetParam(strProName, c_pro_value32);
                    break;
                }
                if (napi_get_value_double(env, proValue, &c_pro_value_double) == napi_ok) {
                    HILOG_INFO("UnwrapWantParam proValue=%{public}lf", c_pro_value_double);
                    want.SetParam(strProName, c_pro_value_double);
                    break;
                }
                HILOG_INFO("UnwrapWantParam unknown proValue of Number");
                break;
            }
            default: {
                if (UnwrapWantParamArray(want, env, strProName, proValue) == nullptr) {
                    return nullptr;
                }
            }
        }
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

void WrapWantParamArray(
    const Want &want, napi_env env, const std::string &key, sptr<AAFwk::IArray> &ao, napi_value jsObject)
{
    napi_value jsArray = nullptr;
    napi_value proValue = nullptr;
    if (AAFwk::Array::IsStringArray(ao)) {
        std::vector<std::string> nativeValue = want.GetStringArrayParam(key);
        if (napi_create_array(env, &jsArray) != napi_ok) {
            return;
        }
        for (uint32_t i = 0; i < nativeValue.size(); i++) {
            proValue = nullptr;
            if (napi_create_string_utf8(env, nativeValue[i].c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
                napi_set_element(env, jsArray, i, proValue);
            }
        }
        napi_set_named_property(env, jsObject, key.c_str(), jsArray);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        std::vector<bool> nativeValue = want.GetBoolArrayParam(key);
        if (napi_create_array(env, &jsArray) != napi_ok) {
            return;
        }
        for (uint32_t i = 0; i < nativeValue.size(); i++) {
            proValue = nullptr;
            if (napi_get_boolean(env, nativeValue[i], &proValue) == napi_ok) {
                napi_set_element(env, jsArray, i, proValue);
            }
        }
        napi_set_named_property(env, jsObject, key.c_str(), jsArray);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        std::vector<int> nativeValue = want.GetIntArrayParam(key);
        if (napi_create_array(env, &jsArray) != napi_ok) {
            return;
        }
        for (uint32_t i = 0; i < nativeValue.size(); i++) {
            proValue = nullptr;
            if (napi_create_int32(env, nativeValue[i], &proValue) == napi_ok) {
                napi_set_element(env, jsArray, i, proValue);
            }
        }
        napi_set_named_property(env, jsObject, key.c_str(), jsArray);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        std::vector<long> nativeValue = want.GetLongArrayParam(key);
        if (napi_create_array(env, &jsArray) != napi_ok) {
            return;
        }
        for (uint32_t i = 0; i < nativeValue.size(); i++) {
            proValue = nullptr;
            if (napi_create_int64(env, nativeValue[i], &proValue) == napi_ok) {
                napi_set_element(env, jsArray, i, proValue);
            }
        }
        napi_set_named_property(env, jsObject, key.c_str(), jsArray);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        std::vector<double> nativeValue = want.GetDoubleArrayParam(key);
        if (napi_create_array(env, &jsArray) != napi_ok) {
            return;
        }
        for (uint32_t i = 0; i < nativeValue.size(); i++) {
            proValue = nullptr;
            if (napi_create_double(env, nativeValue[i], &proValue) == napi_ok) {
                napi_set_element(env, jsArray, i, proValue);
            }
        }
        napi_set_named_property(env, jsObject, key.c_str(), jsArray);
    }
}

napi_value WrapWantParam(const Want &want, napi_env env)
{
    HILOG_INFO("%{public}s,called", __func__);
    napi_value jsParam = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsParam));
    const AAFwk::WantParams nativeParams = want.GetParams();
    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = nativeParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        proValue = nullptr;
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            std::string nativeValue = want.GetStringParam(iter->first);
            HILOG_INFO("%{public}s, %{public}s=%{public}s", __func__, iter->first.c_str(), nativeValue.c_str());
            if (napi_create_string_utf8(env, nativeValue.c_str(), NAPI_AUTO_LENGTH, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            bool natvieValue = want.GetBoolParam(iter->first, false);
            if (napi_get_boolean(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            short natvieValue = want.GetShortParam(iter->first, 0);
            if (napi_create_int32(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            int natvieValue = want.GetIntParam(iter->first, 0);
            if (napi_create_int32(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            long natvieValue = want.GetLongParam(iter->first, 0);
            if (napi_create_int64(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            float natvieValue = want.GetFloatParam(iter->first, 0.0);
            if (napi_create_double(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            double natvieValue = want.GetDoubleParam(iter->first, 0.0);
            if (napi_create_double(env, natvieValue, &proValue) == napi_ok) {
                napi_set_named_property(env, jsParam, iter->first.c_str(), proValue);
            }
        } else {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                WrapWantParamArray(want, env, iter->first, array, jsParam);
            }
        }
    }
    return jsParam;
}

/**
 * @brief Parse the wantParamArray parameters.
 *
 * @param param Indicates the wantParamArray parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWantParamArray(Want &want, napi_env env, std::string strProName, napi_value proValue)
{
    HILOG_INFO("%{public}s,called", __func__);
    bool isArray = false;
    uint32_t arrayLength = 0;
    napi_value valueAry = 0;
    napi_valuetype valueAryType = napi_undefined;

    NAPI_CALL(env, napi_is_array(env, proValue, &isArray));
    NAPI_CALL(env, napi_get_array_length(env, proValue, &arrayLength));
    HILOG_INFO("UnwrapWantParam proValue is array, length=%{public}d", arrayLength);

    std::vector<std::string> stringList;
    std::vector<int> intList;
    std::vector<long> longList;
    std::vector<bool> boolList;
    std::vector<double> doubleList;
    bool isDouble = false;
    for (uint32_t j = 0; j < arrayLength; j++) {
        NAPI_CALL(env, napi_get_element(env, proValue, j, &valueAry));
        NAPI_CALL(env, napi_typeof(env, valueAry, &valueAryType));
        switch (valueAryType) {
            case napi_string: {
                std::string str_ary_value = NapiValueToStringUtf8(env, valueAry);
                HILOG_INFO("UnwrapWantParam string array proValue=%{public}s", str_ary_value.c_str());
                stringList.push_back(str_ary_value);
                break;
            }
            case napi_boolean: {
                bool c_ary_value = false;
                NAPI_CALL(env, napi_get_value_bool(env, valueAry, &c_ary_value));
                HILOG_INFO("UnwrapWantParam bool array proValue=%{public}d", c_ary_value);
                boolList.push_back(c_ary_value);
                break;
            }
            case napi_number: {
                int32_t c_ary_value32 = 0;
                int64_t c_ary_value64 = 0;
                double c_ary_value_double = 0.0;
                if (isDouble) {
                    if (napi_get_value_double(env, valueAry, &c_ary_value_double) == napi_ok) {
                        HILOG_INFO("UnwrapWantParam double array proValue=%{public}lf", c_ary_value_double);
                        doubleList.push_back(c_ary_value_double);
                    }
                    break;
                } else {
                    if (napi_get_value_int32(env, valueAry, &c_ary_value32) == napi_ok) {
                        HILOG_INFO("UnwrapWantParam int array proValue=%{public}d", c_ary_value32);
                        intList.push_back(c_ary_value32);
                        break;
                    }
                }

                if (napi_get_value_int64(env, valueAry, &c_ary_value64) == napi_ok) {
                    HILOG_INFO("UnwrapWantParam int64 array proValue=%{public}lld", c_ary_value64);
                    longList.push_back(c_ary_value64);
                    break;
                }
                if (napi_get_value_double(env, valueAry, &c_ary_value_double) == napi_ok) {
                    HILOG_INFO("UnwrapWantParam double array proValue=%{public}lf", c_ary_value_double);
                    isDouble = true;
                    if (intList.size() > 0) {
                        for (int k = 0; k < (int)intList.size(); k++) {
                            doubleList.push_back(intList[k]);
                        }
                        intList.clear();
                    }
                    doubleList.push_back(c_ary_value_double);
                    break;
                }
                HILOG_INFO("UnwrapWantParam array unkown Number");
                break;
            }
            default:
                HILOG_INFO("UnwrapWantParam array unkown");
                break;
        }
    }
    if (stringList.size() > 0) {
        want.SetParam(strProName, stringList);
    }
    if (intList.size() > 0) {
        want.SetParam(strProName, intList);
    }
    if (longList.size() > 0) {
        want.SetParam(strProName, longList);
    }
    if (boolList.size() > 0) {
        want.SetParam(strProName, boolList);
    }
    if (doubleList.size() > 0) {
        want.SetParam(strProName, doubleList);
    }
    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    return result;
}

/**
 * @brief Init param of wantOptions.
 *
 * @param flagMap Indicates flag of list in Want .
 */
void InitWantOptionsData(std::map<std::string, unsigned int> &flagMap)
{
    flagMap.emplace("authReadUriPermission", Want::FLAG_AUTH_READ_URI_PERMISSION);
    flagMap.emplace("authWriteUriPermission", Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    flagMap.emplace("abilityForwardResult", Want::FLAG_ABILITY_FORWARD_RESULT);
    flagMap.emplace("abilityContinuation", Want::FLAG_ABILITY_CONTINUATION);
    flagMap.emplace("notOhosComponent", Want::FLAG_NOT_OHOS_COMPONENT);
    flagMap.emplace("abilityFormEnabled", Want::FLAG_ABILITY_FORM_ENABLED);
    flagMap.emplace("authPersistableUriPermission", Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
    flagMap.emplace("authPrefixUriPermission", Want::FLAG_AUTH_PREFIX_URI_PERMISSION);
    flagMap.emplace("abilitySliceMultiDevice", Want::FLAG_ABILITYSLICE_MULTI_DEVICE);
    flagMap.emplace("startForegroundAbility", Want::FLAG_START_FOREGROUND_ABILITY);
    flagMap.emplace("installOnDemand", Want::FLAG_INSTALL_ON_DEMAND);
    flagMap.emplace("abilitySliceForwardResult", Want::FLAG_ABILITYSLICE_FORWARD_RESULT);
    flagMap.emplace("installWithBackgroundMode", Want::FLAG_INSTALL_WITH_BACKGROUND_MODE);
}

/**
 * @brief Wrap the wantOptions parameters.
 *
 * @param param Indicates the wantOptions parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value WrapWantOptions(const Want &want, napi_env env)
{
    HILOG_INFO("%{public}s,call", __func__);
    napi_value wantOptions = 0;
    std::map<std::string, unsigned int> flagMap;
    InitWantOptionsData(flagMap);
    unsigned int flags = want.GetFlags();
    bool blValue = false;
    napi_value proValue = 0;
    NAPI_CALL(env, napi_create_object(env, &wantOptions));
    for (auto iter = flagMap.begin(); iter != flagMap.end(); iter++) {
        blValue = ((flags & iter->second) == iter->second);
        NAPI_CALL(env, napi_get_boolean(env, blValue, &proValue));
        HILOG_INFO("%{public}s, %{public}s=%{public}s", __func__, iter->first.c_str(), blValue ? "true" : "false");
        NAPI_CALL(env, napi_set_named_property(env, wantOptions, iter->first.c_str(), proValue));
    }

    return wantOptions;
}
}  // namespace AppExecFwk
}  // namespace OHOS