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

#include "ability_window.h"
#include "ability.h"
#include "ability_handler.h"
#include "app_log_wrapper.h"

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
    APP_LOGI("%{public}s begin.", __func__);
    handler_ = handler;
    ability_ = std::weak_ptr<IAbilityEvent>(ability);
    windowScene_ = std::make_shared<Rosen::WindowScene>();
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Sets the window option for the host ability to create window.
 *
 * @param windowOption Indicates window option.
 */
bool AbilityWindow::SetWindowType(Rosen::WindowType winType)
{
    APP_LOGI("%{public}s begin.", __func__);
    sptr<Rosen::IWindowLifeCycle> windowLifecycle = nullptr;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = nullptr;
    windowScene_->Init(Rosen::WindowScene::DEFAULT_DISPLAY_ID, abilityContext, windowLifecycle);
    auto window = windowScene_->GetMainWindow();
    auto ret = window->SetWindowType(winType);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        APP_LOGE("Set window type error, errcode = %{public}d", ret);
        return false;
    }

    isWindowAttached = true;
    APP_LOGI("%{public}s end.", __func__);
    return true;
}

/**
 * @brief Called when this ability is started.
 *
 */
void AbilityWindow::OnPostAbilityStart()
{
    APP_LOGI("%{public}s begin.", __func__);
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityStart window not attached.");
        return;
    }

    if (windowScene_) {
        APP_LOGI("%{public}s begin windowScene_->GoBackground.", __func__);
        windowScene_->GoBackground();
        APP_LOGI("%{public}s end windowScene_->GoBackground.", __func__);
    }

    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called when this ability is activated.
 *
 */
void AbilityWindow::OnPostAbilityActive()
{
    APP_LOGI("AbilityWindow::OnPostAbilityActive called.");
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityActive window not attached.");
        return;
    }

    if (windowScene_) {
        APP_LOGI("%{public}s begin windowScene_->RequestFocus.", __func__);
        windowScene_->RequestFocus();
        APP_LOGI("%{public}s end windowScene_->RequestFocus.", __func__);

        APP_LOGI("%{public}s begin windowScene_->GoForeground.", __func__);
        windowScene_->GoForeground();
        APP_LOGI("%{public}s end windowScene_->GoForeground.", __func__);
    }

    APP_LOGI("AbilityWindow::OnPostAbilityActive end.");
}

/**
 * @brief Called when this ability is inactivated.
 *
 */
void AbilityWindow::OnPostAbilityInactive()
{
    APP_LOGI("AbilityWindow::OnPostAbilityInactive called.");
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityInactive window not attached.");
        return;
    }

    if (windowScene_) {
        APP_LOGI("%{public}s begin windowScene_->GoBackground.", __func__);
        windowScene_->GoBackground();
        APP_LOGI("%{public}s end windowScene_->GoBackground.", __func__);
    }

    APP_LOGI("AbilityWindow::OnPostAbilityInactive end.");
}

/**
 * @brief Called when this ability is background.
 *
 */
void AbilityWindow::OnPostAbilityBackground()
{
    APP_LOGI("AbilityWindow::OnPostAbilityBackground called.");
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityBackground window not attached.");
        return;
    }

    if (windowScene_) {
        APP_LOGI("%{public}s begin windowScene_->GoBackground.", __func__);
        windowScene_->GoBackground();
        APP_LOGI("%{public}s end windowScene_->GoBackground.", __func__);
    }

    APP_LOGI("AbilityWindow::OnPostAbilityBackground end.");
}

/**
 * @brief Called when this ability is foreground.
 *
 */
void AbilityWindow::OnPostAbilityForeground()
{
    APP_LOGI("AbilityWindow::OnPostAbilityForeground called.");
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityForeground window not attached.");
        return;
    }

    if (windowScene_) {
        APP_LOGI("%{public}s begin windowScene_->GoForeground.", __func__);
        windowScene_->GoForeground();
        APP_LOGI("%{public}s end windowScene_->GoForeground.", __func__);
    }

    APP_LOGI("AbilityWindow::OnPostAbilityForeground end.");
}

/**
 * @brief Called when this ability is stopped.
 *
 */
void AbilityWindow::OnPostAbilityStop()
{
    APP_LOGI("AbilityWindow::OnPostAbilityStop called.");
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::OnPostAbilityStop window not attached.");
        return;
    }

    if (windowScene_) {
        windowScene_->~WindowScene();
        windowScene_ = nullptr;
        APP_LOGI("AbilityWindow::widow:: windowScene_ release end.");
    }

    isWindowAttached = false;
    APP_LOGI("AbilityWindow::OnPostAbilityStop end.");
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a Window object pointer.
 */
const sptr<Rosen::Window> AbilityWindow::GetWindow()
{
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::GetWindow window not attached.");
    }
    return windowScene_ ? windowScene_->GetMainWindow() : nullptr;
}
}  // namespace AppExecFwk
}  // namespace OHOS