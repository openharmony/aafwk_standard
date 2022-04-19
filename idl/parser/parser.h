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

#ifndef OHOS_IDL_PARSER_H
#define OHOS_IDL_PARSER_H

#include "ast/ast_interface_type.h"
#include "ast/ast_method.h"
#include "ast/ast_module.h"
#include "ast/ast_type.h"
#include "parser/lexer.h"
#include "parser/token.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/options.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {
class Parser {
public:
    explicit Parser(const Options& options);

    ~Parser() = default;

    bool Parse(const String& sourceFile);

    AutoPtr<ASTModule> GetModule() const
    {
        return module_;
    }

private:
    class ErrorInfo : public LightRefCountBase {
    public:
        String file_;
        Token token_;
        int lineNo_;
        int columnNo_;
        String message_;
        AutoPtr<ErrorInfo> next_;
    };

    bool ParseFile();

    bool ParseLicense();

    bool ParseInterface();

    bool ParseMethod(ASTInterfaceType* interface);

    bool ParseParameter(ASTMethod* method);

    AutoPtr<ASTNamespace> NameSpaceEmpty();

    AutoPtr<ASTType> ParseType();

    AutoPtr<ASTType> ParseList();

    AutoPtr<ASTType> ParseMap();

    bool ParseSequenceable();

    bool CheckIntegrity();

    bool IsValidTypeName(const String& typeName);

    static bool IsPrimitiveType(Token token)
    {
        return token >= Token::BOOLEAN && token <= Token::STRING;
    }

    void LogError(Token token, const String& message);

    void ShowError();

    static const char* TAG;

    const Options& options_;
    AutoPtr<ASTModule> module_;
    AutoPtr<ASTInterfaceType> parsingInterface_;
    Lexer lexer_;
    AutoPtr<ErrorInfo> errors_;
};
}
}
#endif // OHOS_IDL_PARSER_H
