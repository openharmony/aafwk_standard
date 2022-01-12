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

#include "ability_runtime/js_window_stage.h"

#include <string>
#include "ability_runtime/js_ability_context.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "js_window.h"

namespace OHOS {
namespace AbilityRuntime {
void JsWindowStage::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsWindowStage::Finalizer is called");
    std::unique_ptr<JsWindowStage>(static_cast<JsWindowStage*>(data));
}

NativeValue* JsWindowStage::SetUIContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::SetUIContent is called");
    JsWindowStage* me = AbilityRuntime::CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnSetUIContent(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::GetMainWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::GetMainWindow is called");
    JsWindowStage* me = AbilityRuntime::CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnGetMainWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::On(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::On is called");
    JsWindowStage* me = AbilityRuntime::CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnEvent(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::Off(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::Off is called");
    JsWindowStage* me = AbilityRuntime::CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OffEvent(*engine, *info) : nullptr;
}

void JsWindowStage::AfterForeground()
{
    LifeCycleCallBack(WindowStageEventType::VISIBLE);
}

void JsWindowStage::AfterBackground()
{
    LifeCycleCallBack(WindowStageEventType::INVISIBLE);
}

void JsWindowStage::AfterFocused()
{
    LifeCycleCallBack(WindowStageEventType::FOCUSED);
}

void JsWindowStage::AfterUnFocused()
{
    LifeCycleCallBack(WindowStageEventType::UNFOCUSED);
}

void JsWindowStage::LifeCycleCallBack(WindowStageEventType type)
{
    HILOG_INFO("JsWindowStage::LifeCycleCallBack is called, type: %{public}d", type);
    if (engine_ == nullptr) {
        HILOG_INFO("JsWindowStage::LifeCycleCallBack engine_ is nullptr");
        return;
    }
    for (auto iter = eventCallbackMap_.begin(); iter != eventCallbackMap_.end(); iter++) {
        std::shared_ptr<NativeReference> callback = iter->first;
        int argc = 1;
        NativeValue* argv[1];
        argv[0] = engine_->CreateNumber((int32_t)type);
        engine_->CallFunction(object_, callback->Get(), argv, argc);
    }
}

NativeValue* JsWindowStage::OnSetUIContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnSetUIContent is called");
    if (info.argc < 2) { // 2: minimum param nums
        HILOG_ERROR("JsWindowStage::OnSetUIContent Not enough params");
        return engine.CreateUndefined();
    }
    if (windowScene_ == nullptr || windowScene_->GetMainWindow() == nullptr) {
        HILOG_ERROR("JsWindowStage::OnSetUIContent windowScene_ or MainWindow is nullptr");
        return engine.CreateUndefined();
    }

    // Parse info->argv[0] as abilitycontext
    auto objContext = AbilityRuntime::ConvertNativeValueTo<NativeObject>(info.argv[0]);
    if (objContext == nullptr) {
        HILOG_ERROR("JsWindowStage::OnSetUIContent info->argv[0] InValid");
        return engine.CreateUndefined();
    }

    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(objContext->GetNativePointer());
    auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
    if (abilityContext == nullptr) {
        HILOG_ERROR("JsWindowStage::OnSetUIContent context is nullptr");
        return engine.CreateUndefined();
    }

    HILOG_INFO("JsWindowStage::OnSetUIContent Get context: %{public}p", abilityContext.get());

    // Parse info->argv[1] as url
    std::string contextUrl;
    if (!ConvertFromJsValue(engine, info.argv[1], contextUrl)) {
        HILOG_ERROR("JsWindowStage::OnSetUIContent failed to convert parameter to url");
        return engine.CreateUndefined();
    }
    HILOG_INFO("JsWindowStage::OnSetUIContent Get url: %{public}s", contextUrl.c_str());

    windowScene_->GetMainWindow()->SetUIContent(abilityContext, contextUrl, &engine, nullptr);

    return engine.CreateUndefined();
}

NativeValue* JsWindowStage::OnGetMainWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnGetMainWindow is called");
    if (windowScene_ == nullptr) {
        HILOG_ERROR("JsWindowStage::OnGetMainWindow windowScene_ is nullptr");
        return engine.CreateUndefined();
    }
    AsyncTask::CompleteCallback complete =
        [this](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto window = windowScene_->GetMainWindow();
            if (window != nullptr) {
                task.Resolve(engine, CreateJsWindowObject(engine, window));
                HILOG_INFO("JsWindowStage::OnGetMainWindow success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR),
                    "JsWindowStage::OnGetMainWindow failed."));
            }
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr,
        std::move(complete), &result));
    return result;
}

NativeValue* JsWindowStage::OnEvent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnEvent is called");
    if (windowScene_ == nullptr) {
        HILOG_ERROR("JsWindowStage::OnEvent windowScene_ is nullptr");
        return engine.CreateUndefined();
    }

    if (info.argc < 2) { // 2: minimum param nums
        HILOG_ERROR("JsWindowStage::OnEvent wrong input params");
        return engine.CreateUndefined();
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(engine, info.argv[0], eventString)) {
        HILOG_ERROR("JsWindowStage::OnEvent info->argv[0] Failed to convert parameter to string");
        return engine.CreateUndefined();
    }
    if (eventString.compare("windowStageEvent") != 0) {
        HILOG_ERROR("JsWindowStage::OnEvent info->argv[0] is %{public}s, InValid",
            eventString.c_str());
        return engine.CreateUndefined();
    }

    NativeValue* value = info.argv[1];
    if (!value->IsCallable()) {
        HILOG_ERROR("JsWindowStage::OnEvent info->argv[1] is not callable");
        return engine.CreateUndefined();
    }

    std::shared_ptr<NativeReference> refence = nullptr;
    refence.reset(engine.CreateReference(value, 1));
    eventCallbackMap_[refence] = 1;
    engine_ = &engine;

    AsyncTask::CompleteCallback complete =
        [this](NativeEngine& engine, AsyncTask& task, int32_t status) {
            // regist lifecycle listener
            if (regLifeCycleListenerFlag_ == false) {
                auto window = windowScene_->GetMainWindow();
                if (window != nullptr) {
                    sptr<IWindowLifeCycle> listener = this;
                    window->RegisterLifeCycleListener(listener);
                    regLifeCycleListenerFlag_ = true;
                }
            }
            task.Resolve(engine, engine.CreateUndefined());
            HILOG_INFO("JsWindowStage::OnEvent regist lifecycle success");
        };
    NativeValue* lastParam = (info.argc == 2) ? nullptr : info.argv[2]; // 2: minimum param nums
    NativeValue* result = nullptr;
    AsyncTask::Schedule(engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowStage::OffEvent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OffEvent is called");
    if (windowScene_ == nullptr) {
        HILOG_ERROR("JsWindowStage::OffEvent windowScene_ is nullptr");
        return engine.CreateUndefined();
    }
    if (info.argc < 1 || info.argc > 2) { // 1: minimum param nums, 2: maximum param nums
        HILOG_ERROR("JsWindowStage::OffEvent wrong input params");
        return engine.CreateUndefined();
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(engine, info.argv[0], eventString)) {
        HILOG_ERROR("JsWindowStage::OffEvent info->argv[0] Failed to convert parameter to string");
        return engine.CreateUndefined();
    }
    if (eventString.compare("windowStageEvent") != 0) {
        HILOG_ERROR("JsWindowStage::OffEvent info->argv[0] is InValid");
        return engine.CreateUndefined();
    }

    if (info.argc == 1) { // 1: input param nums
        HILOG_ERROR("JsWindowStage::OffEvent info.argc == 1");
        eventCallbackMap_.clear();
        return engine.CreateUndefined();
    }

    HILOG_INFO("JsWindowStage::OffEvent info.argc == 2");
    NativeValue* value = info.argv[1];
    if (value->IsCallable()) {
        HILOG_INFO("JsWindowStage::OffEvent info->argv[1] is callable type");
        for (auto iter = eventCallbackMap_.begin(); iter != eventCallbackMap_.end(); iter++) {
            std::shared_ptr<NativeReference> callback = iter->first;
            if (value->StrictEquals(callback->Get())) {
                eventCallbackMap_.erase(iter);
                break;
            }
        }
        return engine.CreateUndefined();
    } else if (value->TypeOf() == NativeValueType::NATIVE_UNDEFINED) {
        HILOG_INFO("JsWindowStage::OffEvent info->argv[1] is native undefined type");
        eventCallbackMap_.clear();
    } else {
        HILOG_ERROR("JsWindowStage::OffEvent info->argv[1] is InValid param");
    }
    return engine.CreateUndefined();
}


NativeValue* CreateJsWindowStage(NativeEngine& engine,
    std::shared_ptr<Rosen::WindowScene> windowScene)
{
    HILOG_INFO("JsWindowStage::CreateJsWindowStage is called");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = AbilityRuntime::ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsWindowStage> jsWindowStage =
        std::make_unique<JsWindowStage>(windowScene, objValue);
    object->SetNativePointer(jsWindowStage.release(), JsWindowStage::Finalizer, nullptr);

    AbilityRuntime::BindNativeFunction(engine,
        *object, "setUIContent", JsWindowStage::SetUIContent);
    AbilityRuntime::BindNativeFunction(engine,
        *object, "getMainWindow", JsWindowStage::GetMainWindow);
    AbilityRuntime::BindNativeFunction(engine, *object, "on", JsWindowStage::On);
    AbilityRuntime::BindNativeFunction(engine, *object, "off", JsWindowStage::Off);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
