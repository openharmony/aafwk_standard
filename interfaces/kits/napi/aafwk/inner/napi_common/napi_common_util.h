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

#ifndef OHOS_APPEXECFWK_NAPI_COMMON_UTIL_H
#define OHOS_APPEXECFWK_NAPI_COMMON_UTIL_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_common_data.h"

namespace OHOS {
namespace AppExecFwk {

static constexpr int32_t DEFAULT_BUF_SIZE = 1024;
static constexpr int32_t ASYNC_RST_SIZE = 2;

bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType);
bool IsArrayForNapiValue(napi_env env, napi_value param, uint32_t &arraySize);

napi_value WrapVoidToJS(napi_env env);
napi_value WrapUndefinedToJS(napi_env env);

napi_value CreateJSObject(napi_env env);

napi_value WrapInt32ToJS(napi_env env, int32_t value);
int UnwrapInt32FromJS(napi_env env, napi_value param, int defaultValue = 0);
bool UnwrapInt32FromJS2(napi_env env, napi_value param, int &value);

napi_value WrapLongToJS(napi_env env, long value);
long UnwrapLongFromJS(napi_env env, napi_value param, long defaultValue = 0);
bool UnwrapLongFromJS2(napi_env env, napi_value param, long &value);

napi_value WrapInt64ToJS(napi_env env, int64_t value);
int64_t UnwrapInt64FromJS(napi_env env, napi_value param, int64_t defaultValue = 0);
bool UnwrapInt64FromJS2(napi_env env, napi_value param, int64_t &value);

napi_value WrapBoolToJS(napi_env env, bool value);
bool UnWrapBoolFromJS(napi_env env, napi_value param, bool defaultValue = false);
bool UnwrapBoolFromJS2(napi_env env, napi_value param, bool &value);

napi_value WrapDoubleToJS(napi_env env, double value);
double UnWrapDoubleFromJS(napi_env env, napi_value param, double defaultValue = 0.0);
bool UnWrapDoubleFromJS2(napi_env env, napi_value param, double &value);

napi_value WrapStringToJS(napi_env env, const std::string &value);
std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string &defaultValue = "");
bool UnwrapStringFromJS2(napi_env env, napi_value param, std::string &value);

napi_value WrapArrayInt32ToJS(napi_env env, const std::vector<int> &value);
bool UnwrapArrayInt32FromJS(napi_env env, napi_value param, std::vector<int> &value);

napi_value WrapArrayLongToJS(napi_env env, const std::vector<long> &value);
bool UnwrapArrayLongFromJS(napi_env env, napi_value param, std::vector<long> &value);

napi_value WrapArrayInt64ToJS(napi_env env, const std::vector<int64_t> &value);
bool UnwrapArrayInt64FromJS(napi_env env, napi_value param, std::vector<int64_t> &value);

napi_value WrapArrayDoubleToJS(napi_env env, const std::vector<double> &value);
bool UnwrapArrayDoubleFromJS(napi_env env, napi_value param, std::vector<double> &value);

napi_value WrapArrayBoolToJS(napi_env env, const std::vector<bool> &value);
bool UnwrapArrayBoolFromJS(napi_env env, napi_value param, std::vector<bool> &value);

napi_value WrapArrayStringToJS(napi_env env, const std::vector<std::string> &value);
bool UnwrapArrayStringFromJS(napi_env env, napi_value param, std::vector<std::string> &value);

bool UnwrapArrayComplexFromJS(napi_env env, napi_value param, ComplexArrayData &value);

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
 * @return Return the property value of the specified property name int jsObject on success, otherwise return nullptr.
 */
napi_value GetPropertyValueByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, napi_valuetype expectType);

bool SetPropertyValueByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, napi_value value);

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
bool UnwrapInt32ByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, int32_t &value);

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
bool UnwrapDoubleByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, double &value);

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
bool UnwrapBooleanByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, bool &value);
bool UnwrapBooleanArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<bool> &value);

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
bool UnwrapStringByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, std::string &value);
bool UnwrapStringArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, std::vector<std::string> &value);

bool UnwrapComplexArrayByPropertyName(
    napi_env env, napi_value jsObject, const char *propertyName, ComplexArrayData &value);

void ClearThreadReturnData(ThreadReturnData *data);

napi_value GetCallbackErrorValue(napi_env env, int errCode);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncJSCallbackInfo on success, nullptr on failure
 */
AsyncJSCallbackInfo *CreateAsyncJSCallbackInfo(napi_env env);
void FreeAsyncJSCallbackInfo(AsyncJSCallbackInfo **asyncCallbackInfo);

/**
 * @brief Convert local data to JS data.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data The local data.
 * @param value the JS data.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
bool WrapThreadReturnData(napi_env env, const ThreadReturnData *data, napi_value *value);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param param Parameter list.
 * @param callback Point to asynchronous processing of data.
 *
 * @return Return true successfully, otherwise return false.
 */
bool CreateAsyncCallback(napi_env env, napi_value param, AsyncJSCallbackInfo *callback);

napi_ref CreateCallbackRefFromJS(napi_env env, napi_value param);

/**
 * @brief Asynchronous callback processing.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 * @param param other param.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecuteAsyncCallbackWork(napi_env env, AsyncJSCallbackInfo *asyncCallbackInfo, const AsyncParamEx *param);

/**
 * @brief Asynchronous promise processing.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 * @param param other param.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecutePromiseCallbackWork(napi_env env, AsyncJSCallbackInfo *asyncCallbackInfo, const AsyncParamEx *param);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompleteAsyncCallbackWork(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompleteAsyncVoidCallbackWork(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompletePromiseCallbackWork(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void CompletePromiseVoidCallbackWork(napi_env env, napi_status status, void *data);

std::vector<uint8_t> ConvertU8Vector(napi_env env, napi_value jsValue);

std::vector<std::string> ConvertStringVector(napi_env env, napi_value jsValue);

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APPEXECFWK_NAPI_COMMON_UTIL_H
