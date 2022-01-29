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

#ifndef ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
#define ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>

#include "ability_connect_callback.h"
#include "foundation/aafwk/standard/frameworks/kits/ability/ability_runtime/include/ability_context.h"
#include "js_runtime.h"
#include "event_handler.h"

class NativeObject;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityContext final {
public:
    JsAbilityContext(const std::shared_ptr<AbilityContext>& context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityByCall(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityForResultWithAccount(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* ConnectAbilityWithAccount(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* RequestPermissionsFromUser(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* RestoreWindowStage(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetMissionLabel(NativeEngine* engine, NativeCallbackInfo* info);

    static void ConfigurationUpdated(NativeEngine* engine, std::shared_ptr<NativeReference> &jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<AbilityContext> GetAbilityContext()
    {
        return context_.lock();
    }

private:
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartAbilityByCall(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartAbilityForResultWithAccount(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnConnectAbilityWithAccount(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnRequestPermissionsFromUser(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnRestoreWindowStage(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetMissionLabel(NativeEngine& engine, NativeCallbackInfo& info);

    static bool UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want);
    static NativeValue* WrapWant(NativeEngine& engine, const AAFwk::Want& want);
    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want);
    static NativeValue* WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want);
    static NativeValue* WrapPermissionRequestResult(NativeEngine& engine,
        const std::vector<std::string> &permissions, const std::vector<int> &grantResults);

    std::weak_ptr<AbilityContext> context_;
    int curRequestCode_ = 0;
};

NativeValue* CreateJsAbilityContext(NativeEngine& engine, std::shared_ptr<AbilityContext> context);

class JSAbilityConnection : public AbilityConnectCallback {
public:
    explicit JSAbilityConnection(NativeEngine& engine);
    ~JSAbilityConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void CallJsFailed(int32_t errorCode);
private:
    NativeValue* ConvertElement(const AppExecFwk::ElementName &element);
    NativeEngine& engine_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct KeyCompare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

static std::map<ConnectionKey, sptr<JSAbilityConnection>, KeyCompare> abilityConnects_;
static int64_t g_serialNumber = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H