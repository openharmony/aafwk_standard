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
#ifndef OHOS_APPEXECFWK_WANT_WRAPPER_H
#define OHOS_APPEXECFWK_WANT_WRAPPER_H
#include "feature_ability_common.h"

using Want = OHOS::AAFwk::Want;
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
bool IsExistsByPropertyName(napi_env env, napi_value jsObject, const char *propertyName);

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
    napi_env env, napi_value jsObject, const char *propertyName, napi_valuetype expectType);

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
bool GetStringByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, std::string &nativeValue);

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
bool GetInt32ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int32_t &nativeValue);

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
bool GetInt64ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int64_t &nativeValue);

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
bool GetDoubleByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, double &nativeValue);

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
bool GetBooleanByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, bool &nativeValue);

/**
 * @brief Get the JSObject from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 *
 * @return Return value of JSObject if successful, else return nullptr.
 */
napi_value GetObjectByPropertyName(napi_env env, napi_value jsObject, const char *propertyName);

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
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<std::string> &nativeValue);

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
void GetWantOptions(napi_env env, napi_value jsObject, const char *propertyName, Want &want);

/**
 * @brief Parse the want parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWant(Want &param, napi_env env, napi_value args);
napi_value WrapWant(const Want &param, napi_env env);
/**
 * @brief Parse the elementName parameters.
 *
 * @param param Indicates the elementName parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
void UnwrapElementName(Want &param, napi_env env, napi_value args);
napi_value WrapElementName(const Want &param, napi_env env, napi_value jsWant);

/**
 * @brief Parse the wantParam parameters.
 *
 * @param param Indicates the wantParam parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWantParam(Want &want, napi_env env, napi_value wantParam);
napi_value WrapWantParam(const Want &want, napi_env env);
napi_value WrapEntries(const Want &want, napi_env env);

/**
 * @brief Parse the wantParamArray parameters.
 *
 * @param param Indicates the wantParamArray parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapWantParamArray(Want &want, napi_env env, std::string strProName, napi_value wantParam);

/**
 * @brief Init param of wantOptions.
 *
 * @param flagMap Indicates flag of list in Want .
 */
void InitWantOptionsData(std::map<std::string, unsigned int> &flagMap);

/**
 * @brief Wrap the wantOptions parameters.
 *
 * @param param Indicates the wantOptions parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value WrapWantOptions(const Want &want, napi_env env);

}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_WANT_WRAPPER_H */
