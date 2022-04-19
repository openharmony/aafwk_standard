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

#ifndef OHOS_IDL_STRINGBUILDER_H
#define OHOS_IDL_STRINGBUILDER_H

#include "util/string.h"

namespace OHOS {
namespace Idl {
class StringBuilder {
public:
    ~StringBuilder();

    StringBuilder& Append(char c);

    StringBuilder& Append(const char* string);

    StringBuilder& Append(const String& string);

    StringBuilder& AppendFormat(const char* format, ...);

    String ToString() const;

private:
    bool Grow(size_t size);

    static const char* TAG;
    char* buffer_ = nullptr;
    size_t position_ = 0;
    size_t capacity_ = 0;
};
}
}
#endif // OHOS_IDL_STRINGBUILDER_H
