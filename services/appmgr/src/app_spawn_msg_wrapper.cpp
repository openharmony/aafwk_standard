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

#include "app_spawn_msg_wrapper.h"

#include "securec.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppSpawnMsgWrapper::~AppSpawnMsgWrapper()
{
    FreeMsg();
}

bool AppSpawnMsgWrapper::AssembleMsg(const AppSpawnStartMsg &startMsg)
{
    if (!VerifyMsg(startMsg)) {
        return false;
    }
    FreeMsg();
    int32_t msgSize = sizeof(AppSpawnMsg) + 1;
    msg_ = static_cast<AppSpawnMsg *>(malloc(msgSize));
    if (msg_ == nullptr) {
        HILOG_ERROR("failed to malloc!");
        return false;
    }
    if (memset_s(msg_, msgSize, 0, msgSize) != EOK) {
        HILOG_ERROR("failed to memset!");
        return false;
    }
    msg_->uid = startMsg.uid;
    msg_->gid = startMsg.gid;
    msg_->gidCount = startMsg.gids.size();
    for (uint32_t i = 0; i < msg_->gidCount; ++i) {
        msg_->gidTable[i] = startMsg.gids[i];
    }
    if (strcpy_s(msg_->processName, sizeof(msg_->processName), startMsg.procName.c_str()) != EOK) {
        HILOG_ERROR("failed to transform procName!");
        return false;
    }
    if (strcpy_s(msg_->soPath, sizeof(msg_->soPath), startMsg.soPath.c_str()) != EOK) {
        HILOG_ERROR("failed to transform soPath!");
        return false;
    }
    msg_->accessTokenId = startMsg.accessTokenId;
    if (strcpy_s(msg_->apl, sizeof(msg_->apl), startMsg.apl.c_str()) != EOK) {
        HILOG_ERROR("failed to transform apl!");
        return false;
    }
    if (strcpy_s(msg_->bundleName, sizeof(msg_->bundleName), startMsg.bundleName.c_str()) != EOK) {
        HILOG_ERROR("failed to transform bundleName!");
        return false;
    }

    if (strcpy_s(msg_->renderCmd, sizeof(msg_->renderCmd), startMsg.renderParam.c_str()) != EOK) {
        HILOG_ERROR("failed to transform renderCmd!");
        return false;
    }
    msg_->flags = startMsg.coldStart ? AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT : 0;

    isValid_ = true;
    DumpMsg();
    return isValid_;
}

bool AppSpawnMsgWrapper::VerifyMsg(const AppSpawnStartMsg &startMsg) const
{
    if (startMsg.uid < 0) {
        HILOG_ERROR("invalid uid! [%{public}d]", startMsg.uid);
        return false;
    }

    if (startMsg.gid < 0) {
        HILOG_ERROR("invalid gid! [%{public}d]", startMsg.gid);
        return false;
    }

    if (startMsg.gids.size() > AppSpawn::ClientSocket::MAX_GIDS) {
        HILOG_ERROR("too many app gids!");
        return false;
    }

    for (uint32_t i = 0; i < startMsg.gids.size(); ++i) {
        if (startMsg.gids[i] < 0) {
            HILOG_ERROR("invalid gids array! [%{public}d]", startMsg.gids[i]);
            return false;
        }
    }

    if (startMsg.procName.empty() || startMsg.procName.size() >= AppSpawn::ClientSocket::LEN_PROC_NAME) {
        HILOG_ERROR("invalid procName!");
        return false;
    }

    return true;
}

void AppSpawnMsgWrapper::DumpMsg() const
{
    if (!isValid_) {
        return;
    }
    HILOG_INFO("************AppSpawnMsg*************");
    HILOG_INFO("uid: %{public}d", msg_->uid);
    HILOG_INFO("gid: %{public}d", msg_->gid);
    for (uint32_t i = 0; i < msg_->gidCount; ++i) {
        HILOG_INFO("gidTable[%{public}d]: %{public}d", i, msg_->gidTable[i]);
    }
    HILOG_INFO("procName: %{public}s", msg_->processName);
    HILOG_INFO("soPath: %{private}s", msg_->soPath);
    HILOG_INFO("************************************");
}

void AppSpawnMsgWrapper::FreeMsg()
{
    if (msg_ != nullptr) {
        free(msg_);
        msg_ = nullptr;
        isValid_ = false;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
