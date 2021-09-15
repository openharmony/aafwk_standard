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
void AbilityWindow::Init(std::shared_ptr<AbilityHandler> &handler, std::shared_ptr<Ability> ability)
{
    APP_LOGI("%{public}s begin.", __func__);
    handler_ = handler;
    ability_ = std::weak_ptr<IAbilityEvent>(ability);

    auto wmi = WindowManager::GetInstance();
    if (wmi == nullptr) {
        APP_LOGE("AbilityWindow::Init WindowManager::GetInstance() is nullptr.");
        return;
    }

    auto wret = wmi->Init();
    if (wret != WM_OK) {
        APP_LOGE("AbilityWindow::Init WindowManager::Init() return %d", wret);
        return;
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Sets the window config for the host ability to create window.
 *
 * @param config Indicates window config.
 */
bool AbilityWindow::SetWindowConfig(const sptr<WindowOption> &config)
{
    APP_LOGI("%{public}s begin.", __func__);

    APP_LOGI("config width = %{public}d, height = %{public}d.", config->GetWidth(), config->GetHeight());
    APP_LOGI("config pos_x = %{public}d, pos_y = %{public}d, type = %{public}d.",
        config->GetX(), config->GetY(), config->GetWindowType());

    auto wmi = WindowManager::GetInstance();
    if (wmi == nullptr) {
        APP_LOGE("AbilityWindow::Init WindowManager::GetInstance() is nullptr.");
        return false;
    }

    APP_LOGI("%{public}s begin wms->CreateWindow.", __func__);
    auto retvalCreate = wmi->CreateWindow(windowNew_, config);
    APP_LOGI("%{public}s end wms->CreateWindow.", __func__);
    if (retvalCreate != WM_OK) {
        APP_LOGE("AbilityWindow::SetWindowConfig WindowManager::CreateWindow() return %d", retvalCreate);
        return false;
    }

    if (windowNew_ == nullptr) {
        APP_LOGE("AbilityWindow::SetWindowConfig the window is nullptr.");
        return false;
    }

    auto callback = [abilityWindow = this](KeyEvent event) -> bool { return abilityWindow->OnKeyEvent(event); };
    APP_LOGI("%{public}s begin windowNew_->RegistOnKeyCb.", __func__);
    auto retvalKeyboardKey = windowNew_->OnKey(callback);
    APP_LOGI("%{public}s end windowNew_->RegistOnKeyCb.", __func__);
    if (retvalKeyboardKey != WM_OK) {
        APP_LOGE("AbilityWindow::SetWindowConfig WindowManager::OnKey() return %d", retvalKeyboardKey);
        return false;
    }

    isWindowAttached = true;
    APP_LOGI("%{public}s end.", __func__);

    return true;
}

/**
 * @brief Called when the KeyEvent sent.
 *
 * @param KeyEvent the key event.
 *
 * @return Returns true if the listener has processed the event; returns false otherwise.
 *
 */
bool AbilityWindow::OnKeyEvent(KeyEvent event)
{
    APP_LOGI("%{public}s begin.", __func__);

    bool ret = false;
    std::shared_ptr<IAbilityEvent> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        APP_LOGE("AbilityWindow::OnKeyEvent ability is nullptr.");
        return ret;
    }
    switch (event.GetKeyCode()) {
#ifdef MMI_COMPILE
        case OHOS::KeyEventEnum::KEY_BACK:
#else
        case KeyEvent::CODE_BACK:
#endif
            APP_LOGI("AbilityWindow::OnKeyEvent Back key pressed.");
            if (!event.IsKeyDown()) {
                ret = OnBackPressed(ability);
            }
            break;
        default:
            APP_LOGI("AbilityWindow::OnKeyEvent the key event is %{public}d.", event.GetKeyCode());
            break;
    }
    APP_LOGI("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Called back when the Back key is pressed.
 *
 * @param ability The ability receive the event.
 *
 * @return Returns true if the listener has processed the event; returns false otherwise.
 *
 */
bool AbilityWindow::OnBackPressed(std::shared_ptr<IAbilityEvent> &ability)
{
    APP_LOGI("%{public}s begin.", __func__);
    if (handler_ == nullptr) {
        APP_LOGE("AbilityWindow::OnBackPressed handler_ is nullptr.");
        return false;
    }
    auto task = [abilityRun = ability]() { abilityRun->OnBackPressed(); };
    handler_->PostTask(task);
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

    if (windowNew_ != nullptr) {
        APP_LOGI("%{public}s begin windowNew_->Hide.", __func__);
        windowNew_->Hide();
        APP_LOGI("%{public}s end windowNew_->Hide.", __func__);
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

    if (windowNew_ != nullptr) {
        APP_LOGI("%{public}s begin windowNew_->SwitchTop.", __func__);
        windowNew_->SwitchTop();
        APP_LOGI("%{public}s end windowNew_->SwitchTop.", __func__);

        APP_LOGI("%{public}s begin windowNew_->Show.", __func__);
        windowNew_->Show();
        APP_LOGI("%{public}s end windowNew_->Show.", __func__);
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

    if (windowNew_ != nullptr) {
        APP_LOGI("%{public}s begin windowNew_->Hide.", __func__);
        windowNew_->Hide();
        APP_LOGI("%{public}s end windowNew_->Hide.", __func__);
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

    if (windowNew_ != nullptr) {
        APP_LOGI("%{public}s begin windowNew_->Hide.", __func__);
        windowNew_->Hide();
        APP_LOGI("%{public}s end windowNew_->Hide.", __func__);
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

    if (windowNew_ != nullptr) {
        APP_LOGI("%{public}s begin windowNew_->Show.", __func__);
        windowNew_->Show();
        APP_LOGI("%{public}s end windowNew_->Show.", __func__);
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

    if (windowNew_ != nullptr) {
        int32_t windowID = windowNew_->GetID();
        APP_LOGI("AbilityWindow::widow::DestroyWindow called windowID=%{public}d begin.", windowID);
        windowNew_->Destroy();
        APP_LOGI("AbilityWindow::widow::DestroyWindow called windowID=%{public}d end.", windowID);
        windowNew_ = nullptr;
        APP_LOGI("AbilityWindow::widow:: windowNew_ release end.");
    }

    isWindowAttached = false;
    APP_LOGI("AbilityWindow::OnPostAbilityStop end.");
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a Window object pointer.
 */
const sptr<Window> &AbilityWindow::GetWindow()
{
    if (!isWindowAttached) {
        APP_LOGE("AbilityWindow::GetWindow window not attached.");
    }

    if (windowNew_ == nullptr) {
        APP_LOGE("AbilityWindow::GetWindow the window is nullptr.");
    }
    return windowNew_;
}
}  // namespace AppExecFwk
}  // namespace OHOS