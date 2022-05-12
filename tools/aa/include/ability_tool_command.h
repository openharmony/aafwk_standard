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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_TOOL_COMMAND_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_TOOL_COMMAND_H

#include "ability_command.h"
#include "shell_command.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityToolCommand : public ShellCommand {
public:
    AbilityToolCommand(int argc, char *argv[]);
    ~AbilityToolCommand() override {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsStopService();
    ErrCode RunAsForceStop();
    ErrCode RunAsTestCommand();

    ErrCode ParseStartAbilityArgsFromCmd(Want& want, StartOptions& startoptions);
    ErrCode ParseStopServiceArgsFromCmd(Want& want);
    ErrCode ParseTestArgsFromCmd(std::map<std::string, std::string>& params, bool isDebug);

    std::shared_ptr<AbilityManagerShellCommand> aaShellCmd_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_TOOL_COMMAND_H