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

#ifndef OHOS_IDL_TOKEN_H
#define OHOS_IDL_TOKEN_H

namespace OHOS {
namespace Idl {
enum class Token {
    UNKNOWN              = 0,
    // types
    BOOLEAN,
    CHAR,
    BYTE,
    SHORT,
    INTEGER,
    LONG,
    FLOAT,
    DOUBLE,
    STRING,
    LIST,
    MAP,
    // keywords
    IN,
    INTERFACE,
    ONEWAY,
    OUT,
    INOUT,
    SEQUENCEABLE,
    // symbols
    DOT,   // '.'
    COMMA,   // ','
    SEMICOLON,   // ';'
    BRACES_LEFT,   // '{'
    BRACES_RIGHT,   // '}'
    BRACKETS_LEFT,   // '['
    BRACKETS_RIGHT,   // ']'
    PARENTHESES_LEFT,   // '('
    PARENTHESES_RIGHT,   // ')'
    ANGLE_BRACKETS_LEFT,   // '<'
    ANGLE_BRACKETS_RIGHT,   // '>'
    // others
    IDENTIFIER,
    COMMENT_BLOCK,
    COMMENT_LINE,
    END_OF_FILE,
};
}
}

#endif // OHOS_IDL_TOKEN_H
