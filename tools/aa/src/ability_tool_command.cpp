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
#include "ability_tool_command.h"

#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <regex>

#include "ability_manager_client.h"
#include "element_name.h"
#include "hilog_wrapper.h"
#include "ohos/aafwk/base/bool_wrapper.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string ABILITY_TOOL_NAME = "ability_tool";
const std::string ABILITY_TOOL_HELP_MSG =
    "usage: ability_tool <command> <options>\n"
    "ability_tool commands list:\n"
    "  help                        list available commands\n"
    "  start                       start ability with options\n"
    "  stop-service                stop service with options\n"
    "  force-stop                  force stop the process with bundle name\n"
    "  test                        start the test framework with options\n";

const std::string ABILITY_TOOL_HELP_MSG_START =
    "usage: ability_tool start <options>\n"
    "ability_tool start options list:\n"
    "  --help                      list available options\n"
    "  --device <device-id>        device Id\n"
    "  --ability <ability-name>    ability name\n"
    "  --bundle <bundle-name>      bundle name\n"
    "  --options <key> <value>     start options, such as windowMode 102\n"
    "  --flags <flag>              flags in a want\n"
    "  -C                          cold start\n"
    "  -D                          start with debug mode\n";

const std::string ABILITY_TOOL_HELP_MSG_STOP_SERVICE =
    "usage: ability_tool stop-service <options>\n"
    "ability_tool stop-service options list:\n"
    "  --help                      list available options\n"
    "  --device <device-id>        device Id\n"
    "  --ability <ability-name>    ability name\n"
    "  --bundle <bundle-name>      bundle name\n";

const std::string ABILITY_TOOL_HELP_MSG_FORCE_STOP =
    "usage: ability_tool force-stop <options>\n"
    "ability_tool force-stop options list:\n"
    "  --help                      list available options\n"
    "  <bundle-name>               bundle name\n";

const std::string ABILITY_TOOL_HELP_MSG_TEST =
    "usage: ability_tool test <options>\n"
    "ability_tool test options list:\n"
    "  --help                              list available options\n"
    "  --bundle <bundle-name>              bundle name\n"
    "  --options unittest <test-runner>    test runner need to start\n"
    "  --package-name <package-name>       package name, required for the FA model\n"
    "  --module-name <module-name>         module name, required for the STAGE model\n"
    "  --options <key> <value>             test options, such as testcase test_001\n"
    "  --watchdog <wait-time>              max execute time for this test\n"
    "  -D                                  test with debug mode\n";

const std::string ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION = "error: --ability <ability-name> is expected";
const std::string ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: --bundle <bundle-name> is expected";
const std::string ABILITY_TOOL_HELP_MSG_WINDOW_MODE_INVALID = "error: --options windowMode <value> with invalid param";
const std::string ABILITY_TOOL_HELP_MSG_LACK_VALUE = "error: lack of value of key";
const std::string ABILITY_TOOL_HELP_MSG_ONLY_NUM = "error: current option only support number";
} // namespace

AbilityToolCommand::AbilityToolCommand(int argc, char *argv[]) : ShellCommand(argc, argv, ABILITY_TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        HILOG_INFO("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }

    aaShellCmd_ = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    if (aaShellCmd_.get() == nullptr) {
        HILOG_ERROR("Get aa command failed.");
    }
}

ErrCode AbilityToolCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&AbilityToolCommand::RunAsHelpCommand, this)},
        {"start", std::bind(&AbilityToolCommand::RunAsStartAbility, this)},
        {"stop-service", std::bind(&AbilityToolCommand::RunAsStopService, this)},
        {"force-stop", std::bind(&AbilityToolCommand::RunAsForceStop, this)},
        {"test", std::bind(&AbilityToolCommand::RunAsTestCommand, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::CreateMessageMap()
{
    if (aaShellCmd_.get() == nullptr) {
        HILOG_ERROR("aa shell command is nullptr.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return aaShellCmd_.get()->CreateMessageMap();
}

ErrCode AbilityToolCommand::init()
{
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode AbilityToolCommand::RunAsHelpCommand()
{
    resultReceiver_.append(ABILITY_TOOL_HELP_MSG);
    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::RunAsStartAbility()
{
    Want want;
    StartOptions startoptions;

    ErrCode result = ParseStartAbilityArgsFromCmd(want, startoptions);
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(ABILITY_TOOL_HELP_MSG_START);
        return result;
    }

    result = AbilityManagerClient::GetInstance()->StartAbility(want, startoptions, nullptr);
    if (result != OHOS::ERR_OK) {
        HILOG_ERROR("%{public}s result = %{public}d", STRING_START_ABILITY_NG.c_str(), result);
        if (result != START_ABILITY_WAITING) {
            resultReceiver_ = STRING_START_ABILITY_NG + "\n";
        }
        resultReceiver_.append(GetMessageFromCode(result));
        return result;
    }

    HILOG_INFO("%{public}s", STRING_START_ABILITY_OK.c_str());
    resultReceiver_ = STRING_START_ABILITY_OK + "\n";
    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::RunAsStopService()
{
    Want want;

    ErrCode result = ParseStopServiceArgsFromCmd(want);
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
        return OHOS::ERR_INVALID_VALUE;
    }

    result = AbilityManagerClient::GetInstance()->StopServiceAbility(want);
    if (result != OHOS::ERR_OK) {
        HILOG_ERROR("%{public}s result = %{public}d", STRING_STOP_SERVICE_ABILITY_NG.c_str(), result);
        resultReceiver_ = STRING_STOP_SERVICE_ABILITY_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
        return result;
    }

    HILOG_INFO("%{public}s", STRING_STOP_SERVICE_ABILITY_OK.c_str());
    resultReceiver_ = STRING_STOP_SERVICE_ABILITY_OK + "\n";
    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::RunAsForceStop()
{
    if (argList_.empty()) {
        resultReceiver_.append(ABILITY_TOOL_HELP_MSG_FORCE_STOP + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    std::string bundleName = argList_[0];
    ErrCode result = AbilityManagerClient::GetInstance()->KillProcess(bundleName);
    if (result != OHOS::ERR_OK) {
        HILOG_ERROR("%{public}s result = %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
        resultReceiver_ = STRING_FORCE_STOP_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
        return result;
    }

    HILOG_INFO("%{public}s", STRING_FORCE_STOP_OK.c_str());
    resultReceiver_ = STRING_FORCE_STOP_OK + "\n";
    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::RunAsTestCommand()
{
    std::map<std::string, std::string> params;
    bool isDebug = false;

    ErrCode result = ParseTestArgsFromCmd(params, isDebug);
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(ABILITY_TOOL_HELP_MSG_TEST);
        return result;
    }

    if (aaShellCmd_.get() == nullptr) {
        HILOG_ERROR("aa shell command is nullptr.");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (!aaShellCmd_.get()->IsTestCommandIntegrity(params)) {
        HILOG_ERROR("test command lack of essential args.");
        resultReceiver_ = "error: lack of essential args.\n";
        resultReceiver_.append(ABILITY_TOOL_HELP_MSG_TEST);
        return OHOS::ERR_INVALID_VALUE;
    }
    return aaShellCmd_.get()->StartUserTest(params, isDebug);
}

ErrCode AbilityToolCommand::ParseStartAbilityArgsFromCmd(Want& want, StartOptions& startoptions)
{
    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string key = "";
    std::string value = "";
    int32_t windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    int flags = 0;
    bool isColdStart = false;
    bool isDebugApp = false;
    int option = -1;
    int index = 0;
    const std::string shortOptions = "hd:a:b:o:f:CD";
    const struct option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"device", required_argument, nullptr, 'd'},
        {"ability", required_argument, nullptr, 'a'},
        {"bundle", required_argument, nullptr, 'b'},
        {"options", required_argument, nullptr, 'o'},
        {"flags", required_argument, nullptr, 'f'},
        {"cold-start", no_argument, nullptr, 'C'},
        {"debug", no_argument, nullptr, 'D'},
        {nullptr, 0, nullptr, 0},
    };

    while ((option = getopt_long(argc_, argv_, shortOptions.c_str(), longOptions, &index)) != EOF) {
        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        switch (option) {
            case 'h':
                break;
            case 'd':
                deviceId = optarg;
                break;
            case 'a':
                abilityName = optarg;
                break;
            case 'b':
                bundleName = optarg;
                break;
            case 'o':
                if (optind + 1 >= argc_) {
                    HILOG_DEBUG("'aa %{public}s' %{public}s", cmd_.c_str(), ABILITY_TOOL_HELP_MSG_LACK_VALUE.c_str());
                    resultReceiver_.append(ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n");
                    return OHOS::ERR_INVALID_VALUE;
                }
                key = optarg;
                value = argv_[optind + 1];
                if (key == "windowMode") {
                    windowMode = std::stoi(value);
                }
                break;
            case 'f':
                flags = std::stoi(optarg);
                break;
            case 'C':
                isColdStart = true;
                break;
            case 'D':
                isDebugApp = true;
                break;
            default:
                break;
        }
    }

    // Parameter check
    if (abilityName.size() == 0 || bundleName.size() == 0) {
        HILOG_DEBUG("'aa %{public}s' without enough options.", cmd_.c_str());
        if (abilityName.size() == 0) {
            resultReceiver_.append(ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n");
        }

        if (bundleName.size() == 0) {
            resultReceiver_.append(ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
        }

        return OHOS::ERR_INVALID_VALUE;
    }

    // Get Want
    ElementName element(deviceId, bundleName, abilityName);
    want.SetElement(element);

    WantParams wantParams;
    if (isColdStart) {
        wantParams.SetParam("coldStart", Boolean::Box(isColdStart));
    }
    if (isDebugApp) {
        wantParams.SetParam("debugApp", Boolean::Box(isDebugApp));
    }
    want.SetParams(wantParams);

    if (flags != 0) {
        want.AddFlags(flags);
    }

    // Get StartOptions
    if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED) {
        if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN &&
            windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY &&
            windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY &&
            windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
            HILOG_DEBUG("'aa %{public}s' %{public}s", cmd_.c_str(), ABILITY_TOOL_HELP_MSG_WINDOW_MODE_INVALID.c_str());
            resultReceiver_.append(ABILITY_TOOL_HELP_MSG_WINDOW_MODE_INVALID + "\n");
            return OHOS::ERR_INVALID_VALUE;
        }
        startoptions.SetWindowMode(windowMode);
    }

    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::ParseStopServiceArgsFromCmd(Want& want)
{
    std::string deviceId = "";
    std::string abilityName = "";
    std::string bundleName = "";
    int option = -1;
    int index = 0;
    const std::string shortOptions = "hd:a:b:";
    const struct option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"device", required_argument, nullptr, 'd'},
        {"ability", required_argument, nullptr, 'a'},
        {"bundle", required_argument, nullptr, 'b'},
        {nullptr, 0, nullptr, 0},
    };

    while ((option = getopt_long(argc_, argv_, shortOptions.c_str(), longOptions, &index)) != EOF) {
        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        switch (option) {
            case 'h':
                break;
            case 'd':
                deviceId = optarg;
                break;
            case 'a':
                abilityName = optarg;
                break;
            case 'b':
                bundleName = optarg;
                break;
            default:
                break;
        }
    }

    if (abilityName.size() == 0 || bundleName.size() == 0) {
        HILOG_INFO("'aa %{public}s' without enough options.", cmd_.c_str());
        if (abilityName.size() == 0) {
            resultReceiver_.append(ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n");
        }

        if (bundleName.size() == 0) {
            resultReceiver_.append(ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
        }

        return OHOS::ERR_INVALID_VALUE;
    }

    ElementName element(deviceId, bundleName, abilityName);
    want.SetElement(element);
    return OHOS::ERR_OK;
}

ErrCode AbilityToolCommand::ParseTestArgsFromCmd(std::map<std::string, std::string>& params, bool isDebug)
{
    std::string key;
    std::string value;
    std::smatch sm;
    int option = -1;
    int index = 0;
    const std::string shortOptions = "hb:o:p:m:w:D";
    const struct option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"bundle", required_argument, nullptr, 'b'},
        {"options", required_argument, nullptr, 'o'},
        {"package-name", required_argument, nullptr, 'p'},
        {"module-name", required_argument, nullptr, 'm'},
        {"watchdog", required_argument, nullptr, 'w'},
        {"debug", no_argument, nullptr, 'D'},
        {nullptr, 0, nullptr, 0},
    };

    // Parameter parse with conversion
    while ((option = getopt_long(argc_, argv_, shortOptions.c_str(), longOptions, &index)) != EOF) {
        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        switch (option) {
            case 'h':
                break;
            case 'b':
                params["-b"] = optarg;
                break;
            case 'o':
                if (optind + 1 >= argc_) {
                    HILOG_DEBUG("'aa %{public}s' %{public}s", cmd_.c_str(), ABILITY_TOOL_HELP_MSG_LACK_VALUE.c_str());
                    resultReceiver_.append(ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n");
                    return OHOS::ERR_INVALID_VALUE;
                }
                key = "-s ";
                key.append(optarg);
                params[key] = argv_[optind + 1];
                break;
            case 'p':
                params["-p"] = optarg;
                break;
            case 'm':
                params["-m"] = optarg;
                break;
            case 'w':
                value = optarg;
                if (!(std::regex_match(value, sm, std::regex(STRING_TEST_REGEX_INTEGER_NUMBERS)))) {
                    HILOG_DEBUG("'aa test --watchdog %{public}s", ABILITY_TOOL_HELP_MSG_ONLY_NUM.c_str());
                    resultReceiver_.append(ABILITY_TOOL_HELP_MSG_ONLY_NUM + "\n");
                    return OHOS::ERR_INVALID_VALUE;
                }
                params["-w"] = value;
                break;
            case 'D':
                isDebug = true;
                params["-D"] = "true";
                break;
            default:
                break;
        }
    }

    return OHOS::ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS