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
#include <regex>
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "ability_delegator_registry.h"
#include "ability_runtime/js_ability.h"

#include "ability_runtime/js_ability_context.h"
#include "ability_start_setting.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_configuration.h"
#ifdef SUPPORT_GRAPHICS
#include "js_window_stage.h"
#endif
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "context/context.h"
#include "context/application_context.h"

namespace OHOS {
namespace AbilityRuntime {
Ability *JsAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new JsAbility(static_cast<JsRuntime &>(*runtime));
}

JsAbility::JsAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{}
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
    if (!abilityInfo->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append("/assets/js/");
        if (!abilityInfo->srcPath.empty()) {
            srcPath.append(abilityInfo->srcPath);
        }
        srcPath.append("/").append(abilityInfo->name).append(".abc");
    } else {
        if (abilityInfo->srcEntrance.empty()) {
            HILOG_ERROR("abilityInfo srcEntrance is empty");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        HILOG_INFO("JsAbility srcPath is %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();

    jsAbilityObj_ = jsRuntime_.LoadModule(moduleName, srcPath);

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(jsAbilityObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    auto context = GetAbilityContext();
    NativeValue *contextObj = CreateJsAbilityContext(engine, context);
    shellContextRef_ = std::shared_ptr<NativeReference>(
        jsRuntime_.LoadSystemModule("application.AbilityContext", &contextObj, 1).release());
    contextObj = shellContextRef_->Get();

    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get ability native object");
        return;
    }

    HILOG_INFO("Set ability context");

    nativeObj->SetNativePointer(
        new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        },
        nullptr);
}

void JsAbility::OnStart(const Want &want)
{
    HILOG_INFO("OnStart begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnStart(want);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityCreate(jsAbilityObj_);
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("launchWant", jsWant);
    obj->SetProperty("lastRequestWant", jsWant);

    NativeValue *argv[] = {
        jsWant,
        CreateJsLaunchParam(nativeEngine, GetLaunchParam()),
    };
    CallObjectMethod("onCreate", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformStart");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    HILOG_INFO("OnStart end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnStop()
{
    Ability::OnStop();

    CallObjectMethod("onDestroy");

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformStop");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_INFO("The service connection is not disconnected.");
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityDestroy(jsAbilityObj_);
    }
}

#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";

void JsAbility::OnSceneCreated()
{
    HILOG_INFO("OnSceneCreated begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue *argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformScenceCreated");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityWindowStageCreate(jsAbilityObj_);
    }

    HILOG_INFO("OnSceneCreated end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnSceneRestored()
{
    Ability::OnSceneRestored();
    HILOG_INFO("OnSceneRestored");
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue *argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformScenceRestored");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }
}

void JsAbility::onSceneDestroyed()
{
    HILOG_INFO("onSceneDestroyed begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::onSceneDestroyed();

    CallObjectMethod("onWindowStageDestroy");

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformScenceDestroyed");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityWindowStageDestroy(jsAbilityObj_);
    }
    HILOG_INFO("onSceneDestroyed end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnForeground(const Want &want)
{
    HILOG_INFO("OnForeground begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnForeground(want);

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("lastRequestWant", jsWant);

    CallObjectMethod("onForeground", &jsWant, 1);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformForeground");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityForeground(jsAbilityObj_);
    }
    HILOG_INFO("OnForeground end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnBackground()
{
    Ability::OnBackground();

    CallObjectMethod("onBackground");

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_INFO("Call AbilityDelegator::PostPerformBackground");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityBackground(jsAbilityObj_);
    }
}

std::unique_ptr<NativeReference> JsAbility::CreateAppWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();
    NativeValue *jsWindowStage = Rosen::CreateJsWindowStage(engine, GetScene());
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return jsRuntime_.LoadSystemModule("application.WindowStage", &jsWindowStage, 1);
}

void JsAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

void JsAbility::DoOnForeground(const Want &want)
{
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            HILOG_ERROR("Ability::OnForeground error. abilityContext_ or sceneListener_ is nullptr!");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        if (scene_ == nullptr) {
            HILOG_ERROR("%{public}s error. failed to create WindowScene instance!", __func__);
            return;
        }
        int32_t displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        if (setting_ != nullptr) {
            std::string strDisplayId =
                setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
            std::regex formatRegex("[0-9]{0,9}$");
            std::smatch sm;
            bool flag = std::regex_match(strDisplayId, sm, formatRegex);
            if (flag && !strDisplayId.empty()) {
                displayId = std::stoi(strDisplayId);
                HILOG_INFO("%{public}s success. displayId is %{public}d", __func__, displayId);
            } else {
                HILOG_INFO("%{public}s failed to formatRegex:[%{public}s]", __func__, strDisplayId.c_str());
            }
        }
        auto option = GetWindowOption(want);
        Rosen::WMError ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
        if (ret != Rosen::WMError::WM_OK) {
            HILOG_ERROR("%{public}s error. failed to init window scene!", __func__);
            return;
        }

        // multi-instance ability continuation
        HILOG_INFO("lauch reason = %{public}d", launchParam_.launchReason);
        if (IsRestoredInContinuation()) {
            std::string pageStack;
            GetPageStackFromWant(want, pageStack);
            HandleScope handleScope(jsRuntime_);
            auto &engine = jsRuntime_.GetNativeEngine();
            if (abilityContext_->GetContentStorage()) {
                scene_->GetMainWindow()->SetUIContent(pageStack, &engine,
                    abilityContext_->GetContentStorage()->Get(), true);
            } else {
                HILOG_ERROR("restore: content storage is nullptr");
            }
            OnSceneRestored();
            WaitingDistributedObjectSyncComplete(want);
        } else {
            OnSceneCreated();
        }
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
                AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            HILOG_INFO("set window mode = %{public}d.", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window) {
        HILOG_INFO("Call RegisterDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
        std::weak_ptr<Ability> weakAbility = shared_from_this();
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weakAbility);
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }

    HILOG_INFO("%{public}s begin scene_->GoForeground, sceneFlag_:%{public}d.", __func__, Ability::sceneFlag_);
    scene_->GoForeground(Ability::sceneFlag_);
    HILOG_INFO("%{public}s end scene_->GoForeground.", __func__);
}

void JsAbility::RequsetFocus(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (scene_ == nullptr) {
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_INFO("set window mode = %{public}d.", windowMode);
    }
    scene_->GoForeground(Ability::sceneFlag_);
}
#endif

int32_t JsAbility::OnContinue(WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return false;
    }

    NativeValue *methodOnCreate = obj->GetProperty("onContinue");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onContinue' from Ability object");
        return false;
    }

    napi_value napiWantParams = OHOS::AppExecFwk::WrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), wantParams);
    NativeValue *jsWantParams = reinterpret_cast<NativeValue *>(napiWantParams);

    NativeValue *result = nativeEngine.CallFunction(value, methodOnCreate, &jsWantParams, 1);

    napi_value new_napiWantParams = reinterpret_cast<napi_value>(jsWantParams);
    OHOS::AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), new_napiWantParams, wantParams);

    NativeNumber *numberResult = ConvertNativeValueTo<NativeNumber>(result);
    if (numberResult == nullptr) {
        return false;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(jsAbilityObj_);
    }

    return *numberResult;
}

void JsAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    Ability::OnConfigurationUpdated(configuration);
    HILOG_INFO("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    JsAbilityContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);
    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, 1);
}

void JsAbility::UpdateContextConfiguration()
{
    HILOG_INFO("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    JsAbilityContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, GetAbilityContext()->GetConfiguration());
}

void JsAbility::OnNewWant(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Ability::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("lastRequestWant", jsWant);

    CallObjectMethod("onNewWant", &jsWant, 1);
    HILOG_INFO("%{public}s end.", __func__);
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

sptr<IRemoteObject> JsAbility::CallRequest()
{
    HILOG_INFO("JsAbility::CallRequest begin.");
    if (jsAbilityObj_ == nullptr) {
        HILOG_WARN("JsAbility::CallRequest Obj is nullptr");
        return nullptr;
    }

    if (remoteCallee_ != nullptr) {
        HILOG_INFO("JsAbility::CallRequest get Callee remoteObj.");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    HILOG_DEBUG("JsAbility::CallRequest set runtime scope.");
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    auto value = jsAbilityObj_->Get();
    if (value == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest value is nullptr");
        return nullptr;
    }

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest obj is nullptr");
        return nullptr;
    }

    auto method = obj->GetProperty("onCallRequest");
    if (method == nullptr || !method->IsCallable()) {
        HILOG_ERROR("JsAbility::CallRequest method is %{public}s", method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    auto remoteJsObj = nativeEngine.CallFunction(value, method, nullptr, 0);
    if (remoteJsObj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest JsObj is nullptr");
        return nullptr;
    }

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(
        reinterpret_cast<napi_env>(&nativeEngine), reinterpret_cast<napi_value>(remoteJsObj));
    if (remoteObj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest obj is nullptr");
    }

    remoteCallee_ = remoteObj;
    HILOG_INFO("JsAbility::CallRequest end.");
    return remoteCallee_;
}

void JsAbility::OnRequestPermissionsFromUserResult(
    int requestCode, const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::shared_ptr<AbilityRuntime::AbilityContext> context = GetAbilityContext();
    if (context == nullptr) {
        HILOG_WARN("JsAbility not attached to any runtime context!");
        return;
    }
    context->OnRequestPermissionsFromUserResult(requestCode, permissions, grantResults);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsAbility::CallObjectMethod(const char *name, NativeValue *const *argv, size_t argc)
{
    HILOG_INFO("JsAbility::CallObjectMethod(%{public}s", name);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsAbility::CreateADelegatorAbilityProperty()
{
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    property->token_          = GetAbilityContext()->GetToken();
    property->name_           = GetAbilityName();
    property->lifecycleState_ = GetState();
    property->object_         = jsAbilityObj_;

    return property;
}

void JsAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Ability::Dump(params, info);
    HILOG_INFO("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    // create js array object of params
    NativeValue* argv[] = { CreateNativeArray(nativeEngine, params) };

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found .js");
        return;
    }

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("dump");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get dump from object");
        return;
    }
    HILOG_INFO("Dump CallFunction dump, success");
    NativeValue* dumpInfo = nativeEngine.CallFunction(value, method, argv, 1);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo nullptr.");
        return;
    }
    NativeArray* dumpInfoNative = ConvertNativeValueTo<NativeArray>(dumpInfo);
    if (dumpInfoNative == nullptr) {
        HILOG_ERROR("dumpInfoNative nullptr.");
        return;
    }
    for (uint32_t i = 0; i < dumpInfoNative->GetLength(); i++) {
        std::string dumpInfoStr;
        if (!ConvertFromJsValue(nativeEngine, dumpInfoNative->GetElement(i), dumpInfoStr)) {
            HILOG_ERROR("Parse dumpInfoStr failed");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}
}  // namespace AbilityRuntime
}  // namespace OHOS
