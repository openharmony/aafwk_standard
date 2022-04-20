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

#include "app_spawn_client.h"

#include "bytrace.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t CONNECT_RETRY_DELAY = 200 * 1000;  // 200ms
const int32_t CONNECT_RETRY_MAX_TIMES = 2;
}  // namespace

AppSpawnClient::AppSpawnClient(bool isNWebSpawn)
{
    socket_ = std::make_shared<AppSpawnSocket>(isNWebSpawn);
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

ErrCode AppSpawnClient::OpenConnection()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!socket_) {
        HILOG_ERROR("failed to open connection without socket!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    int32_t retryCount = 1;
    ErrCode errCode = socket_->OpenAppSpawnConnection();
    while (FAILED(errCode) && retryCount <= CONNECT_RETRY_MAX_TIMES) {
        HILOG_WARN("failed to OpenConnection, retry times %{public}d ...", retryCount);
        usleep(CONNECT_RETRY_DELAY);
        errCode = socket_->OpenAppSpawnConnection();
        retryCount++;
    }
    if (SUCCEEDED(errCode)) {
        state_ = SpawnConnectionState::STATE_CONNECTED;
    } else {
        HILOG_ERROR("failed to openConnection, errorCode is %{public}08x", errCode);
        state_ = SpawnConnectionState::STATE_CONNECT_FAILED;
    }
    return errCode;
}

ErrCode AppSpawnClient::StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t retryCount = 1;
    ErrCode errCode = StartProcessImpl(startMsg, pid);
    while (FAILED(errCode) && retryCount <= CONNECT_RETRY_MAX_TIMES) {
        HILOG_WARN("failed to StartProcess, retry times %{public}d ...", retryCount);
        usleep(CONNECT_RETRY_DELAY);
        errCode = StartProcessImpl(startMsg, pid);
        retryCount++;
    }
    return errCode;
}

ErrCode AppSpawnClient::StartProcessImpl(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!socket_) {
        HILOG_ERROR("failed to startProcess without socket!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    ErrCode result = ERR_OK;
    // openconnection failed, return fail
    if (state_ != SpawnConnectionState::STATE_CONNECTED) {
        result = OpenConnection();
        if (FAILED(result)) {
            HILOG_ERROR("connect to appspawn failed!");
            return result;
        }
    }
    std::unique_ptr<AppSpawnClient, void (*)(AppSpawnClient *)> autoCloseConnection(
        this, [](AppSpawnClient *client) { client->CloseConnection(); });

    AppSpawnMsgWrapper msgWrapper;
    if (!msgWrapper.AssembleMsg(startMsg)) {
        HILOG_ERROR("AssembleMsg failed!");
        return ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED;
    }
    AppSpawnPidMsg pidMsg;
    if (msgWrapper.IsValid()) {
        result = socket_->WriteMessage(msgWrapper.GetMsgBuf(), msgWrapper.GetMsgLength());
        if (FAILED(result)) {
            HILOG_ERROR("WriteMessage failed!");
            return result;
        }
        result = socket_->ReadMessage(reinterpret_cast<void *>(pidMsg.pidBuf), LEN_PID);
        if (FAILED(result)) {
            HILOG_ERROR("ReadMessage failed!");
            return result;
        }
    }
    if (pidMsg.pid <= 0) {
        HILOG_ERROR("invalid pid!");
        result = ERR_APPEXECFWK_INVALID_PID;
    } else {
        pid = pidMsg.pid;
    }
    return result;
}

ErrCode AppSpawnClient::GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status)
{
    if (!socket_) {
        HILOG_ERROR("socket_ is null!");
        return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
    }

    ErrCode result = ERR_OK;
    // openconnection failed, return fail
    if (state_ != SpawnConnectionState::STATE_CONNECTED) {
        result = OpenConnection();
        if (FAILED(result)) {
            HILOG_ERROR("connect to appspawn failed!");
            return result;
        }
    }
    std::unique_ptr<AppSpawnClient, void (*)(AppSpawnClient *)> autoCloseConnection(
        this, [](AppSpawnClient *client) { client->CloseConnection(); });

    AppSpawnMsgWrapper msgWrapper;
    if (!msgWrapper.AssembleMsg(startMsg)) {
        HILOG_ERROR("AssembleMsg failed!");
        return ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED;
    }
    if (msgWrapper.IsValid()) {
        result = socket_->WriteMessage(msgWrapper.GetMsgBuf(), msgWrapper.GetMsgLength());
        if (FAILED(result)) {
            HILOG_ERROR("WriteMessage failed!");
            return result;
        }
        result = socket_->ReadMessage(reinterpret_cast<void *>(&status), sizeof(int));
        if (FAILED(result)) {
            HILOG_ERROR("ReadMessage failed!");
            return result;
        }
    }
    return result;
}

SpawnConnectionState AppSpawnClient::QueryConnectionState() const
{
    return state_;
}

void AppSpawnClient::CloseConnection()
{
    if (socket_ && state_ == SpawnConnectionState::STATE_CONNECTED) {
        socket_->CloseAppSpawnConnection();
    }
    state_ = SpawnConnectionState::STATE_NOT_CONNECT;
}

void AppSpawnClient::SetSocket(const std::shared_ptr<AppSpawnSocket> socket)
{
    socket_ = socket;
}
}  // namespace AppExecFwk
}  // namespace OHOS
