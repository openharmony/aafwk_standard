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

#include "shared_memory.h"
#include <cstdlib>
#include <sys/shm.h>
#include <cerrno>
#include <cstring>
#include "hilog_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int SHM_KEY_START = 400000; // chosen randomly
constexpr int SHM_KEY_END   = 500000; // chosen randomly
constexpr unsigned int SHM_READ_WRITE_PERMISSIONS = 0666U;

#ifndef EOK
#define EOK (0)
#endif
}

void SharedMemory::ReleaseShmId(const int shmId)
{
    if (shmId == -1) {
        return;
    }
    if (shmctl(shmId, IPC_RMID, nullptr) == -1) {
        HILOG_ERROR("shmctl IPC_RMID failed: %{public}d.", errno);
        return;
    }
}

int SharedMemory::PushSharedMemory(const void *data, const int size)
{
    if (size < 0) {
        HILOG_ERROR("size less 0");
        return -1;
    }

    static int shmKey = SHM_KEY_START;
    int shmId;
    while ((shmId = shmget(shmKey, size, SHM_READ_WRITE_PERMISSIONS | IPC_CREAT | IPC_EXCL)) < 0) {
        if (errno == EEXIST) {
            ++shmKey;
            if (shmKey >= SHM_KEY_END) {
                shmKey = SHM_KEY_START;
            }
            continue;
        }
        HILOG_ERROR("shmget failed: %{public}d.", errno);
        return -1;
    }
    HILOG_INFO("shmget succeed, shmKey = %{public}d, shmId = %{public}d.", shmKey, shmId);

    void *shared = shmat(shmId, nullptr, 0);
    if (shared == reinterpret_cast<void *>(-1)) {
        ReleaseShmId(shmId);
        HILOG_ERROR("shmat failed: %{public}d.", errno);
        return -1;
    }

    int retCode;
    if ((retCode = memcpy_s(shared, size, data, size)) != EOK) {
        shmdt(shared);
        ReleaseShmId(shmId);
        HILOG_ERROR("memcpy_s failed: %{public}d.", retCode);
        return -1;
    }

    if (shmdt(shared) == -1) {
        ReleaseShmId(shmId);
        HILOG_ERROR("shmdt failed: %{public}d.", errno);
        return -1;
    }

    return shmKey;
}

void* SharedMemory::PopSharedMemory(int shmKey, int size)
{
    void *data = reinterpret_cast<void *>(malloc(size));
    int shmId = shmget(shmKey, 0, 0 | SHM_READ_WRITE_PERMISSIONS);
    if (shmId == -1) {
        HILOG_ERROR("shmId is invalid: %{public}d, %{public}d.", shmId, errno);
        return nullptr;
    }

    void *shared = shmat(shmId, nullptr, 0);
    if (shared == reinterpret_cast<void *>(-1)) {
        HILOG_ERROR("shmat failed %{public}d.", errno);
        ReleaseShmId(shmId);
        return nullptr;
    }

    int retCode = memcpy_s(data, size, shared, size);
    if (retCode != EOK) {
        shmdt(shared);
        ReleaseShmId(shmId);
        HILOG_ERROR("Failed to memory copy, retCode[%{public}d].", retCode);
        return nullptr;
    }

    if (shmdt(shared) == -1) {
        ReleaseShmId(shmId);
        HILOG_ERROR("shmdt failed: %{public}d.", errno);
        return nullptr;
    }

    ReleaseShmId(shmId);

    return data;
}
}  // namespace AAFwk
}  // namespace OHOS