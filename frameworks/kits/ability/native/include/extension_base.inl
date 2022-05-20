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

#include "foundation/aafwk/standard/frameworks/kits/appkit/native/ability_runtime/context/context.h"
#include "foundation/aafwk/standard/frameworks/kits/appkit/native/ability_runtime/context/application_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
template<class C>
void ExtensionBase<C>::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    Extension::Init(record, application, handler, token);
    HILOG_INFO("begin init context");
    context_ = CreateAndInitContext(record, application, handler, token);
}

template<class C>
std::shared_ptr<C> ExtensionBase<C>::CreateAndInitContext(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("begin init base");
    std::shared_ptr<C> context = std::make_shared<C>();
    context->SetToken(token);
    auto appContext = Context::GetApplicationContext();
    context->SetApplicationInfo(appContext->GetApplicationInfo());
    context->SetResourceManager(appContext->GetResourceManager());
    if (record == nullptr) {
        HILOG_ERROR("ServiceExtension::CreateAndInitContext record is nullptr");
        return context;
    }
    HILOG_INFO("begin init abilityInfo");
    auto abilityInfo = record->GetAbilityInfo();
    context->SetAbilityInfo(abilityInfo);
    context->InitHapModuleInfo(abilityInfo);
    context->SetConfiguration(appContext->GetConfiguration());
    return context;
}

template<class C>
std::shared_ptr<C> ExtensionBase<C>::GetContext()
{
    return context_;
}
}
}