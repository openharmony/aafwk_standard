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

#include "js_timer_module.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS::AbilityRuntime {
namespace {
void UvTimerCallback(uv_timer_t* timerHandle)
{
    if (timerHandle == nullptr) {
        HILOG_ERROR("UvTimer callback failed with timerHandle is nullptr.");
        return;
    }

    TimerCallbackNode* callbackNode = (TimerCallbackNode*)timerHandle->data;
    if (callbackNode == nullptr) {
        HILOG_ERROR("UvTimer callback failed with callbackNode is nullptr.");
        return;
    }

    NativeEngine* engine = callbackNode->engine_;
    std::shared_ptr<NativeReference> func = callbackNode->func_;
    if (engine == nullptr || func->Get() == nullptr) {
        HILOG_ERROR("UvTimer callback failed with engine or func is nullptr.");
        return;
    }

    std::vector<std::shared_ptr<NativeReference>> params = callbackNode->params_;
    std::vector<NativeValue*> argc;
    argc.reserve(params.size());
    for (auto arg : params) {
        argc.emplace_back(arg->Get());
    }
    engine->CallFunction(engine->CreateUndefined(), func->Get(), argc.data(), argc.size());

    if (!(callbackNode->isInterval_)) {
        JsTimerModule::GetInstance()->RemoveCallback(callbackNode->callbackId_);
    }
}

NativeValue* SetCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info, bool isInterval)
{
    // Parameter check, must have at least 2 params
    if (info.argc < 2 || info.argv[0]->TypeOf() != NATIVE_FUNCTION || info.argv[1]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Set callback timer failed with invalid parameter.");
        return engine.CreateUndefined();
    }

    // Parse parameter
    std::shared_ptr<NativeReference> func(engine.CreateReference(info.argv[0], 1));
    int64_t delayTime = *ConvertNativeValueTo<NativeNumber>(info.argv[1]);
    auto callbackNode = std::make_shared<TimerCallbackNode>(&engine, func, isInterval);
    for (size_t index = 2; index < info.argc; ++index) {
        callbackNode->PushArgs(std::shared_ptr<NativeReference>(engine.CreateReference(info.argv[index], 1)));
    }

    // Get callbackId
    uint32_t callbackId = JsTimerModule::GetInstance()->AddCallBack(callbackNode);
    callbackNode->callbackId_ = callbackId;

    // Start timer
    uv_timer_t* timerHandle = &(callbackNode->timerHandle_);
    uv_timer_init(engine.GetUVLoop(), timerHandle);
    timerHandle->data = callbackNode.get();
    int ret = uv_timer_start(timerHandle, UvTimerCallback, delayTime, isInterval ? delayTime : 0);
    if (ret != 0) {
        HILOG_ERROR("Start uv timer failed with %{public}d.", ret);
        JsTimerModule::GetInstance()->RemoveCallback(callbackId);
        return engine.CreateUndefined();
    }

    return engine.CreateNumber(callbackId);
}

NativeValue* ClearCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info)
{
    // parameter check, must have at least 1 param
    if (info.argc < 1 || info.argv[0]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Clear callback timer failed with invalid parameter.");
        return engine.CreateUndefined();
    }

    uint32_t callbackId = *ConvertNativeValueTo<NativeNumber>(info.argv[0]);
    auto callbackNode = JsTimerModule::GetInstance()->GetCallBackById(callbackId);
    if (callbackNode == nullptr) {
        return engine.CreateUndefined();
    }

    uv_timer_stop(&(callbackNode->timerHandle_));

    JsTimerModule::GetInstance()->RemoveCallback(callbackId);
    return engine.CreateUndefined();
}

NativeValue* SetTimeout(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Set timeout failed with engine or callback info is nullptr.");
        return nullptr;
    }

    return SetCallbackTimer(*engine, *info, false);
}

NativeValue* SetInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Set interval failed with engine or callback info is nullptr.");
        return nullptr;
    }

    return SetCallbackTimer(*engine, *info, true);
}

NativeValue* ClearTimeoutOrInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Clear timer failed with engine or callback info is nullptr.");
        return nullptr;
    }

    return ClearCallbackTimer(*engine, *info);
}
} // namespace

uint32_t JsTimerModule::AddCallBack(const std::shared_ptr<TimerCallbackNode>& callbackNode)
{
    std::lock_guard<std::mutex> lock(mutex_);

    ++callbackId_;
    callbackNodeMap_[callbackId_] = callbackNode;
    return callbackId_;
}

std::shared_ptr<TimerCallbackNode> JsTimerModule::GetCallBackById(uint32_t callbackId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto callbackNode = callbackNodeMap_.find(callbackId);
    if (callbackNode == callbackNodeMap_.end()) {
        HILOG_ERROR("Get callback failed, callbackId %{public}u not found.", callbackId);
        return nullptr;
    }

    return callbackNode->second;
}

void JsTimerModule::RemoveCallback(uint32_t callbackId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto taskNode = callbackNodeMap_.find(callbackId);
    if (taskNode == callbackNodeMap_.end()) {
        HILOG_ERROR("Remove callback failed, callbackId %{public}u not found.", callbackId);
        return;
    }

    callbackNodeMap_.erase(taskNode);
}

JsTimerModule* JsTimerModule::GetInstance()
{
    static JsTimerModule instance;
    return &instance;
}

void JsTimerModule::InitTimerModule(NativeEngine& engine)
{
    NativeObject* globalObject = ConvertNativeValueTo<NativeObject>(engine.GetGlobal());
    if (!globalObject) {
        HILOG_ERROR("Failed to get global object.");
        return;
    }

    BindNativeFunction(engine, *globalObject, "setTimeout", SetTimeout);
    BindNativeFunction(engine, *globalObject, "setInterval", SetInterval);
    BindNativeFunction(engine, *globalObject, "clearTimeout", ClearTimeoutOrInterval);
    BindNativeFunction(engine, *globalObject, "clearInterval", ClearTimeoutOrInterval);
}
} // OHOS::AbilityRuntime