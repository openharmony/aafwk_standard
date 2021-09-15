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
#include <uv.h>
#include <dlfcn.h>
#include "napi_common_util.h"
#include "securec.h"
#include "hilog_wrapper.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AppExecFwk {

napi_value g_classContext;

using NAPICreateJsRemoteObject = napi_value (*)(napi_env env, const sptr<IRemoteObject> target);

napi_value *GetGlobalClassContext(void)
{
    return &g_classContext;
}

bool CheckAbilityType(AbilityType typeInAbility, AbilityType typeWant)
{
    HILOG_INFO("%{public}s called.", __func__);
    switch (typeWant) {
        case AbilityType::PAGE:
            return typeInAbility == AbilityType::PAGE;
        default:
            return typeInAbility != AbilityType::PAGE;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return false;
}

bool CheckAbilityType(const CBBase *cbBase)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (cbBase == nullptr) {
        HILOG_ERROR("%{public}s cbBase == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = cbBase->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
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
    HILOG_INFO("%{public}s.", __func__);
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
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief GetApplicationInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    appInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;

    if (appInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo, wrong ability type");
        appInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<ApplicationInfo> appInfoPtr = appInfoCB->cbBase.ability->GetApplicationInfo();
    if (appInfoPtr != nullptr) {
        SaveAppInfo(appInfoCB->appInfo, *appInfoPtr);
    } else {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoPtr == nullptr");
        appInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
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
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
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
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
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
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAppInfo(env, appInfoCB->appInfo);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appInfoCB->cbBase.asyncWork));
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppInfoCB *appInfoCB = (AppInfoCB *)data;
    if (appInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, appInfoCB == nullptr");
        return;
    }

    napi_value result = nullptr;
    if (appInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAppInfo(env, appInfoCB->appInfo);
        napi_resolve_deferred(env, appInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appInfoCB->cbBase.errCode);
        napi_reject_deferred(env, appInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appInfoCB->cbBase.asyncWork);
    delete appInfoCB;
    appInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief GetApplicationInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &appInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoAsyncCompleteCB,
            (void *)appInfoCB,
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetApplicationInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoPromise(napi_env env, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppInfoExecuteCB,
            GetAppInfoPromiseCompleteCB,
            (void *)appInfoCB,
            &appInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetApplicationInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetApplicationInfoWrap(napi_env env, napi_callback_info info, AppInfoCB *appInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
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
        ret = GetApplicationInfoAsync(env, args, argcAsync, argcPromise, appInfoCB);
    } else {
        ret = GetApplicationInfoPromise(env, appInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppInfoCB on success, nullptr on failure.
 */
AppInfoCB *CreateAppInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    AppInfoCB *appInfoCB = new (std::nothrow) AppInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
        .cbBase.abilityType = AbilityType::UNKNOWN,
        .cbBase.errCode = NAPI_ERR_NO_ERROR,
    };
    if (appInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return appInfoCB;
}

/**
 * @brief Obtains information about the current application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetApplicationInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s,called", __func__);
    AppInfoCB *appInfoCB = CreateAppInfoCBInfo(env);
    if (appInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetApplicationInfoWrap(env, info, appInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (appInfoCB != nullptr) {
            delete appInfoCB;
            appInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

/**
 * @brief GetBundleName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetBundleNameExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
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
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetBundleName();
    HILOG_INFO("%{public}s end. bundleName=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief GetBundleName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetBundleNameWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
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
        asyncParamEx.resource = "NAPI_GetBundleNameCallback";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetBundleNamePromise";
        asyncParamEx.execute = GetBundleNameExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

/**
 * @brief Get bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetBundleNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetBundleNameWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value WrapProcessInfo(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s called", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s Invalid param(processInfoCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_int32(env, processInfoCB->pid, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "pid", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, processInfoCB->processName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "processName", proValue));
    HILOG_INFO("%{public}s end", __func__);
    return result;
}

/**
 * @brief GetProcessInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute.");
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    if (processInfoCB == nullptr) {
        return;
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessInfo, ability == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&processInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetProcessInfo, wrong ability type");
        processInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<ProcessInfo> processInfoPtr = processInfoCB->cbBase.ability->GetProcessInfo();
    if (processInfoPtr != nullptr) {
        processInfoCB->processName = processInfoPtr->GetProcessName();
        processInfoCB->pid = processInfoPtr->GetPid();
    } else {
        HILOG_ERROR("NAPI_GetProcessInfo, processInfoPtr == nullptr");
        processInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetProcessInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessInfo(env, processInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processInfoCB->cbBase.asyncWork));
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

/**
 * @brief GetProcessInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &processInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoAsyncCompleteCB,
            (void *)processInfoCB,
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete.");
    ProcessInfoCB *processInfoCB = (ProcessInfoCB *)data;
    napi_value result = nullptr;
    if (processInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessInfo(env, processInfoCB);
        napi_resolve_deferred(env, processInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processInfoCB->cbBase.errCode);
        napi_reject_deferred(env, processInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processInfoCB->cbBase.asyncWork);
    delete processInfoCB;
    processInfoCB = nullptr;
    HILOG_INFO("NAPI_GetProcessInfo, main event thread complete end.");
}

/**
 * @brief GetProcessInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoPromise(napi_env env, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessInfoExecuteCB,
            GetProcessInfoPromiseCompleteCB,
            (void *)processInfoCB,
            &processInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetProcessInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessInfoWrap(napi_env env, napi_callback_info info, ProcessInfoCB *processInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
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
        ret = GetProcessInfoAsync(env, args, argcAsync, argcPromise, processInfoCB);
    } else {
        ret = GetProcessInfoPromise(env, processInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to ProcessInfoCB on success, nullptr on failure.
 */
ProcessInfoCB *CreateProcessInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessInfoCB *processInfoCB = new (std::nothrow) ProcessInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (processInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, processInfoCB == nullptr.", __func__);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return processInfoCB;
}

/**
 * @brief Obtains the process Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessInfoCB *processInfoCB = CreateProcessInfoCBInfo(env);
    if (processInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    processInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetProcessInfoWrap(env, info, processInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (processInfoCB != nullptr) {
            delete processInfoCB;
            processInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
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

    AppTypeCB *appTypeCB = new (std::nothrow) AppTypeCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
    }
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
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
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
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
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
    AppTypeCB *appTypeCB = (AppTypeCB *)data;
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
napi_value GetAppTypeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &appTypeCB->cbBase.cbInfo.callback));
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
        ret = GetAppTypeAsync(env, args, argcAsync, argcPromise, appTypeCB);
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

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to ElementNameCB on success, nullptr on failure.
 */
ElementNameCB *CreateElementNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ElementNameCB *elementNameCB = new (std::nothrow) ElementNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, elementNameCB == nullptr.", __func__);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return elementNameCB;
}

napi_value WrapElementName(napi_env env, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(appInfoCB = nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->abilityName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->deviceId.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceId", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->shortName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "shortName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, elementNameCB->uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief GetElementName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    if (data == nullptr) {
        HILOG_ERROR("%{public}s, data == nullptr.", __func__);
        return;
    }
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    if (elementNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, elementNameCB == nullptr");
        return;
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (elementNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        elementNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&elementNameCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo,wrong ability type");
        elementNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<ElementName> elementName = elementNameCB->cbBase.ability->GetElementName();
    if (elementNameCB != nullptr) {
        elementNameCB->deviceId = elementName->GetDeviceID();
        elementNameCB->bundleName = elementName->GetBundleName();
        elementNameCB->abilityName = elementName->GetAbilityName();
        elementNameCB->uri = elementNameCB->cbBase.ability->GetWant()->GetUriString();
        elementNameCB->shortName = "";
    } else {
        elementNameCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapElementName(env, elementNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, elementNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (elementNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, elementNameCB->cbBase.asyncWork));
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetElementNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    ElementNameCB *elementNameCB = (ElementNameCB *)data;
    napi_value result = nullptr;
    if (elementNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapElementName(env, elementNameCB);
        napi_resolve_deferred(env, elementNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, elementNameCB->cbBase.errCode);
        napi_reject_deferred(env, elementNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, elementNameCB->cbBase.asyncWork);
    delete elementNameCB;
    elementNameCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief GetElementName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNamePromise(napi_env env, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    elementNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNamePromiseCompleteCB,
            (void *)elementNameCB,
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetElementName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &elementNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetElementNameExecuteCB,
            GetElementNameAsyncCompleteCB,
            (void *)elementNameCB,
            &elementNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, elementNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
}

/**
 * @brief GetElementName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param elementNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetElementNameWrap(napi_env env, napi_callback_info info, ElementNameCB *elementNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (elementNameCB == nullptr) {
        HILOG_ERROR("%{public}s, appInfoCB == nullptr.", __func__);
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
        ret = GetElementNameAsync(env, args, argcAsync, argcPromise, elementNameCB);
    } else {
        ret = GetElementNamePromise(env, elementNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the elementName object of the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetElementNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ElementNameCB *elementNameCB = CreateElementNameCBInfo(env);
    if (elementNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    elementNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    elementNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetElementNameWrap(env, info, elementNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (elementNameCB != nullptr) {
            delete elementNameCB;
            elementNameCB = nullptr;
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

    AbilityInfoCB *abilityInfoCB = new (std::nothrow) AbilityInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
    }
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

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
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
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
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
    AbilityInfoCB *abilityInfoCB = (AbilityInfoCB *)data;
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
napi_value GetAbilityInfoAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &abilityInfoCB->cbBase.cbInfo.callback));
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
        ret = GetAbilityInfoAsync(env, args, argcAsync, argcPromise, abilityInfoCB);
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

    HapModuleInfoCB *hapModuleInfoCB = new (std::nothrow) HapModuleInfoCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
    }
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
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
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
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
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
    HapModuleInfoCB *hapModuleInfoCB = (HapModuleInfoCB *)data;
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
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &hapModuleInfoCB->cbBase.cbInfo.callback));
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
        ret = GetHapModuleInfoAsync(env, args, argcAsync, argcPromise, hapModuleInfoCB);
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
 * @return Return a pointer to ProcessNameCB on success, nullptr on failure.
 */
ProcessNameCB *CreateProcessNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ProcessNameCB *processNameCB = new (std::nothrow) ProcessNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return processNameCB;
}

/**
 * @brief GetProcessName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute.");
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    if (processNameCB == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, processNameCB == nullptr");
        return;
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (processNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetProcessName, ability == nullptr");
        processNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&processNameCB->cbBase)) {
        HILOG_ERROR("NAPI_GetProcessName,wrong ability type");
        processNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    processNameCB->processName = processNameCB->cbBase.ability->GetProcessName();
    HILOG_INFO("NAPI_GetProcessName, worker pool thread execute end.");
}

napi_value WrapProcessName(napi_env env, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, Invalid param(processNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, processNameCB->processName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapProcessName(env, processNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, processNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (processNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, processNameCB->cbBase.asyncWork));
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetProcessNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetProcessName, main event thread complete.");
    ProcessNameCB *processNameCB = (ProcessNameCB *)data;
    napi_value result = nullptr;
    if (processNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapProcessName(env, processNameCB);
        napi_resolve_deferred(env, processNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, processNameCB->cbBase.errCode);
        napi_reject_deferred(env, processNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, processNameCB->cbBase.asyncWork);
    delete processNameCB;
    processNameCB = nullptr;
    HILOG_INFO("NAPI_GetProcessName, main event thread complete end.");
}

/**
 * @brief GetProcessName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &processNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNameAsyncCompleteCB,
            (void *)processNameCB,
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetProcessName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNamePromise(napi_env env, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    processNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetProcessNameExecuteCB,
            GetProcessNamePromiseCompleteCB,
            (void *)processNameCB,
            &processNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, processNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetProcessName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param ProcessNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetProcessNameWrap(napi_env env, napi_callback_info info, ProcessNameCB *processNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (processNameCB == nullptr) {
        HILOG_ERROR("%{public}s, processNameCB == nullptr.", __func__);
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
        ret = GetProcessNameAsync(env, args, argcAsync, argcPromise, processNameCB);
    } else {
        ret = GetProcessNamePromise(env, processNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the name of the current process.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetProcessNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ProcessNameCB *processNameCB = CreateProcessNameCBInfo(env);
    if (processNameCB == nullptr) {
        return WrapVoidToJS(env);
    }

    processNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    processNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetProcessNameWrap(env, info, processNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr.", __func__);
        if (processNameCB != nullptr) {
            delete processNameCB;
            processNameCB = nullptr;
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
 * @return Return a pointer to CallingBundleCB on success, nullptr on failure.
 */
CallingBundleCB *CreateCallingBundleCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    CallingBundleCB *callingBundleCB = new (std::nothrow) CallingBundleCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return callingBundleCB;
}

/**
 * @brief GetCallingBundle asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundleExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute.");
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, callingBundleCB == nullptr");
        return;
    }

    callingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (callingBundleCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetCallingBundle, ability == nullptr");
        callingBundleCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&callingBundleCB->cbBase)) {
        HILOG_ERROR("NAPI_GetCallingBundle,wrong ability type");
        callingBundleCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
    callingBundleCB->callingBundleName = callingBundleCB->cbBase.ability->GetCallingBundle();
    HILOG_INFO("NAPI_GetCallingBundle, worker pool thread execute end.");
}

napi_value WrapCallingBundle(napi_env env, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s,Invalid param(callingBundleCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, callingBundleCB->callingBundleName.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundleAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapCallingBundle(env, callingBundleCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callingBundleCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (callingBundleCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork));
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCallingBundlePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete.");
    CallingBundleCB *callingBundleCB = (CallingBundleCB *)data;
    napi_value result = nullptr;
    if (callingBundleCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapCallingBundle(env, callingBundleCB);
        napi_resolve_deferred(env, callingBundleCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, callingBundleCB->cbBase.errCode);
        napi_reject_deferred(env, callingBundleCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, callingBundleCB->cbBase.asyncWork);
    delete callingBundleCB;
    callingBundleCB = nullptr;
    HILOG_INFO("NAPI_GetCallingBundle, main event thread complete end.");
}

/**
 * @brief GetCallingBundle Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundleAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &callingBundleCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundleAsyncCompleteCB,
            (void *)callingBundleCB,
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetCallingBundle Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundlePromise(napi_env env, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    callingBundleCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetCallingBundleExecuteCB,
            GetCallingBundlePromiseCompleteCB,
            (void *)callingBundleCB,
            &callingBundleCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callingBundleCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetCallingBundle processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param CallingBundleCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetCallingBundleWrap(napi_env env, napi_callback_info info, CallingBundleCB *callingBundleCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (callingBundleCB == nullptr) {
        HILOG_ERROR("%{public}s, callingBundleCB == nullptr.", __func__);
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
        ret = GetCallingBundleAsync(env, args, argcAsync, argcPromise, callingBundleCB);
    } else {
        ret = GetCallingBundlePromise(env, callingBundleCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the bundle name of the ability that called the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetCallingBundleCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    CallingBundleCB *calloingBundleCB = CreateCallingBundleCBInfo(env);
    if (calloingBundleCB == nullptr) {
        return WrapVoidToJS(env);
    }

    calloingBundleCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    calloingBundleCB->cbBase.abilityType = abilityType;
    napi_value ret = GetCallingBundleWrap(env, info, calloingBundleCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (calloingBundleCB != nullptr) {
            delete calloingBundleCB;
            calloingBundleCB = nullptr;
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

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo{
        .cbInfo.env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
        .ability = ability,
        .native_result = false,
        .errCode = NAPI_ERR_NO_ERROR,
        .abilityType = AbilityType::UNKNOWN,
    };
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return asyncCallbackInfo;
}

void GetContextAsyncExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("GetContextAsync, worker pool thread execute.");
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value result[ARGS_TWO] = {0};
            napi_value callResult = 0;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, g_classContext, 0, nullptr, &result[PARAM1]);
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
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, g_classContext, 0, nullptr, &result);
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
    napi_new_instance(env, g_classContext, 0, nullptr, &result);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
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
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return ret;
}

void GetWantExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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

napi_value GetWantAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argcPromise], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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
            AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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
        ret = GetWantAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
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
    HILOG_INFO("%{public}s, called.", __func__);
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
    HILOG_INFO("%{public}s, end.", __func__);
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

    AbilityNameCB *abilityNameCB = new (std::nothrow) AbilityNameCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
    }
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
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
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
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
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
    AbilityNameCB *abilityNameCB = (AbilityNameCB *)data;
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
napi_value GetAbilityNameAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &abilityNameCB->cbBase.cbInfo.callback));
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
        ret = GetAbilityNameAsync(env, args, argcAsync, argcPromise, abilityNameCB);
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
        HILOG_INFO("%{public}s abilityStartSetting = %{public}p.", __func__, param.setting.get());
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
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
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

    if (asyncCallbackInfo->param.setting == nullptr) {
        asyncCallbackInfo->ability->StartAbility(asyncCallbackInfo->param.want);
    } else {
        asyncCallbackInfo->ability->StartAbility(
            asyncCallbackInfo->param.want, *(asyncCallbackInfo->param.setting.get()));
    }
    HILOG_INFO("%{public}s end.", __func__);
}

void StartAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
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
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &asyncCallbackInfo->cbInfo.callback));
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

    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
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
            StartAbilityExecuteCB,
            StartAbilityPromiseCompletedCB,
            (void *)asyncCallbackInfo,
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s, end.", __func__);
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

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    CallAbilityParam param;
    if (UnwrapParamForWant(env, args[PARAM0], asyncCallbackInfo->abilityType, param) == false) {
        HILOG_ERROR("%{public}s, call UnwrapParamForWant failed.", __func__);
        return nullptr;
    }

    asyncCallbackInfo->param = param;
    if (argcAsync > argcPromise) {
        ret = StartAbilityAsync(env, args, argcAsync, argcPromise, asyncCallbackInfo);
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
    AsyncJSCallbackInfo *asyncCallbackInfo = (AsyncJSCallbackInfo *)data;
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
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, (void **)&ability));

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return connectAbilityCB;
}

void ConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = (ConnectAbilityCB *)data;
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
    ConnectAbilityCB *connectAbilityCB = (ConnectAbilityCB *)data;
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

napi_value ConnectAbilityAsync(napi_env env, napi_value *args, size_t argcAsync, ConnectAbilityCB *connectAbilityCB)
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
        HILOG_INFO("%{public}s find connection:%{public}p exist", __func__, item->second.GetRefPtr());
    } else {
        sptr<NAPIAbilityConnection> conn(new (std::nothrow) NAPIAbilityConnection());
        connectAbilityCB->id = serialNumber_;
        connectAbilityCB->abilityConnection = conn;
        ConnecttionKey key;
        key.id = connectAbilityCB->id;
        key.want = connectAbilityCB->want;
        connects_.emplace(key, conn);
        serialNumber_ < INT64_MAX ? serialNumber_++ : serialNumber_ = 0;
        HILOG_INFO("%{public}s not find connection, make new one:%{public}p.", __func__, conn.GetRefPtr());
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

    ret = ConnectAbilityAsync(env, args, argcAsync, connectAbilityCB);
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

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB{
        .cbBase.cbInfo.env = env,
        .cbBase.asyncWork = nullptr,
        .cbBase.deferred = nullptr,
        .cbBase.ability = ability,
    };
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return connectAbilityCB;
}

void DisConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = (ConnectAbilityCB *)data;
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
    ConnectAbilityCB *connectAbilityCB = (ConnectAbilityCB *)data;
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
    ConnectAbilityCB *connectAbilityCB = (ConnectAbilityCB *)data;
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
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &connectAbilityCB->cbBase.cbInfo.callback));
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
        HILOG_INFO("%{public}s find conn ability:%{public}p exist", __func__, item->second.GetRefPtr());
    } else {
        HILOG_INFO("%{public}s not find conn ability exist.", __func__);
        HILOG_INFO("%{public}s there is no ability to disconnect.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = DisConnectAbilityAsync(env, args, argcAsync, argcPromise, connectAbilityCB);
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

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);
    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(env_, &loop);
#endif  // NAPI_VERSION >= 2

    uv_work_t *work = new uv_work_t;
    ConnectAbilityCB *connectAbilityCB =
        new (std::nothrow) ConnectAbilityCB{.cbBase.cbInfo.env = env_, .cbBase.cbInfo.callback = connectRef_};
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = remoteObject;
    work->data = (void *)connectAbilityCB;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            HILOG_INFO("OnAbilityConnectDone, uv_queue_work");
            // JS Thread
            ConnectAbilityCB *event = (ConnectAbilityCB *)work->data;
            napi_value result[ARGS_TWO] = {0};
            result[PARAM0] = WrapElementName(event->cbBase.cbInfo.env, event->abilityConnectionCB.elementName);
            napi_value jsRemoteObject =
                NAPI_ohos_rpc_CreateJsRemoteObject(event->cbBase.cbInfo.env, event->abilityConnectionCB.connection);
            result[PARAM1] = jsRemoteObject;

            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(event->cbBase.cbInfo.env, &undefined);
            napi_value callResult = 0;
            napi_get_reference_value(event->cbBase.cbInfo.env, event->cbBase.cbInfo.callback, &callback);

            napi_call_function(event->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
            if (event->cbBase.cbInfo.callback != nullptr) {
                napi_delete_reference(event->cbBase.cbInfo.env, event->cbBase.cbInfo.callback);
            }
            delete event;
            delete work;
            HILOG_INFO("OnAbilityConnectDone, uv_queue_work end");
        });
    HILOG_INFO("%{public}s, end.", __func__);
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);

    uv_loop_s *loop = nullptr;

#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(env_, &loop);
#endif  // NAPI_VERSION >= 2
    HILOG_INFO("OnAbilityDisconnectDone bundleName:%{public}s abilityName:%{public}s resultCode:%{public}d",
        element.GetBundleName().c_str(),
        element.GetAbilityName().c_str(),
        resultCode);
    uv_work_t *work = new uv_work_t;
    ConnectAbilityCB *connectAbilityCB =
        new (std::nothrow) ConnectAbilityCB{.cbBase.cbInfo.env = env_, .cbBase.cbInfo.callback = disconnectRef_};
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    work->data = (void *)connectAbilityCB;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            HILOG_INFO("OnAbilityDisconnectDone, uv_queue_work");
            // JS Thread
            ConnectAbilityCB *event = (ConnectAbilityCB *)work->data;
            napi_value result = nullptr;
            result = WrapElementName(event->cbBase.cbInfo.env, event->abilityConnectionCB.elementName);

            napi_value callback = 0;
            napi_value undefined = 0;
            napi_get_undefined(event->cbBase.cbInfo.env, &undefined);
            napi_value callResult = 0;
            napi_get_reference_value(event->cbBase.cbInfo.env, event->cbBase.cbInfo.callback, &callback);

            napi_call_function(event->cbBase.cbInfo.env, undefined, callback, ARGS_ONE, &result, &callResult);
            if (event->cbBase.cbInfo.callback != nullptr) {
                napi_delete_reference(event->cbBase.cbInfo.env, event->cbBase.cbInfo.callback);
            }

            // release connect
            HILOG_INFO("OnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
            std::string bundleName = event->abilityConnectionCB.elementName.GetBundleName();
            std::string abilityName = event->abilityConnectionCB.elementName.GetAbilityName();
            auto item = std::find_if(connects_.begin(),
                connects_.end(),
                [bundleName, abilityName](
                    const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
                    return (bundleName == obj.first.want.GetBundle()) &&
                           (abilityName == obj.first.want.GetElement().GetAbilityName());
                });
            if (item != connects_.end()) {
                // match bundlename && abilityname
                connects_.erase(item);
                HILOG_INFO("OnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
            }

            delete event;
            delete work;
            HILOG_INFO("OnAbilityDisconnectDone, uv_queue_work end");
        });
    HILOG_INFO("%{public}s, end.", __func__);
}

}  // namespace AppExecFwk
}  // namespace OHOS
