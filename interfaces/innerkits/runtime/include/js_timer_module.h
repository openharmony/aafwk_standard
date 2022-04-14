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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_JS_TIMER_MODULE_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_JS_TIMER_MODULE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <uv.h>

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
struct TimerCallbackNode {
    TimerCallbackNode(NativeEngine* engine, std::shared_ptr<NativeReference>& func, bool isInterval)
        : engine_(engine), func_(func), isInterval_(isInterval) {}

    TimerCallbackNode() = delete;
    ~TimerCallbackNode() {};

    void PushArgs(const std::shared_ptr<NativeReference>& ref)
    {
        params_.emplace_back(ref);
    }

    NativeEngine* engine_ = nullptr;
    std::shared_ptr<NativeReference> func_;
    std::vector<std::shared_ptr<NativeReference>> params_;
    bool isInterval_ = true;
    uint32_t callbackId_;
    uv_timer_t timerHandle_;
};

class JsTimerModule final {
public:
    JsTimerModule() = default;
    ~JsTimerModule() = default;

    static JsTimerModule* GetInstance();
    void InitTimerModule(NativeEngine& engine);

    uint32_t AddCallBack(const std::shared_ptr<TimerCallbackNode>& callbackNode);
    std::shared_ptr<TimerCallbackNode> GetCallBackById(uint32_t callbackId);
    void RemoveCallback(uint32_t callbackId);

private:
    std::mutex mutex_;
    std::unordered_map<uint32_t, std::shared_ptr<TimerCallbackNode>> callbackNodeMap_;
    std::atomic<uint32_t> callbackId_ = 0;
};
} // AbilityRuntime
} // OHOS

#endif // FOUNDATION_OHOS_ABILITYRUNTIME_JS_TIMER_MODULE_H