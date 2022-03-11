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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ITEST_OBSERVER_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ITEST_OBSERVER_H

#include "iremote_broker.h"
#include "shell_command_result.h"

namespace OHOS {
namespace AAFwk {
class ITestObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.ITestObserver");

    virtual void TestStatus(const std::string &msg, const int &resultCode) = 0;
    virtual void TestFinished(const std::string &msg, const int &resultCode) = 0;
    virtual ShellCommandResult ExecuteShellCommand(
        const std::string &cmd, const int64_t timeoutSec) = 0;

    enum class Message {
        AA_TEST_STATUS = 1,
        AA_TEST_FINISHED = 2,
        AA_EXECUTE_SHELL_COMMAND = 3,
    };
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ITEST_OBSERVER_H
