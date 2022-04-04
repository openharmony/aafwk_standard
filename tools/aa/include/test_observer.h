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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H

#include "test_observer_stub.h"

namespace OHOS {
namespace AAFwk {
class TestObserver : public TestObserverStub {
public:
    /**
     * Default constructor used to create a TestObserver instance.
     */
    TestObserver();

    /**
     * Deconstructor used to deconstruct.
     */
    virtual ~TestObserver() override;

    /**
     * Outputs test status.
     *
     * @param msg, Indicates the status information.
     * @param resultCode, Indicates the result code.
     */
    virtual void TestStatus(const std::string &msg, const int &resultCode) override;

    /**
     * Outputs information and result code that the test has finished.
     *
     * @param msg, Indicates the status information.
     * @param resultCode, Indicates the result code.
     */
    virtual void TestFinished(const std::string &msg, const int &resultCode) override;

    /**
     * Executes the specified shell command.
     *
     * @param cmd, Indicates the specified shell command.
     * @param timeoutSec, Indicates the specified time out time, in seconds.
     * @return the result of the specified shell command.
     */
    virtual ShellCommandResult ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec) override;

    /**
     * Waits for the test to finish.
     *
     * @param timeoutMs, Indicates the specified time out time, in milliseconds.
     * @return true if the test finish within the specified time; returns false otherwise.
     */
    bool WaitForFinish(const int64_t &timeoutMs);

private:
    bool isFinished_;
    static constexpr int64_t SHELL_COMMAND_TIMEOUT_MAX = 5;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_TEST_OBSERVER_H