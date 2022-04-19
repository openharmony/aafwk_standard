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

#include "util/string_pool.h"
#include "securec.h"
#include "util/logger.h"

namespace OHOS {
namespace Idl {
const char* StringPool::TAG = "StringPool";

StringPool::StringPool()
{
    data_ = reinterpret_cast<char*>(calloc(1, dataCapacity_));
    if (data_ == nullptr) {
        Logger::E(TAG, "Out of memory.");
    }
}

StringPool::~StringPool()
{
    if (data_ != nullptr) {
        free(data_);
    }
}

void StringPool::Add(const String& string)
{
    if (string.IsEmpty() || stringOffsets_.find(string) != stringOffsets_.end()) {
        return;
    }

    ptrdiff_t offset = AddInternal(string);
    if (offset != -1) {
        stringOffsets_[string] = offset;
    }
}

ptrdiff_t StringPool::GetOffset(const String& string)
{
    return stringOffsets_[string];
}

ptrdiff_t StringPool::AddInternal(const String& string)
{
    if (!Grow(string.GetLength() + 1)) {
        return -1;
    }

    char* addr = data_ + dataOffset_;
    if (strcpy_s(addr, dataCapacity_ - dataOffset_, string.string())) {
        Logger::E(TAG, "Error to copy str");
        return -1;
    }
    dataOffset_ += string.GetLength() + 1;
    return addr - data_;
}

bool StringPool::Grow(size_t expand)
{
    size_t newSize = dataOffset_ + expand;
    if (newSize < dataCapacity_) {
        return true;
    }
    // 3->3x capacity expansion
    size_t step = dataCapacity_ * 3;
    newSize = step > newSize ? step : step + newSize;
    char* newData = reinterpret_cast<char*>(calloc(1, newSize));
    if (newData == nullptr) {
        Logger::E(TAG, "Out of memory.");
        return false;
    }
    errno_t ret = memcpy_s(newData, newSize, data_, dataOffset_);
    if (ret != EOK) {
        free(newData);
        newData = nullptr;
        return false;
    }
    free(data_);
    data_ = newData;
    dataCapacity_ = newSize;
    return true;
}
}
}
