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

#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
static void *g_handle = nullptr;
constexpr char SHARED_LIBRARY_FEATURE_ABILITY[] = "/system/lib/module/ability/libfeatureability.z.so";
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
    APP_LOGI("AbilityProcess::StartAbility begin");
    if (ability == nullptr) {
        APP_LOGE("AbilityProcess::StartAbility ability is nullptr");
        return ERR_NULL_OBJECT;
    }

    ErrCode err = ERR_OK;

    if (param.forResultOption == true) {
        if (param.setting == nullptr) {
            APP_LOGI("%{public}s param.setting == nullptr call StartAbilityForResult.", __func__);
            err = ability->StartAbilityForResult(param.want, param.requestCode);
        } else {
            APP_LOGI("%{public}s param.setting != nullptr call StartAbilityForResult.", __func__);
            err = ability->StartAbilityForResult(param.want, param.requestCode, *(param.setting));
        }

        std::lock_guard<std::mutex> lock_l(mutex_);

        std::map<int, CallbackInfo> map;
        auto it = abilityResultMap_.find(ability);
        if (it == abilityResultMap_.end()) {
            APP_LOGI("AbilityProcess::StartAbility ability: %{public}p is not in the abilityResultMap_", ability);
        } else {
            APP_LOGI("AbilityProcess::StartAbility ability: %{public}p is in the abilityResultMap_", ability);
            map = it->second;
        }

        map[param.requestCode] = callback;
        abilityResultMap_[ability] = map;
    } else {
        if (param.setting == nullptr) {
            APP_LOGI("%{public}s param.setting == nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want);
        } else {
            APP_LOGI("%{public}s param.setting != nullptr call StartAbility.", __func__);
            err = ability->StartAbility(param.want, *(param.setting));
        }
    }
    APP_LOGI("AbilityProcess::StartAbility end");
    return err;
}

void AbilityProcess::OnAbilityResult(Ability *ability, int requestCode, int resultCode, const Want &resultData)
{
    APP_LOGI("AbilityProcess::OnAbilityResult begin");

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityResultMap_.find(ability);
    if (it == abilityResultMap_.end()) {
        APP_LOGE("AbilityProcess::OnAbilityResult ability: %{public}p is not in the abilityResultMap", ability);
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        APP_LOGE("AbilityProcess::OnAbilityResult requestCode: %{public}d is not in the map", requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;

    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            APP_LOGE("%{public}s, dlopen failed %{public}s. %{public}s",
                __func__,
                SHARED_LIBRARY_FEATURE_ABILITY,
                dlerror());
            return;
        }
    }

    // get function
    auto func = reinterpret_cast<NAPICallOnAbilityResult>(dlsym(g_handle, FUNC_CALL_ON_ABILITY_RESULT));
    if (func == nullptr) {
        APP_LOGE("%{public}s, dlsym failed %{public}s. %{public}s", __func__, FUNC_CALL_ON_ABILITY_RESULT, dlerror());
        dlclose(g_handle);
        g_handle = nullptr;
        return;
    }
    func(requestCode, resultCode, resultData, callbackInfo);

    map.erase(requestCode);

    abilityResultMap_[ability] = map;
    APP_LOGI("AbilityProcess::OnAbilityResult end");
}

void AbilityProcess::RequestPermissionsFromUser(
    Ability *ability, CallAbilityPermissionParam &param, CallbackInfo callbackInfo)
{
    APP_LOGI("AbilityProcess::RequestPermissionsFromUser begin");
    if (ability == nullptr) {
        APP_LOGE("AbilityProcess::RequestPermissionsFromUser ability is nullptr");
        return;
    }

    ability->RequestPermissionsFromUser(param.permission_list, param.requestCode);

    {
        std::lock_guard<std::mutex> lock_l(mutex_);
        std::map<int, CallbackInfo> map;
        auto it = abilityRequestPermissionsForUserMap_.find(ability);
        if (it == abilityRequestPermissionsForUserMap_.end()) {
            APP_LOGI("AbilityProcess::RequestPermissionsFromUser ability: %{public}p is not in the "
                     "abilityRequestPermissionsForUserMap_",
                ability);
        } else {
            APP_LOGI("AbilityProcess::RequestPermissionsFromUser ability: %{public}p is in the "
                     "abilityRequestPermissionsForUserMap_",
                ability);
            map = it->second;
        }

        map[param.requestCode] = callbackInfo;
        abilityRequestPermissionsForUserMap_[ability] = map;
    }
    APP_LOGI("AbilityProcess::RequestPermissionsFromUser end");
}

void AbilityProcess::OnRequestPermissionsFromUserResult(Ability *ability, int requestCode,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{
    APP_LOGI("AbilityProcess::OnRequestPermissionsFromUserResult begin");
    if (ability == nullptr) {
        APP_LOGE("AbilityProcess::OnRequestPermissionsFromUserResult ability is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock_l(mutex_);

    auto it = abilityRequestPermissionsForUserMap_.find(ability);
    if (it == abilityRequestPermissionsForUserMap_.end()) {
        APP_LOGE("AbilityProcess::OnRequestPermissionsFromUserResult ability: %{public}p is not in the "
                 "abilityRequestPermissionsForUserMap_",
            ability);
        return;
    }
    std::map<int, CallbackInfo> map = it->second;

    auto callback = map.find(requestCode);
    if (callback == map.end()) {
        APP_LOGE("AbilityProcess::OnRequestPermissionsFromUserResult requestCode: %{public}d is not in the map",
            requestCode);
        return;
    }
    CallbackInfo callbackInfo = callback->second;

    // start open featureability lib
    if (g_handle == nullptr) {
        g_handle = dlopen(SHARED_LIBRARY_FEATURE_ABILITY, RTLD_LAZY);
        if (g_handle == nullptr) {
            APP_LOGE("%{public}s, dlopen failed %{public}s. %{public}s",
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
        APP_LOGE("%{public}s, dlsym failed %{public}s. %{public}s",
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
    APP_LOGI("AbilityProcess::OnRequestPermissionsFromUserResult end");
}
}  // namespace AppExecFwk
}  // namespace OHOS