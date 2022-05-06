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

#include "js_ability_manager.h"

#include <cstdint>

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "js_ability_manager_utils.h"
#include "event_runner.h"
#include "napi_common_configuration.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
class JsAbilityManager final {
public:
    JsAbilityManager() = default;
    ~JsAbilityManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityManager::Finalizer is called");
        std::unique_ptr<JsAbilityManager>(static_cast<JsAbilityManager*>(data));
    }

    static NativeValue* GetAbilityRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetAbilityRunningInfos(*engine, *info) : nullptr;
    }

    static NativeValue* GetExtensionRunningInfos(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetExtensionRunningInfos(*engine, *info) : nullptr;
    }

    static NativeValue* UpdateConfiguration(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnUpdateConfiguration(*engine, *info) : nullptr;
    }

    static NativeValue* GetTopAbility(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsAbilityManager* me = CheckParamsAndGetThis<JsAbilityManager>(engine, info);
        return (me != nullptr) ? me->OnGetTopAbility(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnGetAbilityRunningInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::AbilityRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
                if (errcode == 0) {
                    task.Resolve(engine, CreateJsAbilityRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetExtensionRunningInfos(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }
        int upperLimit = -1;
        if (!ConvertFromJsValue(engine, info.argv[0], upperLimit)) {
            HILOG_ERROR("Parse missionId failed");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [upperLimit](NativeEngine &engine, AsyncTask &task, int32_t status) {
                std::vector<AAFwk::ExtensionRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
                if (errcode == 0) {
                    task.Resolve(engine, CreateJsExtensionRunningInfoArray(engine, infos));
                } else {
                    task.Reject(engine, CreateJsError(engine, errcode, "Get mission infos failed."));
                }
            };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnUpdateConfiguration(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc == 0) {
            HILOG_ERROR("Not enough params");
            return engine.CreateUndefined();
        }

        AppExecFwk::Configuration changeConfig;
        if (!UnwrapConfiguration(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), changeConfig)) {
            HILOG_INFO("OnStartAbility start options is used.");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete = [changeConfig](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto errcode = AbilityManagerClient::GetInstance()->UpdateConfiguration(changeConfig);
            if (errcode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, errcode, "update config failed."));
            }
        };

        NativeValue* lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnGetTopAbility(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        AsyncTask::CompleteCallback complete =
            [](NativeEngine &engine, AsyncTask &task, int32_t status) {
                AppExecFwk::ElementName elementName = AbilityManagerClient::GetInstance()->GetTopAbility();
                task.Resolve(engine, CreateJsElementName(engine, elementName));
            };

        NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* JsAbilityManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsAbilityManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object null");
        return nullptr;
    }

    std::unique_ptr<JsAbilityManager> jsAbilityManager = std::make_unique<JsAbilityManager>();
    object->SetNativePointer(jsAbilityManager.release(), JsAbilityManager::Finalizer, nullptr);

    object->SetProperty("AbilityState", AbilityStateInit(engine));

    HILOG_INFO("JsAbilityManagerInit BindNativeFunction called");
    BindNativeFunction(*engine, *object, "getAbilityRunningInfos", JsAbilityManager::GetAbilityRunningInfos);
    BindNativeFunction(*engine, *object, "getExtensionRunningInfos", JsAbilityManager::GetExtensionRunningInfos);
    BindNativeFunction(*engine, *object, "updateConfiguration", JsAbilityManager::UpdateConfiguration);
    BindNativeFunction(*engine, *object, "getTopAbility", JsAbilityManager::GetTopAbility);
    HILOG_INFO("JsAbilityManagerInit end");
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
