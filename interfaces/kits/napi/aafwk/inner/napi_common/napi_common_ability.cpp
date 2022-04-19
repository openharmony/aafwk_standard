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

#include "napi_common_ability.h"

#include <dlfcn.h>
#include <uv.h>

#include "hilog_wrapper.h"
#include "napi_common_util.h"
#include "napi_base_context.h"
#include "napi_remote_object.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {

napi_ref thread_local g_dataAbilityHelper;
bool thread_local g_dataAbilityHelperStatus = false;

using NAPICreateJsRemoteObject = napi_value (*)(napi_env env, const sptr<IRemoteObject> target);

napi_value *GetGlobalClassContext(void)
{
    return AbilityRuntime::GetFAModeContextClassObject();
}

napi_status SaveGlobalDataAbilityHelper(napi_env env, napi_value constructor)
{
    return napi_create_reference(env, constructor, 1, &g_dataAbilityHelper);
}

napi_value GetGlobalDataAbilityHelper(napi_env env)
{
    napi_value constructor;
    NAPI_CALL(env, napi_get_reference_value(env, g_dataAbilityHelper, &constructor));
    return constructor;
}

bool& GetDataAbilityHelperStatus()
{
    return g_dataAbilityHelperStatus;
}

bool CheckAbilityType(AbilityType typeInAbility, AbilityType typeWant)
{
    switch (typeWant) {
        case AbilityType::PAGE:
            if (typeInAbility == AbilityType::PAGE || typeInAbility == AbilityType::DATA) {
                return true;
            }
            return false;
        default:
            return typeInAbility != AbilityType::PAGE;
    }
    return false;
}

bool CheckAbilityType(const CBBase *cbBase)
{
    if (cbBase == nullptr) {
        HILOG_ERROR("%{public}s cbBase == nullptr", __func__);
        return false;
    }

    if (cbBase->ability == nullptr) {
        HILOG_ERROR("%{public}s cbBase->ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = cbBase->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }
    return CheckAbilityType((AbilityType)info->type, cbBase->abilityType);
}

bool CheckAbilityType(const AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

bool CheckAbilityType(const AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }

    HILOG_INFO("%{public}s end.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

void SaveAppInfo(AppInfo_ &appInfo, const ApplicationInfo &appInfoOrg)
{
    appInfo.name = appInfoOrg.name;
    appInfo.description = appInfoOrg.description;
    appInfo.descriptionId = appInfoOrg.descriptionId;
    appInfo.systemApp = appInfoOrg.isSystemApp;
    appInfo.enabled = appInfoOrg.enabled;
    appInfo.label = appInfoOrg.label;
    appInfo.labelId = std::to_string(appInfoOrg.labelId);
    appInfo.icon = appInfoOrg.iconPath;
    appInfo.iconId = std::to_string(appInfoOrg.iconId);
    appInfo.process = appInfoOrg.process;
    appInfo.supportedModes = appInfoOrg.supportedModes;
    for (size_t i = 0; i < appInfoOrg.moduleInfos.size(); i++) {
        appInfo.moduleSourceDirs.emplace_back(appInfoOrg.moduleInfos.at(i).moduleSourceDir);
    }
    for (size_t i = 0; i < appInfoOrg.permissions.size(); i++) {
        appInfo.permissions.emplace_back(appInfoOrg.permissions.at(i));
    }
    for (size_t i = 0; i < appInfoOrg.moduleInfos.size(); i++) {
        appInfo.moduleInfos.emplace_back(appInfoOrg.moduleInfos.at(i));
    }
    appInfo.entryDir = appInfoOrg.entryDir;
}

napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_value result = nullptr;

    // reversible?: boolean
    if (GetContinueAbilityOptionsReversible(env, value, info) == nullptr) {
        return nullptr;
    }

    // deviceId?: string
    if (GetContinueAbilityOptionsDeviceID(env, value, info) == nullptr) {
        return nullptr;
    }

    napi_get_null(env, &result);
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool reversible = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "reversible", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "reversible", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            HILOG_ERROR("%{public}s, Wrong argument type. Bool expected.", __func__);
            return nullptr;
        }
        napi_get_value_bool(env, result, &reversible);
        info.reversible = reversible;
    }
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsDeviceID(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "deviceId", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "deviceId", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            HILOG_ERROR("%{public}s, Wrong argument type. String expected.", __func__);
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        info.deviceId = str;
    }
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value WrapAppInfo(napi_env env, const AppInfo_ &appInfo)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_int32(env, appInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_get_boolean(env, appInfo.systemApp, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "systemApp", proValue));
    NAPI_CALL(env, napi_get_boolean(env, appInfo.enabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "enabled", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.labelId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.iconId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));
    NAPI_CALL(env, napi_create_int32(env, appInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));
    napi_value jsArrayModDirs = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayModDirs));
    for (size_t i = 0; i < appInfo.moduleSourceDirs.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, appInfo.moduleSourceDirs.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayModDirs, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleSourceDirs", jsArrayModDirs));
    napi_value jsArrayPermissions = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayPermissions));
    for (size_t i = 0; i < appInfo.permissions.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, appInfo.permissions.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayPermissions, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "permissions", jsArrayPermissions));
    napi_value jsArrayModuleInfo = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayModuleInfo));
    for (size_t i = 0; i < appInfo.moduleInfos.size(); i++) {
        napi_value jsModuleInfoObject = nullptr;
        NAPI_CALL(env, napi_create_object(env, &jsModuleInfoObject));
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, appInfo.moduleInfos.at(i).moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleName", proValue));
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, appInfo.moduleInfos.at(i).moduleSourceDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleSourceDir", proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayModuleInfo, i, jsModuleInfoObject));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleInfos", jsArrayModuleInfo));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.entryDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "entryDir", proValue));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

/**
 * @brief GetFilesDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetFilesDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetFilesDir();
    HILOG_INFO("%{public}s end. filesDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

void IsUpdatingConfigurationsExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->IsUpdatingConfigurations();
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief PrintDrawnCompleted asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void PrintDrawnCompletedExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->PrintDrawnCompleted();
    HILOG_INFO("%{public}s end", __func__);
}

napi_value NAPI_GetFilesDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}
napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetFilesDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

void GetOrCreateDistributedDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetDistributedFilesDir();
    HILOG_INFO("%{public}s end. filesDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief GetFilesDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetOrCreateDistributedDirWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_ERROR("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetOrCreateDistributedDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCacheDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetCacheDir();
    HILOG_INFO("%{public}s end. CacheDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief NAPI_GetCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_ERROR("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirCallback";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirPromise";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetExternalCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetExternalCacheDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetExternalCacheDir();
    HILOG_INFO(
        "%{public}s end. ExternalCacheDir=%{private}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief NAPI_GetExternalCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetExternalCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirCallback";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirPromise";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetExternalCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value NAPI_IsUpdatingConfigurationsWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsCallback";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsPromise";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_PrintDrawnCompletedWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedCallback";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompleteAsyncVoidCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedPromise";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompletePromiseVoidCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_IsUpdatingConfigurationsWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_PrintDrawnCompletedWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppTypeCB on success, nullptr on failure.
 */
AppTypeCB *CreateAppTypeCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppTypeCB *appTypeCB = new (std::nothrow) AppTypeCB;
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }
    appTypeCB->cbBase.cbInfo.env = env;
    appTypeCB->cbBase.asyncWork = nullptr;
    appTypeCB->cbBase.deferred = nullptr;
    appTypeCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return appTypeCB;
}

/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo,appTypeCB == nullptr");
        return;
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appTypeCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo,ability == nullptr");
        appTypeCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appTypeCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo,wrong ability type");
        appTypeCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    appTypeCB->name = appTypeCB->cbBase.ability->GetAppType();
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));

    result[PARAM0] = GetCallbackErrorValue(env, appTypeCB->cbBase.errCode);
    if (appTypeCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(
                env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appTypeCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appTypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appTypeCB->cbBase.asyncWork));
    delete appTypeCB;
    appTypeCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("GetAppTypePromiseCompleteCB, main event thread complete.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    napi_value result = nullptr;
    if (appTypeCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        napi_create_string_utf8(env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result);
        napi_resolve_deferred(env, appTypeCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appTypeCB->cbBase.errCode);
        napi_reject_deferred(env, appTypeCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appTypeCB->cbBase.asyncWork);
    delete appTypeCB;
    appTypeCB = nullptr;
    HILOG_INFO("GetAppTypePromiseCompleteCB, main event thread complete end.");
}

/**
 * @brief GetAppType Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeAsync(napi_env env, napi_value *args, const size_t argCallback, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypeAsyncCompleteCB,
            (void *)appTypeCB,
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAppType Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypePromise(napi_env env, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appTypeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypePromiseCompleteCB,
            (void *)appTypeCB,
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAppType processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeWrap(napi_env env, napi_callback_info info, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppTypeAsync(env, args, 0, appTypeCB);
    } else {
        ret = GetAppTypePromise(env, appTypeCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppTypeCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AppTypeCB *appTypeCB = CreateAppTypeCBInfo(env);
    if (appTypeCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appTypeCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppTypeWrap(env, info, appTypeCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (appTypeCB != nullptr) {
            delete appTypeCB;
            appTypeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return ret;
}

#ifdef SUPPORT_GRAPHICS
napi_value GetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamGetDisplayOrientationWrap(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamGetDisplayOrientationWrap fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapCallback";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapPromise";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void GetDisplayOrientationExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->GetDisplayOrientation();
    HILOG_INFO("%{public}s end.", __func__);
}

bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 1;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_ERROR("%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s, the first parameter is invalid.", __func__);
            return false;
        }
    }

    return true;
}

napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetDisplayOrientationWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_DEBUG("%{public}s, end.", __func__);
    return ret;
}
#endif

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityInfoCB on success, nullptr on failure.
 */
AbilityInfoCB *CreateAbilityInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AbilityInfoCB *abilityInfoCB = new (std::nothrow) AbilityInfoCB;
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }
    abilityInfoCB->cbBase.cbInfo.env = env;
    abilityInfoCB->cbBase.asyncWork = nullptr;
    abilityInfoCB->cbBase.deferred = nullptr;
    abilityInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return abilityInfoCB;
}

void SaveAbilityInfo(AbilityInfo_ &abilityInfo, const AbilityInfo &abilityInfoOrg)
{
    HILOG_INFO("%{public}s called.", __func__);
    abilityInfo.bundleName = abilityInfoOrg.bundleName;
    abilityInfo.name = abilityInfoOrg.name;
    abilityInfo.label = abilityInfoOrg.label;
    abilityInfo.description = abilityInfoOrg.description;
    abilityInfo.icon = abilityInfoOrg.iconPath;
    abilityInfo.labelId = abilityInfoOrg.applicationInfo.labelId;
    abilityInfo.descriptionId = abilityInfoOrg.applicationInfo.descriptionId;
    abilityInfo.iconId = abilityInfoOrg.applicationInfo.iconId;
    abilityInfo.moduleName = abilityInfoOrg.moduleName;
    abilityInfo.process = abilityInfoOrg.process;
    abilityInfo.isVisible = abilityInfoOrg.visible;

    abilityInfo.type = static_cast<int32_t>(abilityInfoOrg.type);
    abilityInfo.orientation = static_cast<int32_t>(abilityInfoOrg.orientation);
    abilityInfo.launchMode = static_cast<int32_t>(abilityInfoOrg.launchMode);

    abilityInfo.uri = abilityInfoOrg.uri;
    abilityInfo.targetAbility = abilityInfoOrg.targetAbility;

    for (size_t i = 0; i < abilityInfoOrg.permissions.size(); i++) {
        abilityInfo.permissions.emplace_back(abilityInfoOrg.permissions.at(i));
    }
    for (size_t i = 0; i < abilityInfoOrg.deviceTypes.size(); i++) {
        abilityInfo.deviceTypes.emplace_back(abilityInfoOrg.deviceTypes.at(i));
    }
    for (size_t i = 0; i < abilityInfoOrg.deviceCapabilities.size(); i++) {
        abilityInfo.deviceCapabilities.emplace_back(abilityInfoOrg.deviceCapabilities.at(i));
    }

    SaveAppInfo(abilityInfo.appInfo, abilityInfoOrg.applicationInfo);

    abilityInfo.readPermission = abilityInfoOrg.readPermission;
    abilityInfo.writePermission = abilityInfoOrg.writePermission;
    abilityInfo.formEntity = 0;         // no data
    abilityInfo.minFormHeight = 0;      // no data
    abilityInfo.defaultFormHeight = 0;  // no data
    abilityInfo.minFormWidth = 0;       // no data
    abilityInfo.defaultFormWidth = 0;   // no data
    abilityInfo.backgroundModes = 0;    // no data
    abilityInfo.subType = 0;            // no data
    abilityInfo.formEnabled = false;    // no data
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo_ &abilityInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.type), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "type", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "orientation", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "launchMode", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.readPermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "readPermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.writePermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "writePermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.targetAbility.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "targetAbility", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.formEntity, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEntity", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.backgroundModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.subType, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "subType", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.isVisible, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isVisible", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.formEnabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEnabled", proValue));

    napi_value jsArrayPermissions = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayPermissions));
    for (size_t i = 0; i < abilityInfo.permissions.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, abilityInfo.permissions.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayPermissions, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "permissions", jsArrayPermissions));

    napi_value jsArrayDeviceCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayDeviceCapabilities));
    for (size_t i = 0; i < abilityInfo.deviceCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, abilityInfo.deviceCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayDeviceCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceCapabilities", jsArrayDeviceCapabilities));

    napi_value jsArrayDeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayDeviceTypes));
    for (size_t i = 0; i < abilityInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(
            env, napi_create_string_utf8(env, abilityInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayDeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArrayDeviceTypes));

    napi_value appInfo = nullptr;
    appInfo = WrapAppInfo(env, abilityInfo.appInfo);
    NAPI_CALL(env, napi_set_named_property(env, result, "applicationInfo", appInfo));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

napi_value ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    AbilityInfo_ temp;
    SaveAbilityInfo(temp, abilityInfo);
    return WrapAbilityInfo(env, temp);
}

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, abilityInfoCB == nullptr");
        return;
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo,wrong ability type");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfoPtr = abilityInfoCB->cbBase.ability->GetAbilityInfo();
    if (abilityInfoPtr != nullptr) {
        SaveAbilityInfo(abilityInfoCB->abilityInfo, *abilityInfoPtr);
    } else {
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, abilityInfoCB->cbBase.errCode);
    if (abilityInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (abilityInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork));
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    napi_value result = nullptr;
    if (abilityInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
        napi_resolve_deferred(env, abilityInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, abilityInfoCB->cbBase.errCode);
        napi_reject_deferred(env, abilityInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork);
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief GetAbilityInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoAsyncCompleteCB,
            (void *)abilityInfoCB,
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAbilityInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoPromise(napi_env env, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoPromiseCompleteCB,
            (void *)abilityInfoCB,
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAbilityInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoWrap(napi_env env, napi_callback_info info, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityInfoAsync(env, args, 0, abilityInfoCB);
    } else {
        ret = GetAbilityInfoPromise(env, abilityInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AbilityInfoCB *abilityInfoCB = CreateAbilityInfoCBInfo(env);
    if (abilityInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    abilityInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityInfoWrap(env, info, abilityInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (abilityInfoCB != nullptr) {
            delete abilityInfoCB;
            abilityInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to HapModuleInfoCB on success, nullptr on failure.
 */
HapModuleInfoCB *CreateHapModuleInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    HapModuleInfoCB *hapModuleInfoCB = new (std::nothrow) HapModuleInfoCB;
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }
    hapModuleInfoCB->cbBase.cbInfo.env = env;
    hapModuleInfoCB->cbBase.asyncWork = nullptr;
    hapModuleInfoCB->cbBase.deferred = nullptr;
    hapModuleInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return hapModuleInfoCB;
}

void SaveHapModuleInfo(HapModuleInfo_ &hapModuleInfo, const HapModuleInfo &hapModuleInfoOrg)
{
    HILOG_INFO("%{public}s called.", __func__);
    hapModuleInfo.name = hapModuleInfoOrg.name;
    hapModuleInfo.description = hapModuleInfoOrg.description;
    hapModuleInfo.icon = hapModuleInfoOrg.iconPath;
    hapModuleInfo.label = hapModuleInfoOrg.label;
    hapModuleInfo.backgroundImg = hapModuleInfoOrg.backgroundImg;
    hapModuleInfo.moduleName = hapModuleInfoOrg.moduleName;
    hapModuleInfo.supportedModes = hapModuleInfoOrg.supportedModes;
    hapModuleInfo.descriptionId = 0;         // no data
    hapModuleInfo.labelId = 0;               // no data
    hapModuleInfo.iconId = 0;                // no data
    hapModuleInfo.mainAbilityName = "";      // no data
    hapModuleInfo.installationFree = false;  // no data

    for (size_t i = 0; i < hapModuleInfoOrg.reqCapabilities.size(); i++) {
        hapModuleInfo.reqCapabilities.emplace_back(hapModuleInfoOrg.reqCapabilities.at(i));
    }

    for (size_t i = 0; i < hapModuleInfoOrg.deviceTypes.size(); i++) {
        hapModuleInfo.deviceTypes.emplace_back(hapModuleInfoOrg.deviceTypes.at(i));
    }

    for (size_t i = 0; i < hapModuleInfoOrg.abilityInfos.size(); i++) {
        AbilityInfo_ abilityInfo;
        SaveAbilityInfo(abilityInfo, hapModuleInfoOrg.abilityInfos.at(i));
        hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
    }
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &hapModuleInfoCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.icon.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.backgroundImg.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundImg", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(
            env, hapModuleInfoCB.hapModuleInfo.mainAbilityName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "mainAbilityName", proValue));

    NAPI_CALL(env, napi_get_boolean(env, hapModuleInfoCB.hapModuleInfo.installationFree, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "installationFree", proValue));

    napi_value jsArrayreqCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayreqCapabilities));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.reqCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.reqCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayreqCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "reqCapabilities", jsArrayreqCapabilities));

    napi_value jsArraydeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArraydeviceTypes));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArraydeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArraydeviceTypes));

    napi_value abilityInfos = nullptr;
    NAPI_CALL(env, napi_create_array(env, &abilityInfos));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.abilityInfos.size(); i++) {
        napi_value abilityInfo = nullptr;
        abilityInfo = WrapAbilityInfo(env, hapModuleInfoCB.hapModuleInfo.abilityInfos.at(i));
        NAPI_CALL(env, napi_set_element(env, abilityInfos, i, abilityInfo));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityInfos", abilityInfos));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

void GetHapModuleInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, worker pool thread execute.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetHapModuleInfo, hapModuleInfoCB == nullptr");
        return;
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (hapModuleInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetHapModuleInfo, ability == nullptr");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&hapModuleInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetHapModuleInfo,wrong ability type");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<HapModuleInfo> hapModuleInfoPtr = hapModuleInfoCB->cbBase.ability->GetHapModuleInfo();
    if (hapModuleInfoPtr != nullptr) {
        SaveHapModuleInfo(hapModuleInfoCB->hapModuleInfo, *hapModuleInfoPtr);
    } else {
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetHapModuleInfo, worker pool thread execute end.");
}

void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, hapModuleInfoCB->cbBase.errCode);
    if (hapModuleInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapHapModuleInfo(env, *hapModuleInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, hapModuleInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (hapModuleInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete end.");
}

void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    napi_value result = nullptr;
    if (hapModuleInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapHapModuleInfo(env, *hapModuleInfoCB);
        napi_resolve_deferred(env, hapModuleInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, hapModuleInfoCB->cbBase.errCode);
        napi_reject_deferred(env, hapModuleInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork);
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete end.");
}

/**
 * @brief GetHapModuleInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoAsync(
    napi_env env, napi_value *args, const size_t argCallback, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoAsyncCompleteCB,
            (void *)hapModuleInfoCB,
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetHapModuleInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoPromise(napi_env env, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    hapModuleInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoPromiseCompleteCB,
            (void *)hapModuleInfoCB,
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetHapModuleInfoAsync(env, args, 0, hapModuleInfoCB);
    } else {
        ret = GetHapModuleInfoPromise(env, hapModuleInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    HapModuleInfoCB *hapModuleInfoCB = CreateHapModuleInfoCBInfo(env);
    if (hapModuleInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    hapModuleInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetHapModuleInfoWrap(env, info, hapModuleInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (hapModuleInfoCB != nullptr) {
            delete hapModuleInfoCB;
            hapModuleInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppVersionInfoCB on success, nullptr on failure.
 */
AppVersionInfoCB *CreateAppVersionInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppVersionInfoCB *appVersionInfoCB = new (std::nothrow) AppVersionInfoCB;
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appVersionInfoCB->cbBase.cbInfo.env = env;
    appVersionInfoCB->cbBase.asyncWork = nullptr;
    appVersionInfoCB->cbBase.deferred = nullptr;
    appVersionInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return appVersionInfoCB;
}

void SaveAppVersionInfo(AppVersionInfo &appVersionInfo, const std::string appName, const std::string versionName,
    const int32_t versionCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    appVersionInfo.appName = appName;
    appVersionInfo.versionName = versionName;
    appVersionInfo.versionCode = versionCode;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value WrapAppVersionInfo(napi_env env, const AppVersionInfoCB &appVersionInfoCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env,
        napi_create_string_utf8(env, appVersionInfoCB.appVersionInfo.appName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "appName", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, appVersionInfoCB.appVersionInfo.versionName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "versionName", proValue));

    NAPI_CALL(env, napi_create_int32(env, appVersionInfoCB.appVersionInfo.versionCode, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "versionCode", proValue));

    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

void GetAppVersionInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, worker pool thread execute.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetAppVersionInfo, appVersionInfoCB == nullptr");
        return;
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appVersionInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetAppVersionInfo, ability == nullptr");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appVersionInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetAppVersionInfo,wrong ability type");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfoPtr = appVersionInfoCB->cbBase.ability->GetAbilityInfo();
    std::shared_ptr<ApplicationInfo> appInfoPtr = appVersionInfoCB->cbBase.ability->GetApplicationInfo();
    if (abilityInfoPtr != nullptr && appInfoPtr != nullptr) {
        SaveAppVersionInfo(appVersionInfoCB->appVersionInfo, abilityInfoPtr->appName, appInfoPtr->versionName,
            appInfoPtr->versionCode);
    } else {
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetAppVersionInfo, worker pool thread execute end.");
}

void GetAppVersionInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, appVersionInfoCB->cbBase.errCode);
    if (appVersionInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAppVersionInfo(env, *appVersionInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appVersionInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appVersionInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appVersionInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    delete appVersionInfoCB;
    appVersionInfoCB = nullptr;
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete end.");
}

void GetAppVersionInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    napi_value result = nullptr;
    if (appVersionInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAppVersionInfo(env, *appVersionInfoCB);
        napi_resolve_deferred(env, appVersionInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appVersionInfoCB->cbBase.errCode);
        napi_reject_deferred(env, appVersionInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appVersionInfoCB->cbBase.asyncWork);
    delete appVersionInfoCB;
    appVersionInfoCB = nullptr;
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete end.");
}

/**
 * @brief GetAppVersionInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param AppVersionInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppVersionInfoAsync(
    napi_env env, napi_value *args, const size_t argCallback, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appVersionInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(
        env, napi_create_async_work(env, nullptr, resourceName, GetAppVersionInfoExecuteCB,
                 GetAppVersionInfoAsyncCompleteCB, (void *)appVersionInfoCB, &appVersionInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAppVersionInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param AppVersionInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppVersionInfoPromise(napi_env env, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appVersionInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(
        env, napi_create_async_work(env, nullptr, resourceName, GetAppVersionInfoExecuteCB,
                 GetAppVersionInfoPromiseCompleteCB, (void *)appVersionInfoCB, &appVersionInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetAppVersionInfoWrap(napi_env env, napi_callback_info info, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppVersionInfoAsync(env, args, 0, appVersionInfoCB);
    } else {
        ret = GetAppVersionInfoPromise(env, appVersionInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AppVersionInfoCB *appVersionInfoCB = CreateAppVersionInfoCBInfo(env);
    if (appVersionInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appVersionInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppVersionInfoWrap(env, info, appVersionInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (appVersionInfoCB != nullptr) {
            delete appVersionInfoCB;
            appVersionInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (env == nullptr) {
        HILOG_INFO("%{public}s env == nullptr.", __func__);
        return nullptr;
    }

    napi_status ret;
    napi_value global = 0;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_global=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    napi_value abilityObj = 0;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_named_property=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    Ability *ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_value_external=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return nullptr;
    }
    asyncCallbackInfo->cbInfo.env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->deferred = nullptr;
    asyncCallbackInfo->ability = ability;
    asyncCallbackInfo->native_result = false;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = AbilityType::UNKNOWN;

    HILOG_INFO("%{public}s end.", __func__);
    return asyncCallbackInfo;
}

void GetContextAsyncExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("GetContextAsync, worker pool thread execute.");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("GetContextAsync, asyncCallbackInfo == nullptr");
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("GetContextAsync, ability == nullptr");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("GetContextAsync,wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
    HILOG_INFO("GetContextAsync, worker pool thread execute end.");
}

napi_value GetContextAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, *GetGlobalClassContext(), 0, nullptr, &result[PARAM1]);
            } else {
                result[PARAM1] = WrapUndefinedToJS(env);
            }
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetContextAsync, main event thread complete end.");
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetContextPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, *GetGlobalClassContext(), 0, nullptr, &result);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }

            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetContextPromise, main event thread complete end.");
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetContext processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetContextWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s,wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return nullptr;
    }

    napi_value result = nullptr;
    napi_new_instance(env, *GetGlobalClassContext(), 0, nullptr, &result);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContextCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetContextWrap(env, info, asyncCallbackInfo);

    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

void GetWantExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr", __func__);
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s, wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
    if (ptrWant != nullptr) {
        asyncCallbackInfo->param.want = *ptrWant;
    } else {
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value GetWantAsync(napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                result[PARAM1] = WrapWant(env, asyncCallbackInfo->param.want);
            } else {
                result[PARAM1] = WrapUndefinedToJS(env);
            }
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetWantAsync, main event thread complete end.");
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetWantPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                result = WrapWant(env, asyncCallbackInfo->param.want);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }

            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetWantPromise, main event thread complete end.");
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetWantWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetWantWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetWantAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = GetWantPromise(env, asyncCallbackInfo);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetWantWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
AbilityNameCB *CreateAbilityNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AbilityNameCB *abilityNameCB = new (std::nothrow) AbilityNameCB;
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }
    abilityNameCB->cbBase.cbInfo.env = env;
    abilityNameCB->cbBase.asyncWork = nullptr;
    abilityNameCB->cbBase.deferred = nullptr;
    abilityNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return abilityNameCB;
}

napi_value WrapAbilityName(napi_env env, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, Invalid param(abilityNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, abilityNameCB->name.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief GetAbilityName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr", __func__);
        return;
    }
    abilityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityNameCB->cbBase)) {
        HILOG_ERROR("%{public}s, wrong ability type", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    abilityNameCB->name = abilityNameCB->cbBase.ability->GetAbilityName();
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, abilityNameCB->cbBase.errCode);
    if (abilityNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAbilityName(env, abilityNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));
    if (abilityNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork));
    delete abilityNameCB;
    abilityNameCB = nullptr;
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete.");
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    napi_value result = nullptr;
    if (abilityNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAbilityName(env, abilityNameCB);
        napi_resolve_deferred(env, abilityNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, abilityNameCB->cbBase.errCode);
        napi_reject_deferred(env, abilityNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork);
    delete abilityNameCB;
    abilityNameCB = nullptr;
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete end.");
}

/**
 * @brief GetAbilityName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameAsync(napi_env env, napi_value *args, const size_t argCallback, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNameAsyncCompleteCB,
            (void *)abilityNameCB,
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAbilityName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNamePromise(napi_env env, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNamePromiseCompleteCB,
            (void *)abilityNameCB,
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAbilityName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameWrap(napi_env env, napi_callback_info info, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityNameAsync(env, args, 0, abilityNameCB);
    } else {
        ret = GetAbilityNamePromise(env, abilityNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AbilityNameCB *ablityNameCB = CreateAbilityNameCBInfo(env);
    if (ablityNameCB == nullptr) {
        HILOG_ERROR("%{public}s ablityNameCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    ablityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    ablityNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityNameWrap(env, info, ablityNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (ablityNameCB != nullptr) {
            delete ablityNameCB;
            ablityNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void UnwrapAbilityStartSettingForNumber(
    napi_env env, const std::string key, napi_value param, AAFwk::AbilityStartSetting &setting)
{
    int32_t natValue32 = 0;
    double natValueDouble = 0.0;
    bool isReadValue32 = false;
    bool isReadDouble = false;
    if (napi_get_value_int32(env, param, &natValue32) == napi_ok) {
        HILOG_INFO("%{public}s called. Property value=%{private}d.", __func__, natValue32);
        isReadValue32 = true;
    }

    if (napi_get_value_double(env, param, &natValueDouble) == napi_ok) {
        HILOG_INFO("%{public}s called. Property value=%{private}lf.", __func__, natValueDouble);
        isReadDouble = true;
    }

    if (isReadValue32 && isReadDouble) {
        if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
            setting.AddProperty(key, std::to_string(natValueDouble));
        } else {
            setting.AddProperty(key, std::to_string(natValue32));
        }
    } else if (isReadValue32) {
        setting.AddProperty(key, std::to_string(natValue32));
    } else if (isReadDouble) {
        setting.AddProperty(key, std::to_string(natValueDouble));
    }
}

bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);
    HILOG_INFO("%{public}s called. Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        HILOG_INFO("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                HILOG_INFO("%{public}s called. Property value=%{private}s.", __func__, natValue.c_str());
                setting.AddProperty(strProName, natValue);
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                HILOG_INFO("%{public}s called. Property value=%{public}s.", __func__, natValue ? "true" : "false");
                setting.AddProperty(strProName, std::to_string(natValue));
                break;
            }
            case napi_number:
                UnwrapAbilityStartSettingForNumber(env, strProName, jsProValue, setting);
                break;
            default:
                break;
        }
    }

    return true;
}

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
bool UnwrapParamForWant(napi_env env, napi_value args, AbilityType abilityType, CallAbilityParam &param)
{
    HILOG_INFO("%{public}s called.", __func__);
    bool ret = false;
    napi_valuetype valueType = napi_undefined;
    param.setting = nullptr;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        HILOG_ERROR("%{public}s, jsWant == nullptr", __func__);
        return false;
    }

    ret = UnwrapWant(env, jsWant, param.want);

    napi_value jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSetting", napi_object);
    if (jsSettingObj != nullptr) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        if (!UnwrapAbilityStartSetting(env, jsSettingObj, *(param.setting))) {
            HILOG_ERROR("%{public}s, unwrap abilityStartSetting falied.", __func__);
        }
        HILOG_INFO("%{public}s abilityStartSetting", __func__);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void StartAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return;
    }
    if (asyncCallbackInfo->errCode != NAPI_ERR_NO_ERROR) {
        HILOG_ERROR("%{public}s errCode:%{public}d", __func__, asyncCallbackInfo->errCode);
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return;
    }
    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
#ifdef SUPPORT_GRAPHICS
    // inherit split mode
    auto windowMode = asyncCallbackInfo->ability->GetCurrentWindowMode();
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        asyncCallbackInfo->param.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("window mode is %{public}d", windowMode);

    // follow orientation
    asyncCallbackInfo->param.want.SetParam("ohos.aafwk.Orientation", 0);
    if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
        auto orientation = asyncCallbackInfo->ability->GetDisplayOrientation();
        asyncCallbackInfo->param.want.SetParam("ohos.aafwk.Orientation", orientation);
        HILOG_DEBUG("%{public}s display orientation is %{public}d", __func__, orientation);
    }
#endif
    ErrCode ret = ERR_OK;
    if (asyncCallbackInfo->param.setting == nullptr) {
        HILOG_INFO("%{public}s param.setting == nullptr call StartAbility.", __func__);
        ret = asyncCallbackInfo->ability->StartAbility(asyncCallbackInfo->param.want);
    } else {
        HILOG_INFO("%{public}s param.setting != nullptr call StartAbility.", __func__);
        ret = asyncCallbackInfo->ability->StartAbility(asyncCallbackInfo->param.want,
            *(asyncCallbackInfo->param.setting));
    }
    if (ret != ERR_OK) {
        asyncCallbackInfo->errCode = ret;
    }
    HILOG_INFO("%{public}s end. ret:%{public}d", __func__, ret);
}

void StartAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value result[ARGS_TWO] = {0};
    napi_value callResult = 0;
    napi_get_undefined(env, &undefined);
    result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result[PARAM1]);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void StartAbilityPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value result = 0;
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, end.", __func__);
    delete asyncCallbackInfo;
}

napi_value StartAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s async call.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartAbilityExecuteCB,
            StartAbilityCallbackCompletedCB,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    HILOG_INFO("%{public}s async end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s promise call.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartAbilityExecuteCB,
            StartAbilityPromiseCompletedCB,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s promise end.", __func__);
    return promise;
}

/**
 * @brief StartAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
    } else {
        CallAbilityParam param;
        if (UnwrapParamForWant(env, args[PARAM0], asyncCallbackInfo->abilityType, param)) {
            asyncCallbackInfo->param = param;
        } else {
            HILOG_ERROR("%{public}s, call UnwrapParamForWant failed.", __func__);
            asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
        }
    }
    if (argcAsync > argcPromise) {
        ret = StartAbilityAsync(env, args, 1, asyncCallbackInfo);
    } else {
        ret = StartAbilityPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StartAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

bool UnwrapParamStopAbilityWrap(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_ERROR("%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s, the second parameter is invalid.", __func__);
            return false;
        }
    }

    return UnwrapWant(env, argv[PARAM0], asyncCallbackInfo->param.want);
}

void StopAbilityExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->StopAbility(asyncCallbackInfo->param.want);
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value StopAbilityWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = 0;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamStopAbilityWrap(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamStopAbility fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_StopAbilityWrapCallback";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_StopAbilityWrapPromise";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

/**
 * @brief stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StopAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StopAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s. ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
ConnectAbilityCB *CreateConnectAbilityCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return nullptr;
    }
    connectAbilityCB->cbBase.cbInfo.env = env;
    connectAbilityCB->cbBase.asyncWork = nullptr;
    connectAbilityCB->cbBase.deferred = nullptr;
    connectAbilityCB->cbBase.ability = ability;

    return connectAbilityCB;
}

void ConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr.", __func__);
        return;
    }
    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    if (connectAbilityCB->cbBase.ability == nullptr) {
        connectAbilityCB->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr.", __func__);
        return;
    }

    if (!CheckAbilityType(&connectAbilityCB->cbBase)) {
        connectAbilityCB->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        HILOG_ERROR("%{public}s ability type invalid.", __func__);
        return;
    }

    connectAbilityCB->abilityConnection->SetEnv(env);
    connectAbilityCB->abilityConnection->SetConnectCBRef(connectAbilityCB->abilityConnectionCB.callback[0]);
    connectAbilityCB->abilityConnection->SetDisconnectCBRef(connectAbilityCB->abilityConnectionCB.callback[1]);
    connectAbilityCB->result =
        connectAbilityCB->cbBase.ability->ConnectAbility(connectAbilityCB->want, connectAbilityCB->abilityConnection);
    HILOG_INFO("%{public}s end.bundlename:%{public}s abilityname:%{public}s result:%{public}d",
        __func__,
        connectAbilityCB->want.GetBundle().c_str(),
        connectAbilityCB->want.GetElement().GetAbilityName().c_str(),
        connectAbilityCB->result);
}

void ConnectAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value result = 0;
    napi_value callResult = 0;
    napi_get_undefined(env, &undefined);
    HILOG_INFO("%{public}s errCode=%{public}d result=%{public}d id=%{public}" PRId64,
        __func__,
        connectAbilityCB->errCode,
        connectAbilityCB->result,
        connectAbilityCB->id);
    if (connectAbilityCB->errCode != NAPI_ERR_NO_ERROR || connectAbilityCB->result == false) {
        HILOG_INFO("%{public}s connectAbility failed.", __func__);
        // return error code in onFailed asynccallback
        int errorCode = NO_ERROR;
        switch (connectAbilityCB->errCode) {
            case NAPI_ERR_ACE_ABILITY:
                errorCode = ABILITY_NOT_FOUND;
                break;
            case NAPI_ERR_PARAM_INVALID:
                errorCode = INVALID_PARAMETER;
                break;
            default:
                break;
        }
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, errorCode, &result));
        NAPI_CALL_RETURN_VOID(
            env, napi_get_reference_value(env, connectAbilityCB->abilityConnectionCB.callback[PARAM2], &callback));
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_ONE, &result, &callResult));
    }
    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->cbBase.cbInfo.callback));
    }
    if (connectAbilityCB->abilityConnectionCB.callback[PARAM2] != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->abilityConnectionCB.callback[PARAM2]));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork));
    delete connectAbilityCB;
    connectAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value ConnectAbilityAsync(napi_env env, napi_value *args, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ConnectAbilityExecuteCB,
            ConnectAbilityCallbackCompletedCB,
            (void *)connectAbilityCB,
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

/**
 * @brief ConnectAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param connectAbilityCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ConnectAbilityWrap(napi_env env, napi_callback_info info, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = ARGS_TWO;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        connectAbilityCB->errCode = NAPI_ERR_PARAM_INVALID;
        return nullptr;
    }

    if (!UnwrapWant(env, args[PARAM0], connectAbilityCB->want)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapWant fail", __func__);
        return nullptr;
    }

    HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
        __func__,
        connectAbilityCB->want.GetBundle().c_str(),
        connectAbilityCB->want.GetElement().GetAbilityName().c_str());

    std::string bundleName = connectAbilityCB->want.GetBundle();
    std::string abilityName = connectAbilityCB->want.GetElement().GetAbilityName();
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [&bundleName, &abilityName](const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match bundlename && abilityname
        connectAbilityCB->id = item->first.id;
        connectAbilityCB->abilityConnection = item->second;
        HILOG_INFO("%{public}s find connection exist", __func__);
    } else {
        sptr<NAPIAbilityConnection> conn(new (std::nothrow) NAPIAbilityConnection());
        connectAbilityCB->id = serialNumber_;
        connectAbilityCB->abilityConnection = conn;
        ConnecttionKey key;
        key.id = connectAbilityCB->id;
        key.want = connectAbilityCB->want;
        connects_.emplace(key, conn);
        if (serialNumber_ < INT64_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one", __func__);
    }
    HILOG_INFO("%{public}s id:%{public}" PRId64, __func__, connectAbilityCB->id);

    if (argcAsync > PARAM1) {
        napi_value jsMethod = nullptr;
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_object) {
            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onConnect", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onConnect valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM0]));

            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onDisconnect", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onDisconnect valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM1]));

            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onFailed", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onFailed valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM2]));
        } else {
            HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
            return nullptr;
        }
    }

    ret = ConnectAbilityAsync(env, args, connectAbilityCB);
    if (ret != nullptr) {
        // return number to js
        NAPI_CALL(env, napi_create_int64(env, connectAbilityCB->id, &ret));
        HILOG_INFO("%{public}s id=%{public}" PRId64, __func__, connectAbilityCB->id);
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

/**
 * @brief ConnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_ConnectAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = CreateConnectAbilityCBInfo(env);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    connectAbilityCB->cbBase.abilityType = abilityType;
    napi_value ret = ConnectAbilityWrap(env, info, connectAbilityCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
ConnectAbilityCB *CreateDisConnectAbilityCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return nullptr;
    }
    connectAbilityCB->cbBase.cbInfo.env = env;
    connectAbilityCB->cbBase.asyncWork = nullptr;
    connectAbilityCB->cbBase.deferred = nullptr;
    connectAbilityCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return connectAbilityCB;
}

void DisConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr.", __func__);
        return;
    }
    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    if (connectAbilityCB->cbBase.ability == nullptr) {
        connectAbilityCB->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr.", __func__);
        return;
    }

    if (!CheckAbilityType(&connectAbilityCB->cbBase)) {
        connectAbilityCB->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        HILOG_ERROR("%{public}s ability type invalid.", __func__);
        return;
    }

    HILOG_INFO("%{public}s DisconnectAbility called.", __func__);
    connectAbilityCB->cbBase.ability->DisconnectAbility(connectAbilityCB->abilityConnection);
    HILOG_INFO("%{public}s end. bundlename:%{public}s abilityname:%{public}s",
        __func__,
        connectAbilityCB->want.GetBundle().c_str(),
        connectAbilityCB->want.GetElement().GetAbilityName().c_str());
}

void DisConnectAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value result[ARGS_TWO] = {0};
    napi_value callResult = 0;
    napi_get_undefined(env, &undefined);
    result[PARAM0] = GetCallbackErrorValue(env, connectAbilityCB->errCode);
    if (connectAbilityCB->errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapVoidToJS(env);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, connectAbilityCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork));
    delete connectAbilityCB;
    connectAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void DisConnectAbilityPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value result = 0;
    if (connectAbilityCB->errCode == NAPI_ERR_NO_ERROR) {
        result = WrapVoidToJS(env);
        napi_resolve_deferred(env, connectAbilityCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, connectAbilityCB->errCode);
        napi_reject_deferred(env, connectAbilityCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork);
    delete connectAbilityCB;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value DisConnectAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &connectAbilityCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DisConnectAbilityExecuteCB,
            DisConnectAbilityCallbackCompletedCB,
            (void *)connectAbilityCB,
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value DisConnectAbilityPromise(napi_env env, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    connectAbilityCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DisConnectAbilityExecuteCB,
            DisConnectAbilityPromiseCompletedCB,
            (void *)connectAbilityCB,
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief DisConnectAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param connectAbilityCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value DisConnectAbilityWrap(napi_env env, napi_callback_info info, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[PARAM0], &valuetype);
    if (valuetype == napi_number) {
        NAPI_CALL(env, napi_get_value_int64(env, args[PARAM0], &connectAbilityCB->id));
    }

    HILOG_INFO("%{public}s id:%{public}" PRId64, __func__, connectAbilityCB->id);
    int64_t id = connectAbilityCB->id;
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [&id](const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return id == obj.first.id;
        });
    if (item != connects_.end()) {
        // match id
        connectAbilityCB->want = item->first.want;
        connectAbilityCB->abilityConnection = item->second;
        HILOG_INFO("%{public}s find conn ability exist", __func__);
    } else {
        HILOG_INFO("%{public}s there is no ability to disconnect.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = DisConnectAbilityAsync(env, args, ARGS_ONE, connectAbilityCB);
    } else {
        ret = DisConnectAbilityPromise(env, connectAbilityCB);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief DisConnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_DisConnectAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = CreateConnectAbilityCBInfo(env);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    connectAbilityCB->cbBase.abilityType = abilityType;
    napi_value ret = DisConnectAbilityWrap(env, info, connectAbilityCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void NAPIAbilityConnection::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIAbilityConnection::SetConnectCBRef(const napi_ref &ref)
{
    connectRef_ = ref;
}

void NAPIAbilityConnection::SetDisconnectCBRef(const napi_ref &ref)
{
    disconnectRef_ = ref;
}

void UvWorkOnAbilityConnectDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnAbilityConnectDone, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityConnectDone, work is null");
        return;
    }
    // JS Thread
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(work->data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityConnectDone, connectAbilityCB is null");
        return;
    }
    napi_value result[ARGS_TWO] = {0};
    result[PARAM0] =
        WrapElementName(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    napi_value jsRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->abilityConnectionCB.connection);
    result[PARAM1] = jsRemoteObject;

    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(connectAbilityCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = 0;
    napi_get_reference_value(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(
        connectAbilityCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback);
    }
    if (connectAbilityCB != nullptr) {
        delete connectAbilityCB;
        connectAbilityCB = nullptr;
    }
    if (work != nullptr) {
        delete work;
        work = nullptr;
    }
    HILOG_INFO("UvWorkOnAbilityConnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s, remoteObject == nullptr.", __func__);
        return;
    }
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = env_;
    connectAbilityCB->cbBase.cbInfo.callback = connectRef_;
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = remoteObject;
    work->data = (void *)connectAbilityCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityConnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UvWorkOnAbilityDisconnectDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnAbilityDisconnectDone, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityDisconnectDone, work is null");
        return;
    }
    // JS Thread
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(work->data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityDisconnectDone, connectAbilityCB is null");
        return;
    }
    napi_value result = nullptr;
    result = WrapElementName(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);

    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(connectAbilityCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = 0;
    napi_get_reference_value(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(connectAbilityCB->cbBase.cbInfo.env, undefined, callback, ARGS_ONE, &result, &callResult);
    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback);
    }

    // release connect
    HILOG_INFO("UvWorkOnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string bundleName = connectAbilityCB->abilityConnectionCB.elementName.GetBundleName();
    std::string abilityName = connectAbilityCB->abilityConnectionCB.elementName.GetAbilityName();
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [bundleName, abilityName](const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match bundlename && abilityname
        connects_.erase(item);
        HILOG_INFO("UvWorkOnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }

    if (connectAbilityCB != nullptr) {
        delete connectAbilityCB;
        connectAbilityCB = nullptr;
    }
    if (work != nullptr) {
        delete work;
        work = nullptr;
    }
    HILOG_INFO("UvWorkOnAbilityDisconnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    HILOG_INFO("OnAbilityDisconnectDone bundleName:%{public}s abilityName:%{public}s resultCode:%{public}d",
        element.GetBundleName().c_str(),
        element.GetAbilityName().c_str(),
        resultCode);
    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work == nullptr.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = env_;
    connectAbilityCB->cbBase.cbInfo.callback = disconnectRef_;
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    work->data = (void *)connectAbilityCB;

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityDisconnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief AcquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelperCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s,called", __func__);
    DataAbilityHelperCB *dataAbilityHelperCB = new (std::nothrow) DataAbilityHelperCB;
    if (dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s, dataAbilityHelperCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    dataAbilityHelperCB->cbBase.cbInfo.env = env;
    dataAbilityHelperCB->cbBase.ability = nullptr; // temporary value assignment
    dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    dataAbilityHelperCB->cbBase.abilityType = abilityType;
    napi_value ret = AcquireDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr", __func__);
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

/**
 * @brief acquireDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value AcquireDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s,dataAbilityHelperCB == nullptr", __func__);
        return nullptr;
    }

    size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    size_t uriIndex = PARAM0;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, args[0], stageMode);
    if (status != napi_ok) {
        HILOG_INFO("argv[0] is not a context, FA Model");
    } else {
        uriIndex = PARAM1;
        HILOG_INFO("argv[0] is a context, Stage Model: %{public}d", stageMode);
    }

    if (!stageMode) {
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native context instance");
            return nullptr;
        }
        dataAbilityHelperCB->cbBase.ability = ability;

        if (!CheckAbilityType(&dataAbilityHelperCB->cbBase)) {
            dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
            HILOG_ERROR("%{public}s ability type invalid.", __func__);
            return nullptr;
        }
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[uriIndex], &valuetype));
    if (valuetype != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, GetGlobalDataAbilityHelper(env), uriIndex + 1, &args[PARAM0], &result));

    if (!IsTypeForNapiValue(env, result, napi_object)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue isn`t object", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_null)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue is null", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_undefined)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue is undefined", __func__);
        return nullptr;
    }

    if (!GetDataAbilityHelperStatus()) {
        HILOG_ERROR("%{public}s, GetDataAbilityHelperStatus is false", __func__);
        return nullptr;
    }

    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
    HILOG_INFO("%{public}s,end", __func__);
    return result;
}

napi_value UnwrapParamForWantAgent(napi_env &env, napi_value &args, AbilityRuntime::WantAgent::WantAgent *&wantAgent)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    napi_value wantAgentParam = nullptr;
    napi_value result = nullptr;

    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, args, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "wantAgent", &wantAgentParam);
        NAPI_CALL(env, napi_typeof(env, wantAgentParam, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, wantAgentParam, (void **)&wantAgent);
    }

    napi_get_null(env, &result);
    return result;
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return;
    }
    if (asyncCallbackInfo->errCode == NAPI_ERR_PARAM_INVALID) {
        HILOG_ERROR("parse input param failed");
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return;
    }
    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("abilityinfo is null");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    AbilityRuntime::WantAgent::WantAgent wantAgentObj;
    if (!asyncCallbackInfo->wantAgent) {
        HILOG_WARN("input param without wantAgent");
        wantAgentObj = AbilityRuntime::WantAgent::WantAgent();
    } else {
        wantAgentObj = *asyncCallbackInfo->wantAgent;
    }
    asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StartBackgroundRunning(wantAgentObj);

    HILOG_INFO("%{public}s end.", __func__);
}

void BackgroundRunningCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value result[ARGS_TWO] = {0};
    napi_value callResult = 0;
    napi_get_undefined(env, &undefined);
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        result[0] = WrapUndefinedToJS(env);
        napi_create_int32(env, 0, &result[1]);
    } else {
        result[1] = WrapUndefinedToJS(env);
        result[0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
    }

    napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, result, &callResult);

    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void BackgroundRunningPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value result = 0;
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, end.", __func__);
    delete asyncCallbackInfo;
}

napi_value StartBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartBackgroundRunningExecuteCB,
            BackgroundRunningCallbackCompletedCB,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartBackgroundRunningExecuteCB,
            BackgroundRunningPromiseCompletedCB,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s, end.", __func__);
    return promise;
}

napi_value StartBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t paramNums = 3;
    const size_t minParamNums = 2;
    const size_t maxParamNums = 3;
    napi_value args[maxParamNums] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNums, args, NULL, NULL));

    if (paramNums < minParamNums || paramNums > maxParamNums) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (UnwrapParamForWantAgent(env, args[1], asyncCallbackInfo->wantAgent) == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
    }

    if (paramNums == maxParamNums) {
        ret = StartBackgroundRunningAsync(env, args, maxParamNums - 1, asyncCallbackInfo);
    } else {
        ret = StartBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = StartBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void CancelBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo->ability != nullptr) {
        asyncCallbackInfo->ability->StopBackgroundRunning();
    } else {
        HILOG_ERROR("NAPI_PACancelBackgroundRunning, ability == nullptr");
    }
}

napi_value CancelBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        CancelBackgroundRunningExecuteCB,
        BackgroundRunningCallbackCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = 0;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value CancelBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = 0;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        CancelBackgroundRunningExecuteCB,
        BackgroundRunningPromiseCompletedCB,
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end", __func__);
    return promise;
}

napi_value CancelBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, NULL, NULL));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = CancelBackgroundRunningAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = CancelBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = CancelBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
