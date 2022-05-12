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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_COMMAND_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_COMMAND_H

#include "shell_command.h"
#include "ability_manager_interface.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TOOL_NAME = "aa";

const std::string HELP_MSG = "usage: aa <command> <options>\n"
                             "These are common aa commands list:\n"
                             "  help                        list available commands\n"
                             "  start                       start ability with options\n"
                             "  stop-service                stop service with options\n"
                             "  dump                        dump the ability info\n"
                             "  force-stop <bundle-name>    force stop the process with bundle name\n"
                             "  test                        start the test framework with options\n";

const std::string HELP_MSG_SCREEN =
    "usage: aa screen <options>\n"
    "options list:\n"
    "  -h, --help                                                   list available commands\n"
    "  -p, --power <state>                                          power on or off with a state name\n";

const std::string HELP_MSG_START =
    "usage: aa start <options>\n"
    "options list:\n"
    "  -h, --help                                                   list available commands\n"
    "  [-d <device-id>] -a <ability-name> -b <bundle-name> [-D]     start ability with an element name\n";

const std::string HELP_MSG_STOP_SERVICE =
    "usage: aa stop-service <options>\n"
    "options list:\n"
    "  -h, --help                                                   list available commands\n"
    "  [-d <device-id>] -a <ability-name> -b <bundle-name>          stop service with an element name\n";

const std::string HELP_MSG_DUMP = "usage: aa dump <options>\n"
                                  "options list:\n"
                                  "  -h, --help                   list available commands\n"
                                  "  -a, --all                    dump all abilities\n"
                                  "  -s, --stack <number>         dump the ability info of a specificed stack\n"
                                  "  -m, --mission <number>       dump the ability info of a specificed mission\n"
                                  "  -l, --stack-list             dump the mission list of every stack\n"
                                  "  -e, --serv                   dump the service abilities\n"
                                  "  -d, --data                   dump the data abilities\n";

const std::string HELP_MSG_DUMPSYS = "usage: aa dump <options>\n"
                                  "options list:\n"
                                  "  -h, --help                   list available commands\n"
                                  "  -a, --all                    dump all abilities\n"
                                  "  -l, --mission-list           dump mission list\n"
                                  "  -i, --ability                dump abilityRecordId\n"
                                  "  -e, --extension              dump elementName (API7 ExtensionRecords,"
                                                                  "API8 serviceAbilityRecords)\n"
                                  "  -p, --pending                dump pendingWantRecordId\n"
                                  "  -r, --process                dump process\n"
                                  "  -d, --data                   dump the data abilities\n"
                                  "  -u, --userId                 userId\n"
                                  "  -c, --client                 client\n"
                                  "  -c, -u are auxiliary parameters and cannot be used alone\n"
                                  "  The original -s parameter is invalid\n"
                                  "  The original -m parameter is invalid\n";

const std::string HELP_MSG_TEST =
    "usage: aa test <options>\n"
    "options list:\n"
    "  -h, --help                                             list available commands\n"
    "  -b <bundle-name> -s unittest <test-runner>             start the test framework with options\n"
    "                  [-p <package-name>]                    the name of package with test-runner, "
    "required for the FA model\n"
    "                  [-m <module-name>]                     the name of module with test-runner, "
    "required for the STAGE model\n"
    "                  [-s class <test-class>]\n"
    "                  [-s level <test-level>]\n"
    "                  [-s size <test-size>]\n"
    "                  [-s testType <test-testType>]\n"
    "                  [-s timeout <test-timeout>]\n"
    "                  [-s <any-key> <any-value>]\n"
    "                  [-w <wait-time>]\n"
    "                  [-D]\n";

const std::string HELP_MSG_FORCE_STOP = "usage: aa force-stop <bundle-name>\n";

const std::string HELP_MSG_NO_ABILITY_NAME_OPTION = "error: -a <ability-name> is expected";
const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: -b <bundle-name> is expected";

const std::string STRING_START_ABILITY_OK = "start ability successfully.";
const std::string STRING_START_ABILITY_NG = "error: failed to start ability.";

const std::string STRING_STOP_SERVICE_ABILITY_OK = "stop service ability successfully.";
const std::string STRING_STOP_SERVICE_ABILITY_NG = "error: failed to stop service ability.";

const std::string STRING_SCREEN_POWER_ON = "on";

const std::string STRING_SCREEN_POWER_ON_OK = "power on screen successfully.";
const std::string STRING_SCREEN_POWER_ON_NG = "error: failed to power on screen.";
const std::string STRING_SCREEN_POWER_OFF_OK = "power off screen successfully.";
const std::string STRING_SCREEN_POWER_OFF_NG = "error: failed to power off screen.";

const std::string STRING_FORCE_STOP_OK = "force stop process successfully.";
const std::string STRING_FORCE_STOP_NG = "error: failed to force stop process.";

const std::string STRING_START_USER_TEST_OK = "start user test successfully.";
const std::string STRING_START_USER_TEST_NG = "error: failed to start user test.";

const int USER_TEST_COMMAND_START_INDEX = 2;
const int USER_TEST_COMMAND_PARAMS_NUM = 2;
const int TIME_RATE_MS = 1000;

const int NUMBER_TWO = 2;
const int NUMBER_ONE = 1;

const std::string DEBUG_VALUE = "true";
}  // namespace

class AbilityManagerShellCommand : public ShellCommand {
public:
    AbilityManagerShellCommand(int argc, char *argv[]);
    ~AbilityManagerShellCommand() override
    {}

    ErrCode CreateMessageMap() override;
    bool IsTestCommandIntegrity(const std::map<std::string, std::string> &params);
    ErrCode StartUserTest(const std::map<std::string, std::string> &params, const bool isDebug);

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsScreenCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsStopService();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsDumpsysCommand();
    ErrCode RunAsForceStop();
    sptr<IAbilityManager> GetAbilityManagerService();

    ErrCode RunAsDumpCommandOptopt();
    ErrCode MakeWantFromCmd(Want &want, std::string &windowMode);
    ErrCode RunAsTestCommand();
    ErrCode TestCommandError(const std::string &info);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_ABILITY_COMMAND_H
