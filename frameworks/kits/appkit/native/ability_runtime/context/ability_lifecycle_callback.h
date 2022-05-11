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

#ifndef ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
#define ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H

#include <memory>

class NativeEngine;
class NativeValue;
class NativeReference;
struct NativeCallbackInfo;

namespace OHOS {
namespace AbilityRuntime {
class AbilityLifecycleCallback {
public:
    explicit AbilityLifecycleCallback(NativeEngine &engine);
    ~AbilityLifecycleCallback();
    /**
     * Called back when the ability is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void OnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj);

    void SetJsCallback(NativeValue *jsCallback);

private:
    NativeValue *CallLifecycleCBFunction(const std::string &functionName,
        const std::weak_ptr<NativeReference> &abilityObj);
    NativeEngine &engine_;
    std::shared_ptr<NativeReference> jsCallback_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H