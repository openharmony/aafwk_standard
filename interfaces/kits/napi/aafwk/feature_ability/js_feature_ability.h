/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_H
#define OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_H

#include "ability.h"
#include "distribute_req_param.h"
#include "native_engine/native_engine.h"
#include "want.h"
#include "uri.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

class JsFeatureAbility final {
public:
    JsFeatureAbility() = default;
    ~JsFeatureAbility() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateJsFeatureAbility(NativeEngine &engine);
    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info);
private:
    Ability* GetAbility(napi_env env);
    Want GetWant(DistributeReqParam &requestParam);
    bool CheckThenGetDeepLinkUri(DistributeReqParam &requestParam, Uri &uri);
    bool UnWrapRequestParams(napi_env env, napi_value param, DistributeReqParam &requestParam);
    static NativeValue* CreateJsResult(NativeEngine &engine, int32_t errCode, const std::string &message);
    void GetExtraParams(DistributeReqParam &requestParam, Want &want);
    NativeValue* OnStartAbility(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnStartAbilityForResult(NativeEngine &engine, NativeCallbackInfo &info);

    int requestCode_ = 0;
};

NativeValue* JsFeatureAbilityInit(NativeEngine* engine, NativeValue* exportObj);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_FEATURE_ABILITY_H