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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>
#include "ability.h"
#include "form_constants.h"
#include "form_provider_info.h"
#include "form_provider_stub.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormProviderStub
 * The service of the form provider.
 */
class FormProviderClient : public FormProviderStub {
public:
    FormProviderClient() = default;
    virtual ~FormProviderClient() = default;

    /**
     * @brief Acquire to give back an ProviderFormInfo. This is sync API.
     * @param formId The Id of the form.
     * @param want Indicates the {@link Want} structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AcquireProviderFormInfo(
        const int64_t formId,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form was deleted.
     *
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormDelete(
        const int64_t formId,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form was deleted.
     *
     * @param formIds The id list of forms.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormsDelete(
        const std::vector<int64_t> &formIds,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the form need update.
     *
     * @param formId The Id of the form.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormUpdate(
        const int64_t formId,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Event notify when change the form visible.
     *
     * @param formIds The vector of form ids.
     * @param formVisibleType The form visible type, including FORM_VISIBLE and FORM_INVISIBLE.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType, const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

    /**
     * @brief Notify provider when the temp form was cast to normal form.
     *
     * @param formId The Id of the form to update.
     * @param want Indicates the structure containing form info.
     * @param callerToken Caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyFormCastTempForm(
        const int64_t formId,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;
    /**
     * @brief Fire message event to form provider.
     * @param formId The Id of the from.
     * @param message Event message.
     * @param want The want of the request.
     * @param callerToken Form provider proxy object.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FireFormEvent(
        const int64_t formId,
        const std::string &message,
        const Want &want,
        const sptr<IRemoteObject> &callerToken) override;

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
     * @brief Set the owner ability of the form provider client.
     *
     * @param ability The owner ability of the form provider client.
     * @return none.
     */
    void SetOwner(const std::shared_ptr<Ability> ability);

    /**
     * @brief Clear the owner ability of the form provider client.
     *
     * @param ability The owner ability of the form provider client.
     * @return none.
     */
    void ClearOwner(const std::shared_ptr<Ability> ability);

protected:
    bool CheckIsSystemApp() const;
    int HandleDisconnect(const Want &want, const sptr<IRemoteObject> &callerToken);
    int HandleAcquire(const FormProviderInfo &formProviderInfo, const Want &newWant,
        const sptr<IRemoteObject> &callerToken);
    int HandleAcquireStateResult(FormState state, const std::string &provider, const Want &wantArg, const Want &want,
                                 const sptr<IRemoteObject> &callerToken);

private:
    std::shared_ptr<Ability> GetOwner();

private:
    DISALLOW_COPY_AND_MOVE(FormProviderClient);
    mutable std::mutex abilityMutex_;
    std::weak_ptr<Ability> owner_;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_CLIENT_H
