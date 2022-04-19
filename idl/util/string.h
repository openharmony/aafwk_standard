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

#ifndef OHOS_IDL_STRING_H
#define OHOS_IDL_STRING_H

#include <cstddef>

namespace OHOS {
namespace Idl {
class String {
public:
    String() {}

    String(const char* string);

    String(const char* string, size_t length);

    String(const String& other);

    String(String&& other);

    ~String();

    const char* string() const
    {
        return string_;
    }

    operator const char*() const
    {
        return string_;
    }

    bool IsNull() const
    {
        return string_ == nullptr;
    }

    bool IsEmpty() const
    {
        return string_ == nullptr || string_[0] == '\0';
    }

    int GetLength() const;

    char operator[](int index) const;

    bool Equals(const char* string) const;

    bool Equals(const String& other) const;

    int GetHashCode() const;

    int IndexOf(char c, int fromIndex = 0) const;

    int IndexOf(const char* string, int fromIndex = 0) const;

    int IndexOf(const String& other, int fromIndex = 0) const;

    int LastIndexOf(char c, int fromIndex = 0) const;

    int LastIndexOf(const char* string, int fromIndex = 0) const;

    int LastIndexOf(const String& other, int fromIndex = 0) const;

    bool StartsWith(const char* string) const;

    bool StartsWith(const String& other) const;

    bool EndsWith(const char* string) const;

    bool EndsWith(const String& other) const;

    String ToLowerCase() const;

    String ToUpperCase() const;

    String Substring(int begin) const;

    String Substring(int begin, int end) const;

    String Replace(char oldChar, char newChar) const;

    String Replace(const char* target, const char* replacement) const;

    String Replace(const String& target, const String& replacement) const;

    String& operator=(const char* string);

    String& operator=(const String& other);

    String& operator=(String&& other);

    String operator+=(const char* string) const;

    String operator+=(const String& other) const;

    static String Format(const char* format, ...);

    static const char* TAG;
    static constexpr int MAX_SIZE = 262144; // 2^18

private:
    String(int size);

    int LastIndexOfInternal(const char* string, int fromIndex) const;

    char* string_ = nullptr;
};

inline String operator+(const String& string1, const char* string2)
{
    return string1 += string2;
}

struct StringHashFunc {
    int operator()(const String& key) const
    {
        return key.GetHashCode();
    }
};

struct StringEqualFunc {
    bool operator()(const String& lhs, const String& rhs) const
    {
        return lhs.Equals(rhs);
    }
};
}
}
#endif // OHOS_IDL_STRING_H
