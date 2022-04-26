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

#include "js_application_context_utils.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";

class JsApplicationContext {
public:
    explicit JsApplicationContext(std::weak_ptr<ApplicationContext> &&applicationContext)
        : applicationContext_(std::move(applicationContext))
    {
    }
    virtual ~JsApplicationContext() = default;
    static NativeValue *RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *UnRegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    void KeepApplicationContext(std::shared_ptr<ApplicationContext> applicationContext)
    {
        keepApplicationContext_ = applicationContext;
    }

private:
    NativeValue *OnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnUnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    std::shared_ptr<ApplicationContext> keepApplicationContext_;

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;
};

NativeValue* JsApplicationContext::RegisterAbilityLifecycleCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContext* me = CheckParamsAndGetThis<JsApplicationContext>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContext::UnRegisterAbilityLifecycleCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContext* me = UnRegisterAbilityLifecycleCallback<JsApplicationContext>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnRegisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

}  // namespace

NativeValue *CreateJsApplicationContext(
    NativeEngine &engine, std::shared_ptr<ApplicationContext> applicationContext, bool keepApplicationContext)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    auto jsApplicationContext = std::make_unique<JsApplicationContext>(applicationContext);
    if (keepApplicationContext) {
        jsApplicationContext->KeepApplicationContext(applicationContext);
    }

    BindNativeFunction(
        engine, *object, "registerAbilityLifecycleCallback", JsApplicationContext::RegisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "unRegisterAbilityLifecycleCallback",
        JsApplicationContext::UnRegisterAbilityLifecycleCallback);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS