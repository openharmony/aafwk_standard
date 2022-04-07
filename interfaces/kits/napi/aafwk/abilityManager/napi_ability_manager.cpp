/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "napi_ability_manager.h"

#include <pthread.h>
#include <unistd.h>

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

napi_value ParseBundleName(napi_env env, std::string &bundleName, napi_value args)
{
    char buf[NapiAbilityMgr::BUFFER_LENGTH_MAX] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, args, buf, NapiAbilityMgr::BUFFER_LENGTH_MAX, &len);
    HILOG_INFO("bundleName= [%{public}s].", buf);
    bundleName = std::string(buf);
    // create return
    napi_value ret = 0;
    NAPI_CALL(env, napi_create_int32(env, 0, &ret));
    return ret;
}

void GetAllRunningProcessesForResult(napi_env env, const std::vector<RunningProcessInfo> &info, napi_value result)
{
    int32_t index = 0;

    for (const auto &item : info) {
        napi_value objAppProcessInfo = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAppProcessInfo));

        napi_value nPid = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.pid_, &nPid));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "pid", nPid));

        napi_value nProcessName = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.processName_.c_str(), NAPI_AUTO_LENGTH, &nProcessName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "processName", nProcessName));

        napi_value nPkgList = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nPkgList));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "pkgList", nPkgList));

        napi_value nUid = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.uid_, &nUid));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "uid", nUid));

        napi_value nLastMemoryLevel = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NapiAbilityMgr::DEFAULT_STACK_ID, &nLastMemoryLevel));
        NAPI_CALL_RETURN_VOID(
            env, napi_set_named_property(env, objAppProcessInfo, "lastMemoryLevel", nLastMemoryLevel));

        napi_value nWeight = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NapiAbilityMgr::DEFAULT_WEIGHT, &nWeight));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "weight", nWeight));

        napi_value nWeightReasonCode = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::REASON_UNKNOWN), &nWeightReasonCode));
        NAPI_CALL_RETURN_VOID(
            env, napi_set_named_property(env, objAppProcessInfo, "weightReasonCode", nWeightReasonCode));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objAppProcessInfo));
        index++;
    }
}

void GetActiveProcessInfosForResult(napi_env env, const std::vector<RunningProcessInfo> &info, napi_value result)
{
    int32_t index = 0;

    for (const auto &item : info) {
        napi_value objAppProcessInfo = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAppProcessInfo));

        napi_value nPid = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.pid_, &nPid));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "pid", nPid));

        napi_value nUid = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.uid_, &nUid));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "uid", nUid));

        napi_value nProcessName = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.processName_.c_str(), NAPI_AUTO_LENGTH, &nProcessName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "processName", nProcessName));

        napi_value nBundleNames = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nBundleNames));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppProcessInfo, "bundleNames", nBundleNames));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objAppProcessInfo));
        index++;
    }
}

void GetMissionSnapshotInfoForResult(napi_env env, MissionSnapshot &recentMissionInfos, napi_value result)
{
    HILOG_INFO("%{public}s,%{public}d", __PRETTY_FUNCTION__, __LINE__);
    napi_value objTopAbilityInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objTopAbilityInfo));
    napi_value deviceId;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(env, recentMissionInfos.topAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objTopAbilityInfo, "deviceId", deviceId));
    napi_value bundleName;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(
            env, recentMissionInfos.topAbility.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &bundleName));
    HILOG_INFO("bundleName = [%{public}s]", recentMissionInfos.topAbility.GetBundleName().c_str());
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objTopAbilityInfo, "bundleName", bundleName));
    napi_value abilityName;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(
            env, recentMissionInfos.topAbility.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &abilityName));
    HILOG_INFO("abilityName = [%{public}s]", recentMissionInfos.topAbility.GetAbilityName().c_str());
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objTopAbilityInfo, "abilityName", abilityName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "topAbility", objTopAbilityInfo));

#ifdef SUPPORT_GRAPHICS
    if (recentMissionInfos.snapshot) {
        napi_value iconResult = nullptr;
        iconResult = Media::PixelMapNapi::CreatePixelMap(env, recentMissionInfos.snapshot);
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "snapshot", iconResult));
    } else {
#endif
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "snapshot", NapiGetNull(env)));
#ifdef SUPPORT_GRAPHICS
    }
#endif
}

auto NAPI_GetAllRunningProcessesAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("getAllRunningProcesses called(CallBack Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    GetAppManagerInstance()->GetAllRunningProcesses(async_callback_info->info);
};

auto NAPI_GetAllRunningProcessesAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getAllRunningProcesses compeleted(CallBack Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetAllRunningProcessesForResult(env, async_callback_info->info, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_GetAllRunningProcessesPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getAllRunningProcesses compeleted(Promise Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetAllRunningProcessesForResult(env, async_callback_info->info, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_GetAllRunningProcessesWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_GetAllRunningProcessesWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetAllRunningProcessesCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetAllRunningProcessesAsyncExecuteCallback,
            NAPI_GetAllRunningProcessesAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create return
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetAllRunningProcessesPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetAllRunningProcessesAsyncExecuteCallback,
            NAPI_GetAllRunningProcessesPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetAllRunningProcesses(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    bool callBackMode = false;
    if (argc >= 1) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncCallbackInfo *async_callback_info = new (std::nothrow) AsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    if (callBackMode) {
        napi_create_reference(env, argv[0], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_GetAllRunningProcessesWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_GetActiveProcessInfosAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("getActiveProcessInfos called(CallBack Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    GetAppManagerInstance()->GetAllRunningProcesses(async_callback_info->info);
};

auto NAPI_GetActiveProcessInfosAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getActiveProcessInfos compeleted(CallBack Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetActiveProcessInfosForResult(env, async_callback_info->info, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_GetActiveProcessInfosPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getActiveProcessInfos compeleted(Promise Mode)...");
    AsyncCallbackInfo *async_callback_info = (AsyncCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetActiveProcessInfosForResult(env, async_callback_info->info, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_GetActiveProcessInfosWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_GetActiveProcessInfosWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetActiveProcessInfosCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetActiveProcessInfosAsyncExecuteCallback,
            NAPI_GetActiveProcessInfosAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create return
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetActiveProcessInfosPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetActiveProcessInfosAsyncExecuteCallback,
            NAPI_GetActiveProcessInfosPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetActiveProcessInfos(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    bool callBackMode = false;
    if (argc >= 1) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncCallbackInfo *async_callback_info = new (std::nothrow) AsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    if (callBackMode) {
        napi_create_reference(env, argv[0], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_GetActiveProcessInfosWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_KillProcessesByBundleNameAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("killProcessesByBundleName called(CallBack Mode)...");
    AsyncKillProcessCallbackInfo *async_callback_info = (AsyncKillProcessCallbackInfo *)data;
    async_callback_info->result = GetAbilityManagerInstance()->KillProcess(async_callback_info->bundleName);
};

auto NAPI_KillProcessesByBundleNameAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("killProcessesByBundleName compeleted(CallBack Mode)...");
    AsyncKillProcessCallbackInfo *async_callback_info = (AsyncKillProcessCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result[1]);
    napi_get_undefined(env, &undefined);

    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }

    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_KillProcessesByBundleNamePromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("killProcessesByBundleName compeleted(Promise Mode)...");
    AsyncKillProcessCallbackInfo *async_callback_info = (AsyncKillProcessCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_KillProcessesByBundleNameWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncKillProcessCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_KillProcessesByBundleName called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_KillProcessesByBundleNameCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_KillProcessesByBundleNameAsyncExecuteCallback,
            NAPI_KillProcessesByBundleNameAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create return
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_KillProcessesByBundleNamePromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_KillProcessesByBundleNameAsyncExecuteCallback,
            NAPI_KillProcessesByBundleNamePromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_KillProcessesByBundleName(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    std::string bundleName = "";
    napi_valuetype valuetype = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    ParseBundleName(env, bundleName, argv[0]);

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncKillProcessCallbackInfo *async_callback_info = new (std::nothrow) AsyncKillProcessCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->bundleName = bundleName;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_KillProcessesByBundleNameWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_ClearUpApplicationDataAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("clearUpApplicationData called(CallBack Mode)...");
    AsyncClearUpApplicationDataCallbackInfo *async_callback_info = (AsyncClearUpApplicationDataCallbackInfo *)data;
    async_callback_info->result = AAFwk::AbilityManagerClient::GetInstance()->
        ClearUpApplicationData(async_callback_info->bundleName);
};

auto NAPI_ClearUpApplicationDataAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("clearUpApplicationData compeleted(CallBack Mode)...");
    AsyncClearUpApplicationDataCallbackInfo *async_callback_info = (AsyncClearUpApplicationDataCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result[1]);
    napi_get_undefined(env, &undefined);

    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);
    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }

    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_ClearUpApplicationDataPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("clearUpApplicationData compeleted(Promise Mode)...");
    AsyncClearUpApplicationDataCallbackInfo *async_callback_info = (AsyncClearUpApplicationDataCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_ClearUpApplicationDataWrap(napi_env env, napi_callback_info info, bool callBackMode,
    AsyncClearUpApplicationDataCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_ClearUpApplicationData called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_ClearUpApplicationDataCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_ClearUpApplicationDataAsyncExecuteCallback,
            NAPI_ClearUpApplicationDataAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create return
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_ClearUpApplicationDataPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_ClearUpApplicationDataAsyncExecuteCallback,
            NAPI_ClearUpApplicationDataPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}  // namespace AppExecFwk

napi_value NAPI_ClearUpApplicationData(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    std::string bundleName = "";
    napi_valuetype valuetype = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    ParseBundleName(env, bundleName, argv[0]);

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncClearUpApplicationDataCallbackInfo *async_callback_info =
        new (std::nothrow) AsyncClearUpApplicationDataCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->bundleName = bundleName;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_ClearUpApplicationDataWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

void CreateWeightReasonCodeObject(napi_env env, napi_value value)
{
    napi_value nUnknow = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::REASON_UNKNOWN), &nUnknow));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "REASON_UNKNOWN", nUnknow));
    napi_value nForeground = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_FOREGROUND), &nForeground));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_FOREGROUND", nForeground));
    napi_value nForegroundService = nullptr;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_FOREGROUND_SERVICE), &nForegroundService));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_FOREGROUND_SERVICE", nForegroundService));
    napi_value nVisible = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_VISIBLE), &nVisible));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_VISIBLE", nVisible));
    napi_value nPerceptible = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_PERCEPTIBLE), &nPerceptible));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_PERCEPTIBLE", nPerceptible));
    napi_value nService = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_SERVICE), &nService));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_SERVICE", nService));
    napi_value nTopSleeping = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_TOP_SLEEPING), &nTopSleeping));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_TOP_SLEEPING", nTopSleeping));
    napi_value nCantSaveState = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_CANT_SAVE_STATE), &nCantSaveState));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_CANT_SAVE_STATE", nCantSaveState));
    napi_value nWeightCached = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_CACHED), &nWeightCached));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_CACHED", nWeightCached));
    napi_value nWeightGone = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(WeightReasonCode::WEIGHT_GONE), &nWeightGone));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "WEIGHT_GONE", nWeightGone));
}

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = 0;
    napi_get_null(env, &result);
    return result;
}

static napi_value SystemMemoryAttrConvertJSObject(napi_env env, const SystemMemoryAttr &memoryInfo)
{
    napi_value retJsObject = nullptr;
    napi_value jsAvailSysMem = nullptr;
    napi_value jsTotalSysMem = nullptr;
    napi_value jsThreshold = nullptr;
    napi_value jsIsSysInlowMem = nullptr;
    NAPI_CALL(env, napi_create_object(env, &retJsObject));
    NAPI_CALL(env, napi_create_int64(env, memoryInfo.availSysMem_, &jsAvailSysMem));
    NAPI_CALL(env, napi_create_int64(env, memoryInfo.totalSysMem_, &jsTotalSysMem));
    NAPI_CALL(env, napi_create_int64(env, memoryInfo.threshold_, &jsThreshold));
    NAPI_CALL(env, napi_get_boolean(env, memoryInfo.isSysInlowMem_, &jsIsSysInlowMem));

    HILOG_DEBUG("SystemMemoryAttrConvertJSObject %{public}zu %{public}zu %{public}zu %{public}s",
        static_cast<size_t>(memoryInfo.availSysMem_),
        static_cast<size_t>(memoryInfo.totalSysMem_),
        static_cast<size_t>(memoryInfo.threshold_),
        (memoryInfo.isSysInlowMem_ ? "true" : "false"));

    NAPI_CALL(env, napi_set_named_property(env, retJsObject, "availSysMem", jsAvailSysMem));
    NAPI_CALL(env, napi_set_named_property(env, retJsObject, "totalSysMem", jsTotalSysMem));
    NAPI_CALL(env, napi_set_named_property(env, retJsObject, "threshold", jsThreshold));
    NAPI_CALL(env, napi_set_named_property(env, retJsObject, "isSysInlowMem", jsIsSysInlowMem));

    return retJsObject;
}

static void GetSystemMemoryAttrExecute(napi_env env, void *data)
{
    SystemMemroyInfoCB *pdata = static_cast<SystemMemroyInfoCB *>(data);
    if (pdata == nullptr) {
        HILOG_ERROR("GetSystemMemoryAttrExecute input params data is nullptr");
        return;
    }

    if (pdata->info == nullptr) {
        pdata->info = std::make_shared<SystemMemoryAttr>();
        if (pdata->info == nullptr) {
            HILOG_ERROR("GetSystemMemoryAttrExecute pdata->info is nullptr");
            return;
        }
    }
    GetAbilityManagerInstance()->GetSystemMemoryAttr(*pdata->info);
}

static void GetSystemMemoryAttrAsyncComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    const int errorCodeFailed = -1;
    const int errorCodeSuccess = 0;
    const unsigned int argsCount = 2;
    const unsigned int paramFirst = 0;
    const unsigned int paramSecond = 1;
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[argsCount] = {nullptr};
    napi_value callResult = nullptr;
    SystemMemroyInfoCB *pdata = static_cast<SystemMemroyInfoCB *>(data);
    if (pdata == nullptr) {
        HILOG_ERROR("%{public}s, main event thread complete end.", __func__);
        return;
    }

    if (pdata->info == nullptr) {
        result[paramFirst] = GetCallbackErrorValue(env, errorCodeFailed);
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &result[paramSecond]));
        HILOG_ERROR("%{public}s, main event thread complete end.", __func__);
    } else {
        result[paramFirst] = GetCallbackErrorValue(env, errorCodeSuccess);
        result[paramSecond] = SystemMemoryAttrConvertJSObject(env, *pdata->info);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, pdata->callback, &callback));
    NAPI_CALL_RETURN_VOID(
        env, napi_call_function(env, undefined, callback, argsCount, &result[paramFirst], &callResult));
    if (pdata->callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, pdata->callback));
    }

    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, pdata->asyncWork));
    if (pdata != nullptr) {
        delete pdata;
        pdata = nullptr;
    }
    HILOG_INFO("%{public}s, main event thread complete end.", __func__);
}

static void GetSystemMemoryAttrPromiseComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    napi_value result = nullptr;
    SystemMemroyInfoCB *pdata = static_cast<SystemMemroyInfoCB *>(data);
    if (pdata == nullptr) {
        HILOG_ERROR("%{public}s, main event thread complete end.", __func__);
        return;
    }

    if (pdata->info == nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &result));
        HILOG_ERROR("%{public}s, info is nullptr.", __func__);
    } else {
        result = SystemMemoryAttrConvertJSObject(env, *pdata->info);
    }

    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, pdata->deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, pdata->asyncWork));
    if (pdata != nullptr) {
        delete pdata;
        pdata = nullptr;
    }
    HILOG_INFO("%{public}s,  main event thread complete end.", __func__);
}

static napi_value GetSystemMemoryAttrPromiss(napi_env env)
{
    napi_value resourceName = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value retPromise = nullptr;
    SystemMemroyInfoCB *cb = new (std::nothrow) SystemMemroyInfoCB;
    if (cb == nullptr) {
        HILOG_INFO("%{public}s, promise cb new failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retPromise));
        return retPromise;
    }

    cb->asyncWork = nullptr;
    cb->callback = nullptr;
    cb->deferred = nullptr;
    cb->info = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_promise(env, &cb->deferred, &retPromise));
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetSystemMemoryAttrExecute,
            GetSystemMemoryAttrPromiseComplete,
            (void *)cb,
            &asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncWork));
    HILOG_INFO("%{public}s, promise end", __func__);
    return retPromise;
}

static napi_value GetSystemMemoryAttrAsync(napi_env env, napi_value args)
{
    napi_value resourceName = nullptr;
    napi_value retAsync = nullptr;
    napi_valuetype valuetype = napi_undefined;
    SystemMemroyInfoCB *cb = new (std::nothrow) SystemMemroyInfoCB;
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, async cb new failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retAsync));
        return retAsync;
    }

    cb->asyncWork = nullptr;
    cb->callback = nullptr;
    cb->deferred = nullptr;
    cb->info = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args, 1, &cb->callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetSystemMemoryAttrExecute,
            GetSystemMemoryAttrAsyncComplete,
            (void *)cb,
            &cb->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cb->asyncWork));

    HILOG_INFO("%{public}s, async end", __func__);
    NAPI_CALL(env, napi_create_int32(env, 0, &retAsync));
    return retAsync;
}

napi_value NAPI_GetSystemMemoryAttr(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args = nullptr;
    napi_value ret = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, NULL, NULL));
    HILOG_INFO("%{public}s argc = [%{public}zu]", __func__, argc);

    if (argc == 0) {
        // promise
        ret = GetSystemMemoryAttrPromiss(env);
    } else if (argc == 1) {
        // async
        ret = GetSystemMemoryAttrAsync(env, args);
    } else {
        HILOG_ERROR("%{public}s js input param error", __func__);
        NAPI_CALL(env, napi_get_null(env, &ret));
    }

    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

static void GetAppMemorySizeExecute(napi_env env, void *data)
{
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed.", __func__);
        return;
    }
    cb->result = GetAbilityManagerInstance()->GetAppMemorySize();
    HILOG_INFO("%{public}s, result = %{public}d", __func__, cb->result);
}

static void GetAppMemorySizePromiseComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed.", __func__);
        return;
    }
    napi_value result = nullptr;
    napi_create_int32(env, cb->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, cb->deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, cb->asyncWork));
    if (cb != nullptr) {
        delete cb;
        cb = nullptr;
    }
    HILOG_INFO("%{public}s,  main event thread complete end.", __func__);
}

static void GetAppMemorySizeAsyncComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed.", __func__);
        return;
    }
    const int errorCodeFailed = -1;
    const int errorCodeSuccess = 0;
    const unsigned int argsCount = 2;
    const unsigned int paramFirst = 0;
    const unsigned int paramSecond = 1;
    napi_value result[argsCount] = {nullptr};
    if (cb->result == errorCodeFailed) {
        napi_create_int32(env, 1, &result[paramFirst]);
        napi_create_int32(env, cb->result, &result[paramSecond]);
    } else {
        napi_create_int32(env, errorCodeSuccess, &result[paramFirst]);
        napi_create_int32(env, cb->result, &result[paramSecond]);
    }
    napi_value undefined = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    napi_value callback = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, cb->callback, &callback));
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, argsCount, result, &callResult));
    if (cb->callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, cb->callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, cb->asyncWork));
    if (cb != nullptr) {
        delete cb;
        cb = nullptr;
    }
    HILOG_INFO("%{public}s, main event thread complete end.", __func__);
}

static napi_value GetAppMemorySizePromise(napi_env env)
{
    napi_value resourceName = nullptr;
    napi_value retPromise = nullptr;
    CallbackInfo *cb = new (std::nothrow) CallbackInfo;
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retPromise));
        return retPromise;
    }
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_promise(env, &cb->deferred, &retPromise));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, GetAppMemorySizeExecute,
                       GetAppMemorySizePromiseComplete, (void *)cb, &cb->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cb->asyncWork));
    HILOG_INFO("%{public}s, promise end", __func__);
    return retPromise;
}

static napi_value GetAppMemorySizeAsync(napi_env env, napi_value args)
{
    napi_value resourceName = nullptr;
    napi_value retAsync = nullptr;
    napi_valuetype valuetype = napi_undefined;
    CallbackInfo *cb = new (std::nothrow) CallbackInfo;
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retAsync));
        return retAsync;
    }

    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    if (valuetype != napi_function) {
        return retAsync;
    }
    NAPI_CALL(env, napi_create_reference(env, args, 1, &cb->callback));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, GetAppMemorySizeExecute,
                       GetAppMemorySizeAsyncComplete, (void *)cb, &cb->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cb->asyncWork));
    HILOG_INFO("%{public}s, async end", __func__);
    NAPI_CALL(env, napi_create_int32(env, 0, &retAsync));
    return retAsync;
}

napi_value NAPI_GetAppMemorySize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value ret = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("%{public}s argc = [%{public}zu]", __func__, argc);
    if (argc == 0) {
        // promiss
        ret = GetAppMemorySizePromise(env);
    } else if (argc == 1) {
        // async
        ret = GetAppMemorySizeAsync(env, argv[0]);
    } else {
        HILOG_ERROR("%{public}s js input param error", __func__);
        NAPI_CALL(env, napi_get_null(env, &ret));
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

static void IsRamConstrainedDeviceExecute(napi_env env, void *data)
{
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cb failed.", __func__);
        return;
    }
    cb->isRamConstrainedDevice = GetAbilityManagerInstance()->IsRamConstrainedDevice();
}

static void IsRamConstrainedDevicePromiseComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cb failed.", __func__);
        return;
    }
    napi_value result = nullptr;
    napi_get_boolean(env, cb->isRamConstrainedDevice, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, cb->deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, cb->asyncWork));
    if (cb != nullptr) {
        delete cb;
        cb = nullptr;
    }
    HILOG_INFO("%{public}s,  main event thread complete end.", __func__);
}

static napi_value IsRamConstrainedDevicePromise(napi_env env)
{
    napi_value resourceName = nullptr;
    napi_value retPromise = nullptr;
    CallbackInfo *cb = new (std::nothrow) CallbackInfo;
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cb failed.", __func__);
        NAPI_CALL(env, napi_get_null(env, &retPromise));
        return retPromise;
    }
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_create_promise(env, &cb->deferred, &retPromise));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, IsRamConstrainedDeviceExecute,
                       IsRamConstrainedDevicePromiseComplete, (void *)cb, &cb->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cb->asyncWork));
    HILOG_INFO("%{public}s, promise end", __func__);
    return retPromise;
}

static void IsRamConstrainedDeviceAsyncComplete(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, main event thread complete.", __func__);
    CallbackInfo *cb = static_cast<CallbackInfo *>(data);
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed.", __func__);
        return;
    }
    const int errorCodeSuccess = 0;
    const unsigned int argsCount = 2;
    const unsigned int paramFirst = 0;
    const unsigned int paramSecond = 1;
    napi_value result[argsCount] = {nullptr};
    napi_create_int32(env, errorCodeSuccess, &result[paramFirst]);
    napi_get_boolean(env, cb->isRamConstrainedDevice, &result[paramSecond]);
    napi_value undefined = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    napi_value callback = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, cb->callback, &callback));
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, argsCount, result, &callResult));
    if (cb->callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, cb->callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, cb->asyncWork));
    if (cb != nullptr) {
        delete cb;
        cb = nullptr;
    }
    HILOG_INFO("%{public}s, main event thread complete end.", __func__);
}

static napi_value IsRamConstrainedDeviceAsync(napi_env env, napi_value args)
{
    napi_value resourceName = nullptr;
    napi_value retAsync = nullptr;
    napi_valuetype valuetype = napi_undefined;
    CallbackInfo *cb = new (std::nothrow) CallbackInfo;
    if (cb == nullptr) {
        HILOG_ERROR("%{public}s, create cd failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retAsync));
        return retAsync;
    }

    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    if (valuetype != napi_function) {
        HILOG_ERROR("%{public}s, value type failed", __func__);
        NAPI_CALL(env, napi_get_null(env, &retAsync));
        return retAsync;
    }
    NAPI_CALL(env, napi_create_reference(env, args, 1, &cb->callback));
    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, IsRamConstrainedDeviceExecute,
                       IsRamConstrainedDeviceAsyncComplete, (void *)cb, &cb->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, cb->asyncWork));
    HILOG_INFO("%{public}s, async end", __func__);
    NAPI_CALL(env, napi_create_int32(env, 0, &retAsync));
    return retAsync;
}

napi_value NAPI_IsRamConstrainedDevice(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value ret = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("%{public}s argc = [%{public}zu]", __func__, argc);
    if (argc == 0) {
        // promiss
        ret = IsRamConstrainedDevicePromise(env);
    } else if (argc == 1) {
        // async
        ret = IsRamConstrainedDeviceAsync(env, argv[0]);
    } else {
        HILOG_ERROR("%{public}s js input param error", __func__);
        NAPI_CALL(env, napi_get_null(env, &ret));
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
