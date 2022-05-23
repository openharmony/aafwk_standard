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

#include <cinttypes>

#include "hilog_wrapper.h"
#include "form_constants.h"
#include "form_host_client.h"

namespace OHOS {
namespace AppExecFwk {
sptr<FormHostClient> FormHostClient::instance_ = nullptr;
std::mutex FormHostClient::instanceMutex_;

FormHostClient::FormHostClient()
{
}

FormHostClient::~FormHostClient()
{
}

/**
 * @brief Get FormHostClient instance.
 *
 * @return FormHostClient instance.
 */
sptr<FormHostClient> FormHostClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(instanceMutex_);
        if (instance_ == nullptr) {
            instance_ = new FormHostClient();
        }
    }
    return instance_;
}

/**
 * @brief Add form callback.
 *
 * @param formCallback the host's form callback.
 * @param formId The Id of the form.
 * @return none.
 */
void FormHostClient::AddForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0 || formCallback == nullptr) {
        HILOG_ERROR("%{public}s error, invalid formId or formCallback.", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        std::set<std::shared_ptr<FormCallbackInterface>> callbacks;
        callbacks.emplace(formCallback);
        formCallbackMap_.emplace(formId, callbacks);
    } else {
        iter->second.emplace(formCallback);
    }
}

/**
 * @brief Remove form callback.
 *
 * @param formCallback the host's form callback.
 * @param formId The Id of the form.
 * @return none.
 */
void FormHostClient::RemoveForm(std::shared_ptr<FormCallbackInterface> formCallback, const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formId <= 0 || formCallback == nullptr) {
        HILOG_ERROR("%{public}s, invalid formId or formCallback.", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        HILOG_ERROR("%{public}s, not find formId:%{public}s.", __func__, std::to_string(formId).c_str());
        return;
    }
    iter->second.erase(formCallback);
    if (iter->second.empty()) {
        formCallbackMap_.erase(iter);
    }
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Check whether the form exist in the formhosts.
 *
 * @param formId The Id of the form.
 * @return Returns true if contains form; returns false otherwise.
 */
bool FormHostClient::ContainsForm(int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    return formCallbackMap_.find(formId) != formCallbackMap_.end();
}

/**
 * @brief Add form state.
 *
 * @param formStateCallback the host's form state callback.
 * @param want the want of acquiring form state.
 * @return Returns true if contains form; returns false otherwise.
 */
bool FormHostClient::AddFormState(std::shared_ptr<FormStateCallbackInterface> &formStateCallback,
                                  const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    const std::string doubleColon = "::";
    std::string key;
    key.append(bundleName).append(doubleColon).append(abilityName).append(doubleColon)
        .append(want.GetStringParam(AppExecFwk::Constants::PARAM_MODULE_NAME_KEY)).append(doubleColon)
        .append(want.GetStringParam(AppExecFwk::Constants::PARAM_FORM_NAME_KEY)).append(doubleColon)
        .append(std::to_string(want.GetIntParam(AppExecFwk::Constants::PARAM_FORM_DIMENSION_KEY, 1)));
    std::lock_guard<std::mutex> lock(formStateCallbackMutex_);
    auto iter = formStateCallbackMap_.find(key);
    if (iter == formStateCallbackMap_.end()) {
        std::set<std::shared_ptr<FormStateCallbackInterface>> callbacks;
        callbacks.emplace(formStateCallback);
        formStateCallbackMap_.emplace(key, callbacks);
    } else {
        iter->second.insert(formStateCallback);
    }
    HILOG_INFO("%{public}s done.", __func__);
    return true;
}

bool FormHostClient::RegisterUninstallCallback(UninstallCallback callback)
{
    std::lock_guard<std::mutex> lock(uninstallCallbackMutex_);
    uninstallCallback_ = callback;
    return true;
}

/**
 * @brief Request to give back a form.
 *
 * @param formJsInfo Form js info.
 * @return none.
 */
void FormHostClient::OnAcquired(const FormJsInfo &formJsInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    HILOG_INFO("Imamge number is %{public}zu.", formJsInfo.imageDataMap.size());
    int64_t formId = formJsInfo.formId;
    if (formId < 0) {
        HILOG_ERROR("%{public}s error, the passed form id can't be negative.", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        HILOG_ERROR("%{public}s error, not find formId:%{public}s.", __func__, std::to_string(formId).c_str());
        return;
    }
    for (const auto& callback : iter->second) {
        HILOG_INFO("%{public}s, formId: %{public}" PRId64 ", jspath: %{public}s, data: %{public}s",
            __func__, formId, formJsInfo.jsFormCodePath.c_str(), formJsInfo.formData.c_str());
        callback->ProcessFormUpdate(formJsInfo);
    }
}

/**
 * @brief Update form.
 *
 * @param formJsInfo Form js info.
 * @return none.
 */
void FormHostClient::OnUpdate(const FormJsInfo &formJsInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    HILOG_INFO("Imamge number is %{public}zu.", formJsInfo.imageDataMap.size());
    int64_t formId = formJsInfo.formId;
    if (formId < 0) {
        HILOG_ERROR("%{public}s error, the passed form id can't be negative.", __func__);
        return;
    }
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto iter = formCallbackMap_.find(formId);
    if (iter == formCallbackMap_.end()) {
        HILOG_ERROR("%{public}s error, not find formId:%{public}s.", __func__, std::to_string(formId).c_str());
        return;
    }
    for (const auto& callback : iter->second) {
        HILOG_INFO("%{public}s, formId: %{public}" PRId64 ", jspath: %{public}s, data: %{public}s",
            __func__, formId, formJsInfo.jsFormCodePath.c_str(), formJsInfo.formData.c_str());
        callback->ProcessFormUpdate(formJsInfo);
    }
}

/**
 * @brief UnInstall the forms.
 *
 * @param formIds The Id of the forms.
 * @return none.
 */
void FormHostClient::OnUninstall(const std::vector<int64_t> &formIds)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (formIds.empty()) {
        HILOG_ERROR("%{public}s error, formIds is empty.", __func__);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(uninstallCallbackMutex_);
        if (uninstallCallback_ != nullptr) {
            uninstallCallback_(formIds);
        }
    }
    for (auto &formId : formIds) {
        if (formId < 0) {
            HILOG_ERROR("%{public}s error, the passed form id can't be negative.", __func__);
            continue;
        }
        std::lock_guard<std::mutex> lock(callbackMutex_);
        auto iter = formCallbackMap_.find(formId);
        if (iter == formCallbackMap_.end()) {
            HILOG_ERROR("%{public}s error, not find formId:%{public}s.", __func__, std::to_string(formId).c_str());
            continue;
        }
        for (const auto& callback : iter->second) {
            HILOG_ERROR("%{public}s uninstall formId:%{public}s.", __func__, std::to_string(formId).c_str());
            callback->ProcessFormUninstall(formId);
        }
    }
}

/**
 * @brief Form provider is acquire state
 * @param state The form state.
 * @param want The form want.
 */
void FormHostClient::OnAcquireState(FormState state, const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s state:%{public}d.", __func__, state);
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    const std::string doubleColon = "::";
    std::string key;
    key.append(bundleName).append(doubleColon).append(abilityName).append(doubleColon)
        .append(want.GetStringParam(AppExecFwk::Constants::PARAM_MODULE_NAME_KEY)).append(doubleColon)
        .append(want.GetStringParam(AppExecFwk::Constants::PARAM_FORM_NAME_KEY)).append(doubleColon)
        .append(std::to_string(want.GetIntParam(AppExecFwk::Constants::PARAM_FORM_DIMENSION_KEY, 1)));

    std::lock_guard<std::mutex> lock(formStateCallbackMutex_);
    auto iter = formStateCallbackMap_.find(key);
    if (iter == formStateCallbackMap_.end()) {
        HILOG_INFO("form state callback not found");
    } else {
        std::set<std::shared_ptr<FormStateCallbackInterface>> &callbackSet = iter->second;
        for (auto &callback: callbackSet) {
            callback->ProcessAcquireState(state);
        }
        formStateCallbackMap_.erase(iter);
    }
    HILOG_INFO("%{public}s done", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
