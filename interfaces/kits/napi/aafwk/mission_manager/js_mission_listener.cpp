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
    PostMissionCallback("onMissionCreated", missionId);
}

void JsMissionListener::OnMissionDestroyed(int32_t missionId)
{
    PostMissionCallback("onMissionDestroyed", missionId);
}

void JsMissionListener::OnMissionSnapshotChanged(int32_t missionId)
{
    PostMissionCallback("onMissionSnapshotChanged", missionId);
}

void JsMissionListener::OnMissionMovedToFront(int32_t missionId)
{
    PostMissionCallback("onMissionMovedToFront", missionId);
}

void JsMissionListener::AddJsListenerObject(int32_t listenerId, NativeValue* jsListenerObject)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    jsListenerObjectMap_.emplace(
        listenerId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsListenerObject, 1)));
}

void JsMissionListener::RemoveJsListenerObject(int32_t listenerId)
{
    std::lock_guard<std::mutex> lock(listenerMutex_);
    jsListenerObjectMap_.erase(listenerId);
}

bool JsMissionListener::IsEmpty()
{
    return jsListenerObjectMap_.empty();
}

void JsMissionListener::HandleMissionCallback(const char* methodName, int32_t missionId)
{
    NativeValue* argv[] = { CreateJsValue(*engine_, missionId) };
    CallJsMethod(methodName, argv, 1);
}

void JsMissionListener::PostMissionCallback(const std::string &methodName, int32_t missionId)
{
    if (!mainHandler_) {
        HILOG_ERROR("mainHandler_ is nullptr");
        return;
    }

    wptr<JsMissionListener> wpListener = this;
    auto task = [wpListener, methodName, missionId]() {
        auto listener = wpListener.promote();
        if (listener) {
            listener->HandleMissionCallback(methodName.c_str(), missionId);
        }
    };
    mainHandler_->PostTask(task);
}

void JsMissionListener::CallJsMethod(const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("methodName = %{public}s", methodName);
    if (engine_ == nullptr) {
        HILOG_ERROR("engine_ nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(listenerMutex_);
    for (auto &item : jsListenerObjectMap_) {
        NativeValue* value = (item.second)->Get();
        NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get object");
            continue;
        }
        NativeValue* method = obj->GetProperty(methodName);
        if (method == nullptr) {
            HILOG_ERROR("Failed to get onMissionCreated from object");
            continue;
        }
        engine_->CallFunction(value, method, argv, argc);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS