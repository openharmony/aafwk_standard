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

#include "ability_loader.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AbilityLoader &AbilityLoader::GetInstance()
{
    static AbilityLoader abilityLoader;
    return abilityLoader;
}

/**
 * @brief Register Ability Info
 *
 * @param abilityName ability classname
 * @param createFunc  Constructor address
 */
void AbilityLoader::RegisterAbility(const std::string &abilityName, const CreateAblity &createFunc)
{
    abilities_.emplace(abilityName, createFunc);
    HILOG_DEBUG("AbilityLoader::RegisterAbility:%{public}s", abilityName.c_str());
}

/**
 * @brief Register Extension Info
 *
 * @param abilityName Extension classname
 * @param createFunc  Constructor address
 */
void AbilityLoader::RegisterExtension(const std::string &abilityName, const CreateExtension &createFunc)
{
    extensions_.emplace(abilityName, createFunc);
    HILOG_DEBUG("AbilityLoader::RegisterExtension:%{public}s", abilityName.c_str());
}

/**
 * @brief Get Ability address
 *
 * @param abilityName ability classname
 *
 * @return return Ability address
 */
Ability *AbilityLoader::GetAbilityByName(const std::string &abilityName)
{
    auto it = abilities_.find(abilityName);
    if (it != abilities_.end()) {
        return it->second();
    } else {
        HILOG_ERROR("AbilityLoader::GetAbilityByName failed:%{public}s", abilityName.c_str());
    }
    return nullptr;
}

/**
 * @brief Get Extension address
 *
 * @param abilityName Extension classname
 *
 * @return return Extension address
 */
AbilityRuntime::Extension *AbilityLoader::GetExtensionByName(const std::string &abilityName)
{
    auto it = extensions_.find(abilityName);
    if (it != extensions_.end()) {
        return it->second();
    } else {
        HILOG_ERROR("AbilityLoader::GetExtensionByName failed:%{public}s", abilityName.c_str());
    }
    return nullptr;
}

#ifdef ABILITY_WINDOW_SUPPORT
void AbilityLoader::RegisterAbilitySlice(const std::string &sliceName, const CreateSlice &createFunc)
{
    slices_.emplace(sliceName, createFunc);
    HILOG_INFO(HILOG_MODULE_APP, "RegisterAbilitySlice %s", sliceName.c_str());
}

AbilitySlice *AbilityLoader::GetAbilitySliceByName(const std::string &sliceName)
{
    auto it = slices_.find(sliceName);
    if (it != slices_.end()) {
        return it->second();
    } else {
        HILOG_ERROR(HILOG_MODULE_APP, "GetAbilitySliceByName failed: %s", sliceName.c_str());
        return nullptr;
    }
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS