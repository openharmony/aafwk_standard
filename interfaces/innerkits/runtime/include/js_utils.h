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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_JS_UTILS_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_JS_UTILS_H

#include "native_engine/native_engine.h"

#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class HandleScope final {
public:
    explicit HandleScope(JsRuntime& jsRuntime)
    {
        scopeManager_ = jsRuntime.GetNativeEngine().GetScopeManager();
        if (scopeManager_ != nullptr) {
            nativeScope_ = scopeManager_->OpenEscape();
        }
    }

    ~HandleScope()
    {
        if (nativeScope_ != nullptr) {
            scopeManager_->CloseEscape(nativeScope_);
            nativeScope_ = nullptr;
        }
        scopeManager_ = nullptr;
    }

    HandleScope& Escape(NativeValue* value)
    {
        if (nativeScope_ != nullptr) {
            scopeManager_->Escape(nativeScope_, value);
        }
        return *this;
    }

    HandleScope(const HandleScope&) = delete;
    HandleScope(HandleScope&&) = delete;
    HandleScope& operator=(const HandleScope&) = delete;
    HandleScope& operator=(HandleScope&&) = delete;

private:
    NativeScopeManager* scopeManager_ = nullptr;
    NativeScope* nativeScope_ = nullptr;
};

template<class T>
inline T* ConvertNativeValueTo(NativeValue* value)
{
    return (value != nullptr) ? static_cast<T*>(value->GetInterface(T::INTERFACE_ID)) : nullptr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // FOUNDATION_OHOS_ABILITYRUNTIME_JS_UTILS_H
