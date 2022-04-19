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

#ifndef OHOS_IDL_STRINGPOOL_H
#define OHOS_IDL_STRINGPOOL_H

#include <unordered_map>
#include "util/string.h"

namespace OHOS {
namespace Idl {
class StringPool {
public:
    StringPool();

    ~StringPool();

    void Add(const String& string);

    ptrdiff_t GetOffset(const String& string);

    size_t GetSize()
    {
        return dataOffset_;
    }

    char* GetData()
    {
        return data_;
    }

private:
    ptrdiff_t AddInternal(const String& string);

    bool Grow(size_t expand);

    static const char* TAG;
    char* data_ = nullptr;
    size_t dataCapacity_ = 256;
    ptrdiff_t dataOffset_ = 0;
    std::unordered_map<String, ptrdiff_t, StringHashFunc, StringEqualFunc> stringOffsets_;
};
}
}
#endif // OHOS_IDL_STRINGPOOL_H
