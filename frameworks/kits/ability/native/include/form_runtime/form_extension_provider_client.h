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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_PROVIDER_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_PROVIDER_CLIENT_H

#include "form_provider_client.h"

namespace OHOS {
namespace AbilityRuntime {
class FormExtension;
/**
 * @class FormExtensionProviderClient
 * The service of the form extension provider.
 */
class FormExtensionProviderClient : public AppExecFwk::FormProviderClient {
public:
    FormExtensionProviderClient() = default;
    virtual ~FormExtensionProviderClient() = default;

    /**
     * @brief Acquire to give back an ProviderFormInfo. This is sync API.
     * @param formId The Id of the form.
     * @param want Indicates the {@link Want} structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AcquireProviderFormInfo(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form was deleted.
     *
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormDelete(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form was deleted.
     *
     * @param formIds The id list of forms.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form need update.
     *
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormUpdate(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Event notify when change the form visible.
     *
     * @param formIds The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
        const Want &want, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the temp form was cast to normal form.
     *
     * @param formId The Id of the form to update.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller form extension token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormCastTempForm(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Fire message event to form provider.
     * @param formId The Id of the from.
     * @param message Event message.
     * @param want The want of the request.
     * @param callerToken Form provider proxy object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FireFormEvent(const int64_t formId, const std::string &message,
        const Want &want, const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Acquire form state to form provider.
     * @param wantArg The want of onAcquireFormState.
     * @param provider The provider info.
     * @param want The want of the request.
     * @param callerToken Form provider proxy object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AcquireState(const Want &wantArg, const std::string &provider, const Want &want,
                             const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Set the owner form extension of the form provider client.
     *
     * @param formExtension The owner form extension of the form provider client.
     * @return none.
     */
    void SetOwner(const std::shared_ptr<FormExtension> formExtension);

    /**
     * @brief Clear the owner form extension of the form provider client.
     *
     * @param formExtension The owner form extension of the form provider client.
     * @return none.
     */
    void ClearOwner(const std::shared_ptr<FormExtension> formExtension);

private:
    std::shared_ptr<FormExtension> GetOwner();
    std::pair<int, int> CheckParam(const Want &want, const sptr<IRemoteObject> &callerToken);
    int HandleResultCode(int errorCode, const Want &want, const sptr<IRemoteObject> &callerToken);
    void AcquireFormExtensionProviderInfo(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void NotifyFormExtensionDelete(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void NotifyFormExtensionsDelete(const std::vector<int64_t> &formIds, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void NotifyFormExtensionUpdate(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void EventNotifyExtension(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
        const Want &want, const sptr<IRemoteObject> &callerToken);
    void NotifyFormExtensionCastTempForm(const int64_t formId, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void FireFormExtensionEvent(const int64_t formId, const std::string &message, const Want &want,
        const sptr<IRemoteObject> &callerToken);
    void NotifyFormExtensionAcquireState(const Want &wantArg, const std::string &provider, const Want &want,
                                         const sptr<IRemoteObject> &callerToken);

private:
    mutable std::mutex formExtensionMutex_;
    std::weak_ptr<FormExtension> ownerFormExtension_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_PROVIDER_CLIENT_H
