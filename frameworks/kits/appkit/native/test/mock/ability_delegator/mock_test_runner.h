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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_ABILITY_DELEGATOR_MOCK_TEST_RUNNER_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_ABILITY_DELEGATOR_MOCK_TEST_RUNNER_H

#include "gmock/gmock.h"

#include "test_runner.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
}

namespace AppExecFwk {
class MockTestRunner : public TestRunner {
public:

    MockTestRunner() = default;
    virtual ~MockTestRunner() = default;

    virtual void Prepare();
    virtual void Run();

    static bool prepareFlag_;
    static bool runFlag_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_TEST_MOCK_ABILITY_DELEGATOR_MOCK_TEST_RUNNER_H