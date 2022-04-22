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

#include "form_provider_client.h"

#include <cinttypes>

#include "appexecfwk_errors.h"
#include "form_mgr_errors.h"
#include "form_supply_proxy.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Acquire to give back an ProviderFormInfo. This is sync API.
 * @param formId The Id of the form.
 * @param want The want of the form to create.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::AcquireProviderFormInfo(
    const int64_t formId,
    const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);

    Want newWant(want);
    newWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    newWant.SetParam(Constants::FORM_CONNECT_ID, want.GetLongParam(Constants::FORM_CONNECT_ID, 0));
    newWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    newWant.SetParam(Constants::PROVIDER_FLAG, true);
    newWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));
    std::shared_ptr<Ability> ownerAbility = GetOwner();
    if (ownerAbility == nullptr) {
        HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
        FormProviderInfo formProviderInfo;
        newWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
        return HandleAcquire(formProviderInfo, newWant, callerToken);
    }

    HILOG_INFO("%{public}s come, %{public}s.", __func__, ownerAbility->GetAbilityName().c_str());

    if (!CheckIsSystemApp()) {
        HILOG_WARN("%{public}s warn, AcquireProviderFormInfo caller permission denied.", __func__);
        FormProviderInfo formProviderInfo;
        newWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_PERMISSION_DENY);
        return HandleAcquire(formProviderInfo, newWant, callerToken);
    }

    Want createWant(want);
    createWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formId));
    createWant.RemoveParam(Constants::FORM_CONNECT_ID);
    createWant.RemoveParam(Constants::ACQUIRE_TYPE);
    createWant.RemoveParam(Constants::FORM_SUPPLY_INFO);
    FormProviderInfo formProviderInfo = ownerAbility->OnCreate(createWant);
    HILOG_DEBUG("%{public}s, formId: %{public}" PRId64 ", data: %{public}s",
     __func__, formId, formProviderInfo.GetFormDataString().c_str());
    return HandleAcquire(formProviderInfo, newWant, callerToken);
}

/**
 * @brief Notify provider when the form was deleted.
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::NotifyFormDelete(const int64_t formId, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        HILOG_INFO("%{public}s called.", __func__);
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }
        if (!CheckIsSystemApp()) {
            HILOG_WARN("%{public}s caller permission denied", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come, %{public}s", __func__, ownerAbility->GetAbilityName().c_str());
        ownerAbility->OnDelete(formId);
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

/**
 * @brief Notify provider when the forms was deleted.
 *
 * @param formIds The id list of forms.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::NotifyFormsDelete(
    const std::vector<int64_t> &formIds,
    const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        HILOG_INFO("%{public}s called.", __func__);
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }
        if (!CheckIsSystemApp()) {
            HILOG_WARN("%{public}s caller permission denied", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come,formIds size=%{public}zu, abilityName:%{public}s",
            __func__, formIds.size(), ownerAbility->GetAbilityName().c_str());
        for (int64_t formId : formIds) {
            ownerAbility->OnDelete(formId);
        }
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

/**
 * @brief Notify provider when the form need update.
 *
 * @param formId The Id of the form.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::NotifyFormUpdate(
    const int64_t formId,
    const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);

    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, owner ability is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }

        if (!CheckIsSystemApp()) {
            HILOG_ERROR("%{public}s warn, caller permission denied.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come, %{public}s.", __func__, ownerAbility->GetAbilityName().c_str());
        ownerAbility->OnUpdate(formId);
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

/**
 * @brief Event notify when change the form visible.
 *
 * @param formIds The vector of form ids.
 * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::EventNotify(
    const std::vector<int64_t> &formIds,
    const int32_t formVisibleType, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);

    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, owner ability is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }

        if (!CheckIsSystemApp()) {
            HILOG_WARN("%{public}s warn, caller permission denied.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        std::map<int64_t, int32_t> formEventsMap;
        for (const auto &formId : formIds) {
            formEventsMap.insert(std::make_pair(formId, formVisibleType));
        }

        HILOG_INFO("%{public}s come, %{public}s.", __func__, ownerAbility->GetAbilityName().c_str());
        ownerAbility->OnVisibilityChanged(formEventsMap);
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

/**
 * @brief Notify provider when the temp form was cast to normal form.
 * @param formId The Id of the form to update.
 * @param want Indicates the structure containing form info.
 * @param callerToken Caller ability token.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::NotifyFormCastTempForm(
    const int64_t formId,
    const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }
        if (!CheckIsSystemApp()) {
            HILOG_WARN("%{public}s caller permission denied", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come, %{public}s", __func__, ownerAbility->GetAbilityName().c_str());
        ownerAbility->OnCastTemptoNormal(formId);
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}
/**
 * @brief Fire message event to form provider.
 * @param formId The Id of the from.
 * @param message Event message.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::FireFormEvent(
    const int64_t formId,
    const std::string &message,
    const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    // The error code for business operation.
    int errorCode = ERR_OK;
    do {
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }
        if (!CheckIsSystemApp()) {
            HILOG_WARN("%{public}s caller permission denied", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come, %{public}s", __func__, ownerAbility->GetAbilityName().c_str());
        ownerAbility->OnTriggerEvent(formId, message);
    } while (false);

    // The error code for disconnect.
    int disconnectErrorCode = HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK，return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK，return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("%{public}s, disconnect error.", __func__);
        }
        return disconnectErrorCode;
    }
}

/**
 * @brief Acquire form state to form provider.
 * @param wantArg The want of onAcquireFormState.
 * @param provider The provider info.
 * @param want The want of the request.
 * @param callerToken Form provider proxy object.
 * @return Returns ERR_OK on success, others on failure.
 */
int FormProviderClient::AcquireState(const Want &wantArg, const std::string &provider, const Want &want,
                                     const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s called.", __func__);
    // The error code for business operation.
    int errorCode = ERR_OK;
    FormState state = FormState::UNKNOWN;
    do {
        std::shared_ptr<Ability> ownerAbility = GetOwner();
        if (ownerAbility == nullptr) {
            HILOG_ERROR("%{public}s error, ownerAbility is nullptr.", __func__);
            errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
            break;
        }
        if (!CheckIsSystemApp()) {
            HILOG_ERROR("%{public}s caller permission denied", __func__);
            errorCode = ERR_APPEXECFWK_FORM_PERMISSION_DENY;
            break;
        }

        HILOG_INFO("%{public}s come, %{public}s", __func__, ownerAbility->GetAbilityName().c_str());
        state = ownerAbility->OnAcquireFormState(wantArg);
    } while (false);

    HandleAcquireStateResult(state, provider, wantArg, want, callerToken);
    return errorCode;
}

/**
 * @brief Set the owner ability of the form provider client.
 *
 * @param ability The owner ability of the form provider client.
 */
void FormProviderClient::SetOwner(const std::shared_ptr<Ability> ability)
{
    if (ability == nullptr) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(abilityMutex_);
        owner_ = ability;
    }
}

/**
 * @brief Clear the owner ability of the form provider client.
 *
 * @param ability The owner ability of the form provider client.
 */
void FormProviderClient::ClearOwner(const std::shared_ptr<Ability> ability)
{
    if (ability == nullptr) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(abilityMutex_);
        if (!owner_.expired()) {
            std::shared_ptr<Ability> ownerAbility = owner_.lock();
            if (ability == ownerAbility) {
                owner_.reset();
            }
        }
    }
}

std::shared_ptr<Ability> FormProviderClient::GetOwner()
{
    std::shared_ptr<Ability> ownerAbility = nullptr;
    {
        std::lock_guard<std::mutex> lock(abilityMutex_);
        if (!owner_.expired()) {
            ownerAbility = owner_.lock();
        }
    }
    return ownerAbility;
}

bool FormProviderClient::CheckIsSystemApp() const
{
    HILOG_INFO("%{public}s called.", __func__);

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid > Constants::MAX_SYSTEM_APP_UID) {
        HILOG_WARN("%{public}s warn, callingUid is %{public}d, which is larger than %{public}d.", __func__, callingUid,
            Constants::MAX_SYSTEM_APP_UID);
        return false;
    } else {
        HILOG_DEBUG("%{public}s, callingUid = %{public}d.", __func__, callingUid);
        return true;
    }
}

int FormProviderClient::HandleAcquire(
    const FormProviderInfo &formProviderInfo,
    const Want &newWant,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s start, image state is %{public}d",
        __func__, formProviderInfo.GetFormData().GetImageDataState());

    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_WARN("%{public}s warn, IFormSupply is nullptr", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    formSupplyClient->OnAcquire(formProviderInfo, newWant);
    HILOG_INFO("%{public}s end", __func__);
    return ERR_OK;
}

int  FormProviderClient::HandleDisconnect(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_WARN("%{public}s warn, IFormSupply is nullptr", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    HILOG_DEBUG("%{public}s come, connectId: %{public}ld.", __func__,
        want.GetLongParam(Constants::FORM_CONNECT_ID, 0L));

    formSupplyClient->OnEventHandle(want);
    return ERR_OK;
}

int FormProviderClient::HandleAcquireStateResult(FormState state, const std::string &provider, const Want &wantArg,
                                                 const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s start, form state is %{public}d", __func__, state);

    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_ERROR("%{public}s warn, IFormSupply is nullptr", __func__);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }
    formSupplyClient->OnAcquireStateResult(state, provider, wantArg, want);
    HILOG_INFO("%{public}s end", __func__);
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
