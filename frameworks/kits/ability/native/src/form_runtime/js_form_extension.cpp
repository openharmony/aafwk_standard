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

#include "form_runtime/js_form_extension.h"

#include "ability_info.h"
#include "form_provider_data.h"
#include "form_runtime/form_extension_provider_client.h"
#include "form_runtime/js_form_extension_context.h"
#include "form_runtime/js_form_extension_util.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
const int ON_EVENT_PARAMS_SIZE = 2;

JsFormExtension* JsFormExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    HILOG_INFO("JsFormExtension::Create runtime");
    return new JsFormExtension(static_cast<JsRuntime&>(*runtime));
}

JsFormExtension::JsFormExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsFormExtension::~JsFormExtension() = default;

void JsFormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("JsFormExtension::Init begin.");
    FormExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_INFO("JsFormExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsFormExtension::Init ConvertNativeValueTo.");
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsFormExtension object");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsFormExtension::Init CreateJsFormExtensionContext.");
    NativeValue* contextObj = CreateJsFormExtensionContext(engine, context);
    shellContextRef_ = jsRuntime_.LoadSystemModule("application.FormExtensionContext", &contextObj, 1);
    contextObj = shellContextRef_->Get();
    HILOG_INFO("JsFormExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    HILOG_INFO("JsFormExtension::SetProperty.");
    obj->SetProperty("context", contextObj);

    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get form extension native object");
        return;
    }

    HILOG_INFO("Set form extension context");

    nativeObj->SetNativePointer(new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr);

    HILOG_INFO("JsFormExtension::Init end.");
}

OHOS::AppExecFwk::FormProviderInfo JsFormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    HILOG_INFO("%{public}s OnCreate WrapWant end.", __func__);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = { nativeWant };
    NativeValue* nativeResult = CallObjectMethod("onCreate", argv, 1);
    NativeObject* nativeObject = ConvertNativeValueTo<NativeObject>(nativeResult);

    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    if (nativeObject == nullptr) {
        HILOG_ERROR("%{public}s, nativeObject is nullptr", __func__);
        return formProviderInfo;
    }

    NativeValue* nativeDataValue = nativeObject->GetProperty("data");
    if (nativeDataValue == nullptr) {
        HILOG_ERROR("%{public}s, nativeObject get data is nullptr", __func__);
        return formProviderInfo;
    }

    std::string formDataStr;
    if (!ConvertFromJsValue(*nativeEngine, nativeDataValue, formDataStr)) {
        HILOG_ERROR("%{public}s, convert formDataStr failed", __func__);
        return formProviderInfo;
    }

    AppExecFwk::FormProviderData formData = AppExecFwk::FormProviderData(formDataStr);
    nativeDataValue = nativeObject->GetProperty("image");
    if (nativeDataValue != nullptr) {
        std::map<std::string, int> rawImageDataMap;
        UnwrapRawImageDataMap(*nativeEngine, nativeDataValue, rawImageDataMap);
        HILOG_INFO("Image number is %{public}zu", rawImageDataMap.size());
        for (auto entry : rawImageDataMap) {
            formData.AddImageData(entry.first, entry.second);
        }
    }
    formProviderInfo.SetFormData(formData);
    HILOG_INFO("%{public}s called end.", __func__);
    return formProviderInfo;
}

void JsFormExtension::OnDestroy(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormExtension::OnDestroy(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onDestroy", argv, 1);
}

void JsFormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormExtension::OnEvent(formId, message);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    // wrap message
    napi_value napiMessage = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), message.c_str(), NAPI_AUTO_LENGTH, &napiMessage);
    NativeValue* nativeMessage = reinterpret_cast<NativeValue*>(napiMessage);
    NativeValue* argv[] = {nativeFormId, nativeMessage};
    CallObjectMethod("onEvent", argv, ON_EVENT_PARAMS_SIZE);
}

void JsFormExtension::OnUpdate(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormExtension::OnUpdate(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onUpdate", argv, 1);
}

void JsFormExtension::OnCastToNormal(const int64_t formId)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormExtension::OnCastToNormal(formId);

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(reinterpret_cast<napi_env>(nativeEngine), std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    NativeValue* nativeFormId = reinterpret_cast<NativeValue*>(napiFormId);
    NativeValue* argv[] = {nativeFormId};
    CallObjectMethod("onCastToNormal", argv, 1);
}

void JsFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormExtension::OnVisibilityChange(formEventsMap);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    NativeValue* nativeFormEventsMap = nativeEngine->CreateObject();
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(nativeFormEventsMap);
    for (auto item = formEventsMap.begin(); item != formEventsMap.end(); item++) {
        nativeObj->SetProperty(std::to_string(item->first).c_str(), CreateJsValue(*nativeEngine, item->second));
    }
    NativeValue* argv[] = {nativeFormEventsMap};
    CallObjectMethod("onVisibilityChange", argv, 1);
}

sptr<IRemoteObject> JsFormExtension::OnConnect(const OHOS::AAFwk::Want& want)
{
    HILOG_INFO("%{public}s called.", __func__);
    Extension::OnConnect(want);
    if (providerRemoteObject_ == nullptr) {
        HILOG_INFO("%{public}s providerRemoteObject_ is nullptr, need init.", __func__);
        sptr<FormExtensionProviderClient> providerClient = new (std::nothrow) FormExtensionProviderClient();
        std::shared_ptr<JsFormExtension> formExtension = std::static_pointer_cast<JsFormExtension>(shared_from_this());
        providerClient->SetOwner(formExtension);
        providerRemoteObject_ = providerClient->AsObject();
    }
    HILOG_INFO("%{public}s end.", __func__);
    return providerRemoteObject_;
}

NativeValue* JsFormExtension::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsFormExtension::CallObjectMethod(%{public}s), begin", name);
    if (!jsObj_) {
        HILOG_WARN("jsObj_ is nullptr");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get FormExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from FormExtension object", name);
        return nullptr;
    }
    HILOG_INFO("JsFormExtension::CallFunction(%{public}s), success", name);
    return handleScope.Escape(nativeEngine.CallFunction(value, method, argv, argc));
}

void JsFormExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

void JsFormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    Extension::OnConfigurationUpdated(configuration);
    HILOG_INFO("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, 1);
}
} // namespace AbilityRuntime
} // namespace OHOS
