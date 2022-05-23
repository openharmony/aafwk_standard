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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_RUNTIME_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_RUNTIME_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
class EventRunner;
} // namespace AppExecFwk
namespace AbilityRuntime {
class Runtime {
public:
    enum class Language {
        JS = 0,
    };

    struct Options {
        Language lang = Language::JS;
        std::string codePath;
        std::string packagePath;
        std::shared_ptr<AppExecFwk::EventRunner> eventRunner;
        bool loadAce = true;
    };

    static std::unique_ptr<Runtime> Create(const Options& options);

    Runtime() = default;
    virtual ~Runtime() = default;

    virtual Language GetLanguage() const = 0;

    virtual void StartDebugMode(bool needBreakPoint, int32_t instanceId = 0) = 0;
    virtual std::string BuildNativeAndJsBackStackTrace() = 0;
    virtual void DumpHeapSnapshot(bool isPrivate) = 0;

    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_OHOS_ABILITYRUNTIME_RUNTIME_H
