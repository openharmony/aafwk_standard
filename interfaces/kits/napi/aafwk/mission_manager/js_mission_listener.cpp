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

#include "js_mission_listener.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
void JsMissionListener::OnMissionCreated(int32_t missionId)
{
    CallJsMethod("onMissionCreated", missionId);
}

void JsMissionListener::OnMissionDestroyed(int32_t missionId)
{
    CallJsMethod("onMissionDestroyed", missionId);
}

void JsMissionListener::OnMissionSnapshotChanged(int32_t missionId)
{
    CallJsMethod("onMissionSnapshotChanged", missionId);
}

void JsMissionListener::OnMissionMovedToFront(int32_t missionId)
{
    CallJsMethod("onMissionMovedToFront", missionId);
}

void JsMissionListener::AddJsListenerObject(int32_t listenerId, NativeValue* jsListenerObject)
{
    jsListenerObjectMap_.emplace(
        listenerId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsListenerObject, 1)));
}

void JsMissionListener::RemoveJsListenerObject(int32_t listenerId)
{
    jsListenerObjectMap_.erase(listenerId);
}

bool JsMissionListener::IsEmpty()
{
    return jsListenerObjectMap_.empty();
}

void JsMissionListener::CallJsMethod(const std::string &methodName, int32_t missionId)
{
    HILOG_INFO("methodName = %{public}s", methodName.c_str());
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }

    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsMissionListener = this, methodName, missionId](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (jsMissionListener) {
                jsMissionListener->CallJsMethodInner(methodName, missionId);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule(
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMethodInner(const std::string &methodName, int32_t missionId)
{
    for (auto &item : jsListenerObjectMap_) {
        NativeValue* value = (item.second)->Get();
        NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get object");
            continue;
        }
        NativeValue* method = obj->GetProperty(methodName.c_str());
        if (method == nullptr || method->TypeOf() == NATIVE_UNDEFINED) {
            HILOG_ERROR("Failed to get %{public}s from object", methodName.c_str());
            continue;
        }
        NativeValue* argv[] = { CreateJsValue(*engine_, missionId) };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS