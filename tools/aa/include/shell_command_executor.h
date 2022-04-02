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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_EXECUTOR_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_EXECUTOR_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include "event_handler.h"
#include "shell_command_result.h"

namespace OHOS {
namespace AAFwk {
class ShellCommandExecutor : public std::enable_shared_from_this<ShellCommandExecutor> {
public:
    /**
     * A constructor used to create a ShellCommandExecutor instance with the input parameter passed.
     *
     * @param cmd, Indicates the specified shell command.
     * @param timeoutSec, Indicates the specified time out time, in seconds.
     */
    ShellCommandExecutor(const std::string &cmd, const int64_t timeoutSec);

    /**
     * Deconstructor used to deconstruct.
     */
    ~ShellCommandExecutor() = default;

    /**
     * Waits for the result of the shell command.
     *
     * @return the result of the specified shell command.
     */
    ShellCommandResult WaitWorkDone();

private:
    bool DoWork();

private:
    std::string cmd_;
    int64_t timeoutSec_ {0};
    ShellCommandResult cmdResult_;

    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::condition_variable cvWork_;
    std::mutex mtxWork_;
    std::mutex mtxSyncWork_;
    std::mutex mtxCopy_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_EXECUTOR_H