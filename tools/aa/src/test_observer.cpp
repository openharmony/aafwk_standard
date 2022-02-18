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

#include "test_observer.h"

#include <iostream>
#include <thread>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "system_time.h"

namespace OHOS {
namespace AAFwk {
TestObserver::TestObserver()
    : isFinished_(false)
{}

TestObserver::~TestObserver()
{}

void TestObserver::TestStatus(const std::string &msg, const int &resultCode)
{
    HILOG_INFO("enter");
    std::cout << msg << std::endl;
}

void TestObserver::TestFinished(const std::string &msg, const int &resultCode)
{
    HILOG_INFO("enter");
    std::cout << "TestFinished-ResultCode: " + std::to_string(resultCode) << std::endl;
    std::cout << "TestFinished-ResultMsg: " + msg << std::endl;
    isFinished_ = true;
}

ShellCommandResult TestObserver::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutMs)
{
    HILOG_INFO("enter");
    ShellCommandResult result;
    if (cmd.empty()) {
        return result;
    }
    FILE *file = popen(cmd.c_str(), "r");
    if (!file) {
        return result;
    }

    int64_t timeout = (timeoutMs <= 0) ? SHELL_COMMAND_TIMEOUT_MAX : timeoutMs;
    std::this_thread::sleep_for(std::chrono::microseconds(timeout));

    char commandResult[1024] = {0};
    while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
        result.stdResult.append(commandResult);
        std::cout << commandResult;
    }
    result.exitCode = pclose(file);
    file = nullptr;

    return result;
}

bool TestObserver::waitForFinish(const int64_t &timeoutMs)
{
    HILOG_INFO("enter");
    int64_t startTime = SystemTime::GetNowSysTime();
    while (!isFinished_) {
        int64_t nowSysTime = SystemTime::GetNowSysTime();
        if (timeoutMs && (nowSysTime - startTime > timeoutMs)) {
            return false;
        }
        sleep(1);
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS