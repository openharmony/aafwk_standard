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

#include "js_ability_stage.h"

#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<AbilityStage> JsAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    auto& jsRuntime = static_cast<JsRuntime&>(*runtime);
    std::string srcPath(hapModuleInfo.name);

    /* temporary compatibility api8 + config.json */
    if (!hapModuleInfo.isModuleJson) {
        srcPath.append("/assets/js/");
        if (hapModuleInfo.srcPath.empty()) {
            srcPath.append("AbilityStage.abc");
        } else {
            srcPath.append(hapModuleInfo.srcPath);
            srcPath.append("/AbilityStage.abc");
        }
        std::string moduleName(hapModuleInfo.moduleName);
        moduleName.append("::").append("AbilityStage");
        auto moduleObj = jsRuntime.LoadModule(moduleName, srcPath);
        return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
    }

    std::unique_ptr<NativeReference> moduleObj;
    srcPath.append("/");
    if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append(hapModuleInfo.srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        std::string moduleName(hapModuleInfo.moduleName);
        moduleName.append("::").append("AbilityStage");
        moduleObj = jsRuntime.LoadModule(moduleName, srcPath);
        HILOG_INFO("JsAbilityStage srcPath is %{public}s", srcPath.c_str());
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage() = default;

void JsAbilityStage::Init(std::shared_ptr<Context> context)
{
    AbilityStage::Init(context);

    if (!context) {
        HILOG_ERROR("context is nullptr");
        return;
    }

    if (!jsAbilityStageObj_) {
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityStageObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    NativeValue* contextObj = CreateJsAbilityStageContext(engine, context);
    shellContextRef_ = jsRuntime_.LoadSystemModule("application.AbilityStageContext", &contextObj, 1);
    contextObj = shellContextRef_->Get();

    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get ability stage native object");
        return;
    }

    HILOG_INFO("Set ability stage context pointer: %{public}p", context.get());

    nativeObj->SetNativePointer(new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr);
}

void JsAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    HILOG_INFO("JsAbilityStage::OnCreate come");

    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty("onCreate");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, nullptr, 0);
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    HILOG_INFO("JsAbilityStage::OnAcceptWant come");

    AbilityStage::OnAcceptWant(want);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return "";
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return "";
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue* jsWant = reinterpret_cast<NativeValue*>(napiWant);

    NativeValue* methodOnAcceptWant = obj->GetProperty("onAcceptWant");
    if (methodOnAcceptWant == nullptr) {
        HILOG_ERROR("Failed to get 'OnAcceptWant' from AbilityStage object");
        return "";
    }

    NativeValue* argv[] = { jsWant };
    NativeValue* flagNative = nativeEngine.CallFunction(value, methodOnAcceptWant, argv, 1);
    return AppExecFwk::UnwrapStringFromJS(
        reinterpret_cast<napi_env>(&nativeEngine), reinterpret_cast<napi_value>(flagNative));
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdated(configuration);
    HILOG_INFO("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify Ability stage context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, 1);
}

NativeValue* JsAbilityStage::CallObjectMethod(const char* name, NativeValue * const * argv, size_t argc)
{
    HILOG_INFO("JsAbilityStage::CallObjectMethod %{public}s", name);

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from AbilityStage object", name);
        return nullptr;
    }

    return nativeEngine.CallFunction(value, method, argv, argc);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
