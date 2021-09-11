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

namespace OHOS {
namespace AAFwk {
namespace AbilitUtil {
#define CHECK_POINTER_CONTINUE(object)                                                 \
    if (!object) {                                                                     \
        HILOG_ERROR("pointer is nullptr, %{public}s, %{public}d", __func__, __LINE__); \
        continue;                                                                      \
    }

#define CHECK_POINTER(object)                                                          \
    if (!object) {                                                                     \
        HILOG_ERROR("pointer is nullptr, %{public}s, %{public}d", __func__, __LINE__); \
        return;                                                                        \
    }

#define CHECK_POINTER_AND_RETURN(object, value)                                        \
    if (!object) {                                                                     \
        HILOG_ERROR("pointer is nullptr, %{public}s, %{public}d", __func__, __LINE__); \
        return value;                                                                  \
    }

#define CHECK_POINTER_RETURN_BOOL(object)                                              \
    if (!object) {                                                                     \
        HILOG_ERROR("pointer is nullptr, %{public}s, %{public}d", __func__, __LINE__); \
        return false;                                                                  \
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
}  // namespace AbilitUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_UTIL_H
