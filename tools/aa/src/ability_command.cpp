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

#include "ability_command.h"

#include <getopt.h>
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "ohos/aafwk/base/bool_wrapper.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string SHORT_OPTIONS = "ch:d:a:b:p:s:D";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"device", required_argument, nullptr, 'd'},
    {"ability", required_argument, nullptr, 'a'},
    {"bundle", required_argument, nullptr, 'b'},
    {"power", required_argument, nullptr, 'p'},
    {"setting", required_argument, nullptr, 's'},
    {"debug", no_argument, nullptr, 'D'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP = "has:m:lud::e::LS";
const struct option LONG_OPTIONS_DUMP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"all", no_argument, nullptr, 'a'},
    {"stack", required_argument, nullptr, 's'},
    {"mission", required_argument, nullptr, 'm'},
    {"stack-list", no_argument, nullptr, 'l'},
    {"ui", no_argument, nullptr, 'u'},
    {"data", no_argument, nullptr, 'd'},
    {"serv", no_argument, nullptr, 'e'},
    {"mission-list", no_argument, nullptr, 'L'},
    {"mission-infos", no_argument, nullptr, 'S'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

AbilityManagerShellCommand::AbilityManagerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        HILOG_INFO("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

ErrCode AbilityManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&AbilityManagerShellCommand::RunAsHelpCommand, this)},
        {"screen", std::bind(&AbilityManagerShellCommand::RunAsScreenCommand, this)},
        {"start", std::bind(&AbilityManagerShellCommand::RunAsStartAbility, this)},
        {"stop-service", std::bind(&AbilityManagerShellCommand::RunAsStopService, this)},
        {"dump", std::bind(&AbilityManagerShellCommand::RunAsDumpCommand, this)},
        {"force-stop", std::bind(&AbilityManagerShellCommand::RunAsForceStop, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::CreateMessageMap()
{
    messageMap_ = {
        //  code + message
        {
            RESOLVE_ABILITY_ERR,
            "error: resolve ability err.",
        },
        {
            GET_ABILITY_SERVICE_FAILED,
            "error: get ability service failed.",
        },
        {
            ABILITY_SERVICE_NOT_CONNECTED,
            "error: ability service not connected.",
        },
        {
            RESOLVE_APP_ERR,
            "error: resolve app err.",
        },
        {
            STACK_MANAGER_NOT_EXIST,
            "error: stack manager not exist.",
        },
        {
            ABILITY_EXISTED,
            "error: ability existed.",
        },
        {
            CREATE_MISSION_STACK_FAILED,
            "error: create mission stack failed.",
        },
        {
            CREATE_MISSION_RECORD_FAILED,
            "error: create mission record failed.",
        },
        {
            CREATE_ABILITY_RECORD_FAILED,
            "error: create ability record failed.",
        },
        {
            START_ABILITY_WAITING,
            "error: start ability waiting.",
        },
        {
            TERMINATE_LAUNCHER_DENIED,
            "error: terminate launcher denied.",
        },
        {
            CONNECTION_NOT_EXIST,
            "error: connection not exist.",
        },
        {
            INVALID_CONNECTION_STATE,
            "error: invalid connection state.",
        },
        {
            LOAD_ABILITY_TIMEOUT,
            "error: load ability timeout.",
        },
        {
            CONNECTION_TIMEOUT,
            "error: connection timeout.",
        },
        {
            GET_BUNDLE_MANAGER_SERVICE_FAILED,
            "error: get bundle manager service failed.",
        },
        {
            REMOVE_MISSION_ID_NOT_EXIST,
            "error: remove mission id not exist.",
        },
        {
            REMOVE_MISSION_LAUNCHER_DENIED,
            "error: remove mission launcher denied.",
        },
        {
            REMOVE_MISSION_ACTIVE_DENIED,
            "error: remove mission active denied.",
        },
        {
            REMOVE_MISSION_FAILED,
            "error: remove mission failed.",
        },
        {
            INNER_ERR,
            "error: inner err.",
        },
        {
            GET_RECENT_MISSIONS_FAILED,
            "error: get recent missions failed.",
        },
        {
            REMOVE_STACK_LAUNCHER_DENIED,
            "error: remove stack launcher denied.",
        },
        {
            REMOVE_STACK_FAILED,
            "error: remove stack failed.",
        },
        {
            MISSION_STACK_LIST_IS_EMPTY,
            "error: mission stack list is empty.",
        },
        {
            REMOVE_STACK_ID_NOT_EXIST,
            "error: remove stack id not exist.",
        },
        {
            TARGET_ABILITY_NOT_SERVICE,
            "error: target ability not service.",
        },
        {
            TERMINATE_SERVICE_IS_CONNECTED,
            "error: terminate service is connected.",
        },
        {
            START_SERVICE_ABILITY_ACTIVING,
            "error: start service ability activing.",
        },
        {
            MOVE_MISSION_FAILED,
            "error: move mission failed.",
        },
        {
            KILL_PROCESS_FAILED,
            "error: kill process failed.",
        },
        {
            UNINSTALL_APP_FAILED,
            "error: uninstall app failed.",
        },
        {
            TERMINATE_ABILITY_RESULT_FAILED,
            "error: terminate ability result failed.",
        },
        {
            CHECK_PERMISSION_FAILED,
            "error: check permission failed.",
        },
        {
            POWER_OFF_WAITING,
            "error: power off waiting.",
        },
        {
            POWER_OFF_FAILED,
            "error: power off failed.",
        },
        {
            POWER_ON_FAILED,
            "error: power on failed.",
        },
        {
            NO_FIRST_IN_MISSION,
            "error: no first in mission.",
        },
        {
            LOCK_MISSION_DENY_FAILED,
            "error: lock mission deny failed.",
        },
        {
            UNLOCK_MISSION_DENY_FAILED,
            "error: unlock mission deny failed.",
        },
        {
            SET_MISSION_INFO_FAILED,
            "error: set mission info failed.",
        },
        {
            LOCK_MISSION_STATE_DENY_REQUEST,
            "error: lock mission state deny request.",
        },
        {
            MOVE_MISSION_TO_STACK_OUT_OF_SIZE,
            "error: move mission to stack out of size.",
        },
        {
            MOVE_MISSION_TO_STACK_NOT_SAME_WIN_MODE,
            "error: move mission to stack not same win mode.",
        },
        {
            MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION,
            "error: move mission to stack not exist mission.",
        },
        {
            MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN,
            "error: move mission to stack not support multi win.",
        },
        {
            MOVE_MISSION_TO_STACK_TARGET_STACK_OVERFLOW,
            "error: move mission to stack target stack overflow.",
        },
        {
            MOVE_MISSION_TO_STACK_MOVING_DENIED,
            "error: move mission to stack moving denied.",
        },
        {
            MINIMIZE_MULTI_WINDOW_FAILED,
            "error: minimize multi window failed.",
        },
        {
            MAXIMIZE_MULTIWINDOW_NOT_EXIST,
            "error: maximize multiwindow not exist.",
        },
        {
            MAXIMIZE_MULTIWINDOW_FAILED,
            "error: maximize multiwindow failed.",
        },
        {
            CHANGE_FOCUS_ABILITY_FAILED,
            "error: change focus ability failed.",
        },
        {
            GET_FLOATING_STACK_FAILED,
            "error: get floating stack failed.",
        },
        {
            CLOSE_MULTI_WINDOW_FAILED,
            "error: close multi window failed.",
        },
        {
            START_ABILITY_SETTING_FAILED,
            "error: start ability setting failed.",
        },
        {
            START_ABILITY_SETTING_NOT_SUPPORT_MULTI_WIN,
            "error: start ability setting not support multi win.",
        },
        {
            NO_FOUND_ABILITY_BY_CALLER,
            "error: no found ability by caller.",
        },
        {
            ABILITY_VISIBLE_FALSE_DENY_REQUEST,
            "error: ability visible false deny request.",
        },
        {
            GET_BUNDLE_INFO_FAILED,
            "error: get bundle info failed.",
        },
        {
            KILL_PROCESS_KEEP_ALIVE,
            "error: keep alive process can not be killed.",
        },
    };

    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::init()
{
    ErrCode result = AbilityManagerClient::GetInstance()->Connect();

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::RunAsScreenCommand()
{
    HILOG_INFO("enter");

    int result = OHOS::ERR_OK;

    int option = -1;
    int counter = 0;

    std::string powerState = "";

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'aa screen' with no option: aa screen
                    // 'aa screen' with a wrong argument: aa screen xxx
                    HILOG_INFO("'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'p': {
                    // 'aa screen -p' with no argument
                    HILOG_INFO("'aa %{public}s -p' with no argument.", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 0: {
                    // 'aa screen' with an unknown option: aa screen --x
                    // 'aa screen' with an unknown option: aa screen --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    HILOG_INFO("'aa screen' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'aa screen' with an unknown option: aa screen -x
                    // 'aa screen' with an unknown option: aa screen -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    HILOG_INFO("'aa screen' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa screen -h'
                // 'aa screen --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                // 'aa screen -p xxx'

                // save powerState
                if (optarg != nullptr) {
                    powerState = optarg;
                }
                break;
            }
            case 0: {
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        HILOG_INFO("powerState: %{public}s", powerState.c_str());

        if (powerState == STRING_SCREEN_POWER_ON) {
            result = AbilityManagerClient::GetInstance()->PowerOn();
            if (result == OHOS::ERR_OK) {
                HILOG_INFO("%{public}s", STRING_SCREEN_POWER_ON_OK.c_str());
                resultReceiver_ = STRING_SCREEN_POWER_ON_OK + "\n";
            } else {
                HILOG_INFO("%{public}s result = %{public}d", STRING_SCREEN_POWER_ON_NG.c_str(), result);
                resultReceiver_ = STRING_SCREEN_POWER_ON_NG + "\n";

                resultReceiver_.append(GetMessageFromCode(result));
            }
        } else {
            result = AbilityManagerClient::GetInstance()->PowerOff();
            if (result == OHOS::ERR_OK) {
                HILOG_INFO("%{public}s", STRING_SCREEN_POWER_OFF_OK.c_str());
                resultReceiver_ = STRING_SCREEN_POWER_OFF_OK + "\n";
            } else {
                HILOG_INFO("%{public}s result = %{public}d", STRING_SCREEN_POWER_OFF_NG.c_str(), result);
                resultReceiver_ = STRING_SCREEN_POWER_OFF_NG + "\n";

                resultReceiver_.append(GetMessageFromCode(result));
            }
        }
    } else {
        resultReceiver_.append(HELP_MSG_SCREEN);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsStartAbility()
{
    Want want;
    std::string windowMode;
    ErrCode result = MakeWantFromCmd(want, windowMode);
    if (result == OHOS::ERR_OK) {
        int windowModeKey = std::atoi(windowMode.c_str());
        if (windowModeKey > 0) {
            auto setting = AbilityStartSetting::GetEmptySetting();
            if (setting != nullptr) {
                setting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, windowMode);
                result = AbilityManagerClient::GetInstance()->StartAbility(want, *(setting.get()), nullptr, -1);
            }
        } else {
            result = AbilityManagerClient::GetInstance()->StartAbility(want);
        }
        if (result == OHOS::ERR_OK) {
            HILOG_INFO("%{public}s", STRING_START_ABILITY_OK.c_str());
            resultReceiver_ = STRING_START_ABILITY_OK + "\n";
        } else {
            HILOG_INFO("%{public}s result = %{public}d", STRING_START_ABILITY_NG.c_str(), result);
            resultReceiver_ = STRING_START_ABILITY_NG + "\n";

            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append(HELP_MSG_START);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsStopService()
{
    ErrCode result = OHOS::ERR_OK;

    Want want;
    std::string windowMode;
    result = MakeWantFromCmd(want, windowMode);
    if (result == OHOS::ERR_OK) {
        result = AbilityManagerClient::GetInstance()->StopServiceAbility(want);
        if (result == OHOS::ERR_OK) {
            HILOG_INFO("%{public}s", STRING_STOP_SERVICE_ABILITY_OK.c_str());
            resultReceiver_ = STRING_STOP_SERVICE_ABILITY_OK + "\n";
        } else {
            HILOG_INFO("%{public}s result = %{public}d", STRING_STOP_SERVICE_ABILITY_NG.c_str(), result);
            resultReceiver_ = STRING_STOP_SERVICE_ABILITY_NG + "\n";

            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append(HELP_MSG_STOP_SERVICE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsDumpCommand()
{
    ErrCode result = OHOS::ERR_OK;

    std::string args;
    for (auto arg : argList_) {
        args += arg;
        args += " ";
    }

    int option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP.c_str(), LONG_OPTIONS_DUMP, nullptr);

    HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

    if (optind < 0 || optind > argc_) {
        return OHOS::ERR_INVALID_VALUE;
    }

    switch (option) {
        case 'h': {
            // 'aa dump -h'
            // 'aa dump --help'
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 'a': {
            // 'aa dump -a'
            // 'aa dump --all'
            break;
        }
        case 's': {
            // 'aa dump -s xxx'
            // 'aa dump --stack xxx'
            break;
        }
        case 'm': {
            // 'aa dump -m xxx'
            // 'aa dump --mission xxx'
            break;
        }
        case 'l': {
            // 'aa dump -l'
            // 'aa dump --stack-list'
            break;
        }
        case 'u': {
            // 'aa dump -u'
            // 'aa dump --ui'
            break;
        }
        case 'd': {
            // 'aa dump -d'
            // 'aa dump --data'
            break;
        }
        case 'e': {
            // 'aa dump -e'
            // 'aa dump --serv'
            break;
        }
        case 'L': {
            // 'aa dump -L'
            // 'aa dump --mission-list'
            break;
        }
        case 'S': {
            // 'aa dump -S'
            // 'aa dump --mission-infos'
            break;
        }
        case '?': {
            result = RunAsDumpCommandOptopt();
            break;
        }
        default: {
            if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'aa dump' with no option: aa dump
                // 'aa dump' with a wrong argument: aa dump xxx
                HILOG_INFO("'aa dump' with no option.");

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP);
    } else {
        std::vector<std::string> dumpResults;
        result = AbilityManagerClient::GetInstance()->DumpState(args, dumpResults);
        if (result == OHOS::ERR_OK) {
            for (auto it : dumpResults) {
                resultReceiver_ += it + "\n";
            }
        } else {
            HILOG_INFO("failed to dump state.");
        }
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsForceStop()
{
    HILOG_INFO("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    if (argList_.empty()) {
        resultReceiver_.append(HELP_MSG_FORCE_STOP + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }
    HILOG_INFO("Bundle name : %{public}s", argList_[0].c_str());
    ErrCode result = OHOS::ERR_OK;
    result = AbilityManagerClient::GetInstance()->KillProcess(argList_[0]);
    if (result == OHOS::ERR_OK) {
        HILOG_INFO("%{public}s", STRING_FORCE_STOP_OK.c_str());
        resultReceiver_ = STRING_FORCE_STOP_OK + "\n";
    } else {
        HILOG_INFO("%{public}s result = %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
        resultReceiver_ = STRING_FORCE_STOP_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }
    return result;
}

ErrCode AbilityManagerShellCommand::RunAsDumpCommandOptopt()
{
    ErrCode result = OHOS::ERR_OK;

    switch (optopt) {
        case 's': {
            // 'aa dump -s' with no argument: aa dump -s
            // 'aa dump --stack' with no argument: aa dump --stack
            HILOG_INFO("'aa dump -s' with no argument.");

            resultReceiver_.append("error: option ");
            resultReceiver_.append("requires a value.\n");
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 'm': {
            // 'aa dump -m' with no argument: aa dump -m
            // 'aa dump --mission' with no argument: aa dump --mission
            HILOG_INFO("'aa dump -m' with no argument.");

            resultReceiver_.append("error: option ");
            resultReceiver_.append("requires a value.\n");
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 0: {
            // 'aa dump' with an unknown option: aa dump --x
            // 'aa dump' with an unknown option: aa dump --xxx
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

            HILOG_INFO("'aa dump' with an unknown option.");

            resultReceiver_.append(unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        default: {
            // 'aa dump' with an unknown option: aa dump -x
            // 'aa dump' with an unknown option: aa dump -xxx
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

            HILOG_INFO("'aa dump' with an unknown option.");

            resultReceiver_.append(unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
    }

    return result;
}

ErrCode AbilityManagerShellCommand::MakeWantFromCmd(Want &want, std::string &windowMode)
{
    int result = OHOS::ERR_OK;

    int option = -1;
    int counter = 0;

    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    bool isDebugApp = false;
    bool isContinuation = false;

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'aa start' with no option: aa start
                    // 'aa start' with a wrong argument: aa start xxx
                    // 'aa stop-service' with no option: aa stop-service
                    // 'aa stop-service' with a wrong argument: aa stop-service xxx
                    HILOG_INFO("'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'h': {
                    // 'aa start -h'
                    // 'aa stop-service -h'
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'aa start -d' with no argument
                    // 'aa stop-service -d' with no argument
                    HILOG_INFO("'aa %{public}s -d' with no argument.", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'aa start -a' with no argument
                    // 'aa stop-service -a' with no argument
                    HILOG_INFO("'aa %{public}s -a' with no argument.", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'b': {
                    // 'aa start -b' with no argument
                    // 'aa stop-service -b' with no argument
                    HILOG_INFO("'aa %{public}s -b' with no argument.", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 's': {
                    // 'aa start -s' with no argument
                    // 'aa stop-service -s' with no argument
                    HILOG_INFO("'aa %{public}s -s' with no argument.", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append(argv_[optind - 1]);
                    resultReceiver_.append("' requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 0: {
                    // 'aa start' with an unknown option: aa start --x
                    // 'aa start' with an unknown option: aa start --xxx
                    // 'aa stop-service' with an unknown option: aa stop-service --x
                    // 'aa stop-service' with an unknown option: aa stop-service --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    HILOG_INFO("'aa %{public}s' with an unknown option.", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'aa start' with an unknown option: aa start -x
                    // 'aa start' with an unknown option: aa start -xxx
                    // 'aa stop-service' with an unknown option: aa stop-service -x
                    // 'aa stop-service' with an unknown option: aa stop-service -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    HILOG_INFO("'aa %{public}s' with an unknown option.", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa start -h'
                // 'aa start --help'
                // 'aa stop-service -h'
                // 'aa stop-service --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'd': {
                // 'aa start -d xxx'
                // 'aa stop-service -d xxx'

                // save device ID
                if (optarg != nullptr) {
                    deviceId = optarg;
                }
                break;
            }
            case 'a': {
                // 'aa start -a xxx'
                // 'aa stop-service -a xxx'

                // save ability name
                abilityName = optarg;
                break;
            }
            case 'b': {
                // 'aa start -b xxx'
                // 'aa stop-service -b xxx'

                // save bundle name
                bundleName = optarg;
                break;
            }
            case 's': {
                // 'aa start -s xxx'
                // save windowMode
                windowMode = optarg;
                break;
            }
            case 'D': {
                // 'aa start -D'
                // debug app
                isDebugApp = true;
                break;
            }
            case 'c': {
                // 'aa start -c'
                // set ability launch reason = continuation
                isContinuation = true;
                break;
            }
            case 0: {
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (abilityName.size() == 0 || bundleName.size() == 0) {
            // 'aa start [-d <device-id>] -a <ability-name> -b <bundle-name> [-D]'
            // 'aa stop-service [-d <device-id>] -a <ability-name> -b <bundle-name>'
            HILOG_INFO("'aa %{public}s' without enough options.", cmd_.c_str());

            if (abilityName.size() == 0) {
                resultReceiver_.append(HELP_MSG_NO_ABILITY_NAME_OPTION + "\n");
            }

            if (bundleName.size() == 0) {
                resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            }

            result = OHOS::ERR_INVALID_VALUE;
        } else {
            ElementName element(deviceId, bundleName, abilityName);
            want.SetElement(element);

            if (isDebugApp) {
                WantParams wantParams;
                wantParams.SetParam("debugApp", Boolean::Box(isDebugApp));

                want.SetParams(wantParams);
            }
            if (isContinuation) {
                want.AddFlags(Want::FLAG_ABILITY_CONTINUATION);
            }
        }
    }

    return result;
}

}  // namespace AAFwk
}  // namespace OHOS
