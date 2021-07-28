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
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "ability.h"
#include "want.h"
#include "../inner/napi_common/napi_common.h"
#include "dummy_values_bucket.h"
#include "dummy_data_ability_predicates.h"

using Want = OHOS::AAFwk::Want;
using Ability = OHOS::AppExecFwk::Ability;
using AbilityStartSetting = OHOS::AppExecFwk::AbilityStartSetting;

namespace OHOS {
namespace AppExecFwk {
class FeatureAbility;

#define NO_ERROR 0

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
struct AsyncCallbackInfo {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability;
    CallAbilityParam param;
    CallbackInfo *aceCallback;
    bool native_result;
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork;
    napi_deferred deferred;
    Ability *ability = nullptr;
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
    int32_t flags = 0;  // no data
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

struct DataAbilityHelperCB {
    CBBase cbBase;
    napi_ref uri = nullptr;
    napi_value result = nullptr;
};

struct DAHelperInsertCB {
    CBBase cbBase;
    DataAbilityHelper *dataAbilityHelper = nullptr;
    std::string uri;
    ValuesBucket valueBucket;
    int result = 0;
};

static inline std::string NapiValueToStringUtf8(napi_env env, napi_value value)
{
    std::string result = "";
	return UnwrapStringFromJS(env, value, result);
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_FEATURE_ABILITY_COMMON_H */
