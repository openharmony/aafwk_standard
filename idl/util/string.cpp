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

#include "util/string.h"
#include "util/string_builder.h"
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <new>
#include <cstdlib>
#include "securec.h"

#include "util/logger.h"

namespace OHOS {
namespace Idl {
constexpr int LINE_MAX_SIZE = 1024;

using SharedData = struct SharedData {
    SharedData(
        /* [in] */ int refCount,
        /* [in] */ int size)
        : refCount_(refCount), size_(size)
    {}

    static SharedData* Allocate(
        /* [in] */ int size);

    static void AddRef(
        /* [in] */ const void* handle);

    static void Release(
        /* [in] */ const void* handle);

    static char* ToString(
        /* [in] */ SharedData* header)
    {
        return reinterpret_cast<char*>(header + 1);
    }

    static SharedData* GetHeader(
        /* [in] */ const void* handle)
    {
        return reinterpret_cast<SharedData*>(const_cast<void*>(handle)) - 1;
    }

    std::atomic<int> refCount_;
    int size_;
};

SharedData* SharedData::Allocate(
    /* [in] */ int size)
{
    if (size < 0) {
        Logger::E(String::TAG, "Size %d is illegal.", size);
        return nullptr;
    }
    if (size > String::MAX_SIZE) {
        Logger::E(String::TAG, "The string is too large to alloc.");
        return nullptr;
    }

    SharedData* handle = reinterpret_cast<SharedData*>(malloc(sizeof(SharedData) + size + 1));
    if (handle == nullptr) {
        Logger::E(String::TAG, "Fail to malloc %lu bytes memory", size);
        return handle;
    }

    new (handle)SharedData(1, size);
    return handle;
}

void SharedData::AddRef(
    /* [in] */ const void* handle)
{
    if (handle == nullptr) {
        return;
    }

    SharedData* data = GetHeader(handle);
    int before = data->refCount_.fetch_add(1);
    if (before + 1 <= 1) {
        Logger::E(String::TAG, "The refCount %d of %p is error in AddRef.", before, data);
    };
}

void SharedData::Release(
    /* [in] */ const void* handle)
{
    if (handle == nullptr) {
        return;
    }

    SharedData* data = GetHeader(handle);
    int before = data->refCount_.fetch_sub(1);
    if (before - 1 == 0) {
        free(data);
    } else if (before - 1 < 0) {
        Logger::E(String::TAG, "The refCount %d of %p is error in Release.", before - 1, data);
    };
}


const char* String::TAG = "String";

String::String(
    /* [in] */ const char* string)
{
    if (string != nullptr) {
        string_ = SharedData::ToString(SharedData::Allocate(strlen(string)));
        if (string_ != nullptr) {
            (void)strcpy_s(string_, strlen(string) + 1, string);
        }
    }
}

String::String(
    /* [in] */ const char* string,
    /* [in] */ size_t length)
{
    if (string !=  nullptr) {
        string_ = SharedData::ToString(SharedData::Allocate(length));
        if (string_ != nullptr) {
            errno_t ret = memcpy_s(string_, length + 1, string, length);
            if (ret == EOK) {
                string_[length] = '\0';
            } else {
                free(string_);
                string_ = nullptr;
            }
        }
    }
}

String::String(
    /* [in] */ const String& other)
{
    string_ = other.string_;
    SharedData::AddRef(string_);
}

String::String(
    /* [in] */ String&& other)
{
    string_ = other.string_;
    other.string_ = nullptr;
}

String::String(
    /* [in] */ int size)
{
    string_ = SharedData::ToString(SharedData::Allocate(size));
    if (string_ != nullptr) {
        (void)memset_s(string_, size + 1, 0, size + 1);
    }
}

String::~String()
{
    SharedData::Release(string_);
}

int String::GetLength() const
{
    if (string_ == nullptr) {
        return 0;
    }

    return SharedData::GetHeader(string_)->size_;
}

char String::operator[](
    /* [in] */ int index) const
{
    if (index < 0 || index >= GetLength()) {
        return '\0';
    }
    return string_[index];
}

bool String::Equals(
    /* [in] */ const char* string) const
{
    if (string_ == nullptr && string == nullptr) {
        return true;
    }

    if (string != nullptr && string_ != nullptr) {
        if ((size_t)GetLength() != strlen(string)) {
            return false;
        }
        return strcmp(string, string_) == 0;
    }

    return false;
}

bool String::Equals(
    /* [in] */ const String& other) const
{
    if (string_ == nullptr && other.string_ == nullptr) {
        return true;
    }

    if (string_ != nullptr && other.string_ != nullptr) {
        if (GetLength() != other.GetLength()) {
            return false;
        }
        return strcmp(string_, other.string_) == 0;
    }
    return false;
}

int String::GetHashCode() const
{
    // BKDR Hash Function
    unsigned int seed = 31; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    const char* string = string_;
    if (string != nullptr) {
        for ( ; *string; ++string) {
            hash = hash * seed + (*string);
        }
    }
    return (hash & 0x7FFFFFFF);
}

int String::IndexOf(
    /* [in] */ char c,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || c == '\0') {
        return -1;
    }

    if (fromIndex < 0) {
        fromIndex = 0;
    } else if (fromIndex >= GetLength()) {
        return -1;
    }

    char* p = string_ + fromIndex;
    char* end = string_ + GetLength();
    while (p != end) {
        if (*p == c) {
            return p - string_;
        }
        p++;
    }
    return -1;
}

int String::IndexOf(
    /* [in] */ const char* string,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || string == nullptr || string[0] == '\0') {
        return -1;
    }

    if (fromIndex < 0) {
        fromIndex = 0;
    } else if (fromIndex >= GetLength()) {
        return -1;
    }

    char* c = strstr(string_ + fromIndex, string);
    return c != nullptr ? c - string_ : -1;
}

int String::IndexOf(
    /* [in] */ const String& other,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || other.IsEmpty()) {
        return -1;
    }

    if (fromIndex < 0) {
        fromIndex = 0;
    } else if (fromIndex >= GetLength()) {
        return -1;
    }

    char* c = strstr(string_ + fromIndex, other.string_);
    return c != nullptr ? c - string_ : -1;
}

int String::LastIndexOf(
    /* [in] */ char c,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || c == '\0') {
        return -1;
    }

    if (fromIndex < 0) {
        return -1;
    } else if (fromIndex == 0 || fromIndex >= GetLength()) {
        fromIndex = GetLength() - 1;
    }
    char* p = string_ + fromIndex;
    while (p != string_) {
        if (*p == c) {
            return p - string_;
        }
        p--;
    }
    return -1;
}

int String::LastIndexOf(
    /* [in] */ const char* string,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || string == nullptr || string[0] == '\0') {
        return -1;
    }

    if (fromIndex < 0) {
        return -1;
    } else if (fromIndex == 0 || fromIndex >= GetLength()) {
        fromIndex = GetLength() - 1;
    }

    return LastIndexOfInternal(string, fromIndex);
}

int String::LastIndexOf(
    /* [in] */ const String& other,
    /* [in] */ int fromIndex) const
{
    if (IsEmpty() || other.IsEmpty()) {
        return -1;
    }

    if (fromIndex < 0) {
        return -1;
    } else if (fromIndex == 0 || fromIndex >= GetLength()) {
        fromIndex = GetLength() - 1;
    }

    return LastIndexOfInternal(other.string(), fromIndex);
}

int String::LastIndexOfInternal(
    /* [in] */ const char* string,
    /* [in] */ int fromIndex) const
{
    int sourceLen = GetLength();
    int stringLen = strlen(string);
    int rightIndex = sourceLen - stringLen;
    if (fromIndex > rightIndex) {
        fromIndex = rightIndex;
    }

    int stringLastIndex = stringLen - 1;
    char stringLastChar = string[stringLastIndex];
    int min = stringLen - 1;
    int i = min + fromIndex;

startSearchLastChar:
    while (true) {
        while (i >= min && string_[i] != stringLastChar) {
            i--;
        }
        if (i < min) {
            return -1;
        }
        int j = i - 1;
        int start = j - (stringLen - 1);
        int k = stringLastIndex - 1;

        while (j > start) {
            if (string_[j--] != string[k--]) {
                i--;
                goto startSearchLastChar;
            }
        }
        return start + 1;
    }
}

bool String::StartsWith(
    /* [in] */ const char* string) const
{
    if (string == nullptr || string_ == nullptr) {
        return false;
    }

    if (string[0] == '\0' && string_[0] == '\0') {
        return true;
    }

    size_t count = strlen(string);
    if (count > (size_t)GetLength()) {
        return false;
    }

    return memcmp(string_, string, count) == 0;
}

bool String::StartsWith(
    /* [in] */ const String& other) const
{
    if (other.string_ == nullptr || string_ == nullptr) {
        return false;
    }

    if (other.string_[0] == '\0' && string_[0] == '\0') {
        return true;
    }

    size_t count = other.GetLength();
    if (count > (size_t)GetLength()) {
        return false;
    }

    return memcmp(string_, other.string_, count) == 0;
}

bool String::EndsWith(
    /* [in] */ const char* string) const
{
    if (string == nullptr || string_ == nullptr) {
        return false;
    }

    if (string[0] == '\0') {
        return true;
    }

    size_t count = strlen(string);
    size_t len = GetLength();
    if (count > len) {
        return false;
    }

    return memcmp(string_ + len - count, string, count) == 0;
}

bool String::EndsWith(
    /* [in] */ const String& other) const
{
    if (other.string_ == nullptr || string_ == nullptr) {
        return false;
    }

    if (other.string_[0] == '\0') {
        return true;
    }

    size_t count = other.GetLength();
    size_t len = GetLength();
    if (count > len) {
        return false;
    }

    return memcmp(string_ + len - count, other.string_, count) == 0;
}

String String::ToLowerCase() const
{
    if (IsEmpty()) {
        return *this;
    }

    size_t size = GetLength();
    for (size_t i = 0; i < size; i++) {
        if (isupper(string_[i])) {
            String newStr(string_);
            for (size_t j = i; j < size; j++) {
                newStr.string_[j] = tolower(newStr.string_[j]);
            }
            return newStr;
        }
    }
    return *this;
}

String String::ToUpperCase() const
{
    if (IsEmpty()) {
        return *this;
    }

    size_t size = GetLength();
    for (size_t i = 0; i < size; i++) {
        if (islower(string_[i])) {
            String newStr(string_);
            for (size_t j = i; j < size; j++) {
                newStr.string_[j] = toupper(newStr.string_[j]);
            }
            return newStr;
        }
    }
    return *this;
}

String String::Substring(
    /* [in] */ int begin) const
{
    if (begin < 0 || begin >= GetLength()) {
        return String();
    }

    return String(string_ + begin);
}

String String::Substring(
    /* [in] */ int begin,
    /* [in] */ int end) const
{
    if (begin < 0 || end > GetLength() || begin > end) {
        return String();
    }

    return String(string_ + begin, end - begin);
}

String String::Replace(
    /* [in] */ char oldChar,
    /* [in] */ char newChar) const
{
    if (oldChar == newChar) {
        return *this;
    }

    size_t size = GetLength();
    for (size_t i = 0; i < size; i++) {
        if (string_[i] == oldChar) {
            String newStr(string_);
            for (size_t j = i; j < size; j++) {
                if (newStr.string_[j] == oldChar) {
                    newStr.string_[j] = newChar;
                }
            }
            return newStr;
        }
    }
    return *this;
}

String String::Replace(
    /* [in] */ const char* target,
    /* [in] */ const char* replacement) const
{
    if (target == nullptr || target[0] == '\0' || replacement == nullptr) {
        return *this;
    }

    int index = IndexOf(target);
    if (index == -1) {
        return *this;
    }

    StringBuilder sb;
    int begin = 0;
    int step = strlen(target);
    while (index != -1) {
        sb.Append(Substring(begin, index));
        sb.Append(replacement);
        begin = index + step;
        index = IndexOf(target, begin);
    }
    sb.Append(Substring(begin));
    return sb.ToString();
}

String String::Replace(
    /* [in] */ const String& target,
    /* [in] */ const String& replacement) const
{
    if (target.IsEmpty() || replacement.IsNull()) {
        return *this;
    }

    int index = IndexOf(target);
    if (index== -1) {
        return *this;
    }

    StringBuilder sb;
    int begin = 0;
    int step = target.GetLength();
    while (index != -1) {
        sb.Append(Substring(begin, index));
        sb.Append(replacement);
        begin = index + step;
        index = IndexOf(target, begin);
    }
    sb.Append(Substring(begin));
    return sb.ToString();
}

String& String::operator=(
    /* [in] */ const char* string)
{
    SharedData::Release(string_);

    if (string == nullptr) {
        string_ = nullptr;
        return *this;
    }

    string_ = SharedData::ToString(SharedData::Allocate(strlen(string)));
    if (string_ != nullptr) {
        (void)strcpy_s(string_, strlen(string) + 1, string);
    }
    return *this;
}

String& String::operator=(
    /* [in] */ const String& other)
{
    if (string_ == other.string_) {
        return *this;
    }

    SharedData::Release(string_);
    SharedData::AddRef(other.string_);
    string_ = other.string_;
    return *this;
}

String& String::operator=(
    /* [in] */ String&& other)
{
    SharedData::Release(string_);
    string_ = other.string_;
    other.string_ = nullptr;
    return *this;
}

String String::operator+=(
    /* [in] */ const char* string) const
{
    if (string == nullptr || string[0] == '\0') {
        return *this;
    }

    int thisSize = GetLength();
    int newSize = thisSize + strlen(string);
    String newString(newSize);
    if (newString.string_ != nullptr) {
        (void)memcpy_s(newString.string_, newSize + 1, string_, thisSize);
        (void)strcpy_s(newString.string_ + thisSize, newSize + 1 - thisSize,  string);
    }
    return newString;
}

String String::operator+=(
    /* [in] */ const String& other) const
{
    if (other.IsEmpty()) {
        return *this;
    }

    int thisSize = GetLength();
    int newSize = thisSize + other.GetLength();
    String newString(newSize);
    if (newString.string_ != nullptr) {
        (void)memcpy_s(newString.string_, newSize + 1, string_, thisSize);
        (void)strcpy_s(newString.string_ + thisSize, newSize + 1 - thisSize, other.string_);
    }
    return newString;
}

String String::Format(
    /* [in] */ const char* format, ...)
{
    va_list args, argsCopy;

    va_start(args, format);
    va_copy(argsCopy, args);

    char buf[LINE_MAX_SIZE] = {0};
    int len = vsnprintf_s(buf, LINE_MAX_SIZE, LINE_MAX_SIZE - 1, format, args);
    String string;
    if (len <= 0) {
        va_end(args);
        va_end(argsCopy);
        return string;
    }

    string = String(len);
    if (string.string_ == nullptr) {
        va_end(args);
        va_end(argsCopy);
        return string;
    }

    if (vsnprintf_s(string.string_, len + 1, len, format, argsCopy) < 0) {
        va_end(args);
        va_end(argsCopy);
        return string;
    }

    va_end(args);
    va_end(argsCopy);
    return string;
}

}
}
