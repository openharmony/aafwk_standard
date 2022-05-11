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
AbilityLifecycleCallback::AbilityLifecycleCallback(NativeEngine &engine)
    : engine_(engine)
{
}

AbilityLifecycleCallback::~AbilityLifecycleCallback() = default;

void AbilityLifecycleCallback::OnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("AbilityLifecycleCallback onAbilityCreate");
    CallLifecycleCBFunction("onAbilityCreate", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityWindowStageCreate", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityWindowStageDestroy", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityForeground", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityBackground", abilityObj);
}
void AbilityLifecycleCallback::OnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallLifecycleCBFunction("onAbilityContinue", abilityObj);
}

void AbilityLifecycleCallback::SetJsCallback(NativeValue *jsCallback)
{
    jsCallback_ = std::shared_ptr<NativeReference>(engine_.CreateReference(jsCallback, 1));
}

NativeValue *AbilityLifecycleCallback::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<NativeReference> &abilityObj)
{
    if (functionName.empty()) {
        HILOG_ERROR("Invalid function name");
        return nullptr;
    }

    if (!jsCallback_) {
        HILOG_ERROR("Invalid jsCallback");
        return nullptr;
    }

    auto value = jsCallback_->Get();
    auto obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto method = obj->GetProperty(functionName.data());
    if (method == nullptr) {
        HILOG_ERROR("Failed to get %{public}s from object", functionName.data());
        return nullptr;
    }

    auto nativeAbilityObj = engine_.CreateNull();
    if (!abilityObj.expired()) {
        if (abilityObj.lock() != nullptr) {
            nativeAbilityObj = abilityObj.lock()->Get();
        }
    }

    NativeValue* argv[] = { nativeAbilityObj };
    return engine_.CallFunction(value, method, argv, ArraySize(argv));
}
}  // namespace AbilityRuntime
}  // namespace OHOS