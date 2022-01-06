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

#ifndef OHOS_ABILITY_RUNTIME_JS_MISSION_LISTENER_H
#define OHOS_ABILITY_RUNTIME_JS_MISSION_LISTENER_H

#include <map>
#include <mutex>

#include "event_handler.h"
#include "mission_listener_stub.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class JsMissionListener : public AAFwk::MissionListenerStub {
public:
    JsMissionListener(NativeEngine* engine, std::shared_ptr<OHOS::AppExecFwk::EventHandler> &handler)
        : engine_(engine), mainHandler_(handler) {}
    virtual ~JsMissionListener() = default;

    void OnMissionCreated(int32_t missionId) override;
    void OnMissionDestroyed(int32_t missionId) override;
    void OnMissionSnapshotChanged(int32_t missionId) override;
    void OnMissionMovedToFront(int32_t missionId) override;
    void AddJsListenerObject(int32_t listenerId, NativeValue* jsListenerObject);
    void RemoveJsListenerObject(int32_t listenerId);
    bool IsEmpty();
    void HandleMissionCallback(const char* methodName, int32_t missionId);
private:
    void PostMissionCallback(const std::string &methodName, int32_t missionId);
    void CallJsMethod(const char* methodName, NativeValue* const* argv = nullptr, size_t argc = 0);

    NativeEngine* engine_ = nullptr;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsListenerObjectMap_;
    std::mutex listenerMutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_JS_MISSION_LISTENER_H */