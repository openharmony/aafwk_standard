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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H

#include "test_observer_stub.h"

namespace OHOS {
namespace AAFwk {
class TestObserver : public TestObserverStub {
public:

    TestObserver();

    virtual ~TestObserver() override;

    virtual void TestStatus(const std::string &msg, const int &resultCode) override;
    virtual void TestFinished(const std::string &msg, const int &resultCode) override;
    virtual ShellCommandResult ExecuteShellCommand(
        const std::string &cmd, const int64_t timeoutMs) override;
    bool WaitForFinish(const int64_t &timeoutMs);

private:
    bool isFinished_;
    static constexpr int64_t SHELL_COMMAND_TIMEOUT_MAX = 5000;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H