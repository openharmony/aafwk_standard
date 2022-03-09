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

#include "ability_process.h"

#include <dlfcn.h>

#include "hilog_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
static void *g_handle = nullptr;
#ifdef _ARM64_
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib64/module/ability/libfeatureability.z.so";
#else
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib/module/ability/libfeatureability.z.so";
#endif
constexpr char FUNC_CALL_ON_ABILITY_RESULT[] = "CallOnAbilityResult";
using NAPICallOnAbilityResult = void (*)(int requestCode, int resultCode, const Want &resultData, CallbackInfo cb);
constexpr char FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT[] = "CallOnRequestPermissionsFromUserResult";
using NAPICallOnRequestPermissionsFromUserResult = void (*)(int requestCode,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults, CallbackInfo callbackInfo);

std::shared_ptr<AbilityProcess> AbilityProcess::instance_ = nullptr;
std::map<Ability *, std::map<int, CallbackInfo>> AbilityProcess::abilityResultMap_;
std::map<Ability *, std::map<int, CallbackInfo>> AbilityProcess::abilityRequestPermissionsForUserMap_;
std::mutex AbilityProcess::mutex_;
std::shared_ptr<AbilityProcess> AbilityProcess::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityProcess>();
        }
    }
    return instance_;
}

AbilityProcess::AbilityProcess()
{}

AbilityProcess::~AbilityProcess()
{}

ErrCode AbilityProcess::StartAbility(Ability *ability, CallAbilityParam param, CallbackInfo callback)
{
    HILOG_INFO("AbilityProcess::StartAbility begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::StartAbility ability is nullptr");
        return ERR_NULL_OBJECT;
    }

    // inherit split mode
    auto windowMode = ability->GetCurrentWindowMode();
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        param.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("window mode is %{public}d", windowMode);

    ErrCode err = ERR_OK;
    if (param.forResultOption == true) {
        if (param.setting == nullptr) {
            HILOG_INFO("%{public}s param.setting == nullptr call StartAbilityForResult.", __func__);
            err = ability->StartAbilityForResult(param.want, param.requestCode);
        } else {
            HILOG_INFO("%{public}s param.setting != nullptr call StartAbilityForResult.", __func__);
            err = ability->StartAbilityForResult(param.want, param.requestCode, *(param.setting));
        }

        std::lock_guard<std::mutex> lock_l(mutex_);

        std::map<int, CallbackInfo> map;
        auto it = abilityResultMap_.find(ability);
        if (it == abilityResultMap_.end()) {
            HILOG_INFO("AbilityProcess::StartAbility ability: %{public}p is not in the abilityResultMap_", ability);
        } else {
            HILOG_INFO("AbilityProcess::StartAbility ability: %{public}p is in the abilityResultMap_", ability);
            map = it->second;
        }
        callback.errCode = err;
        map[param.requestCode] = callback;
        abilityResultMap_[ability] = map;
    } else {
        if (param.setting == nullptr) {
            HILOG_INFO("%{public}s param.setting == nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want);
        } else {
            HILOG_INFO("%{public}s param.setting != nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want, *(param.setting));
        }
    }
    HILOG_INFO("AbilityProcess::StartAbility end");
    return err;
}

void AbilityProcess::OnAbilityResult(Ability *ability, int requestCode, int resultCode, const Want &resultData)
{
    HILOG_INFO("AbilityProcess::OnAbilityResult begin");

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        HILOG_ERROR("AbilityProcess::OnAbilityResult ability: %{public}p is not in the abilityResultMap", ability);
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        HILOG_ERROR("AbilityProcess::OnAbilityResult requestCode: %{public}d is not in the map", requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;

    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            HILOG_ERROR("%{public}s, dlopen failed %{public}s. %{public}s",
                __func__,
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }

    // get function
    auto func = reinterpret_cast<NAPICallOnAbilityResult>(dlsym(g_handle, FUNC_CALL_ON_ABILITY_RESULT));
    if (func == nullptr) {
        HILOG_ERROR(
            "%{public}s, dlsym failed %{public}s. %{public}s", __func__, FUNC_CALL_ON_ABILITY_RESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, resultCode, resultData, callbackInfo);

    map.erase(requestCode);

    abilityResultMap_[ability] = map;
    HILOG_INFO("AbilityProcess::OnAbilityResult end");
}

void AbilityProcess::RequestPermissionsFromUser(
    Ability *ability, CallAbilityPermissionParam &param, CallbackInfo callbackInfo)
{
    HILOG_INFO("AbilityProcess::RequestPermissionsFromUser begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::RequestPermissionsFromUser ability is nullptr");
        return;
    }

    ability->RequestPermissionsFromUser(param.permission_list, param.requestCode);

    {
        std::lock_guard<std::mutex> lock_l(mutex_);
        std::map<int, CallbackInfo> map;
        auto it = abilityRequestPermissionsForUserMap_.find(ability);
        if (it == abilityRequestPermissionsForUserMap_.end()) {
            HILOG_INFO("AbilityProcess::RequestPermissionsFromUser ability: %{public}p is not in the "
                     "abilityRequestPermissionsForUserMap_",
                ability);
        } else {
            HILOG_INFO("AbilityProcess::RequestPermissionsFromUser ability: %{public}p is in the "
                     "abilityRequestPermissionsForUserMap_",
                ability);
            map = it->second;
        }

        map[param.requestCode] = callbackInfo;
        abilityRequestPermissionsForUserMap_[ability] = map;
    }
    HILOG_INFO("AbilityProcess::RequestPermissionsFromUser end");
}

void AbilityProcess::OnRequestPermissionsFromUserResult(Ability *ability, int requestCode,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{
    HILOG_INFO("AbilityProcess::OnRequestPermissionsFromUserResult begin");
    if (ability == nullptr) {
        HILOG_ERROR("AbilityProcess::OnRequestPermissionsFromUserResult ability is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityRequestPermissionsForUserMap_.find(ability);
    if (it == abilityRequestPermissionsForUserMap_.end()) {
        HILOG_ERROR("AbilityProcess::OnRequestPermissionsFromUserResult ability: %{public}p is not in the "
                 "abilityRequestPermissionsForUserMap_",
            ability);
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        HILOG_ERROR("AbilityProcess::OnRequestPermissionsFromUserResult requestCode: %{public}d is not in the map",
            requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;

    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            HILOG_ERROR("%{public}s, dlopen failed %{public}s. %{public}s",
                __func__,
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }

    // get function
    auto func = reinterpret_cast<NAPICallOnRequestPermissionsFromUserResult>(
        dlsym(g_handle, FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT));
    if (func == nullptr) {
        HILOG_ERROR("%{public}s, dlsym failed %{public}s. %{public}s",
            __func__,
            FUNC_CALL_ON_REQUEST_PERMISSIONS_FROM_USERRESULT,
            dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, permissions, grantResults, callbackInfo);
    map.erase(requestCode);

    abilityRequestPermissionsForUserMap_[ability] = map;
    HILOG_INFO("AbilityProcess::OnRequestPermissionsFromUserResult end");
}
}  // namespace AppExecFwk
}  // namespace OHOS