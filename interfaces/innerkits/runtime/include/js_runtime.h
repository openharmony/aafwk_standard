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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_JS_RUNTIME_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_JS_RUNTIME_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "native_engine/native_engine.h"

#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
} // namespace AppExecFwk
namespace AbilityRuntime {
class TimerTask;
class JsRuntime : public Runtime {
public:
    static std::unique_ptr<Runtime> Create(const Options& options);

    ~JsRuntime() override = default;

    NativeEngine& GetNativeEngine() const
    {
        return *nativeEngine_;
    }

    Language GetLanguage() const override
    {
        return Language::JS;
    }

    std::unique_ptr<NativeReference> LoadModule(const std::string& moduleName, const std::string& modulePath);
    std::unique_ptr<NativeReference> LoadSystemModule(
        const std::string& moduleName, NativeValue* const* argv = nullptr, size_t argc = 0);
    void PostTask(const TimerTask& task, const std::string& name, int64_t delayTime);
    void RemoveTask(const std::string& name);
    NativeValue* SetCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info, bool isInterval);
    NativeValue* ClearCallbackTimer(NativeEngine& engine, NativeCallbackInfo& info);
    std::string BuildNativeAndJsBackStackTrace() override;

    virtual bool RunScript(const std::string& path);
    virtual bool RunSandboxScript(const std::string& path);

protected:
    JsRuntime() = default;

    virtual bool Initialize(const Options& options);
    void Deinitialize();

    bool isArkEngine_ = false;
    bool debugMode_ = false;
    std::unique_ptr<NativeEngine> nativeEngine_;
    std::string codePath_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;

    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    uint32_t callbackId_ = 0;

    std::unordered_map<std::string, NativeReference*> modules_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // FOUNDATION_OHOS_ABILITYRUNTIME_JS_RUNTIME_H
