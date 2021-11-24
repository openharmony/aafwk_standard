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

#ifndef NAPI_ABILITY_MANAGER_H
#define NAPI_ABILITY_MANAGER_H

#include <vector>

#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "running_process_info.h"
#include "system_memory_attr.h"
#include "ability_mission_info.h"

using RunningProcessInfo = OHOS::AppExecFwk::RunningProcessInfo;
using AbilityMissionInfo = OHOS::AAFwk::AbilityMissionInfo;


namespace OHOS {
namespace AppExecFwk {
const int BUFFER_LENGTH_MAX = 128;
const int DEFAULT_STACK_ID = 1;
const int DEFAULT_LAST_MEMORY_LEVEL = -1;
const int DEFAULT_WEIGHT = -1;
const int MAX_MISSION_NUM = 65535;
const int QUERY_RECENT_RUNNING_MISSION_INFO_TYPE = 2;
const int BUSINESS_ERROR_CODE_OK = 0;
const uint8_t NUMBER_OF_PARAMETERS_TWO = 2;
const uint8_t NUMBER_OF_PARAMETERS_THREE = 3;

struct AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    std::vector<RunningProcessInfo> info;
};

struct AsyncMissionInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    int32_t maxMissionNum = 0;
    int32_t queryType = 0;
    std::vector<AbilityMissionInfo> recentMissionInfo;
};

struct AsyncRemoveMissionCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    int32_t index = -1;
    int32_t result = -1;
};

struct AsyncRemoveMissionsCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    std::vector<std::int32_t> missionIds;
    int32_t result = -1;
};

struct AsyncClearMissionsCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    int32_t result = -1;
};

struct AsyncMoveMissionToTopCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    int32_t index = -1;
    int32_t result = -1;
};

struct AsyncKillProcessCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    std::string bundleName;
    int32_t result = -1;
};

struct AsyncClearUpApplicationDataCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    std::string bundleName;
    int32_t result = -1;
};

struct AsyncPreviousMissionInfosCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {0};
    int32_t maxMissionNum = 0;
    std::vector<AbilityMissionInfo> previousMissionInfo;
};

struct SystemMemroyInfoCB {
    std::shared_ptr<SystemMemoryAttr> info = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

napi_value NAPI_GetAllRunningProcesses(napi_env env, napi_callback_info info);
napi_value NAPI_GetActiveProcessInfos(napi_env env, napi_callback_info info);
napi_value NAPI_QueryRunningAbilityMissionInfos(napi_env env, napi_callback_info info);
napi_value NAPI_GetActiveAbilityMissionInfos(napi_env env, napi_callback_info info);
napi_value NAPI_QueryRecentAbilityMissionInfos(napi_env env, napi_callback_info info);
napi_value NAPI_GetPreviousAbilityMissionInfos(napi_env env, napi_callback_info info);
napi_value NAPI_RemoveMission(napi_env env, napi_callback_info info);
napi_value NAPI_RemoveMissions(napi_env env, napi_callback_info info);
napi_value NAPI_ClearMissions(napi_env env, napi_callback_info info);
napi_value NAPI_MoveMissionToTop(napi_env env, napi_callback_info info);
napi_value NAPI_KillProcessesByBundleName(napi_env env, napi_callback_info info);
napi_value NAPI_ClearUpApplicationData(napi_env env, napi_callback_info info);
void CreateWeightReasonCodeObject(napi_env env, napi_value value);
napi_value GetCallbackErrorValue(napi_env env, int errCode);
napi_value NAPI_GetSystemMemoryAttr(napi_env env, napi_callback_info);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  //  NAPI_ABILITY_MANAGER_H