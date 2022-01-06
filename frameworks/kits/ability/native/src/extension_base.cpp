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

#include "extension_base.h"

#include "ability_local_record.h"
#include "extension_context.h"
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
    std::shared_ptr<C> context = std::make_shared<C>();
    context->SetToken(token);
    return context;
}

template<class C>
std::shared_ptr<C> ExtensionBase<C>::GetContext()
{
    return context_;
}
}
}