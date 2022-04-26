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

namespace OHOS {
namespace AbilityRuntime {
class Ability;
class JsApplicationContextAbilityLifecycleCallback {
public:
    JsApplicationContextAbilityLifecycleCallback() = default;
    virtual ~JsApplicationContextAbilityLifecycleCallback() = default;
    /**
     * Called back when the ability is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityCreate(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityWindowStageCreate(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityWindowStageDestroy(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityDestroy(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityForeground(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityBackground(const std::shared_ptr<Ability> &ability);

    /**
     * Called back when the ability window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    void onAbilityContinue(const std::shared_ptr<Ability> &ability);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H