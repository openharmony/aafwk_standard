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

#include "ability_runtime/js_ability_context.h"

#include <cstdint>

#include "bytrace.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"
#include "ability_runtime/js_caller_complex.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "want.h"
#include "event_handler.h"

#ifdef SUPPORT_GRAPHICS
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ERROR_CODE_ONE = 1;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

void JsAbilityContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsAbilityContext::Finalizer is called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

NativeValue* JsAbilityContext::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityByCall(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityByCall(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::StartAbilityForResultWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResultWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnConnectAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::ConnectAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnConnectAbilityWithAccount(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnDisconnectAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::RequestPermissionsFromUser(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnRequestPermissionsFromUser(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::RestoreWindowStage(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnRestoreWindowStage(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::SetMissionLabel(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnSetMissionLabel(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::IsTerminating(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnIsTerminating(*engine, *info) : nullptr;
}

#ifdef SUPPORT_GRAPHICS
NativeValue* JsAbilityContext::SetMissionIcon(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnSetMissionIcon(*engine, *info) : nullptr;
}
#endif

NativeValue* JsAbilityContext::OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbility is called.");

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("Start ability, ability name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbility start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                return;
            }
            auto errcode = (unwrapArgc == 1) ?
                context->StartAbility(want, -1) : context->StartAbility(want, startOptions, -1);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
            }
        };

    NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbilityWithAccount is called");
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    HILOG_INFO("abilityName=%{public}s", want.GetElement().GetAbilityName().c_str());
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        return engine.CreateUndefined();
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && info.argv[INDEX_TWO]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[ARGC_TWO]), startOptions);
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, startOptions, unwrapArgc](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }

                auto errcode = (unwrapArgc == INDEX_TWO) ?
                    context->StartAbilityWithAccount(want, accountId, -1) : context->StartAbilityWithAccount(
                        want, accountId, startOptions, -1);
                if (errcode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Start Ability failed."));
                }
        };

    NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityByCall(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("JsAbilityContext::%{public}s, called", __func__);
    constexpr size_t ARGC_ONE = 1;
    constexpr size_t ARGC_TWO = 2;
    if (info.argc < ARGC_ONE || info.argv[0]->TypeOf() != NATIVE_OBJECT) {
        HILOG_ERROR("int put params count error");
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), want);
    InheritWindowMode(want);

    std::shared_ptr<StartAbilityByCallParameters> calls = std::make_shared<StartAbilityByCallParameters>();
    if (calls == nullptr) {
        HILOG_ERROR("calls create error");
        return engine.CreateUndefined();
    }

    NativeValue* lastParam = ((info.argc == ARGC_TWO) ? info.argv[ARGC_ONE] : nullptr);
    NativeValue* retsult = nullptr;

    calls->callerCallBack = std::make_shared<CallerCallBack>();

    auto callBackDone = [calldata = calls] (const sptr<IRemoteObject> &obj) {
        HILOG_DEBUG("OnStartAbilityByCall callBackDone mutexlock");
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        HILOG_DEBUG("OnStartAbilityByCall callBackDone remoteCallee assignment");
        calldata->remoteCallee = obj;
        calldata->condition.notify_all();
        HILOG_INFO("OnStartAbilityByCall callBackDone is called end");
    };

    auto releaseListen = [](const std::string &str) {
        HILOG_INFO("OnStartAbilityByCall releaseListen is called %{public}s", str.c_str());
    };

    auto callExecute = [calldata = calls] () {
        constexpr int CALLER_TIME_OUT = 10; // 10s
        std::unique_lock<std::mutex> lock(calldata->mutexlock);
        if (calldata->remoteCallee != nullptr) {
            HILOG_INFO("OnStartAbilityByCall callExecute callee isn`t nullptr");
            return;
        }

        if (calldata->condition.wait_for(lock, std::chrono::seconds(CALLER_TIME_OUT)) == std::cv_status::timeout) {
            HILOG_ERROR("OnStartAbilityByCall callExecute waiting callee timeout");
            calldata->err = -1;
        }
        HILOG_DEBUG("OnStartAbilityByCall callExecute end");
    };

    auto callComplete = [weak = context_, calldata = calls] (
        NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (calldata->err != 0) {
            HILOG_ERROR("OnStartAbilityByCall callComplete err is %{public}d", calldata->err);
            task.Reject(engine, CreateJsError(engine, calldata->err, "callComplete err."));
            return;
        }

        auto context = weak.lock();
        if (context != nullptr && calldata->callerCallBack != nullptr && calldata->remoteCallee != nullptr) {
            task.Resolve(engine,
                CreateJsCallerComplex(engine, context, calldata->remoteCallee, calldata->callerCallBack));
        } else {
            HILOG_ERROR("OnStartAbilityByCall callComplete params error %{public}s is nullptr",
                context == nullptr ? "context" :
                    (calldata->remoteCallee == nullptr ? "remoteCallee" : "callerCallBack"));
            task.Reject(engine, CreateJsError(engine, -1, "Create Call Failed."));
        }

        HILOG_DEBUG("OnStartAbilityByCall callComplete end");
    };

    calls->callerCallBack->SetCallBack(callBackDone);
    calls->callerCallBack->SetOnRelease(releaseListen);

    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnStartAbilityByCall context is nullptr");
        return engine.CreateUndefined();
    }

    if (context->StartAbility(want, calls->callerCallBack) != 0) {
        HILOG_ERROR("OnStartAbilityByCall StartAbility is failed");
        return engine.CreateUndefined();
    }

    if (calls->remoteCallee == nullptr) {
        HILOG_INFO("OnStartAbilityByCall async wait execute");
        AsyncTask::Schedule(
            engine,
            CreateAsyncTaskWithLastParam(
                engine, lastParam, std::move(callExecute), std::move(callComplete), &retsult));
    } else {
        HILOG_INFO("OnStartAbilityByCall promiss return result execute");
        AsyncTask::Schedule(
            engine,
            CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(callComplete), &retsult));
    }

    HILOG_DEBUG("JsAbilityContext::%{public}s, called end", __func__);
    return retsult;
}

NativeValue* JsAbilityContext::OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info)
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
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityForResult start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }

    NativeValue* lastParam = info.argc == unwrapArgc ? nullptr : info.argv[unwrapArgc];
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
        (unwrapArgc == 1) ? context->StartAbilityForResult(want, curRequestCode_, std::move(task)) :
            context->StartAbilityForResult(want, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_INFO("OnStartAbilityForResult is called end");
    return result;
}

NativeValue* JsAbilityContext::OnStartAbilityForResultWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbilityForResultWithAccount is called");
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    if (!JsAbilityContext::UnWrapWant(engine, info.argv[0], want)) {
        HILOG_ERROR("%s Failed to parse want!", __func__);
        return engine.CreateUndefined();
    }
    InheritWindowMode(want);
    decltype(info.argc) unwrapArgc = 1;
    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        return engine.CreateUndefined();
    }
    unwrapArgc++;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_TWO && info.argv[INDEX_TWO]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("OnStartAbilityForResultWithAccount start options is used.");
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_TWO]), startOptions);
        unwrapArgc++;
    }
    NativeValue* lastParam = info.argc == unwrapArgc ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    std::unique_ptr<AsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want& want) {
        HILOG_INFO("OnStartAbilityForResultWithAccount async callback is called");
        NativeValue* abilityResult = JsAbilityContext::WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, 1, "failed to get result data!"));
        } else {
            asyncTask->Resolve(engine, abilityResult);
        }
        HILOG_INFO("OnStartAbilityForResultWithAccount async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, 1, "context is released!"));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        (unwrapArgc == INDEX_TWO) ? context->StartAbilityForResultWithAccount(
            want, accountId, curRequestCode_, std::move(task)) : context->StartAbilityForResultWithAccount(
                want, accountId, startOptions, curRequestCode_, std::move(task));
    }
    HILOG_INFO("OnStartAbilityForResultWithAccount is called end");
    return result;
}

NativeValue* JsAbilityContext::OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
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

    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
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

NativeValue* JsAbilityContext::OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("Connect ability called.");
    // only support two params
    if (info.argc != ARGC_TWO) {
        HILOG_ERROR("Connect ability failed, not enough params.");
        return engine.CreateUndefined();
    }

    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), want);
    HILOG_INFO("Connect ability called, callee:%{public}s.%{public}s.",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(engine);
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
    HILOG_INFO("%{public}s not find connection, make new one", __func__);
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_INFO("OnConnectAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("Connect ability failed, context is released.");
                task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                return;
            }
            if (!context->ConnectAbility(want, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

NativeValue* JsAbilityContext::OnConnectAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnConnectAbility is called");
    // only support three params
    if (info.argc != ARGC_THREE) {
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

    int32_t accountId = 0;
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[1]), accountId)) {
        HILOG_INFO("%{public}s called, the second parameter is invalid.", __func__);
        return engine.CreateUndefined();
    }

    // unwarp connection
    sptr<JSAbilityConnection> connection = new JSAbilityConnection(engine);
    connection->SetJsConnectionObject(info.argv[INDEX_TWO]);
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
    HILOG_INFO("%{public}s not find connection, make new one", __func__);
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, accountId, connection, connectId](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("OnConnectAbilityWithAccount begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                    return;
                }
                HILOG_INFO("context->ConnectAbilityWithAccount connection:%{public}d", (int32_t)connectId);
                if (!context->ConnectAbilityWithAccount(want, accountId, connection)) {
                    connection->CallJsFailed(ERROR_CODE_ONE);
                }
                task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
    return engine.CreateNumber(connectId);
}

NativeValue* JsAbilityContext::OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnDisconnectAbility is called");
    // only support one or two params
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    // unwrap want
    AAFwk::Want want;
    // unwrap connectId
    int64_t connectId = -1;
    sptr<JSAbilityConnection> connection = nullptr;
    napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), &connectId);
    HILOG_INFO("Disconnect ability begin, connection:%{public}d.", (int32_t)connectId);
    auto item = std::find_if(abilityConnects_.begin(),
        abilityConnects_.end(),
        [&connectId](const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
            return connectId == obj.first.id;
        });
    if (item != abilityConnects_.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        HILOG_INFO("%{public}s find conn ability exist", __func__);
    } else {
        HILOG_INFO("%{public}s not find conn exist.", __func__);
    }
    // begin disconnect
    AsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
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
            context->DisconnectAbility(want, connection);
            task.Resolve(engine, engine.CreateUndefined());
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnTerminateSelf is called");

    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto abilityContext = context_.lock();
    if (abilityContext != nullptr) {
        abilityContext->SetTerminating(true);
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

NativeValue* JsAbilityContext::OnRequestPermissionsFromUser(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnRequestPermissionsFromUser is called");

    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    std::vector<std::string> permissionList;
    if (!OHOS::AppExecFwk::UnwrapArrayStringFromJS(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]), permissionList)) {
        HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
        return engine.CreateUndefined();
    }

    if (permissionList.size() == 0) {
        HILOG_ERROR("%{public}s called, params do not meet specification.", __func__);
    }

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
    NativeValue* result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    PermissionRequestTask task =
        [&engine, asyncTask](const std::vector<std::string> &permissions, const std::vector<int> &grantResults) {
        HILOG_INFO("OnRequestPermissionsFromUser async callback is called");
        NativeValue* requestResult = JsAbilityContext::WrapPermissionRequestResult(engine, permissions, grantResults);
        if (requestResult == nullptr) {
            HILOG_WARN("wrap requestResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, 1, "failed to get granted result data!"));
        } else {
            asyncTask->Resolve(engine, requestResult);
        }
        HILOG_INFO("OnRequestPermissionsFromUser async callback is called end");
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, 1, "context is released!"));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        context->RequestPermissionsFromUser(permissionList, curRequestCode_, std::move(task));
    }
    HILOG_INFO("OnRequestPermissionsFromUser is called end");
    return result;
}

NativeValue* JsAbilityContext::OnRestoreWindowStage(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnRestoreWindowStage is called, argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("OnRestoreWindowStage need one parameters");
        return engine.CreateUndefined();
    }
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("OnRestoreWindowStage context is released");
        return engine.CreateUndefined();
    }
    auto errcode = context->RestoreWindowStage(engine, info.argv[0]);
    if (errcode != 0) {
        return CreateJsError(engine, errcode, "RestoreWindowStage failed.");
    }
    return engine.CreateUndefined();
}

NativeValue* JsAbilityContext::OnSetMissionLabel(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnSetMissionLabel is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionLabel, Not enough params");
        return engine.CreateUndefined();
    }

    std::string label;
    if (!ConvertFromJsValue(engine, info.argv[0], label)) {
        HILOG_ERROR("OnSetMissionLabel, parse label failed.");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, label](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, 1, "Context is released"));
                return;
            }

            auto errcode = context->SetMissionLabel(label);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, errcode, "SetMissionLabel failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::OnIsTerminating(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnIsTerminating is called");
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnIsTerminating context is nullptr");
        return engine.CreateUndefined();
    }
    return engine.CreateBoolean(context->IsTerminating());
}

#ifdef SUPPORT_GRAPHICS
NativeValue* JsAbilityContext::OnSetMissionIcon(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnSetMissionIcon is called, argc = %{public}d", static_cast<int>(info.argc));
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnSetMissionIcon, Not enough params");
        return engine.CreateUndefined();
    }

    auto icon = OHOS::Media::PixelMapNapi::GetPixelMap(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[0]));
    if (!icon) {
        HILOG_ERROR("OnSetMissionIcon, parse icon failed.");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, icon](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released when set mission icon");
                task.Reject(engine, CreateJsError(engine, -1, "Context is released"));
                return;
            }

            auto errcode = context->SetMissionIcon(icon);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, errcode, "SetMissionIcon failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[1];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}
#endif

bool JsAbilityContext::UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("%s argv == nullptr!", __func__);
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
}

NativeValue* JsAbilityContext::WrapWant(NativeEngine& engine, const AAFwk::Want& want)
{
    return reinterpret_cast<NativeValue*>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
}

bool JsAbilityContext::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want)
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

NativeValue* JsAbilityContext::WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want)
{
    NativeValue* jAbilityResult = engine.CreateObject();
    NativeObject* abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", JsAbilityContext::WrapWant(engine, want));
    return jAbilityResult;
}

NativeValue* JsAbilityContext::WrapPermissionRequestResult(NativeEngine& engine,
    const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{
    NativeValue* jsPermissionRequestResult = engine.CreateObject();
    NativeObject* permissionRequestResult = ConvertNativeValueTo<NativeObject>(jsPermissionRequestResult);
    permissionRequestResult->SetProperty("permissions", CreateNativeArray(engine, permissions));
    permissionRequestResult->SetProperty("authResults", CreateNativeArray(engine, grantResults));
    return jsPermissionRequestResult;
}

void JsAbilityContext::InheritWindowMode(AAFwk::Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
#ifdef SUPPORT_GRAPHICS
    // only split mode need inherit
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("context is nullptr.");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("%{public}s called end. window mode is %{public}d", __func__, windowMode);
#endif
}

void JsAbilityContext::ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_INFO("ConfigurationUpdated begin.");
    if (jsContext == nullptr || config == nullptr) {
        HILOG_INFO("jsContext is nullptr.");
        return;
    }

    NativeValue* value = jsContext->Get();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr.");
        return;
    }

    NativeValue* method = object->GetProperty("onUpdateConfiguration");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }
    HILOG_INFO("JSAbilityConnection::CallFunction onUpdateConfiguration, success");

    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    NativeValue* argv[] = {CreateJsConfiguration(*engine, *config)};
    engine->CallFunction(value, method, argv, ARGC_ONE);
}

NativeValue* CreateJsAbilityContext(NativeEngine& engine, std::shared_ptr<AbilityContext> context)
{
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        object->SetProperty("abilityInfo", CreateJsAbilityInfo(engine, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        object->SetProperty("config", CreateJsConfiguration(engine, *configuration));
    }

    BindNativeFunction(engine, *object, "startAbility", JsAbilityContext::StartAbility);
    BindNativeFunction(engine, *object, "startAbilityWithAccount", JsAbilityContext::StartAbilityWithAccount);
    BindNativeFunction(engine, *object, "startAbilityByCall", JsAbilityContext::StartAbilityByCall);
    BindNativeFunction(engine, *object, "startAbilityForResult", JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(engine, *object, "startAbilityForResultWithAccount",
        JsAbilityContext::StartAbilityForResultWithAccount);
    BindNativeFunction(engine, *object, "connectAbility", JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "connectAbilityWithAccount", JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "disconnectAbility", JsAbilityContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "terminateSelf", JsAbilityContext::TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "requestPermissionsFromUser", JsAbilityContext::RequestPermissionsFromUser);
    BindNativeFunction(engine, *object, "restoreWindowStage", JsAbilityContext::RestoreWindowStage);
    BindNativeFunction(engine, *object, "setMissionLabel", JsAbilityContext::SetMissionLabel);
    BindNativeFunction(engine, *object, "isTerminating", JsAbilityContext::IsTerminating);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(engine, *object, "setMissionIcon", JsAbilityContext::SetMissionIcon);
#endif
    return objValue;
}

JSAbilityConnection::JSAbilityConnection(NativeEngine& engine) : engine_(engine) {}

JSAbilityConnection::~JSAbilityConnection() = default;

void JSAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }

    wptr<JSAbilityConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode] {
        sptr<JSAbilityConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone");
}

void JSAbilityConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
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

    // wrap RemoteObject
    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        reinterpret_cast<napi_env>(&engine_), remoteObject);
    NativeValue* nativeRemoteObject = reinterpret_cast<NativeValue*>(napiRemoteObject);
    NativeValue* argv[] = { ConvertElement(element), nativeRemoteObject };
    engine_.CallFunction(value, methodOnConnect, argv, ARGC_TWO);
    HILOG_INFO("OnAbilityConnectDone end");
}

void JSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_INFO("handler_ nullptr");
        return;
    }

    wptr<JSAbilityConnection> connection = this;
    auto task = [connection, element, resultCode] {
        sptr<JSAbilityConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_INFO("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone");
}

void JSAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
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
        [bundleName, abilityName] (
            const std::map<ConnectionKey, sptr<JSAbilityConnection>>::value_type &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                    (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != abilityConnects_.end()) {
        // match bundlename && abilityname
        abilityConnects_.erase(item);
        HILOG_INFO("OnAbilityDisconnectDone erase abilityConnects_.size:%{public}zu", abilityConnects_.size());
    }

    NativeValue* argv[] = { ConvertElement(element) };
    HILOG_INFO("OnAbilityDisconnectDone CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
}

void JSAbilityConnection::CallJsFailed(int32_t errorCode)
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

    NativeValue* argv[] = {engine_.CreateNumber(errorCode)};
    HILOG_INFO("CallJsFailed CallFunction success");
    engine_.CallFunction(value, method, argv, ARGC_ONE);
    HILOG_INFO("CallJsFailed end");
}

NativeValue* JSAbilityConnection::ConvertElement(const AppExecFwk::ElementName &element)
{
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(reinterpret_cast<napi_env>(&engine_), element);
    return reinterpret_cast<NativeValue*>(napiElementName);
}

void JSAbilityConnection::SetJsConnectionObject(NativeValue* jsConnectionObject)
{
    jsConnectionObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsConnectionObject, 1));
}
}  // namespace AbilityRuntime
}  // namespace OHOS