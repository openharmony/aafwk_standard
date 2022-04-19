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

#include "util/light_refcount_base.h"

namespace OHOS {
namespace Idl {
int LightRefCountBase::AddRef()
{
    const int beforeCount = refCount_.fetch_add(1, std::memory_order_relaxed);
    return beforeCount + 1;
}

int LightRefCountBase::Release()
{
    const int beforeCount = refCount_.fetch_sub(1, std::memory_order_release);
    if (beforeCount - 1 == 0) {
        delete this;
    }
    return beforeCount - 1;
}
}
}
