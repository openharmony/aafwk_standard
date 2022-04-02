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
    enum class OBJSTATE {
        OBJ_NORMAL,
        OBJ_EXECUTION,
        OBJ_RELEASE
    };

    explicit JsCallerComplex(
        NativeEngine& engine, std::shared_ptr<AbilityContext> context, sptr<IRemoteObject> callee,
        std::shared_ptr<CallerCallBack> callerCallBack) : context_(context), callee_(callee),
        releaseCallBackEngine_(engine), callerCallBackObj_(callerCallBack), jsreleaseCallBackObj_(nullptr)
    {
        AddJsCallerComplex(this);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        currentState_ = OBJSTATE::OBJ_NORMAL;
    };
    virtual~JsCallerComplex()
    {
        RemoveJsCallerComplex(this);
    };

    static bool ReleaseObject(JsCallerComplex* data)
    {
        HILOG_DEBUG("ReleaseObject begin %{public}p", data);
        if (data == nullptr) {
            HILOG_ERROR("ReleaseObject begin, but input parameters is nullptr");
            return false;
        }

        if (!data->ChangeCurrentState(OBJSTATE::OBJ_RELEASE)) {
            auto handler = data->GetEventHandler();
            if (handler == nullptr) {
                HILOG_ERROR("ReleaseObject error end, Get eventHandler failed");
                return false;
            }
            auto releaseObjTask = [pdata = data] () {
                if (!FindJsCallerComplex(pdata)) {
                    HILOG_ERROR("ReleaseObject error end, but input parameters does not found");
                    return;
                }
                ReleaseObject(pdata);
            };

            handler->PostTask(releaseObjTask, "FinalizerRelease");
            return false;
        } else {
            // when the object is about to be destroyed, does not reset state
            std::unique_ptr<JsCallerComplex> delObj(data);
        }
        HILOG_DEBUG("ReleaseObject success end");
        return true;
    }

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_DEBUG("JsCallerComplex::%{public}s begin.", __func__);
        if (data == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, but input parameters is nullptr", __func__);
            return;
        }

        auto ptr = static_cast<JsCallerComplex*>(data);
        if (!FindJsCallerComplex(ptr)) {
            HILOG_ERROR("JsCallerComplex::%{public}s is called, but input parameters does not found", __func__);
            return;
        }

        ReleaseObject(ptr);
        HILOG_DEBUG("JsCallerComplex::%{public}s end.", __func__);
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

    static bool AddJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            HILOG_ERROR("JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter != jsCallerComplexManagerList.end()) {
            HILOG_ERROR("JsAbilityContext::%{public}s, address exists", __func__);
            return false;
        }

        auto iterRet = jsCallerComplexManagerList.emplace(ptr);
        HILOG_DEBUG("JsAbilityContext::%{public}s, execution ends and retval is %{public}s",
            __func__, iterRet.second ? "true" : "false");
        return iterRet.second;
    }

    static bool RemoveJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            HILOG_ERROR("JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            HILOG_ERROR("JsAbilityContext::%{public}s, input parameters not found", __func__);
            return false;
        }

        jsCallerComplexManagerList.erase(ptr);
        HILOG_DEBUG("JsAbilityContext::%{public}s, called", __func__);
        return true;
    }

    static bool FindJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            HILOG_ERROR("JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }
        auto ret = true;
        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            ret = false;
        }
        HILOG_DEBUG("JsAbilityContext::%{public}s, execution ends and retval is %{public}s",
            __func__, ret ? "true" : "false");
        return ret;
    }

    static bool FindJsCallerComplexAndChangeState(JsCallerComplex* ptr, OBJSTATE state)
    {
        if (ptr == nullptr) {
            HILOG_ERROR("JsAbilityContext::%{public}s, input parameters is nullptr", __func__);
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            HILOG_ERROR("JsAbilityContext::%{public}s, execution end, but not found", __func__);
            return false;
        }

        auto ret = ptr->ChangeCurrentState(state);
        HILOG_DEBUG("JsAbilityContext::%{public}s, execution ends and ChangeCurrentState retval is %{public}s",
            __func__, ret ? "true" : "false");

        return ret;
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        return callee_;
    }

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler()
    {
        return handler_;
    }

    bool ChangeCurrentState(OBJSTATE state)
    {
        auto ret = false;
        if (stateMechanismMutex_.try_lock() == false) {
            HILOG_ERROR("mutex try_lock false");
            return ret;
        }

        if (currentState_ == OBJSTATE::OBJ_NORMAL) {
            currentState_ = state;
            ret = true;
            HILOG_DEBUG("currentState_ == OBJSTATE::OBJ_NORMAL");
        } else if (currentState_ == state) {
            ret = true;
            HILOG_DEBUG("currentState_ == state");
        } else {
            ret = false;
            HILOG_DEBUG("ret = false");
        }

        stateMechanismMutex_.unlock();
        return ret;
    }

    OBJSTATE GetCurrentState()
    {
        return currentState_;
    }

    void StateReset()
    {
        currentState_ = OBJSTATE::OBJ_NORMAL;
    }

private:

    void OnReleaseNotify(const std::string &str)
    {
        HILOG_DEBUG("OnReleaseNotify begin");
        if (handler_ == nullptr) {
            HILOG_ERROR("handler parameters error");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindJsCallerComplex(notify)) {
                HILOG_ERROR("ptr not found, address error");
                return;
            }
            notify->OnReleaseNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnReleaseNotify");
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
        HILOG_DEBUG("OnReleaseNotifyTask CallFunction call done");
        callee_ = nullptr;
        StateReset();
        HILOG_DEBUG("OnReleaseNotifyTask end");
    }

    NativeValue* ReleaseInner(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("JsCallerComplex::%{public}s, called", __func__);
        if (callerCallBackObj_ == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s, CallBacker is nullptr", __func__);
            return CreateJsError(engine, -1, "CallerComplex callback is nullptr.");
        }

        auto context = context_.lock();
        if (context == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s, context is nullptr", __func__);
            return CreateJsError(engine, -1, "CallerComplex get context failed.");
        }

        auto retErr = context->ReleaseAbility(callerCallBackObj_);
        if (retErr != ERR_OK) {
            HILOG_ERROR("JsCallerComplex::%{public}s, ReleaseAbility failed %{public}d",
                __func__, static_cast<int>(retErr));
            return CreateJsError(engine, -1, "CallerComplex get context failed.");
        }

        return engine.CreateUndefined();
    }

    NativeValue* SetOnReleaseCallBackInner(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("JsCallerComplex::%{public}s, begin", __func__);
        constexpr size_t ARGC_TWO = 2;
        if (info.argc >= ARGC_TWO) {
            HILOG_ERROR("JsCallerComplex::%{public}s, Invalid input params", __func__);
            return CreateJsError(engine, -1, "CallerComplex on release CallBack input params error.");
        }
        if (!info.argv[0]->IsCallable()) {
            HILOG_ERROR("JsCallerComplex::%{public}s, IsCallable is %{public}s.",
                __func__, ((info.argv[0]->IsCallable()) ? "true" : "false"));
            return CreateJsError(engine, -1, "CallerComplex on release CallBack input params not function.");
        }

        if (callerCallBackObj_ == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s, param1 is nullptr", __func__);
            return CreateJsError(engine, -1, "CallerComplex on release CallBacker is nullptr.");
        }

        auto param1 = info.argv[0];
        if (param1 == nullptr) {
            HILOG_ERROR("JsCallerComplex::%{public}s, param1 is nullptr", __func__);
            return CreateJsError(engine, -1, "CallerComplex on release input params isn`t function.");
        }

        jsreleaseCallBackObj_.reset(releaseCallBackEngine_.CreateReference(param1, 1));
        auto task = [notify = this] (const std::string &str) {
            if (!FindJsCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                HILOG_ERROR("ptr not found, address error");
                return;
            }
            notify->OnReleaseNotify(str);
        };
        callerCallBackObj_->SetOnRelease(task);
        HILOG_DEBUG("JsCallerComplex::%{public}s, end", __func__);
        return engine.CreateUndefined();
    }

private:
    std::weak_ptr<AbilityContext> context_;
    sptr<IRemoteObject> callee_;
    NativeEngine& releaseCallBackEngine_;
    std::shared_ptr<CallerCallBack> callerCallBackObj_;
    std::unique_ptr<NativeReference> jsreleaseCallBackObj_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex stateMechanismMutex_;
    OBJSTATE currentState_;

    static std::set<JsCallerComplex*> jsCallerComplexManagerList;
    static std::mutex jsCallerComplexMutex;
};

std::set<JsCallerComplex*> JsCallerComplex::jsCallerComplexManagerList;
std::mutex JsCallerComplex::jsCallerComplexMutex;
} // nameless

NativeValue* CreateJsCallerComplex(
    NativeEngine& engine, std::shared_ptr<AbilityContext> context, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack)
{
    HILOG_DEBUG("JsCallerComplex::%{public}s, begin", __func__);
    if (callee == nullptr || callerCallBack == nullptr || context == nullptr) {
        HILOG_ERROR("%{public}s is called, input params error. %{public}s is nullptr", __func__,
            (callee == nullptr) ? ("callee") : ((context == nullptr) ? ("context") : ("callerCallBack")));
        return engine.CreateUndefined();
    }

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
    if (callee == nullptr) {
        HILOG_ERROR("%{public}s is called, input params is nullptr", __func__);
        return engine.CreateUndefined();
    }
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