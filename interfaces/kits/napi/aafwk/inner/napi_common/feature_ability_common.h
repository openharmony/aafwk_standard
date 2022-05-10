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

#ifndef OHOS_APPEXECFWK_FEATURE_ABILITY_COMMON_H
#define OHOS_APPEXECFWK_FEATURE_ABILITY_COMMON_H
#include "ability.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_common.h"
#include "napi_common_util.h"
#include "pac_map.h"
#include "values_bucket.h"
#include "want.h"

using Want = OHOS::AAFwk::Want;
using Ability = OHOS::AppExecFwk::Ability;
using AbilityStartSetting = OHOS::AppExecFwk::AbilityStartSetting;

namespace OHOS {
namespace AppExecFwk {
class FeatureAbility;

struct CallAbilityParam {
    Want want;
    int requestCode = 0;
    bool forResultOption = false;
    std::shared_ptr<AbilityStartSetting> setting = nullptr;
};

struct OnAbilityCallback {
    int requestCode = 0;
    int resultCode = 0;
    Want resultData;
    CallbackInfo cb;
};

struct ContinueAbilityOptionsInfo {
    bool reversible = false;
    std::string deviceId;
};

struct AsyncCallbackInfo {
    CallbackInfo cbInfo;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    Ability *ability = nullptr;
    AbilityRuntime::WantAgent::WantAgent *wantAgent = nullptr;
    CallAbilityParam param;
    CallbackInfo aceCallback;
    bool native_result;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
    ContinueAbilityOptionsInfo optionInfo;
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability = nullptr;
    AbilityType abilityType = AbilityType::UNKNOWN;
    int errCode = 0;
};

struct AppInfo_ {
    std::string name;
    std::string description;
    int32_t descriptionId = 0;
    bool systemApp = false;
    bool enabled = true;  // no data
    std::string label;
    std::string labelId;
    std::string icon;
    std::string iconId;
    std::string process;
    int32_t supportedModes = 0;
    std::vector<std::string> moduleSourceDirs;
    std::vector<std::string> permissions;
    std::vector<ModuleInfo> moduleInfos;
    std::string entryDir;
};

struct AppInfoCB {
    CBBase cbBase;
    AppInfo_ appInfo;
};

struct AppTypeCB {
    CBBase cbBase;
    std::string name;
};

struct AbilityInfo_ {
    std::string bundleName;
    std::string name;
    std::string label;
    std::string description;
    std::string icon;
    int32_t labelId;
    int32_t descriptionId;
    int32_t iconId;
    std::string moduleName;
    std::string process;
    std::string targetAbility;  // no data
    int32_t backgroundModes;    // no data
    bool isVisible = true;
    bool formEnabled = false;  // no data
    int32_t type = 0;
    int32_t subType = 0;  // no data
    int32_t orientation = 0;
    int32_t launchMode = 0;
    std::vector<std::string> permissions;
    std::vector<std::string> deviceTypes;
    std::vector<std::string> deviceCapabilities;
    std::string readPermission;   // no data
    std::string writePermission;  // no data
    AppInfo_ appInfo;
    int32_t formEntity;         // no data
    int32_t minFormHeight;      // no data
    int32_t defaultFormHeight;  // no data
    int32_t minFormWidth;       // no data
    int32_t defaultFormWidth;   // no data
    std::string uri;
};
struct AbilityInfoCB {
    CBBase cbBase;
    AbilityInfo_ abilityInfo;
};

struct AbilityNameCB {
    CBBase cbBase;
    std::string name;
};

struct ProcessInfoCB {
    CBBase cbBase;
    pid_t pid = 0;
    std::string processName;
};

struct ProcessNameCB {
    CBBase cbBase;
    std::string processName;
};

struct CallingBundleCB {
    CBBase cbBase;
    std::string callingBundleName;
};

struct GetOrCreateLocalDirCB {
    CBBase cbBase;
    std::string rootDir;
};

struct DatabaseDirCB {
    CBBase cbBase;
    std::string dataBaseDir;
};

struct PreferencesDirCB {
    CBBase cbBase;
    std::string preferencesDir;
};

struct ElementNameCB {
    CBBase cbBase;
    std::string deviceId;
    std::string bundleName;
    std::string abilityName;
    std::string uri;
    std::string shortName;
};

struct HapModuleInfo_ {
    std::string name;
    std::string description;
    int32_t descriptionId = 0;  // no data
    std::string icon;
    std::string label;
    int32_t labelId = 0;  // no data
    int32_t iconId = 0;   // no data
    std::string backgroundImg;
    int32_t supportedModes = 0;
    std::vector<std::string> reqCapabilities;
    std::vector<std::string> deviceTypes;
    std::vector<AbilityInfo_> abilityInfos;
    std::string moduleName;
    std::string mainAbilityName;  // no data
    bool installationFree;        // no data
};

struct HapModuleInfoCB {
    CBBase cbBase;
    HapModuleInfo_ hapModuleInfo;
};

struct AppVersionInfo {
    std::string appName;
    std::string versionName;
    int32_t versionCode = 0;
};

struct AppVersionInfoCB {
    CBBase cbBase;
    AppVersionInfo appVersionInfo;
};

struct DataAbilityHelperCB {
    CBBase cbBase;
    napi_ref uri = nullptr;
    napi_value result = nullptr;
};

struct DAHelperInsertCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    int result = 0;
    int execResult;
};

class NAPIAbilityConnection;
struct AbilityConnectionCB {
    napi_env env;
    napi_ref callback[3] = {0};  // onConnect/onDisconnect/onFailed
    int resultCode = 0;
    ElementName elementName;
    sptr<IRemoteObject> connection;
};
struct ConnectAbilityCB {
    CBBase cbBase;
    Want want;
    sptr<NAPIAbilityConnection> abilityConnection;
    AbilityConnectionCB abilityConnectionCB;
    int64_t id;
    bool result;
    int errCode = 0;
};

struct DAHelperNotifyChangeCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    int execResult;
};

class NAPIDataAbilityObserver;
struct DAHelperOnOffCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    sptr<NAPIDataAbilityObserver> observer;
    std::string uri;
    int result = 0;
    std::vector<DAHelperOnOffCB *> NotifyList;
    std::vector<DAHelperOnOffCB *> DestoryList;
};

struct ShowOnLockScreenCB {
    CBBase cbBase;
    bool isShow;
};

struct SetWakeUpScreenCB {
    CBBase cbBase;
    bool wakeUp;
};

static inline std::string NapiValueToStringUtf8(napi_env env, napi_value value)
{
    std::string result = "";
    return UnwrapStringFromJS(env, value, result);
}

static inline bool NapiValueToArrayStringUtf8(napi_env env, napi_value param, std::vector<std::string> &result)
{
    return UnwrapArrayStringFromJS(env, param, result);
}

struct DAHelperGetTypeCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperGetFileTypesCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string mimeTypeFilter;
    std::vector<std::string> result;
    int execResult;
};

struct DAHelperNormalizeUriCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};
struct DAHelperDenormalizeUriCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string result = "";
    int execResult;
};

struct DAHelperDeleteCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperQueryCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> result;
    int execResult;
};

struct DAHelperUpdateCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    NativeRdb::ValuesBucket valueBucket;
    NativeRdb::DataAbilityPredicates predicates;
    int result = 0;
    int execResult;
};

struct DAHelperCallCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string method;
    std::string arg;
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> result;
    int execResult;
};

struct DAHelperErrorCB {
    CBBase cbBase;
    int execResult;
};
struct DAHelperBatchInsertCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::vector<NativeRdb::ValuesBucket> values;
    int result = 0;
    int execResult;
};
struct DAHelperOpenFileCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    std::string mode;
    int result = 0;
    int execResult;
};

struct DAHelperReleaseCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    bool result = false;
};

struct DAHelperExecuteBatchCB {
    CBBase cbBase;
    std::string uri;
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::vector<std::shared_ptr<DataAbilityResult>> result;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_FEATURE_ABILITY_COMMON_H */
