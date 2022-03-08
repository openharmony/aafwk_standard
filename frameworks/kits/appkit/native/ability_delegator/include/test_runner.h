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

#ifndef FOUNDATION_APPEXECFWK_OHOS_TEST_RUNNER_H
#define FOUNDATION_APPEXECFWK_OHOS_TEST_RUNNER_H

#include <memory>
#include "ability_delegator_args.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
}

namespace AppExecFwk {
class TestRunner {
public:
    static std::unique_ptr<TestRunner> Create(
        const std::unique_ptr<AbilityRuntime::Runtime> &runtime, const std::shared_ptr<AbilityDelegatorArgs> &args);

    TestRunner() = default;
    virtual ~TestRunner() = default;

    virtual void Prepare();
    virtual void Run();
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_TEST_RUNNER_H
