/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "test_observer_proxy.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
TestObserverProxy::TestObserverProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<ITestObserver>(object)
{
    HILOG_INFO("test observer proxy instance is created");
}

TestObserverProxy::~TestObserverProxy()
{
    HILOG_INFO("test observer proxy is destroyed");
}

void TestObserverProxy::TestStatus(const std::string &msg, const int &resultCode)
{
    HILOG_INFO("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Failed to send cmd to service due to remote object is null");
        return;
    }

    if (!data.WriteString(msg)) {
        HILOG_ERROR("Failed to write string msg");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("Failed to write resultCode");
        return;
    }

    int32_t result = remote->SendRequest(
        static_cast<uint32_t>(ITestObserver::Message::AA_TEST_STATUS), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        HILOG_ERROR("Failed to SendRequest, error code: %{public}d", result);
        return;
    }
}

void TestObserverProxy::TestFinished(const std::string &msg, const int &resultCode)
{
    HILOG_INFO("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Failed to send cmd to service due to remote object is null");
        return;
    }

    if (!data.WriteString(msg)) {
        HILOG_ERROR("Failed to write string msg");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("Failed to write resultCode");
        return;
    }

    int32_t result = remote->SendRequest(
        static_cast<uint32_t>(ITestObserver::Message::AA_TEST_FINISHED), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        HILOG_ERROR("Failed to SendRequest, error code: %{public}d", result);
        return;
    }
}

ShellCommandResult TestObserverProxy::ExecuteShellCommand(
    const std::string &cmd, const int64_t timeoutMs)
{
    HILOG_INFO("start");

    ShellCommandResult result;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Failed to send cmd to service due to remote object is null");
        return result;
    }

    if (!data.WriteString(cmd)) {
        HILOG_ERROR("Failed to write string cmd");
        return result;
    }

    if (!data.WriteInt64(timeoutMs)) {
        HILOG_ERROR("Failed to write timeoutMs");
        return result;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(ITestObserver::Message::AA_EXECUTE_SHELL_COMMAND), data, reply, option);
    if (ret != OHOS::NO_ERROR) {
        HILOG_ERROR("Failed to SendRequest, error code: %{public}d", ret);
        return result;
    }
    ShellCommandResult *resultPtr = reply.ReadParcelable<ShellCommandResult>();
    if (!resultPtr) {
        HILOG_ERROR("Failed to read result");
        return result;
    }
    result = *resultPtr;
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS