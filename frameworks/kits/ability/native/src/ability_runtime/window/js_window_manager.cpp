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
#include "js_window_manager.h"
#include "js_runtime_utils.h"
#include "js_window.h"
#include "js_window_utils.h"
#include "native_engine/native_reference.h"
#include "window_manager_hilog.h"
#include "window_option.h"

using namespace OHOS::Rosen;

namespace OHOS {
namespace AbilityRuntime {
using namespace AbilityRuntime;
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "JsWindowManager"};
}

class JsWindowManager {
public:
    JsWindowManager() = default;
    ~JsWindowManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        WLOGFI("JsWindowManager::Finalizer is called");
        std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
    }

    static NativeValue* CreateWindow(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
        return (me != nullptr) ? me->OnCreateWindow(*engine, *info) : nullptr;
    }

    static NativeValue* FindWindow(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
        return (me != nullptr) ? me->OnFindWindow(*engine, *info) : nullptr;
    }

private:

    NativeValue* OnCreateWindow(NativeEngine& engine, NativeCallbackInfo& info)
    {
        WLOGFI("JsOnCreateWindow is called");

        if (info.argc < ARGC_TWO) {
            WLOGFE("OnCreateWindow MUST set windowname");
            return engine.CreateUndefined();
        }

        std::string windowName;
        if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
            WLOGFE("Failed to convert parameter to windowName");
            return engine.CreateUndefined();
        }

        NativeNumber* nativeType = ConvertNativeValueTo<NativeNumber>(info.argv[1]);
        if (nativeType == nullptr) {
            WLOGFE("Failed to convert parameter to windowType");
            return engine.CreateUndefined();
        }
        WindowType winType = static_cast<WindowType>(static_cast<uint32_t>(*nativeType));
        sptr<WindowOption> windowOption = new WindowOption();
        windowOption->SetWindowType(winType);
        AsyncTask::CompleteCallback complete =
            [windowName, &windowOption](NativeEngine& engine, AsyncTask& task, int32_t status) {
                sptr<Window> window = Window::Create(windowName, windowOption);
                if (window != nullptr) {
                    task.Resolve(engine, CreateJsWindowObject(engine, window));
                    WLOGFI("JsWindowManager::OnCreateWindow success");
                } else {
                    task.Reject(engine, CreateJsError(engine,
                        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "JsWindowManager::OnCreateWindow failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnFindWindow(NativeEngine& engine, NativeCallbackInfo& info)
    {
        std::string windowName;
        if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
            WLOGFE("Failed to convert parameter to windowName");
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [windowName](NativeEngine& engine, AsyncTask& task, int32_t status) {
                sptr<Window> window = Window::Find(windowName);
                if (window != nullptr) {
                    task.Resolve(engine, CreateJsWindowObject(engine, window));
                    WLOGFI("JsWindowManager::OnFindWindow success");
                } else {
                    task.Reject(engine, CreateJsError(engine,
                        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "JsWindow::OnFindWindow failed."));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        if (lastParam == nullptr) {
            WLOGFI("JsWindowManager::OnFindWindow lastParam is nullptr");
        }
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};

NativeValue* JsWindowManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    WLOGFI("JsWindowManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        WLOGFE("JsWindowManagerInit engine or exportObj is nullptr");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        WLOGFE("JsWindowManagerInit object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowManager> jsWinManager = std::make_unique<JsWindowManager>();
    object->SetNativePointer(jsWinManager.release(), JsWindowManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "create", JsWindowManager::CreateWindow);
    BindNativeFunction(*engine, *object, "find", JsWindowManager::FindWindow);

    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS