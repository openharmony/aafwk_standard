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

#include "napi_ability_manager.h"

#include <pthread.h>
#include <unistd.h>

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
    napi_valuetype valuetype = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    char buf[NapiAbilityMgr::BUFFER_LENGTH_MAX] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, args, buf, NapiAbilityMgr::BUFFER_LENGTH_MAX, &len);
    HILOG_INFO("bundleName= [%{public}s].", buf);
    bundleName = std::string(buf);
    // create reutrn
    napi_value ret = 0;
    NAPI_CALL(env, napi_create_int32(env, 0, &ret));
    return ret;
}

void GetRecentMissionsForResult(
    napi_env env, const std::vector<AbilityMissionInfo> &recentMissionInfos, napi_value result)
{
    int32_t index = 0;
    for (const auto &item : recentMissionInfos) {
        napi_value objRecentMissionInfo = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objRecentMissionInfo));

        napi_value id = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.id, &id));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "id", id));

        napi_value baseAbility = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &baseAbility));

        napi_value deviceId = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.baseAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "deviceId", deviceId));

        napi_value bundleName = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.baseAbility.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &bundleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "bundleName", bundleName));

        napi_value abilityName = nullptr;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, item.baseAbility.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &abilityName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "abilityName", abilityName));

        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "baseAbility", baseAbility));

        napi_value topAbility = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &topAbility));

        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.topAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "deviceId", deviceId));

        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.topAbility.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &bundleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "bundleName", bundleName));

        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, item.topAbility.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &abilityName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "abilityName", abilityName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "topAbility", topAbility));

        napi_value missionDescription = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &missionDescription));

        napi_value label = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.missionDescription.label.c_str(), NAPI_AUTO_LENGTH, &label));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, missionDescription, "label", label));

        napi_value iconPath = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.missionDescription.iconPath.c_str(), NAPI_AUTO_LENGTH, &iconPath));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, missionDescription, "iconPath", iconPath));

        NAPI_CALL_RETURN_VOID(
            env, napi_set_named_property(env, objRecentMissionInfo, "missionDescription", missionDescription));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objRecentMissionInfo));
        index++;
    }
}

void GetAbilityMissionInfosForResult(
    napi_env env, const std::vector<AbilityMissionInfo> &recentMissionInfos, napi_value result)
{
    int32_t index = 0;
    for (const auto &item : recentMissionInfos) {
        napi_value objRecentMissionInfo = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objRecentMissionInfo));

        napi_value id = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item.id, &id));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "missionId", id));

        napi_value baseAbility = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &baseAbility));

        napi_value deviceId = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.baseAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "deviceId", deviceId));

        napi_value bundleName = nullptr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.baseAbility.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &bundleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "bundleName", bundleName));

        napi_value abilityName = nullptr;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, item.baseAbility.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &abilityName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "abilityName", abilityName));

        napi_value uri = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &uri));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "uri", uri));

        napi_value shortName = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &shortName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, baseAbility, "shortName", shortName));

        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "bottomAbility", baseAbility));

        napi_value topAbility = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &topAbility));

        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.topAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "deviceId", deviceId));

        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item.topAbility.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &bundleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "bundleName", bundleName));

        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(env, item.topAbility.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &abilityName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "abilityName", abilityName));

        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &uri));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "uri", uri));

        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &shortName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, topAbility, "shortName", shortName));

        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "topAbility", topAbility));

        napi_value nWindowMode = nullptr;
        int32_t windowMode = 0;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, windowMode, &nWindowMode));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objRecentMissionInfo, "windowMode", nWindowMode));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objRecentMissionInfo));
        index++;
    }
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

int32_t RemoveMissionsForResult(const std::vector<int32_t> &missionIds)
{
    int32_t error = 0;
    for (auto item : missionIds) {
        error = GetAbilityManagerInstance()->RemoveMission(item);
        if (error != 0) {
            return error;
        }
    }
    return error;
}

void GetMissionSnapshotInfoForResult(napi_env env, MissionSnapshot &recentMissionInfos, napi_value result)
{
    HILOG_INFO("%{public}s,%{public}d", __PRETTY_FUNCTION__, __LINE__);
    napi_value objTopAbilityInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objTopAbilityInfo));
    napi_value deviceId;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(env, recentMissionInfos.topAbility.GetDeviceID().c_str(), NAPI_AUTO_LENGTH, &deviceId));
    HILOG_INFO("deviceId = [%{public}s]", recentMissionInfos.topAbility.GetDeviceID().c_str());
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

    if (recentMissionInfos.snapshot) {
        HILOG_INFO("width = [%{public}d]", recentMissionInfos.snapshot->GetWidth());
        HILOG_INFO("height = [%{public}d]", recentMissionInfos.snapshot->GetHeight());
        napi_value iconResult = nullptr;
        iconResult = Media::PixelMapNapi::CreatePixelMap(env, recentMissionInfos.snapshot);
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "snapshot", iconResult));
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "snapshot", NapiGetNull(env)));
    }
}

auto NAPI_QueryRecentAbilityMissionInfosAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("queryRecentAbilityMissionInfos called(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    HILOG_INFO("maxMissionNum = [%{public}d]", async_callback_info->maxMissionNum);
    HILOG_INFO("queryType = [%{public}d]", async_callback_info->queryType);

    GetAbilityManagerInstance()->GetRecentMissions(
        async_callback_info->maxMissionNum, async_callback_info->queryType, async_callback_info->recentMissionInfo);
};

auto NAPI_QueryRecentAbilityMissionInfosAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("queryRecentAbilityMissionInfos compeleted(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetRecentMissionsForResult(env, async_callback_info->recentMissionInfo, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_QueryRecentAbilityMissionInfosPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("queryRecentAbilityMissionInfos compeleted(Promise Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetRecentMissionsForResult(env, async_callback_info->recentMissionInfo, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_QueryRecentAbilityMissionInfosWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncMissionInfosCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_QueryRecentAbilityMissionInfos called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_QueryRecentAbilityMissionInfosCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_QueryRecentAbilityMissionInfosAsyncExecuteCallback,
            NAPI_QueryRecentAbilityMissionInfosAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 1, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_QueryRecentAbilityMissionInfosPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_QueryRecentAbilityMissionInfosAsyncExecuteCallback,
            NAPI_QueryRecentAbilityMissionInfosPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_QueryRecentAbilityMissionInfos(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_THREE;
    napi_value argv[NUMBER_OF_PARAMETERS_THREE] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");

    napi_valuetype valuetype1 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));
    int32_t value1 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &value1));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_THREE) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[NUMBER_OF_PARAMETERS_TWO], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncMissionInfosCallbackInfo *async_callback_info = new (std::nothrow) AsyncMissionInfosCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->maxMissionNum = value0;
    async_callback_info->queryType = value1;
    if (callBackMode) {
        napi_create_reference(env, argv[NUMBER_OF_PARAMETERS_TWO], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_QueryRecentAbilityMissionInfosWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_GetpreviousAbilityMissionInfosAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("getpreviousAbilityMissionInfos called(CallBack Mode)...");
    AsyncPreviousMissionInfosCallbackInfo *async_callback_info = (AsyncPreviousMissionInfosCallbackInfo *)data;
    HILOG_INFO("maxMissionNum = [%{public}d]", async_callback_info->maxMissionNum);

    GetAbilityManagerInstance()->GetRecentMissions(
        async_callback_info->maxMissionNum, 1, async_callback_info->previousMissionInfo);
};

auto NAPI_GetpreviousAbilityMissionInfosAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getpreviousAbilityMissionInfos compeleted(CallBack Mode)...");
    AsyncPreviousMissionInfosCallbackInfo *async_callback_info = (AsyncPreviousMissionInfosCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetAbilityMissionInfosForResult(env, async_callback_info->previousMissionInfo, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_GetpreviousAbilityMissionInfosPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getpreviousAbilityMissionInfos compeleted(Promise Mode)...");
    AsyncPreviousMissionInfosCallbackInfo *async_callback_info = (AsyncPreviousMissionInfosCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetAbilityMissionInfosForResult(env, async_callback_info->previousMissionInfo, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_GetpreviousAbilityMissionInfosWrap(napi_env env, napi_callback_info info, bool callBackMode,
    AsyncPreviousMissionInfosCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_GetpreviousAbilityMissionInfosWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetpreviousAbilityMissionInfosWrap", NAPI_AUTO_LENGTH, &resourceName);

        napi_value resource = nullptr;
        napi_create_async_work(env,
            resource,
            resourceName,
            NAPI_GetpreviousAbilityMissionInfosAsyncExecuteCallback,
            NAPI_GetpreviousAbilityMissionInfosAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 1, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(
            env, "NAPI_GetpreviousAbilityMissionInfosWrapPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetpreviousAbilityMissionInfosAsyncExecuteCallback,
            NAPI_GetpreviousAbilityMissionInfosPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetPreviousAbilityMissionInfos(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncPreviousMissionInfosCallbackInfo *async_callback_info =
        new (std::nothrow) AsyncPreviousMissionInfosCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->maxMissionNum = value0;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_GetpreviousAbilityMissionInfosWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_QueryRunningAbilityMissionInfosAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("NAPI_QueryRunningAbilityMissionInfosWrap called(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    HILOG_INFO("maxMissionNum = [%{public}d]", async_callback_info->maxMissionNum);
    HILOG_INFO("queryType = [%{public}d]", async_callback_info->queryType);

    GetAbilityManagerInstance()->GetRecentMissions(
        async_callback_info->maxMissionNum, async_callback_info->queryType, async_callback_info->recentMissionInfo);
    HILOG_INFO("size = [%{public}zu]", async_callback_info->recentMissionInfo.size());
};

auto NAPI_QueryRunningAbilityMissionInfosAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("NAPI_QueryRunningAbilityMissionInfosWrap compeleted(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetRecentMissionsForResult(env, async_callback_info->recentMissionInfo, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_QueryRunningAbilityMissionInfosPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("queryRunningAbilityMissionInfos compeleted(Promise Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetRecentMissionsForResult(env, async_callback_info->recentMissionInfo, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_QueryRunningAbilityMissionInfosWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncMissionInfosCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_QueryRunningAbilityMissionInfosWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_QueryRunningAbilityMissionInfosCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_QueryRunningAbilityMissionInfosAsyncExecuteCallback,
            NAPI_QueryRunningAbilityMissionInfosAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_QueryRunningAbilityMissionInfosPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_QueryRunningAbilityMissionInfosAsyncExecuteCallback,
            NAPI_QueryRunningAbilityMissionInfosPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_QueryRunningAbilityMissionInfos(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncMissionInfosCallbackInfo *async_callback_info = new (std::nothrow) AsyncMissionInfosCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->maxMissionNum = value0;
    async_callback_info->queryType = NapiAbilityMgr::QUERY_RECENT_RUNNING_MISSION_INFO_TYPE;

    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_QueryRunningAbilityMissionInfosWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_GetActiveAbilityMissionInfosAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("NAPI_GetActiveAbilityMissionInfosWrap called(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    HILOG_INFO("maxMissionNum = [%{public}d]", async_callback_info->maxMissionNum);
    HILOG_INFO("queryType = [%{public}d]", async_callback_info->queryType);

    GetAbilityManagerInstance()->GetRecentMissions(
        async_callback_info->maxMissionNum, async_callback_info->queryType, async_callback_info->recentMissionInfo);
    HILOG_INFO("size = [%{public}zu]", async_callback_info->recentMissionInfo.size());
};

auto NAPI_GetActiveAbilityMissionInfosAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("NAPI_GetActiveAbilityMissionInfosWrap compeleted(CallBack Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result[NUMBER_OF_PARAMETERS_TWO] = {0};
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_array(env, &result[1]);
    GetAbilityMissionInfosForResult(env, async_callback_info->recentMissionInfo, result[1]);
    napi_get_undefined(env, &undefined);
    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, NUMBER_OF_PARAMETERS_TWO, &result[0], &callResult);

    if (async_callback_info->callback[0] != nullptr) {
        napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_GetActiveAbilityMissionInfosPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("getActiveAbilityMissionInfos compeleted(Promise Mode)...");
    AsyncMissionInfosCallbackInfo *async_callback_info = (AsyncMissionInfosCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetAbilityMissionInfosForResult(env, async_callback_info->recentMissionInfo, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_GetActiveAbilityMissionInfosWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncMissionInfosCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_GetActiveAbilityMissionInfosWrap called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetActiveAbilityMissionInfosCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetActiveAbilityMissionInfosAsyncExecuteCallback,
            NAPI_GetActiveAbilityMissionInfosAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_GetActiveAbilityMissionInfosPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetActiveAbilityMissionInfosAsyncExecuteCallback,
            NAPI_GetActiveAbilityMissionInfosPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_GetActiveAbilityMissionInfos(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncMissionInfosCallbackInfo *async_callback_info = new (std::nothrow) AsyncMissionInfosCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->maxMissionNum = value0;
    async_callback_info->queryType = NapiAbilityMgr::QUERY_RECENT_RUNNING_MISSION_INFO_TYPE;

    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_GetActiveAbilityMissionInfosWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
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
        // create reutrn
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
        // create reutrn
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

auto NAPI_RemoveMissionAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("removeMission called(CallBack Mode)...");
    AsyncRemoveMissionCallbackInfo *async_callback_info = (AsyncRemoveMissionCallbackInfo *)data;
    async_callback_info->result = GetAbilityManagerInstance()->RemoveMission(async_callback_info->index);
};

auto NAPI_RemoveMissionAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("removeMission compeleted(CallBack Mode)...");
    AsyncRemoveMissionCallbackInfo *async_callback_info = (AsyncRemoveMissionCallbackInfo *)data;
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

auto NAPI_RemoveMissionPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("removeMission compeleted(Promise Mode)...");
    AsyncRemoveMissionCallbackInfo *async_callback_info = (AsyncRemoveMissionCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_RemoveMissionWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncRemoveMissionCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_RemoveMission called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_RemoveMissionCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_RemoveMissionAsyncExecuteCallback,
            NAPI_RemoveMissionAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_RemoveMissionPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_RemoveMissionAsyncExecuteCallback,
            NAPI_RemoveMissionPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_RemoveMission(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncRemoveMissionCallbackInfo *async_callback_info = new (std::nothrow) AsyncRemoveMissionCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->index = value0;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_RemoveMissionWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_RemoveMissionsAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("removeMissions called(CallBack Mode)...");
    AsyncRemoveMissionsCallbackInfo *async_callback_info = (AsyncRemoveMissionsCallbackInfo *)data;
    async_callback_info->result = RemoveMissionsForResult(async_callback_info->missionIds);
};

auto NAPI_RemoveMissionsAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("removeMissions compeleted(CallBack Mode)...");
    AsyncRemoveMissionsCallbackInfo *async_callback_info = (AsyncRemoveMissionsCallbackInfo *)data;
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

auto NAPI_RemoveMissionsPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("removeMissions compeleted(Promise Mode)...");
    AsyncRemoveMissionsCallbackInfo *async_callback_info = (AsyncRemoveMissionsCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_RemoveMissionsWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncRemoveMissionsCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_RemoveMissions called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_RemoveMissionsCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_RemoveMissionsAsyncExecuteCallback,
            NAPI_RemoveMissionsAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_RemoveMissionsPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_RemoveMissionsAsyncExecuteCallback,
            NAPI_RemoveMissionsPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_RemoveMissions(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    uint32_t arrayLength = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[0], &arrayLength));
    HILOG_INFO("property is array, length=%{public}d", arrayLength);
    NAPI_ASSERT(env, arrayLength > 0, "The array is empty.");

    std::vector<int32_t> missionIds = {};
    for (size_t i = 0; i < arrayLength; i++) {
        napi_value napiMissionIds = nullptr;
        napi_get_element(env, argv[0], i, &napiMissionIds);

        napi_valuetype valuetype0 = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, napiMissionIds, &valuetype0));
        NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");
        int32_t value0 = 0;
        NAPI_CALL(env, napi_get_value_int32(env, napiMissionIds, &value0));
        missionIds.push_back(value0);
    }

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncRemoveMissionsCallbackInfo *async_callback_info = new (std::nothrow) AsyncRemoveMissionsCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->missionIds = missionIds;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_RemoveMissionsWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_ClearMissionsAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("clearMissions called(CallBack Mode)...");
    AsyncClearMissionsCallbackInfo *async_callback_info = (AsyncClearMissionsCallbackInfo *)data;
    async_callback_info->result = GetAbilityManagerInstance()->RemoveStack(NapiAbilityMgr::DEFAULT_STACK_ID);
};

auto NAPI_ClearMissionsAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("clearMissions compeleted(CallBack Mode)...");
    AsyncClearMissionsCallbackInfo *async_callback_info = (AsyncClearMissionsCallbackInfo *)data;
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

auto NAPI_ClearMissionsPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("clearMissions compeleted(Promise Mode)...");
    AsyncClearMissionsCallbackInfo *async_callback_info = (AsyncClearMissionsCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_ClearMissionsWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncClearMissionsCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_ClearMissions called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_ClearMissionsCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_ClearMissionsAsyncExecuteCallback,
            NAPI_ClearMissionsAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_ClearMissionsPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_ClearMissionsAsyncExecuteCallback,
            NAPI_ClearMissionsPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_ClearMissions(napi_env env, napi_callback_info info)
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

    AsyncClearMissionsCallbackInfo *async_callback_info = new (std::nothrow) AsyncClearMissionsCallbackInfo {
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

    napi_value ret = NAPI_ClearMissionsWrap(env, info, callBackMode, *async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (nullptr) : (ret));
}

auto NAPI_MoveMissionToTopAsyncExecuteCallback = [](napi_env env, void *data) {
    HILOG_INFO("moveMissionToTop called(CallBack Mode)...");
    AsyncMoveMissionToTopCallbackInfo *async_callback_info = (AsyncMoveMissionToTopCallbackInfo *)data;
    async_callback_info->result = GetAbilityManagerInstance()->MoveMissionToTop(async_callback_info->index);
};

auto NAPI_MoveMissionToTopAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("moveMissionToTop compeleted(CallBack Mode)...");
    AsyncMoveMissionToTopCallbackInfo *async_callback_info = (AsyncMoveMissionToTopCallbackInfo *)data;
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

auto NAPI_MoveMissionToTopPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("moveMissionToTop compeleted(Promise Mode)...");
    AsyncMoveMissionToTopCallbackInfo *async_callback_info = (AsyncMoveMissionToTopCallbackInfo *)data;
    napi_value result = nullptr;
    napi_create_int32(async_callback_info->env, async_callback_info->result, &result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_MoveMissionToTopWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncMoveMissionToTopCallbackInfo &async_callback_info)
{
    HILOG_INFO("NAPI_MoveMissionToTop called...");
    if (callBackMode) {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_MoveMissionToTopCallBack", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_MoveMissionToTopAsyncExecuteCallback,
            NAPI_MoveMissionToTopAsyncCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info.asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName = nullptr;
        napi_create_string_latin1(env, "NAPI_MoveMissionToTopPromise", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info.deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_MoveMissionToTopAsyncExecuteCallback,
            NAPI_MoveMissionToTopPromiseCompleteCallback,
            (void *)&async_callback_info,
            &async_callback_info.asyncWork);
        napi_queue_async_work(env, async_callback_info.asyncWork);
        return promise;
    }
}

napi_value NAPI_MoveMissionToTop(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_OF_PARAMETERS_TWO;
    napi_value argv[NUMBER_OF_PARAMETERS_TWO] = {};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0 = napi_valuetype::napi_null;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");
    int32_t value0 = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value0));

    bool callBackMode = false;
    if (argc >= NUMBER_OF_PARAMETERS_TWO) {
        napi_valuetype valuetype = napi_valuetype::napi_null;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncMoveMissionToTopCallbackInfo *async_callback_info = new (std::nothrow) AsyncMoveMissionToTopCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };
    if (async_callback_info == nullptr) {
        return nullptr;
    }

    async_callback_info->index = value0;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_MoveMissionToTopWrap(env, info, callBackMode, *async_callback_info);
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
        // create reutrn
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
        // create reutrn
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

auto NAPI_GetAbilityMissionSnapshotAsyncExecute = [](napi_env env, void *data) {
    HILOG_INFO("GetAbilityMissionSnapshotWrap called...");
    AsyncGetMissionSnapshot *async_callback_info = (AsyncGetMissionSnapshot *)data;
    AAFwk::AbilityManagerClient::GetInstance()->GetMissionSnapshot(async_callback_info->missionId,
        async_callback_info->missionSnapshot);
};

auto NAPI_GetAbilityMissionSnapshotAsyncCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetAbilityMissionSnapshotWrap compeleted(CallBack Mode)...");
    AsyncGetMissionSnapshot *async_callback_info = (AsyncGetMissionSnapshot *)data;
    napi_value result[2] = {0};
    napi_value callback;
    napi_value undefined;
    napi_value callResult = 0;

    result[0] = GetCallbackErrorValue(async_callback_info->env, NapiAbilityMgr::BUSINESS_ERROR_CODE_OK);
    napi_create_object(async_callback_info->env, &result[1]);
    GetMissionSnapshotInfoForResult(
        async_callback_info->env, async_callback_info->missionSnapshot, result[1]);
    napi_get_undefined(env, &undefined);

    napi_get_reference_value(env, async_callback_info->callback[0], &callback);
    napi_call_function(env, undefined, callback, 2, &result[0], &callResult);
    if (async_callback_info->callback[0] != nullptr) {
            napi_delete_reference(env, async_callback_info->callback[0]);
    }
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

auto NAPI_GetAbilityMissionSnapshotPromiseCompleteCallback = [](napi_env env, napi_status status, void *data) {
    HILOG_INFO("GetAbilityMissionSnapshotWrap compeleted(Promise Mode)...");
    AsyncGetMissionSnapshot *async_callback_info = (AsyncGetMissionSnapshot *)data;
    napi_value result;
    napi_create_object(async_callback_info->env, &result);
    GetMissionSnapshotInfoForResult(async_callback_info->env, async_callback_info->missionSnapshot, result);
    napi_resolve_deferred(async_callback_info->env, async_callback_info->deferred, result);
    napi_delete_async_work(env, async_callback_info->asyncWork);
    delete async_callback_info;
};

napi_value NAPI_GetAbilityMissionSnapshotWrap(
    napi_env env, napi_callback_info info, bool callBackMode, AsyncGetMissionSnapshot *async_callback_info)
{
    HILOG_INFO("NAPI_GetAbilityMissionSnapshotWrap called...");
    if (callBackMode) {
        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_GetAbilityMissionSnapshotWrap", NAPI_AUTO_LENGTH, &resourceName);

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetAbilityMissionSnapshotAsyncExecute,
            NAPI_GetAbilityMissionSnapshotAsyncCompleteCallback,
            (void *)async_callback_info,
            &async_callback_info->asyncWork);

        NAPI_CALL(env, napi_queue_async_work(env, async_callback_info->asyncWork));
        // create reutrn
        napi_value ret = 0;
        NAPI_CALL(env, napi_create_int32(env, 0, &ret));
        return ret;
    } else {
        napi_value resourceName;
        napi_create_string_latin1(env, "NAPI_GetAbilityMissionSnapshotWrap", NAPI_AUTO_LENGTH, &resourceName);

        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        async_callback_info->deferred = deferred;

        napi_create_async_work(env,
            nullptr,
            resourceName,
            NAPI_GetAbilityMissionSnapshotAsyncExecute,
            NAPI_GetAbilityMissionSnapshotPromiseCompleteCallback,
            (void *)async_callback_info,
            &async_callback_info->asyncWork);
        napi_queue_async_work(env, async_callback_info->asyncWork);
        return promise;
    }
}

napi_value NAPI_GetAbilityMissionSnapshot(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    const size_t maxSize = 2;
    napi_value argv[argc];
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    HILOG_INFO("argc = [%{public}zu]", argc);

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Numbers expected.");
    int32_t missionId;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &missionId));

    bool callBackMode = false;
    if (argc >= maxSize) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        callBackMode = true;
    }

    AsyncGetMissionSnapshot *async_callback_info =
        new (std::nothrow) AsyncGetMissionSnapshot {.env = env, .asyncWork = nullptr, .deferred = nullptr};
    if (async_callback_info == nullptr) {
        return NapiGetNull(env);
    }

    async_callback_info->missionId = missionId;
    if (callBackMode) {
        napi_create_reference(env, argv[1], 1, &async_callback_info->callback[0]);
    }

    napi_value ret = NAPI_GetAbilityMissionSnapshotWrap(env, info, callBackMode, async_callback_info);
    if (ret == nullptr) {
        delete async_callback_info;
        async_callback_info = nullptr;
    }

    return ((callBackMode) ? (NapiGetNull(env)) : (ret));
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
        // promiss
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
}  // namespace AppExecFwk
}  // namespace OHOS
