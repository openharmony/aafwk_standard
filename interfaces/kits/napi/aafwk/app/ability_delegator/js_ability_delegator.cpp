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

#include "js_ability_delegator.h"

#include "ability_delegator_registry.h"
#include "ability_runtime/js_ability.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "napi_remote_object.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorJs {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityMonitor>> monitorRecord;
std::map<std::shared_ptr<NativeReference>, sptr<IRemoteObject>> ablityRecord;

void JSAbilityDelegator::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JSAbilityDelegator::Finalizer is called");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

NativeValue *JSAbilityDelegator::AddAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnAddAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::RemoveAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnRemoveAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::WaitAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnWaitAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAppContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAppContext(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAbilityState(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAbilityState(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetCurrentTopAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetCurrentTopAbility(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityForeground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityForeground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityBackground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityBackground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::Print(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnPrint(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::ExecuteShellCommand(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnExecuteShellCommand(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::FinishTest(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnFinishTest(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::OnAddAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnAddAbilityMonitor is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseJSMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityMonitor AsyncTask is called");
        AbilityDelegatorRegistry::GetAbilityDelegator()->AddAbilityMonitor(monitor);
        task.Resolve(engine, engine.CreateUndefined());
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnRemoveAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnRemoveAbilityMonitor is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseJSMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) mutable {
        HILOG_INFO("OnRemoveAbilityMonitor AsyncTask is called");
        AbilityDelegatorRegistry::GetAbilityDelegator()->RemoveAbilityMonitor(monitor);
        task.Resolve(engine, engine.CreateUndefined());
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));

    for (auto iter = monitorRecord.begin(); iter != monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if ((info.argv[INDEX_ZERO])->StrictEquals(jsMonitor->Get())) {
            monitorRecord.erase(iter);
            break;
        }
    }

    return result;
}

NativeValue *JSAbilityDelegator::OnWaitAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnWaitAbilityMonitor is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseJSMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        return engine.CreateUndefined();
    }

    bool hasTimeoutPara = false;
    int64_t timeout = 0;
    if (info.argc >= ARGC_TWO) {
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], timeout)) {
            HILOG_ERROR("Parse timeout failed");
        } else {
            hasTimeoutPara = true;
        }
    }

    int argcnum = info.argc;
    AsyncTask::CompleteCallback complete =
        [argcnum, monitor, timeout, hasTimeoutPara, this](NativeEngine &engine, AsyncTask &task, int32_t status) {
            HILOG_INFO("OnWaitAbilityMonitor AsyncTask is called");
            sptr<IRemoteObject> remoteObject = nullptr;
            if (((argcnum == ARGC_TWO) && !hasTimeoutPara) || (argcnum == ARGC_ONE)) {
                remoteObject = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->WaitAbilityMonitor(monitor);
            } else if (((argcnum == ARGC_TWO) && hasTimeoutPara) || (argcnum == ARGC_THREE)) {
                remoteObject =
                    AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->WaitAbilityMonitor(monitor, timeout);
            }
            task.Resolve(engine, CreateJsAbilityObject(engine, remoteObject));
        };

    NativeValue *lastParam = nullptr;
    if ((argcnum == ARGC_ONE) || ((argcnum == ARGC_TWO) && hasTimeoutPara)) {
        lastParam = nullptr;
    } else if ((argcnum == ARGC_TWO) && !hasTimeoutPara) {
        lastParam = info.argv[INDEX_ONE];
    } else if (argcnum == ARGC_THREE) {
        lastParam = info.argv[INDEX_TWO];
    }
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnPrint(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnPrint is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::string msg;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse para failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [msg](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnPrint AsyncTask is called");
        AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->Print(msg);
        task.Resolve(engine, engine.CreateUndefined());
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnExecuteShellCommand(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnExecuteShellCommand is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::string cmd;
    bool hasTimeoutPara = false;
    int64_t timeoutSecs = 0;
    if (!ParseJSExecuteShellCommandPara(engine, info, cmd, timeoutSecs, hasTimeoutPara)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [cmd, timeoutSecs](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnExecuteShellCommand AsyncTask is called");
        std::unique_ptr<ShellCmdResult> shellResult =
            AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->ExecuteShellCommand(cmd, timeoutSecs);
        task.Resolve(engine, CreateJsShellCmdResult(engine, shellResult));
    };

    int argcnum = info.argc;
    NativeValue *lastParam = nullptr;
    if ((argcnum == ARGC_ONE) || ((argcnum == ARGC_TWO) && hasTimeoutPara)) {
        lastParam = nullptr;
    } else if ((argcnum == ARGC_TWO) && !hasTimeoutPara) {
        lastParam = info.argv[INDEX_ONE];
    } else if (argcnum == ARGC_THREE) {
        lastParam = info.argv[INDEX_TWO];
    }
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnGetAppContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnGetAppContext is called, argc = %{public}d", static_cast<int>(info.argc));

    AsyncTask::CompleteCallback complete = [](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetAppContext AsyncTask is called");
        std::shared_ptr<AbilityRuntime::Context> context =
            AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->GetAppContext();
        task.Resolve(engine, CreateJsBaseContext(engine, context, false));
    };

    NativeValue *lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnGetAbilityState(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnGetAbilityState is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseJSAbilityPara(engine, info.argv[INDEX_ZERO], remoteObject)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [remoteObject](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetAbilityState AsyncTask is called");
        AbilityDelegator::AbilityState lifeState =
            AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->GetAbilityState(remoteObject);
        task.Resolve(engine, CreateJsAbilityState(engine, lifeState));
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnGetCurrentTopAbility(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnGetCurrentTopAbility is called, argc = %{public}d", static_cast<int>(info.argc));

    AsyncTask::CompleteCallback complete = [this](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetCurrentTopAbility AsyncTask is called");
        sptr<IRemoteObject> remoteObject =
            AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->GetCurrentTopAbility();
        task.Resolve(engine, CreateJsAbilityObject(engine, remoteObject));
    };

    NativeValue *lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityForeground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnDoAbilityForeground is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseJSAbilityPara(engine, info.argv[INDEX_ZERO], remoteObject)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [remoteObject](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityForeground AsyncTask is called");
        bool ret = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->DoAbilityForeground(remoteObject);
        task.Resolve(engine, CreateJsBool(engine, ret));
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityBackground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnDoAbilityBackground is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseJSAbilityPara(engine, info.argv[INDEX_ZERO], remoteObject)) {
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [remoteObject](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityBackground AsyncTask is called");
        bool ret = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->DoAbilityBackground(remoteObject);
        task.Resolve(engine, CreateJsBool(engine, ret));
    };

    NativeValue *lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnFinishTest(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnFinishTest is called, argc = %{public}d", static_cast<int>(info.argc));

    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::string msg;
    int64_t code = 0;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse para failed");
        return engine.CreateUndefined();
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], code)) {
        HILOG_ERROR("Parse para argv[1] failed");
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete = [msg, code](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnFinishTest AsyncTask is called");
        AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()->FinishUserTest(msg, code);
        task.Resolve(engine, engine.CreateUndefined());
    };
    NativeValue *lastParam = (info.argc == ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule(engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::ParseJSMonitorPara(
    NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("ParseJSMonitorPara is called, monitorRecord size = %{public}zu", monitorRecord.size());

    for (auto iter = monitorRecord.begin(); iter != monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if (value->StrictEquals(jsMonitor->Get())) {
            HILOG_ERROR("monitor existed");
            monitor = iter->second;
            return monitor ? engine.CreateNull() : nullptr;
        }
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto abilityNameValue = object->GetProperty("abilityName");
    if (abilityNameValue == nullptr) {
        return nullptr;
    }
    std::string abilityName;
    if (!ConvertFromJsValue(engine, abilityNameValue, abilityName)) {
        return nullptr;
    }
    std::shared_ptr<JSAbilityMonitor> abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
    abilityMonitor->SetJsAbilityMonitorEnv(&engine);
    abilityMonitor->SetJsAbilityMonitor(value);

    monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    if (!monitor) {
        HILOG_INFO("Failed to create monitor");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(value, 1));
    monitorRecord.emplace(reference, monitor);

    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseJSAbilityPara(
    NativeEngine &engine, NativeValue *value, sptr<OHOS::IRemoteObject> &remoteObject)
{
    HILOG_INFO("ParseJSAbilityPara is called");

    for (auto iter = ablityRecord.begin(); iter != ablityRecord.end(); ++iter) {
        if (value->StrictEquals(iter->first->Get())) {
            remoteObject = iter->second;
            return remoteObject ? engine.CreateNull() : nullptr;
        }
    }

    HILOG_ERROR("Ablity doesn't exist");
    remoteObject = nullptr;

    return nullptr;
}

NativeValue *JSAbilityDelegator::ParseJSExecuteShellCommandPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::string &cmd, int64_t &timeoutSecs, bool &hasTimeoutPara)
{
    HILOG_INFO("ParseJSExecuteShellCommandPara is called");

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], cmd)) {
        HILOG_ERROR("Parse para argv[0] failed");
        return nullptr;
    }

    if ((info.argc >= ARGC_TWO) && ((info.argv[INDEX_ONE])->TypeOf() == NativeValueType::NATIVE_NUMBER)) {
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], timeoutSecs)) {
            HILOG_ERROR("Parse para argv[1] failed");
            return nullptr;
        }
        hasTimeoutPara = true;
    } else {
        hasTimeoutPara = false;
    }

    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::CreateJsAbilityObject(NativeEngine &engine, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("CreateJsAbilityObject is called");

    if (!remoteObject) {
        return engine.CreateUndefined();
    }

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(objValue, 1));
    ablityRecord[reference] = remoteObject;

    return objValue;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
