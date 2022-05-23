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
#include "form_binding_data.h"

#include "form_provider_data.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
class FormBindingData {
public:
    explicit FormBindingData(const std::shared_ptr<AppExecFwk::FormProviderData>& formProviderData)
        : formProviderData_(formProviderData) {}
    ~FormBindingData() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("FormBindingData::Finalizer is called");
        std::unique_ptr<FormBindingData>(static_cast<FormBindingData*>(data));
    }

    static NativeValue* CreateFormBindingData(NativeEngine* engine, NativeCallbackInfo* info)
    {
        FormBindingData* me = CheckParamsAndGetThis<FormBindingData>(engine, info);
        return (me != nullptr) ? me->OnCreateFormBindingData(*engine, *info) : nullptr;
    }

    static NativeValue* AddFormBindingImage(NativeEngine* engine, NativeCallbackInfo* info)
    {
        HILOG_INFO("%{public}s called.", __func__);
        FormBindingData* me = CheckParamsAndGetThis<FormBindingData>(engine, info);
        return (me != nullptr) ? me->OnAddFormBindingImage(*engine, *info) : nullptr;
    }
private:
    NativeValue* OnCreateFormBindingData(NativeEngine& engine, NativeCallbackInfo& info);
    std::shared_ptr<AppExecFwk::FormProviderData> formProviderData_;
    NativeValue* OnAddFormBindingImage(NativeEngine& engine, NativeCallbackInfo& info);
};

NativeValue* FormBindingData::OnCreateFormBindingData(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("%{public}s called.", __func__);
    std::string formDataStr;
    if (info.argc > 0) {
        NativeValue* nativeValue = nullptr;
        if ((info.argv[0])->TypeOf() == NATIVE_STRING) {
            HILOG_DEBUG("%{public}s called, param type is string.", __func__);
            nativeValue = info.argv[0];
        } else if ((info.argv[0])->TypeOf() == NATIVE_OBJECT) {
            HILOG_DEBUG("%{public}s called, param type is object.", __func__);
            napi_env napiEnv = reinterpret_cast<napi_env>(&engine);
            napi_value globalValue = nullptr;
            napi_get_global(napiEnv, &globalValue);
            napi_value jsonValue;
            napi_get_named_property(napiEnv, globalValue, "JSON", &jsonValue);

            napi_value stringifyValue = nullptr;
            napi_get_named_property(napiEnv, jsonValue, "stringify", &stringifyValue);
            napi_value funcArgv[1] = { reinterpret_cast<napi_value>(info.argv[0]) };
            napi_value transValue = nullptr;
            napi_call_function(napiEnv, jsonValue, stringifyValue, 1, funcArgv, &transValue);
            nativeValue = reinterpret_cast<NativeValue*>(transValue);
        } else {
            HILOG_ERROR("%{public}s, param type not string or object", __func__);
            return engine.CreateUndefined();
        }

        if (!ConvertFromJsValue(engine, nativeValue, formDataStr)) {
            HILOG_ERROR("%{public}s, Parse formDataStr failed", __func__);
            return engine.CreateUndefined();
        }
    }
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    formProviderData_->SetDataString(formDataStr);
    object->SetProperty("data", CreateJsValue(engine, formDataStr));
    HILOG_INFO("%{public}s called:%{public}s", __func__, formDataStr.c_str());
    return objValue;
}

NativeValue* FormBindingData::OnAddFormBindingImage(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (info.argc == 0) {
        HILOG_ERROR("%{public}s, not enough params", __func__);
        return engine.CreateUndefined();
    }

    NativeValue* objValue = info.argv[1];
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("image", info.argv[0]);
    HILOG_INFO("%{public}s called end.", __func__);
    return objValue;
}
}

NativeValue* FormBindingDataInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("%{public}s engine or exportObj nullptr.", __func__);
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("%{public}s convertNativeValueTo result is nullptr.", __func__);
        return nullptr;
    }

    auto formProviderData = std::make_shared<AppExecFwk::FormProviderData>();
    auto formBindingData = std::make_unique<FormBindingData>(formProviderData);
    object->SetNativePointer(formBindingData.release(), FormBindingData::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "createFormBindingData", FormBindingData::CreateFormBindingData);
    BindNativeFunction(*engine, *object, "addFormBindingImage", FormBindingData::AddFormBindingImage);

    HILOG_INFO("%{public}s called end.", __func__);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
