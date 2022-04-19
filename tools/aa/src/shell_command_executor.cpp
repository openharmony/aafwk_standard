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

#include "shell_command_executor.h"

#include <chrono>
#include <cinttypes>
#include <iostream>

#include "hilog_wrapper.h"

using namespace std::chrono_literals;
namespace OHOS {
namespace AAFwk {
ShellCommandExecutor::ShellCommandExecutor(const std::string &cmd, const int64_t timeoutSec)
    : cmd_(cmd), timeoutSec_(timeoutSec)
{
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
}

ShellCommandResult ShellCommandExecutor::WaitWorkDone()
{
    HILOG_INFO("enter");

    if (!DoWork()) {
        HILOG_INFO("Failed to execute command : \"%{public}s\"", cmd_.data());
        return cmdResult_;
    }

    std::unique_lock<std::mutex> workLock(mtxWork_);

    auto condition = [this]() { return isDone_; };
    if (timeoutSec_ <= 0) {
        cvWork_.wait(workLock, condition);
    } else if (!cvWork_.wait_for(workLock, timeoutSec_ * 1s, condition)) {
        HILOG_WARN("Command execution timed out! cmd : \"%{public}s\", timeoutSec : %{public}" PRId64,
            cmd_.data(), timeoutSec_);
        std::cout << "Warning! Command execution timed out! cmd : " << cmd_ << ", timeoutSec : " << timeoutSec_
            << std::endl;

        ShellCommandResult realResult;
        realResult.exitCode = -1;
        {
            std::lock_guard<std::mutex> copyLock(mtxCopy_);
            realResult.stdResult = cmdResult_.stdResult;
        }
        return realResult;
    }

    HILOG_INFO("Command execution complete, cmd : \"%{public}s\", exitCode : %{public}d",
        cmd_.data(), cmdResult_.exitCode);
    return cmdResult_;
}

bool ShellCommandExecutor::DoWork()
{
    HILOG_INFO("enter");

    if (cmd_.empty()) {
        HILOG_ERROR("Invalid command");
        return false;
    }

    if (!handler_) {
        HILOG_ERROR("Invalid event handler");
        return false;
    }

    auto self(shared_from_this());
    handler_->PostTask([this, self]() {
        HILOG_INFO("DoWork async task begin, cmd : \"%{public}s\"", cmd_.data());

        FILE *file = popen(cmd_.c_str(), "r");
        if (!file) {
            HILOG_ERROR("Failed to call popen, cmd : \"%{public}s\"", cmd_.data());

            {
                std::unique_lock<std::mutex> workLock(mtxWork_);
                isDone_ = true;
            }
            cvWork_.notify_one();
            HILOG_INFO("DoWork async task end, cmd : \"%{public}s\"", cmd_.data());
            return;
        }

        char commandResult[1024] = {'\0'};
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            {
                std::lock_guard<std::mutex> copyLock(mtxCopy_);
                cmdResult_.stdResult.append(commandResult);
            }
            std::cout << commandResult;
        }

        cmdResult_.exitCode = pclose(file);
        file = nullptr;

        {
            std::unique_lock<std::mutex> workLock(mtxWork_);
            isDone_ = true;
        }
        cvWork_.notify_one();
        HILOG_INFO("DoWork async task end, cmd : \"%{public}s\"", cmd_.data());
    });

    return true;
}
}  // namespace AAFwk
}  // namespace OHOS