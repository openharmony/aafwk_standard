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

#include "app_spawn_socket.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
// arg "AppSpawn" or "NWebSpawn" cannot be defined as string object since REGISTER_SYSTEM_ABILITY will
// firstly start without init this string object, which leads to error.

AppSpawnSocket::AppSpawnSocket(bool isNWebSpawn)
{
    clientSocket_ = isNWebSpawn ?
        std::make_unique<AppSpawn::ClientSocket>("/dev/unix/socket/NWebSpawn") :
        std::make_unique<AppSpawn::ClientSocket>("AppSpawn");
}

AppSpawnSocket::~AppSpawnSocket()
{}

ErrCode AppSpawnSocket::OpenAppSpawnConnection()
{
    HILOG_DEBUG("ready to open connection");
    if (clientSocket_) {
        if (clientSocket_->CreateClient() != ERR_OK) {
            HILOG_ERROR("failed to create socketClient");
            return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
        }
        if (clientSocket_->ConnectSocket() != ERR_OK) {
            HILOG_ERROR("failed to connect socket");
            clientSocket_->CloseClient();
            return ERR_APPEXECFWK_CONNECT_APPSPAWN_FAILED;
        }
        HILOG_DEBUG("connection has been opened");
        return ERR_OK;
    }
    HILOG_ERROR("failed to open connection without socket");
    return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
}

void AppSpawnSocket::CloseAppSpawnConnection()
{
    if (clientSocket_) {
        clientSocket_->CloseClient();
    }
}

ErrCode AppSpawnSocket::WriteMessage(const void *buf, const int32_t len)
{
    HILOG_DEBUG("ready to write message");
    if (len <= 0) {
        HILOG_ERROR("failed to write message due to invalid length of message");
        return ERR_INVALID_VALUE;
    }
    if (buf == nullptr) {
        HILOG_ERROR("failed to write message due to null buf");
        return ERR_INVALID_VALUE;
    }
    if (clientSocket_) {
        if (clientSocket_->WriteSocketMessage(buf, len) != len) {
            HILOG_ERROR("failed to write message due to invalid write length");
            return ERR_APPEXECFWK_SOCKET_WRITE_FAILED;
        }
        HILOG_DEBUG("write message success");
        return ERR_OK;
    }

    HILOG_ERROR("failed to write message without socket");
    return ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET;
}

ErrCode AppSpawnSocket::ReadMessage(void *buf, const int32_t len)
{
    HILOG_DEBUG("ready to read message");
    if (len <= 0) {
        HILOG_ERROR("failed to read message due to invalid length of cache");
        return ERR_INVALID_VALUE;
    }
    if (buf == nullptr) {
        HILOG_ERROR("failed to read message due to null buf");
        return ERR_INVALID_VALUE;
    }
    if (clientSocket_) {
        if (clientSocket_->ReadSocketMessage(buf, len) != len) {
            HILOG_ERROR("failed to read message due to invalid read length");
            return ERR_APPEXECFWK_SOCKET_READ_FAILED;
        }
        HILOG_DEBUG("read message success");
        return ERR_OK;
    }
    HILOG_ERROR("failed to read message without socket");
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppSpawnSocket::SetClientSocket(const std::shared_ptr<OHOS::AppSpawn::ClientSocket> clientSocket)
{
    clientSocket_ = clientSocket;
}
}  // namespace AppExecFwk
}  // namespace OHOS
