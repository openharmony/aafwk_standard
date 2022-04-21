/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_window.h"
#include "ability.h"
#include "ability_handler.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AbilityWindow::AbilityWindow()
{}

AbilityWindow::~AbilityWindow()
{}

/**
 * @brief Init the AbilityWindow object.
 *
 * @param handler The EventHandler of the Ability the AbilityWindow belong.
 */
void AbilityWindow::Init(std::shared_ptr<AbilityHandler>& handler, std::shared_ptr<Ability> ability)
{
    HILOG_INFO("%{public}s come.", __func__);
    handler_ = handler;
    ability_ = std::weak_ptr<IAbilityEvent>(ability);
    windowScene_ = std::make_shared<Rosen::WindowScene>();
}

/**
 * @brief Sets the window config for the host ability to create window.
 *
 * @param winType Indicates window config.
 * @param abilityContext Indicates runtime ability context.
 * @param listener Indicates window lifecycle listener.
 * @return true if init window success.
 */
bool AbilityWindow::InitWindow(Rosen::WindowType winType,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    sptr<Rosen::IWindowLifeCycle> &listener, int32_t displayId, sptr<Rosen::WindowOption> option)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = windowScene_->Init(displayId, abilityContext, listener, option);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("%{public}s error. failed to init window scene!", __func__);
        return false;
    }

    auto window = windowScene_->GetMainWindow();
    if (!window) {
        HILOG_INFO("%{public}s window is nullptr.", __func__);
        return false;
    }

    ret = window->SetWindowType(winType);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("Set window type error, errcode = %{public}d", ret);
        return false;
    }
    winType_ = winType;

    isWindowAttached = true;
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

/**
 * @brief Called when this ability is started.
 *
 */
void AbilityWindow::OnPostAbilityStart()
{
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityStart window not attached.");
        return;
    }

    if (windowScene_) {
        HILOG_INFO("%{public}s begin windowScene_->GoBackground.", __func__);
        windowScene_->GoBackground();
        HILOG_INFO("%{public}s end windowScene_->GoBackground.", __func__);
    }
}

/**
 * @brief Called when this ability is activated.
 *
 */
void AbilityWindow::OnPostAbilityActive()
{
    HILOG_INFO("AbilityWindow::OnPostAbilityActive called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityActive window not attached.");
        return;
    }
    HILOG_INFO("AbilityWindow::OnPostAbilityActive end.");
}

/**
 * @brief Called when this ability is inactivated.
 *
 */
void AbilityWindow::OnPostAbilityInactive()
{
    HILOG_INFO("AbilityWindow::OnPostAbilityInactive called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityInactive window not attached.");
        return;
    }
    HILOG_INFO("AbilityWindow::OnPostAbilityInactive end.");
}

/**
 * @brief Called when this ability is background.
 *
 */
void AbilityWindow::OnPostAbilityBackground(uint32_t sceneFlag)
{
    HILOG_INFO("AbilityWindow::OnPostAbilityBackground called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityBackground window not attached.");
        return;
    }

    if (windowScene_) {
        HILOG_INFO("%{public}s begin windowScene_->GoBackground, sceneFlag:%{public}d.", __func__, sceneFlag);
        windowScene_->GoBackground(sceneFlag);
        HILOG_INFO("%{public}s end windowScene_->GoBackground.", __func__);
    }

    HILOG_INFO("AbilityWindow::OnPostAbilityBackground end.");
}

/**
 * @brief Called when this ability is foreground.
 *
 */
void AbilityWindow::OnPostAbilityForeground(uint32_t sceneFlag)
{
    HILOG_INFO("AbilityWindow::OnPostAbilityForeground called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityForeground window not attached.");
        return;
    }

    if (windowScene_) {
        HILOG_INFO("%{public}s begin windowScene_->GoForeground, sceneFlag:%{public}d.", __func__, sceneFlag);
        windowScene_->GoForeground(sceneFlag);
        HILOG_INFO("%{public}s end windowScene_->GoForeground.", __func__);
    }

    HILOG_INFO("AbilityWindow::OnPostAbilityForeground end.");
}

/**
 * @brief Called when this ability is stopped.
 *
 */
void AbilityWindow::OnPostAbilityStop()
{
    HILOG_INFO("AbilityWindow::OnPostAbilityStop called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityStop window not attached.");
        return;
    }

    if (windowScene_) {
        windowScene_ = nullptr;
        HILOG_INFO("AbilityWindow::window windowScene_ release end.");
    }

    isWindowAttached = false;
    HILOG_INFO("AbilityWindow::OnPostAbilityStop end.");
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a Window object pointer.
 */
const sptr<Rosen::Window> AbilityWindow::GetWindow()
{
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::GetWindow window not attached.");
    }
    return windowScene_ ? windowScene_->GetMainWindow() : nullptr;
}
}  // namespace AppExecFwk
}  // namespace OHOS
