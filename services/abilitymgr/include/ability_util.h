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

#ifndef OHOS_AAFWK_ABILITY_UTIL_H
#define OHOS_AAFWK_ABILITY_UTIL_H

#include <string>
#include "hilog_wrapper.h"
#include "ability_config.h"
#include "ipc_skeleton.h"
#include "sa_mgr_client.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "utils/system/safwk/native/include/system_ability_definition.h"
#include "ability_manager_errors.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t ROOT_UID = 0;

static constexpr unsigned int CHANGE_CONFIG_ALL_CHANGED = 0xFFFFFFFF;
static constexpr unsigned int CHANGE_CONFIG_NONE = 0x00000000;
static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
static constexpr unsigned int CHANGE_CONFIG_LAYOUT = 0x00000002;
static constexpr unsigned int CHANGE_CONFIG_FONTSIZE = 0x00000004;
static constexpr unsigned int CHANGE_CONFIG_ORIENTATION = 0x00000008;
static constexpr unsigned int CHANGE_CONFIG_DENSITY = 0x00000010;

#define CHECK_POINTER_CONTINUE(object)      \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        continue;                           \
    }

#define CHECK_POINTER_IS_NULLPTR(object)    \
    if (object == nullptr) {                \
        HILOG_ERROR("pointer is nullptr."); \
        return;                             \
    }

#define CHECK_POINTER(object)               \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        return;                             \
    }

#define CHECK_POINTER_LOG(object, log)   \
    if (!object) {                       \
        HILOG_ERROR("%{public}s:", log); \
        return;                          \
    }

#define CHECK_POINTER_AND_RETURN(object, value) \
    if (!object) {                              \
        HILOG_ERROR("pointer is nullptr.");     \
        return value;                           \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log) \
    if (!object) {                                       \
        HILOG_ERROR("%{public}s:", log);                 \
        return value;                                    \
    }

#define CHECK_POINTER_RETURN_BOOL(object)   \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        return false;                       \
    }

#define CHECK_RET_RETURN_RET(object, log)                         \
    if (object != ERR_OK) {                                       \
        HILOG_ERROR("%{public}s, ret : %{public}d", log, object); \
        return object;                                            \
    }

#define CHECK_TRUE_RETURN_RET(object, value, log) \
    if (object) {                                 \
        HILOG_WARN("%{public}s", log);            \
        return value;                             \
    }

[[maybe_unused]] static bool IsSystemDialogAbility(const std::string &bundleName, const std::string &abilityName)
{
    if (abilityName == AbilityConfig::SYSTEM_DIALOG_NAME && bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return true;
    }

    if (abilityName == AbilityConfig::DEVICE_MANAGER_NAME && bundleName == AbilityConfig::DEVICE_MANAGER_BUNDLE_NAME) {
        return true;
    }

    return false;
}

[[maybe_unused]] static std::string ConvertBundleNameSingleton(const std::string &bundleName, const std::string &name)
{
    std::string strName =
        AbilityConfig::MISSION_NAME_MARK_HEAD + bundleName + AbilityConfig::MISSION_NAME_SEPARATOR + name;
    return strName;
}

static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
[[maybe_unused]] static int64_t SystemTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

[[maybe_unused]] static int64_t UTCTimeSeconds()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t)(t.tv_sec);
}

static sptr<AppExecFwk::IBundleMgr> GetBundleManager()
{
    auto bundleObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
}

[[maybe_unused]] static int JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo, int callerUid = -1)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    if (!abilityInfo.visible) {
        HILOG_ERROR("ability visible is false");
        if (callerUid == -1) {
            callerUid = IPCSkeleton::GetCallingUid();
        }
        if (ROOT_UID == callerUid) {
            HILOG_ERROR("uid is root");
            return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
        }
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
        auto isSystemApp = bms->CheckIsSystemAppByUid(callerUid);
        if (callerUid != SYSTEM_UID && !isSystemApp) {
            HILOG_ERROR("caller is not systemAp or system");
            std::string bundleName;
            bool result = bms->GetBundleNameForUid(callerUid, bundleName);
            if (!result) {
                HILOG_ERROR("GetBundleNameForUid fail");
                return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
            }
            if (bundleName != abilityInfo.bundleName) {
                HILOG_ERROR("caller ability bundlename not equal abilityInfo.bundleName bundleName: %{public}s "
                            "abilityInfo.bundleName: %{public}s",
                    bundleName.c_str(),
                    abilityInfo.bundleName.c_str());
                return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
            }
        }
    }
    HILOG_DEBUG("%{public}s end", __func__);
    return ERR_OK;
}

inline bool IsSystemUiApp(const AppExecFwk::AbilityInfo &info)
{
    if (info.bundleName != AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return false;
    }
    return (info.name == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR || info.name == AbilityConfig::SYSTEM_UI_STATUS_BAR);
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_UTIL_H
