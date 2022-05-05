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

#include "mock_test_observer_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
MockTestObserverStub::~MockTestObserverStub()
{}
    
int MockTestObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

void MockTestObserverStub::TestStatus(const std::string &msg, const int64_t &resultCode)
{
    HILOG_INFO("MockTestObserverStub::TestStatus is called");

    testStatusFlag = true;
}

void MockTestObserverStub::TestFinished(const std::string &msg, const int64_t &resultCode)
{}

ShellCommandResult MockTestObserverStub::ExecuteShellCommand(
    const std::string &cmd, const int64_t timeoutMs)
{
    return ShellCommandResult();
}
}  // namespace AAFwk
}  // namespace OHOS