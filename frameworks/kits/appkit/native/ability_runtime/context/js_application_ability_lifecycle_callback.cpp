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

#include "js_application_ability_lifecycle_callback.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

void JsApplicationContextAbilityLifecycleCallback::onAbilityCreate(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityCreate");
}

void JsApplicationContextAbilityLifecycleCallback::onAbilityWindowStageCreate(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityWindowStageCreate");
}

void JsApplicationContextAbilityLifecycleCallback::onAbilityWindowStageDestroy(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityWindowStageDestroy");
}

void JsApplicationContextAbilityLifecycleCallback::onAbilityDestroy(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityDestroy");
}

void JsApplicationContextAbilityLifecycleCallback::onAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityForeground");
}

void JsApplicationContextAbilityLifecycleCallback::onAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityBackground");
}
void JsApplicationContextAbilityLifecycleCallback::onAbilityContinue(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("JsApplicationContextAbilityLifecycleCallback onAbilityContinue");
}
}  // namespace AbilityRuntime
}  // namespace OHOS