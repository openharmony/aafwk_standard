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

#include "ability_runtime/js_ability.h"

#include "ability_runtime/js_ability_context.h"
#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
Ability* JsAbility::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsAbility(static_cast<JsRuntime&>(*runtime));
}

JsAbility::JsAbility(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsAbility::~JsAbility() = default;

void JsAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    Ability::Init(abilityInfo, application, handler, token);

    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    std::string srcPath(abilityInfo->package);
    srcPath.append("/assets/js/");
    if (!abilityInfo->srcPath.empty()) {
        srcPath.append(abilityInfo->srcPath);
    }
    srcPath.append("/").append(abilityInfo->name).append(".abc");

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsAbilityObj_ = jsRuntime_.LoadModule(moduleName, srcPath);

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    auto context = GetAbilityContext();
    NativeValue* contextObj = CreateJsAbilityContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.AbilityContext", &contextObj, 1);
    contextObj = shellContextRef->Get();

    context->Bind(jsRuntime_, shellContextRef.release());
    obj->SetProperty("context", contextObj);
}

void JsAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    auto jsWant = CreateJsWantObject(nativeEngine, want);
    obj->SetProperty("launchWant", jsWant);
    obj->SetProperty("lastRequestWant", jsWant);

    NativeValue* argv[] = {
        jsWant,
        CreateJsLaunchParam(nativeEngine, GetLaunchParam()),
    };
    CallObjectMethod("onCreate", argv, ArraySize(argv));
}

void JsAbility::OnStop()
{
    Ability::OnStop();

    CallObjectMethod("onDestroy");
}

void JsAbility::OnSceneCreated()
{
    Ability::OnSceneCreated();

    CallObjectMethod("onWindowStageCreate");
}

void JsAbility::onSceneDestroyed()
{
    Ability::onSceneDestroyed();

    CallObjectMethod("onWindowStageDestroy");
}

void JsAbility::OnForeground(const Want &want)
{
    Ability::OnForeground(want);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    auto jsWant = CreateJsWantObject(nativeEngine, want);
    obj->SetProperty("lastRequestWant", jsWant);

    CallObjectMethod("onForeground", &jsWant, 1);
}

void JsAbility::OnBackground()
{
    Ability::OnBackground();

    CallObjectMethod("onBackground");
}

void JsAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Ability::OnAbilityResult(requestCode, resultCode, resultData);
    std::shared_ptr<AbilityRuntime::AbilityContext> context = GetAbilityContext();
    if (context == nullptr) {
        HILOG_WARN("JsAbility not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsAbility::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsAbility::CallObjectMethod(%{public}s", name);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
