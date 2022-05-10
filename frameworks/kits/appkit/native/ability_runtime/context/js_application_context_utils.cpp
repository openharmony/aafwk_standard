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

#include <map>

#include "application_context.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t MAX_ERROR_CODE = -1;

class JsApplicationContextUtils {
public:
    explicit JsApplicationContextUtils(std::weak_ptr<ApplicationContext> &&applicationContext)
        : applicationContext_(std::move(applicationContext))
    {
    }
    virtual ~JsApplicationContextUtils() = default;
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue *RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *UnregisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);

    NativeValue *OnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnUnregisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);

    void KeepApplicationContext(std::shared_ptr<ApplicationContext> applicationContext)
    {
        keepApplicationContext_ = applicationContext;
    }
protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    std::shared_ptr<ApplicationContext> keepApplicationContext_;
};

void JsApplicationContextUtils::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JsApplicationContextUtils::Finalizer is called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

NativeValue *JsApplicationContextUtils::RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnregisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnRegisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return engine.CreateUndefined();
    }
    std::shared_ptr<AbilityLifecycleCallback> callback = std::make_shared<AbilityLifecycleCallback>(engine);
    int64_t callbackId = serialNumber_;
    if (serialNumber_ < INT64_MAX) {
        serialNumber_++;
    } else {
        HILOG_ERROR("callback id exceed maximum");
        return engine.CreateNumber(MAX_ERROR_CODE);
    }
    callback->SetJsCallback(info.argv[INDEX_ZERO]);
    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    keepApplicationContext_->RegisterAbilityLifecycleCallback(callbackId, callback);
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnUnregisterAbilityLifecycleCallback is called");
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }
    int64_t callbackId = -1;
    napi_get_value_int64(
        reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &callbackId);
    AsyncTask::CompleteCallback complete = [applicationContext = keepApplicationContext_, callbackId](
                                               NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnUnregisterAbilityLifecycleCallback begin");
        if (applicationContext == nullptr) {
            HILOG_ERROR("applicationContext is nullptr");
            task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "applicationContext is nullptr"));
            return;
        }
        applicationContext->UnregisterAbilityLifecycleCallback(callbackId);
        task.Resolve(engine, engine.CreateUndefined());
    };
    NativeValue *lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}
}  // namespace

NativeValue *CreateJsApplicationContext(
    NativeEngine &engine, std::shared_ptr<ApplicationContext> applicationContext, bool keepApplicationContext)
{
    HILOG_INFO("CreateJsApplicationContext start");
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    if (keepApplicationContext) {
        jsApplicationContextUtils->KeepApplicationContext(applicationContext);
    }
    SetNamedNativePointer(engine, *object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    BindNativeFunction(engine, *object, "registerAbilityLifecycleCallback",
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "unregisterAbilityLifecycleCallback",
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    HILOG_INFO("CreateJsApplicationContext end");
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS