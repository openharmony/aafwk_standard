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

#include "ability_runtime/js_ability_context.h"

#include <cstdint>

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

class JsAbilityContext final {
public:
    JsAbilityContext(const std::shared_ptr<AbilityContext>& context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
    }

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
    }

    static NativeValue* StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
    }

    static NativeValue* ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
    }

    static NativeValue* DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
    }

    static NativeValue* TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
    }

    static NativeValue* TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
        return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbility is called");

        if (info.argc == ARGC_ZERO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), want);

        AsyncTask::CompleteCallback complete =
            [weak = context_, want](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }

                auto errcode = context->StartAbility(want, -1);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnStartAbilityForResult is called");

        if (info.argc == ARGC_ZERO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        AAFwk::Want want;
        if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
            HILOG_ERROR("%s Failed to parse want!", __func__);
            return engine.CreateUndefined();
        }

        NativeValue* lastParam = info.argc == 1 ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        std::unique_ptr<AsyncTask> uasyncTask =
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
        RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want& want) {
            HILOG_INFO("OnStartAbilityForResult async callback is called");
            NativeValue* abilityResult = JsAbilityContext::WrapAbilityResult(engine, resultCode, want);
            if (abilityResult == nullptr) {
                HILOG_WARN("wrap abilityResult failed");
                asyncTask->Reject(engine, CreateJsError(engine, 1, "failed to get result data!"));
            } else {
                asyncTask->Resolve(engine, abilityResult);
            }
            HILOG_INFO("OnStartAbilityForResult async callback is called end");
        };
        auto context = context_.lock();
        if (context == nullptr) {
            HILOG_WARN("context is released");
            asyncTask->Reject(engine, CreateJsError(engine, 1, "context is released!"));
        } else {
            curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
            context->StartAbilityForResult(want, curRequestCode_, std::move(task));
        }
        HILOG_INFO("OnStartAbilityForResult is called end");
        return result;
    }

    NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnTerminateSelfWithResult is called");

        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        int resultCode = 0;
        AAFwk::Want want;
        if (!JsAbilityContext::UnWrapAbilityResult(engine, info.argv[0], resultCode, want)) {
            HILOG_ERROR("%s Failed to parse ability result!", __func__);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_, want, resultCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }

                context->TerminateAbilityWithResult(want, resultCode);
                task.Resolve(engine, engine.CreateUndefined());
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        HILOG_INFO("OnTerminateSelfWithResult is called end");
        return result;
    }

    NativeValue* OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnConnectAbility is called");
        // only support two params
        if (info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        // unwrap want
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s",
            __func__,
            want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());

        // unwarp connection
        sptr<JSAbilityConnection> connection = new JSAbilityConnection(&engine);
        connection->SetJsConnectionObject(info.argv[1]);
        int64_t connectId = g_serialNumber;
        ConnectionKey key;
        key.id = g_serialNumber;
        key.want = want;
        abilityConnects_.emplace(key, connection);
        if (g_serialNumber < INT64_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one:%{public}p.", __func__, connection.GetRefPtr());
        AsyncTask::CompleteCallback complete =
            [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnConnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                HILOG_INFO("context->ConnectAbility connection:%{public}d", (int32_t)connectId);
                if (!context->ConnectAbility(want, connection)) {
                    connection->CallJsFailed();
                }
                task.Resolve(engine, engine.CreateUndefined());
            };
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(connectId);
    }

    NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnDisconnectAbility is called");
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        // unwrap connectId
        int64_t connectId = -1;
        sptr<JSAbilityConnection> connection = nullptr;
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), &connectId);
        HILOG_INFO("OnDisconnectAbility connection:%{public}d", (int32_t)connectId);
        auto item = std::find_if(abilityConnects_.begin(),
            abilityConnects_.end(),
            [&connectId](const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
                return connectId == obj.first.id;
            });
        if (item != abilityConnects_.end()) {
            // match id
            connection = item->second;
            HILOG_INFO("%{public}s find conn ability:%{public}p exist", __func__, item->second.GetRefPtr());
        } else {
            HILOG_INFO("%{public}s not find conn exist.", __func__);
        }
        // begin disconnect
        AsyncTask::CompleteCallback complete =
            [weak = context_, connection](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnDisconnectAbility begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                if (connection == nullptr) {
                    HILOG_WARN("connection nullptr");
                    task.Reject(engine, CreateJsError(engine, 2, "not found connection"));
                    return;
                }
                HILOG_INFO("context->DisconnectAbility");
                context->DisconnectAbility(connection);
                task.Resolve(engine, engine.CreateUndefined());
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnTerminateSelf is called");

        if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }

                auto errcode = context->TerminateSelf();
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Terminate Ability failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    static bool UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
    {
        if (argv == nullptr) {
            HILOG_WARN("%s argv == nullptr!", __func__);
            return false;
        }
        return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
    }

    static NativeValue* WrapWant(NativeEngine& engine, const AAFwk::Want& want)
    {
        return reinterpret_cast<NativeValue*>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
    }

    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want)
    {
        if (argv == nullptr) {
            HILOG_WARN("%s argv == nullptr!", __func__);
            return false;
        }
        if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
            HILOG_WARN("%s invalid type of abilityResult!", __func__);
            return false;
        }
        NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
        NativeValue* jResultCode = jObj->GetProperty("resultCode");
        if (jResultCode == nullptr) {
            HILOG_WARN("%s jResultCode == nullptr!", __func__);
            return false;
        }
        if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
            HILOG_WARN("%s invalid type of resultCode!", __func__);
            return false;
        }
        resultCode = int64_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
        NativeValue* jWant = jObj->GetProperty("want");
        if (jWant == nullptr) {
            HILOG_WARN("%s jWant == nullptr!", __func__);
            return false;
        }
        if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
            HILOG_WARN("%s invalid type of want!", __func__);
            return false;
        }
        return JsAbilityContext::UnWrapWant(engine, jWant, want);
    }

    static NativeValue* WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want)
    {
        NativeValue* jAbilityResult = engine.CreateObject();
        NativeObject* abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
        abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
        abilityResult->SetProperty("want", JsAbilityContext::WrapWant(engine, want));
        return jAbilityResult;
    }

    std::weak_ptr<AbilityContext> context_;
    int curRequestCode_ = 0;
};


} // namespace

NativeValue* CreateJsAbilityContext(NativeEngine& engine, std::shared_ptr<AbilityContext> context)
{
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        object->SetProperty("abilityInfo", CreateJsAbilityInfo(engine, *abilityInfo));
    }

    BindNativeFunction(engine, *object, "startAbility", JsAbilityContext::StartAbility);
    BindNativeFunction(engine, *object, "startAbilityForResult", JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(engine, *object, "connectAbility", JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "disconnectAbility", JsAbilityContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "terminateSelf", JsAbilityContext::TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", JsAbilityContext::TerminateSelfWithResult);
    return objValue;
}

JSAbilityConnection::JSAbilityConnection(NativeEngine* engine) : engine_(engine) {}

JSAbilityConnection::~JSAbilityConnection() = default;

void JSAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    NativeValue* methodOnConnect = obj->GetProperty("onConnect");
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }
    HILOG_INFO("JSAbilityConnection::CallFunction onConnect, success");
    // two params
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    // wrap RemoteObject
    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(engine_.get()), remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = { ConvertElement(element), nativeRemoteObject };
    engine_->CallFunction(value, methodOnConnect, argv, ARGC_TWO);
    HILOG_INFO("OnAbilityConnectDone end");
}

void JSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_INFO("OnAbilityDisconnectDone abilityConnects_.size:%{public}zu", abilityConnects_.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    auto item = std::find_if(abilityConnects_.begin(), abilityConnects_.end(),
        [bundleName, abilityName] (const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
            return (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != abilityConnects_.end()) {
        // match bundlename && abilityname
        abilityConnects_.erase(item);
        HILOG_INFO("OnAbilityDisconnectDone erase abilityConnects_.size:%{public}zu", abilityConnects_.size());
    }
    // one params
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    NativeValue* argv[] = { ConvertElement(element) };
    HILOG_INFO("OnAbilityDisconnectDone CallFunction success");
    engine_->CallFunction(value, method, argv, ARGC_ONE);
}

void JSAbilityConnection::CallJsFailed()
{
    HILOG_INFO("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    NativeValue* value = jsConnectionObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("onFailed");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    HILOG_INFO("CallJsFailed CallFunction success");
    // no params
    engine_->CallFunction(value, method, nullptr, ARGC_ZERO);
    HILOG_INFO("CallJsFailed end");
}

NativeValue* JSAbilityConnection::ConvertElement(const AppExecFwk::ElementName &element)
{
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(engine_.get()), element);
    return reinterpret_cast<NativeValue*>(napiElementName);
}

void JSAbilityConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_->CreateReference(jsConnectionObject, 1));
}
}  // namespace AbilityRuntime
}  // namespace OHOS
