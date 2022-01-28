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

#include "js_ability_delegator_registry.h"

#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator.h"
#include "js_ability_delegator_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
namespace {
constexpr int32_t ARGC_ONE = 1;

class JsAbilityDelegatorRegistry {
public:
    JsAbilityDelegatorRegistry() = default;
    ~JsAbilityDelegatorRegistry() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_INFO("JsAbilityDelegatorRegistry::Finalizer is called");
        std::unique_ptr<JsAbilityDelegatorRegistry>(static_cast<JsAbilityDelegatorRegistry *>(data));
    }

    static NativeValue *GetAbilityDelegator(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsAbilityDelegatorRegistry *me = CheckParamsAndGetThis<JsAbilityDelegatorRegistry>(engine, info);
        return (me != nullptr) ? me->OnGetAbilityDelegator(*engine, *info) : nullptr;
    }

    static NativeValue *GetArguments(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsAbilityDelegatorRegistry *me = CheckParamsAndGetThis<JsAbilityDelegatorRegistry>(engine, info);
        return (me != nullptr) ? me->OnGetArguments(*engine, *info) : nullptr;
    }

private:
    NativeValue *OnGetAbilityDelegator(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc > ARGC_ONE) {
            HILOG_ERROR("Params not match");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete = [](NativeEngine &engine, AsyncTask &task, int32_t status) {
            task.Resolve(engine, CreateJsAbilityDelegator(engine));
        };

        NativeValue *lastParam = (info.argc == ARGC_ONE) ? info.argv[0] : nullptr;
        NativeValue *result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue *OnGetArguments(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc > ARGC_ONE) {
            HILOG_ERROR("Params not match");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete = [](NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> abilityDelegatorArgs =
                AppExecFwk::AbilityDelegatorRegistry::GetArguments();
            task.Resolve(engine, CreateJsAbilityDelegatorArguments(engine, abilityDelegatorArgs));
        };

        NativeValue *lastParam = (info.argc == ARGC_ONE) ? info.argv[0] : nullptr;
        NativeValue *result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue *JsAbilityDelegatorRegistryInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_INFO("JsAbilityDelegatorManagerInit is called");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::unique_ptr<JsAbilityDelegatorRegistry> jsDelegatorManager =
        std::make_unique<JsAbilityDelegatorRegistry>();
    object->SetNativePointer(jsDelegatorManager.release(), JsAbilityDelegatorRegistry::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "getAbilityDelegator", JsAbilityDelegatorRegistry::GetAbilityDelegator);
    BindNativeFunction(*engine, *object, "getArguments", JsAbilityDelegatorRegistry::GetArguments);

    return engine->CreateUndefined();
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS