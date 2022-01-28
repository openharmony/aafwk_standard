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

#include "js_ability_monitor.h"

#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
JSAbilityMonitor::JSAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

void JSAbilityMonitor::onAbilityCreate()
{
    HILOG_INFO("onAbilityCreate is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue* value = jsAbilityMonitor_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onAbilityCreate");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onAbilityCreate from object");
        return;
    }

    NativeValue* argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onAbilityCreate is called end");
}

void JSAbilityMonitor::onAbilityForeground()
{
    HILOG_INFO("onAbilityForeground is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onAbilityForeground");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onAbilityForeground from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onAbilityForeground is called end");
}

void JSAbilityMonitor::onAbilityBackground()
{
    HILOG_INFO("onAbilityBackground is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onAbilityBackground");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onAbilityBackground from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onAbilityBackground is called end");
}

void JSAbilityMonitor::onAbilityDestroy()
{
    HILOG_INFO("onAbilityDestroy is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onAbilityDestroy");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onAbilityDestroy from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onAbilityDestroy is called end");
}

void JSAbilityMonitor::onWindowStageCreate()
{
    HILOG_INFO("onWindowStageCreate is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onWindowStageCreate");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onWindowStageCreate from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onWindowStageCreate is called end");
}

void JSAbilityMonitor::onWindowStageRestore()
{
    HILOG_INFO("onWindowStageRestore is called");

    if (jsAbilityMonitor_ == nullptr) {
        HILOG_ERROR("jsAbilityMonitor_ nullptr");
        return;
    }

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onWindowStageRestore");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onWindowStageRestore from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onWindowStageRestore is called end");
}

void JSAbilityMonitor::onWindowStageDestroy()
{
    HILOG_INFO("onWindowStageDestroy is called");

    NativeValue *value = jsAbilityMonitor_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue *method = obj->GetProperty("onWindowStageDestroy");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onWindowStageDestroy from object");
        return;
    }

    NativeValue *argv[] = {};
    engine_->CallFunction(value, method, argv, 0);
    HILOG_INFO("onWindowStageDestroy is called end");
}

void JSAbilityMonitor::SetJsAbilityMonitor(NativeValue *jsAbilityMonitor)
{
    HILOG_INFO("SetJsAbilityMonitor is called");

    jsAbilityMonitor_ = std::unique_ptr<NativeReference>(engine_->CreateReference(jsAbilityMonitor, 1));
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS