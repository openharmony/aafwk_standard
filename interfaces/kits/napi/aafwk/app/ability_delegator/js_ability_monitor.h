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

#ifndef OHOS_ABILITY_DELEGATOR_JS_ABILITY_MONITOR_H
#define OHOS_ABILITY_DELEGATOR_JS_ABILITY_MONITOR_H

#include <memory>
#include "iability_monitor.h"
#include "hilog_wrapper.h"
#include "native_engine/native_reference.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AppExecFwk;
class JSAbilityMonitor {
public:
    /**
     * A constructor used to create a JSAbilityMonitor instance with the input parameter passed.
     *
     * @param abilityName Indicates the specified ability name for monitoring the lifecycle state changes
     * of the ability.
     */
    explicit JSAbilityMonitor(const std::string &abilityName);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~JSAbilityMonitor() = default;

    /**
     * Called when ability is started.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onAbilityCreate();

    /**
     * Called when ability is in foreground.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onAbilityForeground();

    /**
     * Called when ability is in background.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onAbilityBackground();

    /**
     * Called when ability is stopped.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onAbilityDestroy();

    /**
     * Called when window stage is created.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onWindowStageCreate();

    /**
     * Called when window stage is restored.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onWindowStageRestore();

    /**
     * Called when window stage is destroyed.
     * Then call the corresponding method on the js side through the saved js object.
     */
    void onWindowStageDestroy();

    /**
     * Sets the js object.
     *
     * @param jsAbilityMonitor Indicates the js object.
     */
    void SetJsAbilityMonitor(NativeValue *jsAbilityMonitor);

    /**
     * Sets the native engine.
     *
     * @param engine Indicates the native engine.
     */
    void SetJsAbilityMonitorEnv(NativeEngine *engine)
    {
        engine_ = engine;
    }

    /**
     * Obtains the saved js object.
     *
     * @return the saved js object.
     */
    std::unique_ptr<NativeReference> &GetJsAbilityMonitor()
    {
        return jsAbilityMonitor_;
    }

private:
    std::string abilityName_;
    NativeEngine* engine_ = nullptr;
    std::unique_ptr<NativeReference> jsAbilityMonitor_ = nullptr;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_DELEGATOR_JS_ABILITY_MONITOR_H