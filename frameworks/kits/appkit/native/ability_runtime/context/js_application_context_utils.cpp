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
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_hap_module_info_utils.h"
#include "js_resource_manager_utils.h"
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
    static NativeValue *CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *SwitchArea(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetArea(NativeEngine* engine, NativeCallbackInfo* info);

    NativeValue *OnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnUnregisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);

    NativeValue *OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info);

    static NativeValue *GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetTempDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info);

    void KeepApplicationContext(std::shared_ptr<ApplicationContext> applicationContext)
    {
        keepApplicationContext_ = applicationContext;
    }

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    NativeValue *OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetArea(NativeEngine& engine, NativeCallbackInfo& info);
    std::shared_ptr<ApplicationContext> keepApplicationContext_;
    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
};

NativeValue *JsApplicationContextUtils::CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateBundleContext(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }

    std::string bundleName;
    if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
        HILOG_ERROR("Parse bundleName failed");
        return engine.CreateUndefined();
    }

    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        HILOG_ERROR("bundleContext is nullptr");
        return engine.CreateUndefined();
    }

    JsRuntime &jsRuntime = *static_cast<JsRuntime *>(engine.GetJsEngine());
    NativeValue *value = CreateJsBaseContext(engine, bundleContext, true);
    return jsRuntime.LoadSystemModule("application.Context", &value, 1)->Get();
}

NativeValue *JsApplicationContextUtils::SwitchArea(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::SwitchArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSwitchArea(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }

    int mode;
    if (!ConvertFromJsValue(engine, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return engine.CreateUndefined();
    }

    applicationContext->SwitchArea(mode);

    NativeValue *thisVar = info.thisVar;
    NativeObject *object = ConvertNativeValueTo<NativeObject>(thisVar);
    BindNativeProperty(*object, "cacheDir", GetCacheDir);
    BindNativeProperty(*object, "tempDir", GetTempDir);
    BindNativeProperty(*object, "filesDir", GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", GetBundleCodeDir);
    return engine.CreateUndefined();
}

NativeValue* JsApplicationContextUtils::GetArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetArea(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    int area = applicationContext->GetArea();
    return engine.CreateNumber(area);
}

NativeValue *JsApplicationContextUtils::GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetCacheDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetTempDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetTempDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDistributedFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDistributedFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDatabaseDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetPreferencesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetBundleCodeDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

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

    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    if (callback_ != nullptr) {
        return engine.CreateNumber(callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    int callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    keepApplicationContext_->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnUnregisterAbilityLifecycleCallback is called");
    int32_t errCode = 0;
    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        errCode = ERROR_CODE_ONE;
    }
    int64_t callbackId = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], callbackId)) {
        HILOG_ERROR("Parse callbackId failed");
        errCode = ERROR_CODE_ONE;
    }
    HILOG_INFO("callbackId is %{public}d.", (int32_t)callbackId);
    AsyncTask::CompleteCallback complete =
        [&applicationContext = keepApplicationContext_, &callback = callback_, callbackId, errCode](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_INFO("OnUnregisterAbilityLifecycleCallback begin");
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            if (applicationContext == nullptr) {
                HILOG_ERROR("applicationContext is nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "applicationContext is nullptr"));
                return;
            }
            callback->UnRegister(callbackId);
            if (callback->IsEmpty()) {
                applicationContext->UnregisterAbilityLifecycleCallback(callback);
            }
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

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto hapModuleInfo = applicationContext->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        object->SetProperty("currentHapModuleInfo", CreateJsHapModuleInfo(engine, *hapModuleInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager));
    }

    BindNativeProperty(*object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(engine, *object, "registerAbilityLifecycleCallback",
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "unregisterAbilityLifecycleCallback",
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "createBundleContext", JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(engine, *object, "switchArea", JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(engine, *object, "getArea", JsApplicationContextUtils::GetArea);
    HILOG_INFO("CreateJsApplicationContext end");
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
