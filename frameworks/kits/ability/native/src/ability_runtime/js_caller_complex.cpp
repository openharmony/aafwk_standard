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
#include <string>
#include <set>

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ability_runtime/js_caller_complex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless
static std::map<NativeValueType, std::string> logcast = {
    { NATIVE_UNDEFINED, std::string("NATIVE_UNDEFINED") },
    { NATIVE_NULL, std::string("NATIVE_NULL") },
    { NATIVE_BOOLEAN, std::string("NATIVE_BOOLEAN") },
    { NATIVE_NUMBER, std::string("NATIVE_NUMBER") },
    { NATIVE_STRING, std::string("NATIVE_STRING") },
    { NATIVE_SYMBOL, std::string("NATIVE_SYMBOL") },
    { NATIVE_OBJECT, std::string("NATIVE_OBJECT") },
    { NATIVE_FUNCTION, std::string("NATIVE_FUNCTION") },
    { NATIVE_EXTERNAL, std::string("NATIVE_EXTERNAL") },
    { NATIVE_BIGINT, std::string("NATIVE_BIGINT") },
};

class JsCallerComplex {
public:
    explicit JsCallerComplex(
        NativeEngine& engine, std::shared_ptr<AbilityContext> context, sptr<IRemoteObject> callee,
        std::shared_ptr<CallerCallBack> callerCallBack) : context_(context), callee_(callee),
        releaseCallBackEngine_(engine), callerCallBackObj_(callerCallBack), jsreleaseCallBackObj_(nullptr)
        {
            handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        };
    virtual~JsCallerComplex() {};

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        if (data == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, but input parameters is nullptr", __func__);
            return;
        }
        std::unique_ptr<JsCallerComplex>(static_cast<JsCallerComplex*>(data));
    }

    static NativeValue* JsRelease(NativeEngine* engine, NativeCallbackInfo* info)
    {
        if (engine == nullptr || info == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, but input parameters %{public}s is nullptr",
                __func__,
                ((engine == nullptr) ? "engine" : "info"));
            return nullptr;
        }

        auto object = CheckParamsAndGetThis<JsCallerComplex>(engine, info);
        if (object == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, CheckParamsAndGetThis return nullptr", __func__);
            return nullptr;
        }

        return object->ReleaseInner(*engine, *info);
    }

    static NativeValue* JsSetOnReleaseCallBack(NativeEngine* engine, NativeCallbackInfo* info)
    {
        if (engine == nullptr || info == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, but input parameters %{public}s is nullptr",
                __func__,
                ((engine == nullptr) ? "engine" : "info"));
            return nullptr;
        }

        auto object = CheckParamsAndGetThis<JsCallerComplex>(engine, info);
        if (object == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, CheckParamsAndGetThis return nullptr", __func__);
            return nullptr;
        }

        return object->SetOnReleaseCallBackInner(*engine, *info);
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        return callee_;
    }

private:

    void OnReleaseNotify(const std::string &str)
    {
        HILOG_DEBUG("OnReleaseNotify begin");
        if (handler_ == nullptr) {
            HILOG_ERROR("");
            return;
        }

        auto task = [notify = this, &str] () { notify->OnReleaseNotifyTask(str); };
        handler_->PostTask(task, "OnReleaseNotify");
        HILOG_DEBUG("OnReleaseNotify end");
    }

    void OnReleaseNotifyTask(const std::string &str)
    {
        HILOG_DEBUG("OnReleaseNotifyTask begin");
        if (jsreleaseCallBackObj_ == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s, jsreleaseObj is nullptr", __func__);
            return;
        }

        NativeValue* value = jsreleaseCallBackObj_->Get();
        NativeValue* callback = jsreleaseCallBackObj_->Get();
        NativeValue* args[] = { CreateJsValue(releaseCallBackEngine_, str) };
        releaseCallBackEngine_.CallFunction(value, callback, args, 1);
        HILOG_DEBUG("OnReleaseNotifyTask end");
    }

    NativeValue* ReleaseInner(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("JsCallerComplex::%{public}s, called", __func__);
        constexpr size_t ARGC_ZERO = 0;
        constexpr size_t ARGC_ONE = 1;
        int errCode = 0;
        AsyncTask::ExecuteCallback execute =
            [weak = context_, callback = callerCallBackObj_, &errCode] (NativeEngine& engine, AsyncTask& task) {
                auto context = weak.lock();
                if (context == nullptr) {
                    HILOG_ERROR("JsCallerComplex::%{public}s, context is nullptr", "ReleaseInner::ExecuteCallback");
                    errCode = -1;
                    return;
                }

                errCode = context->ReleaseAbility(callback);
                HILOG_DEBUG("JsCallerComplex::%{public}s %{public}d, end", "ReleaseInner::ExecuteCallback", errCode);
            };
        AsyncTask::CompleteCallback complete =
            [errCode] (NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errCode, "CallerComplex Release Failed."));
                }
                HILOG_DEBUG("JsCallerComplex::%{public}s, end", "ReleaseInner::CompleteCallback");
            };

        NativeValue* lastParam = ((info.argc == ARGC_ONE) ? info.argv[ARGC_ZERO] : nullptr);
        NativeValue* retsult = nullptr;
        AsyncTask::Schedule(
            engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &retsult));

        return retsult;
    }

    NativeValue* SetOnReleaseCallBackInner(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("JsCallerComplex::%{public}s, begin", __func__);
        constexpr size_t ARGC_ONE = 1;
        constexpr size_t ARGC_TWO = 2;
        bool errCode = true;
        if (info.argc >= ARGC_TWO) {
            HILOG_ERROR("JsCallerComplex::%{public}s, Invalid input params", __func__);
            return engine.CreateUndefined();
        }
        if (!info.argv[0]->IsCallable()) {
            HILOG_ERROR("JsCallerComplex::%{public}s, IsCallable is %{public}s.",
                __func__, ((info.argv[0]->IsCallable()) ? "true" : "false"));
            return engine.CreateUndefined();
        }
        while (errCode) {
            if (callerCallBackObj_ == nullptr) {
                HILOG_ERROR("JsCallerComplex::%{public}s, param1 is nullptr", __func__);
                errCode = false;
                break;
            }
            auto param1 = info.argv[0];
            if (param1 == nullptr) {
                HILOG_ERROR("JsCallerComplex::%{public}s, param1 is nullptr", __func__);
                errCode = false;
                break;
            }
            jsreleaseCallBackObj_ =
                std::unique_ptr<NativeReference>(releaseCallBackEngine_.CreateReference(param1, 1));
            auto task = [notify = this] (const std::string &str) { notify->OnReleaseNotify(str); };
            callerCallBackObj_->SetOnRelease(task);
            break;
        }
        AsyncTask::CompleteCallback complete =
            [errCode] (NativeEngine& engine, AsyncTask& task, int32_t status) {
                if (errCode) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, -1, "CallerComplex On Release CallBack Failed."));
                }
                HILOG_DEBUG("JsCallerComplex::%{public}s, %{public}s end", "ReleaseInner::CompleteCallback",
                    (errCode ? "true" : "false"));
            };
        NativeValue* lastParam = ((info.argc == ARGC_TWO) ? info.argv[ARGC_ONE] : nullptr);
        NativeValue* retsult = nullptr;
        AsyncTask::Schedule(engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &retsult));
        HILOG_DEBUG("JsCallerComplex::%{public}s, end", __func__);
        return retsult;
    }

private:
    std::weak_ptr<AbilityContext> context_;
    sptr<IRemoteObject> callee_;
    NativeEngine& releaseCallBackEngine_;
    std::shared_ptr<CallerCallBack> callerCallBackObj_;
    std::unique_ptr<NativeReference> jsreleaseCallBackObj_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // nameless

NativeValue* CreateJsCallerComplex(
    NativeEngine& engine, std::shared_ptr<AbilityContext> context, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack)
{
    HILOG_DEBUG("JsCallerComplex::%{public}s, begin", __func__);
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    auto jsCaller = std::make_unique<JsCallerComplex>(engine, context, callee, callerCallBack);
    if (jsCaller == nullptr) {
        HILOG_ERROR("%{public}s is called, but make_unique<JsCallerComplex> is nullptr", __func__);
        return engine.CreateUndefined();
    }

    auto remoteObj = jsCaller->GetRemoteObject();
    if (remoteObj == nullptr) {
        HILOG_ERROR("%{public}s is called,remoteObj is nullptr", __func__);
        return engine.CreateUndefined();
    }

    object->SetNativePointer(jsCaller.release(), JsCallerComplex::Finalizer, nullptr);
    object->SetProperty("callee", CreateJsCalleeRemoteObject(engine, remoteObj));
    BindNativeFunction(engine, *object, "release", JsCallerComplex::JsRelease);
    BindNativeFunction(engine, *object, "onRelease", JsCallerComplex::JsSetOnReleaseCallBack);

    HILOG_DEBUG("JsCallerComplex::%{public}s, end", __func__);
    return objValue;
}

NativeValue* CreateJsCalleeRemoteObject(NativeEngine& engine, sptr<IRemoteObject> callee)
{
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(&engine), callee);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);

    if (nativeRemoteObject == nullptr) {
        HILOG_ERROR("%{public}s is called, but remoteObj is nullptr", __func__);
    }

    return nativeRemoteObject;
}
} // AbilityRuntime
} // OHOS