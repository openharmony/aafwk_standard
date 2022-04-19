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

#ifndef OHOS_IDL_FILE_H
#define OHOS_IDL_FILE_H

#include <cstddef>
#include <cstdio>

#include "util/string.h"

namespace OHOS {
namespace Idl {
class File {
public:
    File(const String& path, int mode);

    ~File();

    bool IsValid()
    {
        return fd_ != nullptr;
    }

    String GetPath()
    {
        return path_;
    }

    char GetChar();

    char PeekChar();

    bool IsEof() const;

    int GetCharLineNumber() const
    {
        return lineNo_;
    }

    int GetCharColumnNumber() const
    {
        return columnNo_;
    }

    bool ReadData(void* data, size_t size);

    bool WriteData(const void* data, size_t size);

    void Flush();

    bool Reset();

    bool Skip(long size);

    void Close();

    static constexpr unsigned int READ = 0x1;
    static constexpr unsigned int WRITE = 0x2;
    static constexpr unsigned int APPEND = 0x4;

private:
    int Read();

    static constexpr int BUFFER_SIZE = 1024;

    char buffer_[BUFFER_SIZE] = {0};
    size_t size_ = 0;
    size_t position_ = 0;
    size_t columnNo_ = 1;
    size_t lineNo_ = 1;
    bool isEof_ = false;
    bool isError_ = false;

    FILE* fd_ = nullptr;
    String path_;
    unsigned int mode_ = 0;
};
}
}
#endif // OHOS_IDL_STRING_H
