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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_CONTINUATION_INTERFACE_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_CONTINUATION_INTERFACE_H

#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityContinuation {
public:
    IAbilityContinuation() = default;
    virtual ~IAbilityContinuation() = default;

    virtual bool OnStartContinuation() = 0;

    virtual bool OnSaveData(WantParams &saveData) = 0;

    virtual bool OnRestoreData(WantParams &restoreData) = 0;

    virtual void OnCompleteContinuation(int result) = 0;

    virtual void OnRemoteTerminated() = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_CONTINUATION_INTERFACE_H