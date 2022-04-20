/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <string>
#include <vector>

#include "ipc_debug.h"
#include "ipc_skeleton.h"
#include "log_tags.h"
#include "test_client.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_ID_IPC, "IPCTestClient" };

enum class TestCommand {
    TEST_CMD_NONE = 0,
    TEST_CMD_INT_TRANS = 1,
    TEST_CMD_STRING_TRANS = 2
};

namespace {
std::vector<std::string> GetArgvOptions(int argc, char **argv)
{
    std::vector<std::string> argvOptions;
    for (int i = 1; i < argc; i++) {
        argvOptions.emplace_back(std::string(argv[i]));
    }
    return argvOptions;
}
}

int main(int argc, char *argv[])
{
    TestCommand commandId = TestCommand::TEST_CMD_INT_TRANS;
    if (argc > 1) {
        commandId = TestCommand(atoi(argv[1]));
    } else {
        ZLOGE(LABEL, "unknown command");
    }
    std::vector<std::string> argvOptions;
    argvOptions = GetArgvOptions(argc, argv);
    std::unique_ptr<TestClient> testClient = std::make_unique<TestClient>();
    if (testClient->ConnectService()) {
        return -1;
    }

    ZLOGE(LABEL, "commandId= : %{public}d", commandId);
    switch (commandId) {
        case TestCommand::TEST_CMD_INT_TRANS:
            testClient->StartIntTransaction();
            break;
        case TestCommand::TEST_CMD_STRING_TRANS:
            testClient->StartStringTransaction();
            break;
        default:
            ZLOGI(LABEL, "main arg error");
            break;
    }

    IPCSkeleton::JoinWorkThread();
    return 0;
}
