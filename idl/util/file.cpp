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

#include "util/file.h"

#include <cstdlib>
#include <cstring>
#include "securec.h"

namespace OHOS {
namespace Idl {
#ifdef __MINGW32__
constexpr unsigned int File::READ;
constexpr unsigned int File::WRITE;
constexpr unsigned int File::APPEND;
#endif

File::File(const String& path, int mode)
    : mode_(mode)
{
    if (path.IsEmpty()) {
        return;
    }

    if (mode_ & READ) {
        fd_ = fopen(path.string(), "r");
    } else if (mode_ & WRITE) {
        fd_ = fopen(path.string(), "w+");
    } else if (mode_ & APPEND) {
        fd_ = fopen(path.string(), "a+");
    }

    if (fd_ != nullptr) {
#ifndef __MINGW32__
        char* absolutePath = realpath(path.string(), nullptr);
        if (absolutePath != nullptr) {
            path_ = absolutePath;
            free(absolutePath);

        } else {
            path_ = path;
        }
#else
        char absolutePath[_MAX_PATH];
        _fullpath(absolutePath, path.string(), _MAX_PATH);
        path_ = absolutePath;
#endif
    }
}

File::~File()
{
    Close();
}

char File::GetChar()
{
    char c = PeekChar();

    if (position_ + 1 <= size_) {
        position_++;

        if (c != '\n') {
            columnNo_++;
        } else {
            columnNo_ = 0;
            lineNo_++;
        }
    }
    return c;
}

char File::PeekChar()
{
    if (position_ + 1 > size_) {
        int ret = Read();
        if (ret == -1) {
            isEof_ = true;
        }
    }

    return buffer_[position_];
}

bool File::IsEof() const
{
    return isEof_ || buffer_[position_] == static_cast<char>(-1);
}

int File::Read()
{
    if (isEof_ || isError_) {
        return -1;
    }

    (void)memset_s(buffer_, BUFFER_SIZE, 0, BUFFER_SIZE);
    size_t count = fread(buffer_, 1, BUFFER_SIZE - 1, fd_);
    if (count < BUFFER_SIZE - 1) {
        isError_ = ferror(fd_) != 0;
        buffer_[count] = -1;
    }
    size_ = count;
    position_ = 0;
    return count != 0 ? count : -1;
}

bool File::ReadData(void* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return true;
    }

    if (fd_ == nullptr) {
        return false;
    }

    size_t count = fread(data, size, 1, fd_);
    return count == 1;
}

bool File::WriteData(const void* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return true;
    }

    if (fd_ == nullptr || !(mode_ & (WRITE | APPEND))) {
        return false;
    }

    size_t count = fwrite(data, size, 1, fd_);
    return count == 1;
}

void File::Flush()
{
    if ((mode_ & (WRITE | APPEND)) && fd_ != nullptr) {
        fflush(fd_);
    }
}

bool File::Reset()
{
    if (fd_ == nullptr) {
        return false;
    }

    return fseek(fd_, 0, SEEK_SET) == 0;
}

bool File::Skip(long size)
{
    if (fd_ == nullptr) {
        return false;
    }

    return fseek(fd_, size, SEEK_CUR) == 0;
}

void File::Close()
{
    if (fd_ != nullptr) {
        fclose(fd_);
        fd_ = nullptr;
    }
}
}
}
