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

#ifndef OHOS_ABILITY_DELEGATOR_JS_ABILITY_DELEGATOR_H
#define OHOS_ABILITY_DELEGATOR_JS_ABILITY_DELEGATOR_H

#include <string>
#include "ability_monitor.h"
#include "js_ability_monitor.h"

namespace OHOS {
namespace AbilityDelegatorJs {
class JSAbilityDelegator {
public:
    JSAbilityDelegator() = default;
    ~JSAbilityDelegator() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint);

    static NativeValue *AddAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *RemoveAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *WaitAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *Print(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *ExecuteShellCommand(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetAppContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetAbilityState(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *GetCurrentTopAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *DoAbilityForeground(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *DoAbilityBackground(NativeEngine *engine, NativeCallbackInfo *info);

private:
    NativeValue *OnAddAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnRemoveAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnWaitAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnPrint(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnExecuteShellCommand(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetAppContext(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetAbilityState(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetCurrentTopAbility(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnDoAbilityForeground(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnDoAbilityBackground(NativeEngine &engine, NativeCallbackInfo &info);

private:
    NativeValue *CreateJsAbilityObject(NativeEngine &engine, const sptr<IRemoteObject> &remoteObject);
    NativeValue *ParseJSMonitorPara(NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityMonitor> &monitor);
    NativeValue *ParseJSAbilityPara(NativeEngine &engine, NativeValue *value, sptr<OHOS::IRemoteObject> &remoteObject);
    NativeValue *ParseJSExecuteShellCommandPara(
        NativeEngine& engine, NativeCallbackInfo &info, std::string &cmd, int64_t &timeoutSecs, bool &hasTimeoutPara);
    void SetJsAbilityMonitor(std::string jstring);
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_ABILITY_DELEGATOR_H