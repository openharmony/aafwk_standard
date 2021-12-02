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

#ifndef OHOS_AAFWK_INTERFACES_SHARED_MEMORY_H
#define OHOS_AAFWK_INTERFACES_SHARED_MEMORY_H

namespace OHOS {
namespace AAFwk {
class SharedMemory {
public:
    SharedMemory() = default;
    ~SharedMemory() = default;

    static void ReleaseShmId(const int shmId);
    static void* PopSharedMemory(int shmKey, int size);
    static int PushSharedMemory(const void *data, const int size);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_INTERFACES_SHARED_MEMORY_H