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

#include "shell_cmd_result.h"

namespace OHOS {
namespace AppExecFwk {
ShellCmdResult::ShellCmdResult(const int32_t exitCode, const std::string &stdResult)
    : exitCode_(exitCode), stdResult_(stdResult)
{}

ShellCmdResult::ShellCmdResult(const AAFwk::ShellCommandResult &result)
{
    exitCode_  = result.exitCode;
    stdResult_ = result.stdResult;
}

void ShellCmdResult::SetExitCode(const int32_t exitCode)
{
    exitCode_ = exitCode;
}

int32_t ShellCmdResult::GetExitCode() const
{
    return exitCode_;
}

void ShellCmdResult::SetStdResult(const std::string &stdResult)
{
    stdResult_ = stdResult;
}

std::string ShellCmdResult::GetStdResult() const
{
    return stdResult_;
}

std::string ShellCmdResult::Dump()
{
    return "ShellCmdResult { exitCode = " + std::to_string(exitCode_) + ", stdResult = " + stdResult_ + "}";
}
}  // namespace AppExecFwk
}  // namespace OHOS
