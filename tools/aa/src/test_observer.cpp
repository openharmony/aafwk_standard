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

#include "test_observer.h"

#include <cinttypes>
#include <iostream>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "shell_command_executor.h"
#include "system_time.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AAFwk {
TestObserver::TestObserver() : isFinished_(false)
{}

TestObserver::~TestObserver()
{}

void TestObserver::TestStatus(const std::string &msg, const int64_t &resultCode)
{
    HILOG_INFO("enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);
    std::cout << msg << std::endl;
}

void TestObserver::TestFinished(const std::string &msg, const int64_t &resultCode)
{
    HILOG_INFO("enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);
    std::cout << "TestFinished-ResultCode: " + std::to_string(resultCode) << std::endl;
    std::cout << "TestFinished-ResultMsg: " + msg << std::endl;
    isFinished_ = true;
}

ShellCommandResult TestObserver::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec)
{
    HILOG_INFO("enter, cmd : \"%{public}s\", timeoutSec : %{public}" PRId64, cmd.data(), timeoutSec);

    auto cmdExecutor = std::make_shared<ShellCommandExecutor>(cmd, timeoutSec);
    if (!cmdExecutor) {
        HILOG_ERROR("Failed to create ShellCommandExecutor intance");
        return {};
    }

    return cmdExecutor->WaitWorkDone();
}

bool TestObserver::WaitForFinish(const int64_t &timeoutMs)
{
    HILOG_INFO("enter");

    auto realTime = timeoutMs > 0 ? timeoutMs : 0;
    int64_t startTime = SystemTime::GetNowSysTime();
    while (!isFinished_) {
        int64_t nowSysTime = SystemTime::GetNowSysTime();
        if (realTime && (nowSysTime - startTime > realTime)) {
            return false;
        }
        sleep(1);
    }

    HILOG_INFO("User test finished");
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS