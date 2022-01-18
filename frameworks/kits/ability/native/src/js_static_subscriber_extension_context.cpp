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

#include "js_static_subscriber_extension_context.h"

#include "hilog_wrapper.h"

#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
class JsStaticSubscriberExtensionContext final {
public:
    explicit JsStaticSubscriberExtensionContext(const std::shared_ptr<StaticSubscriberExtensionContext>& context)
        : context_(context) {}
    ~JsStaticSubscriberExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsStaticSubscriberExtensionContext::Finalizer is called");
        std::unique_ptr<JsStaticSubscriberExtensionContext>(
            static_cast<JsStaticSubscriberExtensionContext*>(data));
    }
private:
    std::weak_ptr<StaticSubscriberExtensionContext> context_;
};
} // namespace

NativeValue* CreateJsStaticSubscriberExtensionContext(NativeEngine& engine,
    std::shared_ptr<StaticSubscriberExtensionContext> context)
{
    HILOG_INFO("CreateJsStaticSubscriberExtensionContext begin");
    NativeValue *objValue = CreateJsExtensionContext(engine, context);
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsStaticSubscriberExtensionContext> jsContext =
        std::make_unique<JsStaticSubscriberExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsStaticSubscriberExtensionContext::Finalizer, nullptr);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
