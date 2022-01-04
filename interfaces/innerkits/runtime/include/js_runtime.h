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
#include <list>
#include <memory>
#include <string>

#include "native_engine/native_engine.h"

#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
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

protected:
    JsRuntime() = default;

    virtual bool Initialize(const Options& options);
    void Deinitialize();
    bool LoadModuleFile(const std::string& moduleName, const std::string& modulePath);

    struct ModuleFileInfo {
        std::string moduleName;
        std::string fileName;
        size_t fileLength = 0;
        std::unique_ptr<char[]> fileData;
    };

    bool isArkEngine_ = false;
    std::unique_ptr<NativeEngine> nativeEngine_;
    std::string codePath_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    std::list<ModuleFileInfo> modules_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // FOUNDATION_OHOS_ABILITYRUNTIME_JS_RUNTIME_H
