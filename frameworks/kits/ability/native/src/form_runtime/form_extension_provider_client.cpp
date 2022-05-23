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

#include "form_runtime/form_extension_provider_client.h"

#include "string_ex.h"

#include "appexecfwk_errors.h"
#include "event_handler.h"
#include "event_runner.h"
#include "form_extension.h"
#include "form_mgr_errors.h"
#include "form_supply_proxy.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

/**
 * @brief Acquire to give back an ProviderFormInfo. This is sync API.
 * @param formId The Id of the form.
 * @param want The want of the form to create.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::AcquireProviderFormInfo(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_WARN("%{public}s warn, IFormSupply is nullptr", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    Want connectWant(want);
    connectWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    connectWant.SetParam(Constants::FORM_CONNECT_ID, want.GetLongParam(Constants::FORM_CONNECT_ID, 0));
    connectWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    connectWant.SetParam(Constants::PROVIDER_FLAG, true);
    connectWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));

    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_WARN("%{public}s warn, AcquireProviderFormInfo caller permission denied.", __func__);
        FormProviderInfo formProviderInfo;
        connectWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_PERMISSION_DENY);
        return FormProviderClient::HandleAcquire(formProviderInfo, connectWant, callerToken);
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> acquireProviderInfoFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->AcquireFormExtensionProviderInfo(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(acquireProviderInfoFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::AcquireFormExtensionProviderInfo(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    Want connectWant(want);
    connectWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    connectWant.SetParam(Constants::FORM_CONNECT_ID, want.GetLongParam(Constants::FORM_CONNECT_ID, 0));
    connectWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    connectWant.SetParam(Constants::PROVIDER_FLAG, true);
    connectWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));

    FormProviderInfo formProviderInfo;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        connectWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    } else {
        Want createWant(want);
        createWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));
        createWant.RemoveParam(Constants::FORM_CONNECT_ID);
        createWant.RemoveParam(Constants::ACQUIRE_TYPE);
        createWant.RemoveParam(Constants::FORM_SUPPLY_INFO);
        createWant.SetElement(want.GetElement());

        formProviderInfo = ownerFormExtension->OnCreate(createWant);
        HILOG_DEBUG("%{public}s, formId: %{public}s, data: %{public}s",
            __func__, createWant.GetStringParam(Constants::PARAM_FORM_IDENTITY_KEY).c_str(),
            formProviderInfo.GetFormDataString().c_str());
    }

    int error = FormProviderClient::HandleAcquire(formProviderInfo, connectWant, callerToken);
    if (error != ERR_OK) {
        HILOG_ERROR("%{public}s HandleAcquire failed", __func__);
        HandleResultCode(error, connectWant, callerToken);
    }
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Notify provider when the form was deleted.
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::NotifyFormDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionDeleteFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionDelete(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionDeleteFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnDestroy(formId);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Notify provider when the forms was deleted.
 *
 * @param formIds The id list of forms.
 * @param want The want of the request.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionsDeleteFunc = [client = sptr<FormExtensionProviderClient>(this),
        formIds, want, callerToken]() {
        client->NotifyFormExtensionsDelete(formIds, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionsDeleteFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionsDelete(const std::vector<int64_t> &formIds,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        for (int64_t formId : formIds) {
            ownerFormExtension->OnDestroy(formId);
        }
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Notify provider when the form need update.
 *
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::NotifyFormUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionUpdateFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionUpdate(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionUpdateFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnUpdate(formId);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Event notify when change the form visible.
 *
 * @param formIds The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> eventNotifyExtensionFunc = [client = sptr<FormExtensionProviderClient>(this),
        formIds, formVisibleType, want, callerToken]() {
        client->EventNotifyExtension(formIds, formVisibleType, want, callerToken);
    };
    mainHandler->PostSyncTask(eventNotifyExtensionFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::EventNotifyExtension(const std::vector<int64_t> &formIds,
    const int32_t formVisibleType, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        std::map<int64_t, int32_t> formEventsMap;
        for (const auto &formId : formIds) {
            formEventsMap.insert(std::make_pair(formId, formVisibleType));
        }
        ownerFormExtension->OnVisibilityChange(formEventsMap);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Notify provider when the temp form was cast to normal form.
 * @param formId The Id of the form to update.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller form extension token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::NotifyFormCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionCastTempFormFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionCastTempForm(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionCastTempFormFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnCastToNormal(formId);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Fire message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::FireFormEvent(const int64_t formId, const std::string &message,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> fireFormExtensionEventFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, message, want, callerToken]() {
        client->FireFormExtensionEvent(formId, message, want, callerToken);
    };
    mainHandler->PostSyncTask(fireFormExtensionEventFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::FireFormExtensionEvent(const int64_t formId, const std::string &message,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnEvent(formId, message);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Acquire form state to form provider.
 * @param wantArg The want of onAcquireFormState.
 * @param provider The provider info.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormExtensionProviderClient::AcquireState(const Want &wantArg, const std::string &provider, const Want &want,
                                              const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("%{public}s CheckParam failed", __func__);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionAcquireStateFunc = [client = sptr<FormExtensionProviderClient>(this),
        wantArg, provider, want, callerToken]() {
        client->NotifyFormExtensionAcquireState(wantArg, provider, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionAcquireStateFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionAcquireState(const Want &wantArg, const std::string &provider,
                                                                  const Want &want,
                                                                  const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormState state = FormState::UNKNOWN;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ownerFormExtension is nullptr.", __func__);
    } else {
        state = ownerFormExtension->OnAcquireFormState(wantArg);
    }
    HandleAcquireStateResult(state, provider, wantArg, want, callerToken);
    HILOG_INFO("%{public}s called end.", __func__);
}

/**
 * @brief Set the owner form extension of the form provider client.
 *
 * @param formExtension The owner form extension of the form provider client.
 */
void FormExtensionProviderClient::SetOwner(const std::shared_ptr<FormExtension> formExtension)
{
    if (formExtension == nullptr) {
        HILOG_ERROR("%{public}s error, SetOwner::formExtension is nullptr.", __func__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(formExtensionMutex_);
        ownerFormExtension_ = formExtension;
    }
}

/**
 * @brief Clear the owner form extension of the form provider client.
 *
 * @param formExtension The owner form extension of the form provider client.
 */
void FormExtensionProviderClient::ClearOwner(const std::shared_ptr<FormExtension> formExtension)
{
    if (formExtension == nullptr) {
        HILOG_ERROR("%{public}s error, ClearOwner::formExtension is nullptr.", __func__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(formExtensionMutex_);
        std::shared_ptr<FormExtension> ownerFormExtension = ownerFormExtension_.lock();
        if (formExtension == ownerFormExtension) {
            ownerFormExtension_.reset();
        }
    }
}

std::shared_ptr<FormExtension> FormExtensionProviderClient::GetOwner()
{
    std::shared_ptr<FormExtension> owner = nullptr;
    {
        std::lock_guard<std::mutex> lock(formExtensionMutex_);
        owner = ownerFormExtension_.lock();
    }
    return owner;
}

int FormExtensionProviderClient::HandleResultCode(int errorCode, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    // The error code for disconnect.
    int disconnectErrorCode = FormProviderClient::HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

std::pair<int, int> FormExtensionProviderClient::CheckParam(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_ERROR("%{public}s warn, IFormSupply is nullptr", __func__);
        return std::pair<int, int>(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED, ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    }
    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_ERROR("%{public}s warn, caller permission denied.", __func__);
        int errorCode = HandleResultCode(ERR_APPEXECFWK_FORM_PERMISSION_DENY, want, callerToken);
        return std::pair<int, int>(ERR_APPEXECFWK_FORM_PERMISSION_DENY, errorCode);
    }
    return std::pair<int, int>(ERR_OK, ERR_OK);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
