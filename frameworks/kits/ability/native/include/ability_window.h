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

#ifndef OHOS_ABILITY_WINDOW_H
#define OHOS_ABILITY_WINDOW_H

#include <map>

#include "nocopyable.h"
#include "window.h"
#include "window_option.h"
#include "window_scene.h"
#include "../../ability_runtime/include/ability_context.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityEvent;
class Ability;
class AbilityHandler;
class AbilityWindow : public NoCopyable {
public:
    AbilityWindow();
    virtual ~AbilityWindow();

    /**
     * @brief Init the AbilityWindow object.
     *
     * @param handler The EventHandler of the Ability the AbilityWindow belong.
     */
    void Init(std::shared_ptr<AbilityHandler> &handler, std::shared_ptr<Ability> ability);

    /**
     * @brief Sets the window config for the host ability to create window.
     *
     * @param winType Indicates window config.
     * @param abilityContext Indicates runtime ability context.
     * @param listener Indicates window lifecycle listener.
     * @return true if init window success.
     */
    bool InitWindow(Rosen::WindowType winType,
        std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
        sptr<Rosen::IWindowLifeCycle> &listener, int32_t displayId, sptr<Rosen::WindowOption> option);

    /**
     * @brief Called when this ability is started.
     *
     */
    void OnPostAbilityStart();

    /**
     * @brief Called when this ability is activated.
     *
     */
    void OnPostAbilityActive();

    /**
     * @brief Called when this ability is inactivated.
     *
     */
    void OnPostAbilityInactive();

    /**
     * @brief Called when this ability is background.
     *
     */
    void OnPostAbilityBackground();

    /**
     * @brief Called when this ability is foreground.
     *
     */
    void OnPostAbilityForeground();

    /**
     * @brief Called when this ability is stopped.
     *
     */
    void OnPostAbilityStop();

    /**
     * @brief Get the window belong to the ability.
     *
     * @return Returns a Window object pointer.
     */
    const sptr<Rosen::Window> GetWindow();

private:
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    std::weak_ptr<IAbilityEvent> ability_;
    std::shared_ptr<Rosen::WindowScene> windowScene_;
    bool isWindowAttached = false;
    Rosen::WindowType winType_ = Rosen::WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_WINDOW_H