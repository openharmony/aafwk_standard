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

#ifndef OHOS_IDL_LOGGER_H
#define OHOS_IDL_LOGGER_H

#include <cstdarg>

namespace OHOS {
namespace Idl {
class Logger {
public:
    static void D(const char* tag, const char* format, ...);

    static void E(const char* tag, const char* format, ...);

    static void V(const char* tag, const char* format, ...);

    static void SetLevel(int level)
    {
        level_ = level;
    }

    static constexpr int VERBOSE = 0;
    static constexpr int DEBUG = 1;
    static constexpr int ERROR = 2;
    static constexpr int NOLOG = 3;

private:
    Logger();

    ~Logger();

    static void Log(const char* tag, const char* format, va_list args);

    static void Err(const char* tag, const char* format, va_list args);

    static int level_;
};
}
}
#endif // OHOS_IDL_LOGGER_H
