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

#include "form_extension.h"

#include "ability_loader.h"
#include "form_extension_context.h"
#include "form_runtime/js_form_extension.h"
#include "hilog_wrapper.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
FormExtension* FormExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new FormExtension();
    }
    HILOG_INFO("FormExtension::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsFormExtension::Create(runtime);
        default:
            return new FormExtension();
    }
}

void FormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<FormExtensionContext>::Init(record, application, handler, token);
    HILOG_INFO("FormExtension begin init");
}

std::shared_ptr<FormExtensionContext> FormExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<FormExtensionContext> context =
        ExtensionBase<FormExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (record == nullptr) {
        HILOG_ERROR("FormExtension::CreateAndInitContext record is nullptr");
        return context;
    }
    context->SetAbilityInfo(record->GetAbilityInfo());
    return context;
}

OHOS::AppExecFwk::FormProviderInfo FormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("%{public}s called.", __func__);
    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    return formProviderInfo;
}

void FormExtension::OnDestroy(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void FormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void FormExtension::OnUpdate(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void FormExtension::OnCastToNormal(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void FormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    HILOG_INFO("%{public}s called.", __func__);
}

FormState FormExtension::OnAcquireFormState(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    return FormState::DEFAULT;
}
}
}