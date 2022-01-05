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

#include "foundation/aafwk/standard/frameworks/kits/ability/ability_runtime/include/ability_context.h"

#include "ability_connect_callback_stub.h"

class NativeEngine;
class NativeObject;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsAbilityContext(NativeEngine& engine, std::shared_ptr<AbilityContext> context);

class JSAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit JSAbilityConnection(NativeEngine* engine);
    ~JSAbilityConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void CallJsFailed();
private:
    NativeValue* ConvertElement(const AppExecFwk::ElementName &element);
    std::unique_ptr<NativeEngine> engine_;
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
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
