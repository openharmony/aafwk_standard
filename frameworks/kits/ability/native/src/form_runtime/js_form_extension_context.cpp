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

#include "form_runtime/js_form_extension_context.h"

#include <cinttypes>
#include <cstdint>

#include "form_runtime/js_form_extension_util.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const int UPDATE_FORM_PARAMS_SIZE = 2;
class JsFormExtensionContext final {
public:
    JsFormExtensionContext(const std::shared_ptr<FormExtensionContext>& context) : context_(context) {}
    ~JsFormExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsFormExtensionContext>(static_cast<JsFormExtensionContext*>(data));
    }

    static NativeValue* UpdateForm(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsFormExtensionContext* me = CheckParamsAndGetThis<JsFormExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnUpdateForm(*engine, *info) : nullptr;
    }
private:
    std::weak_ptr<FormExtensionContext> context_;

    NativeValue* OnUpdateForm(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("%{public}s called.", __func__);
        if (info.argc < UPDATE_FORM_PARAMS_SIZE) {
            HILOG_ERROR("Not enough params, not enough params");
            return engine.CreateUndefined();
        }

        std::string strFormId;
        int64_t formId = ConvertFromJsValue(engine, info.argv[0], strFormId) ? std::stoll(strFormId) : -1;

        AppExecFwk::FormProviderData formProviderData;
        std::string formDataStr = "{}";
        std::map<std::string, int> rawImageDataMap;
        NativeObject* nativeObject = ConvertNativeValueTo<NativeObject>(info.argv[1]);
        if (nativeObject != nullptr) {
            NativeValue* nativeDataValue = nativeObject->GetProperty("data");
            if (nativeDataValue == nullptr || !ConvertFromJsValue(engine, nativeDataValue, formDataStr)) {
                HILOG_ERROR("%{public}s called. nativeDataValue is nullptr or ConvertFromJsValue failed", __func__);
            }
            nativeDataValue = nativeObject->GetProperty("image");
            if (nativeDataValue != nullptr) {
                UnwrapRawImageDataMap(engine, nativeDataValue, rawImageDataMap);
            }
        } else {
            HILOG_ERROR("%{public}s called. nativeObject is nullptr", __func__);
        }

        formProviderData = AppExecFwk::FormProviderData(formDataStr);
        HILOG_INFO("Image number is %{public}zu", rawImageDataMap.size());
        for (auto entry : rawImageDataMap) {
            formProviderData.AddImageData(entry.first, entry.second);
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, formId, formProviderData](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                auto errcode = context->UpdateForm(formId, formProviderData);
                if (errcode == ERR_OK) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "update form failed."));
                }
            };

        NativeValue* lastParam =
            (info.argc == UPDATE_FORM_PARAMS_SIZE) ? nullptr : info.argv[UPDATE_FORM_PARAMS_SIZE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* CreateJsFormExtensionContext(NativeEngine& engine, std::shared_ptr<FormExtensionContext> context)
{
    HILOG_INFO("%{public}s called.", __func__);
    NativeValue* objValue = CreateJsExtensionContext(engine, context);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsFormExtensionContext> jsContext = std::make_unique<JsFormExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsFormExtensionContext::Finalizer, nullptr);

    BindNativeFunction(engine, *object, "updateForm", JsFormExtensionContext::UpdateForm);

    HILOG_INFO("%{public}s called end.", __func__);
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
