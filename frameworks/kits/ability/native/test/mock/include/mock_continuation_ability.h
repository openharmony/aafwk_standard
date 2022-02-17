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

#ifndef FOUNDATION_APPEXECFWK_OHOS_MOCK_CONTINUATION_ABILITY_H
#define FOUNDATION_APPEXECFWK_OHOS_MOCK_CONTINUATION_ABILITY_H

#include <gtest/gtest.h>

#include "ability.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockContinuationAbility : public Ability {
public:
    MockContinuationAbility() = default;
    virtual ~MockContinuationAbility() = default;

    bool OnStartContinuation()
    {
        GTEST_LOG_(INFO) << "MockContinuationAbility::OnStartContinuation called";
        return true;
    }

    bool OnSaveData(WantParams &saveData)
    {
        GTEST_LOG_(INFO) << "MockContinuationAbility::OnSaveData called";
        return true;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_MOCK_PAGE_ABILITY_H