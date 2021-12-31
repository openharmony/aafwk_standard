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

#ifndef ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H
#define ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H

#include <memory>

#include "service_extension_context.h"
#include "ability_connect_callback_stub.h"

class NativeEngine;
class NativeValue;
class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsServiceExtensionContext(NativeEngine& engine, std::shared_ptr<ServiceExtensionContext> context);

class JSServiceExtensionConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void SetNativeEngine(NativeEngine* engine);
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void CallJsFailed();
private:
    NativeEngine* engine_ = nullptr;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

struct ConnecttionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnecttionKey &key1, const ConnecttionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

static std::map<ConnecttionKey, sptr<JSServiceExtensionConnection>, key_compare> connects_;
static int64_t serialNumber_ = 0;
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H
