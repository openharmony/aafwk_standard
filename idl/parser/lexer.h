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

#ifndef OHOS_IDL_LEXER_H
#define OHOS_IDL_LEXER_H

#include <ctype.h>
#include <memory>
#include <unordered_map>

#include "parser/token.h"
#include "util/file.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {

class Lexer {
public:
    Lexer();

    ~Lexer();

    bool OpenSourceFile(
        /* [in] */ const String& filePath);

    Token GetToken(
        /* [in] */ bool skipComment = true);

    Token PeekToken(
        /* [in] */ bool skipComment = true);

    inline String GetIdentifier() const
    {
        return identifier_;
    }

    inline String GetComment() const
    {
        return comment_;
    }

    void SkipCurrentLine();

    bool SkipCurrentLine(
        /* [in] */ char untilChar);

    inline std::shared_ptr<File> GetSourceFile() const
    {
        return currentFile_;
    }

    String DumpToken() const;

    inline int GetTokenLineNumber() const
    {
        return tokenLineNo_;
    }

    inline int GetTokenColumnNumber() const
    {
        return tokenColumnNo_;
    }

    static int TokenToChar(
        /* [in] */ Token token);

private:
    void InitializeKeywords();

    Token ReadToken(
        /* [in] */ bool skipComment);

    Token ReadIdentifier(
        /* [in] */ char c);

    Token ReadLineComment(
        /* [in] */ char c);

    Token ReadBlockComment(
        /* [in] */ char c);

    inline static bool IsAlphabet(
        /* [in] */ char c)
    {
        return isalpha(c);
    }

    inline static bool IsDecimalDigital(
        /* [in] */ char c)
    {
        return isdigit(c);
    }

    inline static bool IsSpace(
        /* [in] */ char c)
    {
        return isspace(c);
    }

    static const char* TAG;
    std::unordered_map<String, Token, StringHashFunc, StringEqualFunc> keywords_;
    Token currentToken_ = Token::UNKNOWN;
    int tokenLineNo_ = 0;
    int tokenColumnNo_ = 0;
    String identifier_;
    String comment_;
    bool havePeek_ = false;
    std::shared_ptr<File> currentFile_;
};

}
}

#endif // OHOS_IDL_LEXER_H
