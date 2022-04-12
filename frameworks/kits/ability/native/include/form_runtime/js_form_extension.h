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

#ifndef FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_H
#define FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_H

#include "configuration.h"
#include "form_extension.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class FormExtension;
class JsRuntime;
/**
 * @brief js form extension components.
 */
class JsFormExtension : public FormExtension {
public:
    JsFormExtension(JsRuntime& jsRuntime);
    virtual ~JsFormExtension() override;

    static JsFormExtension* Create(const std::unique_ptr<Runtime>& runtime);

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    OHOS::AppExecFwk::FormProviderInfo OnCreate(const OHOS::AAFwk::Want& want) override;

    void OnDestroy(const int64_t formId) override;

    void OnEvent(const int64_t formId, const std::string& message) override;

    void OnUpdate(const int64_t formId) override;

    void OnCastToNormal(const int64_t formId) override;

    void OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap) override;

    sptr<IRemoteObject> OnConnect(const OHOS::AAFwk::Want& want) override;

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    FormState OnAcquireFormState(const Want &want) override;

private:
    NativeValue* CallObjectMethod(const char* name, NativeValue* const* argv = nullptr, size_t argc = 0);

    void GetSrcPath(std::string &srcPath);

    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    sptr<IRemoteObject> providerRemoteObject_ = nullptr;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_OHOS_JS_FORM_EXTENSION_H