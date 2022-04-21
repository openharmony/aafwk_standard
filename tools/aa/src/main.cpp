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

#include <cstring>

#include "ability_command.h"
#include "ability_tool_command.h"

int main(int argc, char *argv[])
{
    if (strcmp(argv[0], "aa") == 0 || strcmp(argv[0], "/system/bin/aa") == 0) {
        OHOS::AAFwk::AbilityManagerShellCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    } else if (strcmp(argv[0], "ability_tool") == 0 || strcmp(argv[0], "/system/bin/ability_tool") == 0) {
        OHOS::AAFwk::AbilityToolCommand cmd(argc, argv);
        std::cout << cmd.ExecCommand();
    }

    return 0;
}