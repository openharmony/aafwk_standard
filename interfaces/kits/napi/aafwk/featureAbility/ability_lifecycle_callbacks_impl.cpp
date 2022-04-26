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

#include "ability_lifecycle_callbacks_impl.h"

#include "hilog_wrapper.h"

void AbilityLifecycleCallbacksImpl::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityStart");
}
void AbilityLifecycleCallbacksImpl::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}

void AbilityLifecycleCallbacksImpl::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}
void AbilityLifecycleCallbacksImpl::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}
void AbilityLifecycleCallbacksImpl::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}
void AbilityLifecycleCallbacksImpl::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}
void AbilityLifecycleCallbacksImpl::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}

void AbilityLifecycleCallbacksImpl::OnAbilitySaveState(const PacMap &outState)
{
    HILOG_INFO("AbilityLifecycleCallbacksImpl OnAbilityInactive");
}