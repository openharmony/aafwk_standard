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

#ifndef OHOS_IDL_STRING_H
#define OHOS_IDL_STRING_H

#include <cstddef>

namespace OHOS {
namespace Idl {

class String {
public:
    String()
    {}

    String(
        /* [in] */ const char* string);

    String(
        /* [in] */ const char* string,
        /* [in] */ size_t length);

    String(
        /* [in] */ const String& other);

    String(
        /* [in] */ String&& other);

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

    char operator[](
        /* [in] */ int index) const;

    bool Equals(
        /* [in] */ const char* string) const;

    bool Equals(
        /* [in] */ const String& other) const;

    int GetHashCode() const;

    int IndexOf(
        /* [in] */ char c,
        /* [in] */ int fromIndex = 0) const;

    int IndexOf(
        /* [in] */ const char* string,
        /* [in] */ int fromIndex = 0) const;

    int IndexOf(
        /* [in] */ const String& other,
        /* [in] */ int fromIndex = 0) const;

    int LastIndexOf(
        /* [in] */ char c,
        /* [in] */ int fromIndex = 0) const;

    int LastIndexOf(
        /* [in] */ const char* string,
        /* [in] */ int fromIndex = 0) const;

    int LastIndexOf(
        /* [in] */ const String& other,
        /* [in] */ int fromIndex = 0) const;

    bool StartsWith(
        /* [in] */ const char* string) const;

    bool StartsWith(
        /* [in] */ const String& other) const;

    bool EndsWith(
        /* [in] */ const char* string) const;

    bool EndsWith(
        /* [in] */ const String& other) const;

    String ToLowerCase() const;

    String ToUpperCase() const;

    String Substring(
        /* [in] */ int begin) const;

    String Substring(
        /* [in] */ int begin,
        /* [in] */ int end) const;

    String Replace(
        /* [in] */ char oldChar,
        /* [in] */ char newChar) const;

    String Replace(
        /* [in] */ const char* target,
        /* [in] */ const char* replacement) const;

    String Replace(
        /* [in] */ const String& target,
        /* [in] */ const String& replacement) const;

    String& operator=(
        /* [in] */ const char* string);

    String& operator=(
        /* [in] */ const String& other);

    String& operator=(
        /* [in] */ String&& other);

    String operator+=(
        /* [in] */ const char* string) const;

    String operator+=(
        /* [in] */ const String& other) const;

    static String Format(
        /* [in] */ const char* format, ...);

    static const char* TAG;
    static constexpr int MAX_SIZE = 262144; // 2^18

private:
    String(
        /* [in] */ int size);

    int LastIndexOfInternal(
        /* [in] */ const char* string,
        /* [in] */ int fromIndex) const;

    char* string_ = nullptr;
};

String operator+(
    /* [in] */ const String& string1,
    /* [in] */ const char* string2)
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
