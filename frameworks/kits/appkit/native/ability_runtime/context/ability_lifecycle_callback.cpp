/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_lifecycle_callback.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsAbilityLifecycleCallback::JsAbilityLifecycleCallback(NativeEngine* engine)
    : engine_(engine)
{
}

int32_t JsAbilityLifecycleCallback::serialNumber_ = 0;

void JsAbilityLifecycleCallback::CallJsMethodInner(
    const std::string &methodName, const std::shared_ptr<NativeReference> &ability)
{
    for (auto &callback : callbacks_) {
        if (!callback.second) {
            HILOG_ERROR("Invalid jsCallback");
            return;
        }

        auto value = callback.second->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get object");
            return;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("Failed to get %{public}s from object", methodName.data());
            return;
        }

        auto nativeAbilityObj = engine_->CreateNull();
        if (ability != nullptr) {
            nativeAbilityObj = ability->Get();
        }

        NativeValue *argv[] = { nativeAbilityObj };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}

void JsAbilityLifecycleCallback::CallJsMethod(
    const std::string &methodName, const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("methodName = %{public}s", methodName.c_str());
    // js callback should run in js thread
    auto ability = abilityObj.lock();
    if (!ability) {
        return;
    }
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [JsAbilityLifecycleCallback = this, methodName, ability](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (JsAbilityLifecycleCallback) {
                JsAbilityLifecycleCallback->CallJsMethodInner(methodName, ability);
            }
        });
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule(*engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsAbilityLifecycleCallback::OnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityCreate", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityWindowStageCreate", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityWindowStageDestroy", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityDestroy", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityForeground", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityBackground", abilityObj);
}

void JsAbilityLifecycleCallback::OnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityContinue", abilityObj);
}

int32_t JsAbilityLifecycleCallback::Register(NativeValue *jsCallback)
{
    if (engine_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (callbackId < INT32_MAX) {
        callbackId++;
    } else {
        callbackId = -1;
    }
    callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
    return callbackId;
}

void JsAbilityLifecycleCallback::UnRegister(int32_t callbackId)
{
    callbacks_.erase(callbackId);
}

bool JsAbilityLifecycleCallback::IsEmpty()
{
    return callbacks_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS