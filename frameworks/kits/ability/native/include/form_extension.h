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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_H

#include "extension_base.h"
#include "form_provider_info.h"
#include "form_state_info.h"

namespace OHOS {
namespace AbilityRuntime {
class FormExtensionContext;
class Runtime;
/**
 * @brief Form extension components.
 */
class FormExtension : public ExtensionBase<FormExtensionContext> {
public:
    FormExtension() = default;
    virtual ~FormExtension() = default;

    /**
     * @brief Create Form Extension.
     *
     * @param runtime The runtime.
     * @return The FormExtension instance.
     */
    static FormExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create and init context.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<FormExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called to return a FormProviderInfo object.
     *
     * <p>You must override this method if your ability will serve as a form provider to provide a form for clients.
     * The default implementation returns nullptr. </p>
     *
     * @param want   Indicates the detailed information for creating a FormProviderInfo.
     *               The Want object must include the form ID, form name of the form,
     *               which can be obtained from Ability#PARAM_FORM_IDENTITY_KEY,
     *               Ability#PARAM_FORM_NAME_KEY, and Ability#PARAM_FORM_DIMENSION_KEY,
     *               respectively. Such form information must be managed as persistent data for further form
     *               acquisition, update, and deletion.
     *
     * @return Returns the created FormProviderInfo object.
     */
    virtual OHOS::AppExecFwk::FormProviderInfo OnCreate(const OHOS::AAFwk::Want& want);

    /**
     * @brief Called to notify the form provider that a specified form has been destroyed. Override this method if
     * you want your application, as the form provider, to be notified of form deletion.
     *
     * @param formId Indicates the ID of the deleted form.
     * @return None.
     */
    virtual void OnDestroy(const int64_t formId);

    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @param message Form event message.
     */
    virtual void OnEvent(const int64_t formId, const std::string& message);

    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @return none.
     */
    virtual void OnUpdate(const int64_t formId);

    /**
     * @brief Called when the form provider is notified that a temporary form is successfully converted to
     * a normal form.
     *
     * @param formId Indicates the ID of the form.
     * @return None.
     */
    virtual void OnCastToNormal(const int64_t formId);

    /**
     * @brief Called when the form provider receives form events from the fms.
     *
     * @param formEventsMap Indicates the form events occurred. The key in the Map object indicates the form ID,
     *                      and the value indicates the event type, which can be either FORM_VISIBLE
     *                      or FORM_INVISIBLE. FORM_VISIBLE means that the form becomes visible,
     *                      and FORM_INVISIBLE means that the form becomes invisible.
     * @return none.
     */
    virtual void OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap);

    /**
     * @brief Called to notify the form provider to acquire form state.
     *
     * @param want   Indicates the detailed information about the form to be obtained, including
     *               the bundle name, module name, ability name, form name and form dimension.
     * @return none.
     */
    virtual FormState OnAcquireFormState(const Want &want);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_EXTENSION_H