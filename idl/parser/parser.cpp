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

#include "parser/parser.h"
#include <cstdio>
#include "ast/ast_array_type.h"
#include "ast/ast_list_type.h"
#include "ast/ast_map_type.h"
#include "ast/ast_parameter.h"
#include "ast/ast_sequenceable_type.h"
#include "util/logger.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
const char* Parser::TAG = "Parser";

Parser::Parser(const Options& options)
    : options_(options)
{}

bool Parser::Parse(const String& sourceFile)
{
    bool ret = lexer_.OpenSourceFile(sourceFile);
    if (!ret) {
        Logger::E(TAG, "Fail to open file \"%s\".", sourceFile.string());
        return false;
    }
    ret = ParseFile();
    ret = CheckIntegrity() && ret;
    if (!ret) {
        ShowError();
        return false;
    }
    if (options_.DoDumpAST()) {
        String astStr = module_->Dump("");
        printf("%s\n", astStr.string());
    }
    return ret;
}

bool Parser::ParseFile()
{
    bool ret = true;

    module_ = new ASTModule();
    module_->SetIdlFile(lexer_.GetSourceFile()->GetPath());

    ParseLicense();

    Token token;
    while ((token = lexer_.PeekToken()) != Token::END_OF_FILE) {
        switch (token) {
            case Token::BRACKETS_LEFT:
            case Token::INTERFACE:
                ret = ParseInterface() && ret;
                continue;
            case Token::SEQUENCEABLE:
                ret = ParseSequenceable() && ret;
                continue;
            case Token::COMMENT_LINE:
                lexer_.GetToken();
                continue;
            default:
                LogError(token, String::Format("%s is not expected.", lexer_.DumpToken().string()));
                lexer_.GetToken();
                ret = false;
                continue;
        }
    }
    lexer_.GetToken();

    return ret;
}

bool Parser::ParseLicense()
{
    Token token = lexer_.PeekToken(false);
    if (token != Token::COMMENT_BLOCK) {
        return false;
    }

    lexer_.GetToken(false);

    module_->SetLicense(lexer_.GetComment());

    return true;
}

bool Parser::ParseInterface()
{
    bool ret = true;
    bool hasProperties = false;
    bool oneway = false;
    Token token = lexer_.GetToken();
    if (token == Token::BRACKETS_LEFT) {
        token = lexer_.PeekToken();
        if (token != Token::ONEWAY) {
            LogError(Token::IDENTIFIER,
                String::Format("\"%s\" is an illegal interface property.", lexer_.DumpToken().string()));
            if (token != Token::BRACKETS_RIGHT) {
                lexer_.SkipCurrentLine(Lexer::TokenToChar(Token::BRACKETS_RIGHT));
            }
            ret = false;
        }
        lexer_.GetToken();
        oneway = true;
        hasProperties = true;
        token = lexer_.PeekToken();
        if (token != Token::BRACKETS_RIGHT) {
            LogError(Token::IDENTIFIER, String("\"]\" is expected."));
            while (token != Token::BRACKETS_RIGHT && token != Token::INTERFACE && token != Token::END_OF_FILE) {
                lexer_.GetToken();
                token = lexer_.PeekToken();
            }
            ret = false;
        } else {
            lexer_.GetToken();
        }

        token = lexer_.PeekToken();
        if (token != Token::INTERFACE) {
            LogError(Token::IDENTIFIER, String("\"interface\" is expected."));
            ret = false;
        } else {
            lexer_.GetToken();
        }
    }
    String interfaceFullName;
    token = lexer_.PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(token, String::Format("%s is not expected.", lexer_.DumpToken().string()));
        lexer_.SkipCurrentLine();
        return false;
    } else {
        lexer_.GetToken();
        interfaceFullName = lexer_.GetIdentifier();
        token = lexer_.PeekToken();
    }
    if (token != Token::SEMICOLON && token != Token::BRACES_LEFT) {
        LogError(token, String::Format("%s is not expected.", lexer_.DumpToken().string()));
        lexer_.SkipCurrentLine();
        return false;
    }

    if (interfaceFullName.IsEmpty()) {
        LogError(Token::IDENTIFIER, String("Interface name is expected."));
        return false;
    } else if (!IsValidTypeName(interfaceFullName)) {
        LogError(Token::IDENTIFIER, String::Format("Interface name \"%s\" is illegal.", interfaceFullName.string()));
        return false;
    } else if (interfaceFullName.IndexOf(".") == -1) {
        if (!(options_.GetTargetLanguage().Equals("ts"))) {
            LogError(Token::IDENTIFIER, String::Format("Interface name \"%s\" does not have namespace.",
            interfaceFullName.string()));
            return false;
        }
    }

    AutoPtr<ASTInterfaceType> interface = new ASTInterfaceType();
    parsingInterface_ = interface;
    int index = interfaceFullName.LastIndexOf('.');
    if (index != -1) {
        interface->SetName(interfaceFullName.Substring(index + 1));
        interface->SetNamespace(module_->ParseNamespace(interfaceFullName.Substring(0, index + 1)));
    } else {
        interface->SetName(interfaceFullName);
        interface->SetNamespace(NameSpaceEmpty());
    }

    // read ';'
    lexer_.GetToken();
    if (token == Token::SEMICOLON) {
        if (hasProperties) {
            LogError(Token::IDENTIFIER, String("Interface forward declaration should not have properties."));
            return false;
        }
        interface->SetExternal(true);
        module_->AddInterface(interface);
        return true;
    } else {
        if (!interface->GetName().Equals(module_->GetName())) {
            LogError(Token::IDENTIFIER, String::Format("Module name \"%s\" is not equal to interface name  \"%s\".",
                module_->GetName().string(), interface->GetName().string()));
            return false;
        }

        interface->SetLicense(module_->GetLicense());
        interface->SetOneway(oneway);

        while (token != Token::BRACES_RIGHT && token != Token::END_OF_FILE) {
            ret = ParseMethod(interface) && ret;
            token = lexer_.PeekToken();
        }

        if (token != Token::BRACES_RIGHT) {
            ret = false;
        } else {
            lexer_.GetToken();
            module_->AddInterface(interface);
        }

        return ret;
    }
}

bool Parser::ParseMethod(ASTInterfaceType* interface)
{
    bool ret = true;
    bool oneway = false;
    Token token;

    token = lexer_.PeekToken();
    if (token == Token::BRACKETS_LEFT) {
        lexer_.GetToken();
        token = lexer_.PeekToken();
        if (token != Token::ONEWAY) {
            LogError(Token::IDENTIFIER, String::Format("\"%s\" is an illegal method property.",
                lexer_.DumpToken().string()));

            if (token != Token::BRACKETS_RIGHT) {
                lexer_.SkipCurrentLine(Lexer::TokenToChar(Token::BRACKETS_RIGHT));
            }
            ret = false;
        }
        lexer_.GetToken();

        oneway = true;

        token = lexer_.PeekToken();
        if (token != Token::BRACKETS_RIGHT) {
            LogError(Token::IDENTIFIER, String("\"]\" is expected."));
            ret = false;
        } else {
            lexer_.GetToken();
        }
    }
    AutoPtr<ASTType> type = ParseType();
    if (type == nullptr) {
        token = lexer_.PeekToken();
        if (token != Token::BRACES_RIGHT) {
            // jump over colon
            lexer_.GetToken();
            while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
                token = lexer_.PeekToken();
                if (token == Token::BRACES_RIGHT) {
                    break;
                }
                lexer_.GetToken();
            }
        }
        return false;
    }
    if (interface->IsOneway()) {
        if (!type->IsVoidType()) {
            LogError(token, String("void return type expected in oneway interface."));
            return false;
        }
    }
    token = lexer_.PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(token, String("Method name is expected."));
        if (token != Token::BRACES_RIGHT) {
            // jump over colon
            lexer_.GetToken();
            while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
                token = lexer_.PeekToken();
                if (token == Token::BRACES_RIGHT) {
                    break;
                }
                lexer_.GetToken();
            }
        }
        return false;
    }
    token = lexer_.GetToken();

    AutoPtr<ASTMethod> method = new ASTMethod();
    method->SetName(lexer_.GetIdentifier());
    method->SetOneway(oneway);
    method->SetReturnType(type);
    if (method->IsOneway()) {
        if (!method->GetReturnType()->IsVoidType()) {
            LogError(token, String("void return type expected in oneway method."));
            return false;
        }
    }
    token = lexer_.PeekToken();
    if (token != Token::PARENTHESES_LEFT) {
        LogError(token, String("\"(\" is expected."));
        if (token != Token::BRACES_RIGHT) {
            // jump over colon
            lexer_.GetToken();
            while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
                token = lexer_.PeekToken();
                if (token == Token::BRACES_RIGHT) {
                    break;
                }
                lexer_.GetToken();
            }
        }
        return false;
    }
    token = lexer_.GetToken();

    token = lexer_.PeekToken();
    while (token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
        ret = ParseParameter(method) && ret;
        token = lexer_.PeekToken();
        if (token == Token::COMMA) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
        }
    }

    if (interface->IsOneway() || method->IsOneway()) {
        for (size_t i = 0; i< method->GetParameterNumber(); i++) {
            auto parameter = method->GetParameter(i);
            if (parameter->IsOutParameter()) {
                LogError(token, String("out parameter type not expected in oneway method."));
                return false;
            }
        }
    }
    lexer_.GetToken();
    if (!ret) {
        lexer_.SkipCurrentLine();
        return false;
    }

    token = lexer_.PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(token, String("\";\" is expected."));
        if (token != Token::BRACES_RIGHT) {
            lexer_.SkipCurrentLine(Lexer::TokenToChar(Token::BRACES_RIGHT));
        }
        return false;
    }
    lexer_.GetToken();

    interface->AddMethod(method);

    return ret;
}

bool Parser::ParseParameter(ASTMethod* method)
{
    Token token = lexer_.PeekToken();
    if (token != Token::BRACKETS_LEFT) {
        LogError(token, String("\"[\" is expected."));
        // jump to ',' or ')'
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
        }
        return false;
    }
    lexer_.GetToken();

    AutoPtr<ASTParameter> parameter = new ASTParameter();

    token = lexer_.PeekToken();
    while (token != Token::BRACKETS_RIGHT && token != Token::END_OF_FILE) {
        switch (token) {
            case Token::IN:
                lexer_.GetToken();
                parameter->SetInParameter(true);
                break;
            case Token::OUT:
                lexer_.GetToken();
                parameter->SetOutParameter(true);
                break;
            case Token::INOUT:
                lexer_.GetToken();
                parameter->SetInParameter(true);
                parameter->SetOutParameter(true);
                break;
            default:
                LogError(token, String("\"in\" or \"out\" or \"inout\" is expected."));
                break;
        }
        token = lexer_.PeekToken();
        if (token == Token::COMMA) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
            continue;
        }
        if (token != Token::BRACKETS_RIGHT) {
            LogError(token, String("\",\" or \"]\" is expected."));
            // jump to ',' or ')'
            while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
                lexer_.GetToken();
                token = lexer_.PeekToken();
            }
            return false;
        }
    }
    // read ']'
    lexer_.GetToken();

    AutoPtr<ASTType> type = ParseType();
    if (type == nullptr) {
        // jump to ',' or ')'
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
        }
        return false;
    }

    token = lexer_.PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(token, String("Parameter name is expected."));
        // jump to ',' or ')'
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_.GetToken();
            token = lexer_.PeekToken();
        }
        return false;
    }
    lexer_.GetToken();

    parameter->SetName(lexer_.GetIdentifier());
    parameter->SetType(type);
    method->AddParameter(parameter);

    return true;
}

AutoPtr<ASTType> Parser::ParseType()
{
    AutoPtr<ASTType> type;

    Token token = lexer_.PeekToken();
    if (IsPrimitiveType(token)) {
        lexer_.GetToken();
        type = module_->FindType(lexer_.DumpToken());
    } else if (token == Token::LIST) {
        type = ParseList();
    } else if (token == Token::MAP) {
        type = ParseMap();
    } else if (token == Token::IDENTIFIER) {
        lexer_.GetToken();
        if (parsingInterface_ != nullptr &&
            parsingInterface_->GetName().Equals(lexer_.GetIdentifier())) {
            type = parsingInterface_.Get();
        } else {
            type = module_->FindType(lexer_.GetIdentifier());
        }
    } else {
        LogError(token, String("Type name is expected."));
        return nullptr;
    }

    if (type == nullptr) {
        LogError(token, String::Format("Type \"%s\" was not declared in the module.", lexer_.DumpToken().string()));
    }

    token = lexer_.PeekToken();
    if (token == Token::BRACKETS_LEFT) {
        lexer_.GetToken();
        token = lexer_.PeekToken();
        if (token != Token::BRACKETS_RIGHT) {
            LogError(token, String("\"]\" is expected."));
            return nullptr;
        }
        lexer_.GetToken();

        AutoPtr<ASTArrayType> arrayType = new ASTArrayType();
        arrayType->SetElementType(type);

        type = module_->FindType(arrayType->ToString());
        if (type == nullptr) {
            module_->AddType(arrayType);
            type = static_cast<ASTType*>(arrayType.Get());
        }
    }

    return type;
}

AutoPtr<ASTType> Parser::ParseList()
{
    lexer_.GetToken();

    Token token = lexer_.PeekToken();
    if (token != Token::ANGLE_BRACKETS_LEFT) {
        LogError(token, String("\"<\" is expected."));
        return nullptr;
    }
    lexer_.GetToken();

    AutoPtr<ASTType> type = ParseType();
    if (type == nullptr) {
        lexer_.SkipCurrentLine('>');
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token != Token::ANGLE_BRACKETS_RIGHT) {
        LogError(token, String("\">\" is expected."));
        return nullptr;
    }
    lexer_.GetToken();

    AutoPtr<ASTListType> list = new ASTListType();
    list->SetElementType(type);

    AutoPtr<ASTType> ret = module_->FindType(list->ToString());
    if (ret == nullptr) {
        module_->AddType(list);
        ret = list.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseMap()
{
    lexer_.GetToken();

    Token token = lexer_.PeekToken();
    if (token != Token::ANGLE_BRACKETS_LEFT) {
        LogError(token, String("\"<\" is expected."));
        return nullptr;
    }
    lexer_.GetToken();

    AutoPtr<ASTType> keyType = ParseType();
    if (keyType == nullptr) {
        lexer_.SkipCurrentLine('>');
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token != Token::COMMA) {
        LogError(token, String("\",\" is expected."));
        return nullptr;
    }
    lexer_.GetToken();

    AutoPtr<ASTType> valueType = ParseType();
    if (valueType == nullptr) {
        lexer_.SkipCurrentLine('>');
        return nullptr;
    }

    token = lexer_.PeekToken();
    if (token != Token::ANGLE_BRACKETS_RIGHT) {
        LogError(token, String("\">\" is expected."));
        return nullptr;
    }
    lexer_.GetToken();

    AutoPtr<ASTMapType> map = new ASTMapType();
    map->SetKeyType(keyType);
    map->SetValueType(valueType);

    AutoPtr<ASTType> ret = module_->FindType(map->ToString());
    if (ret == nullptr) {
        module_->AddType(map);
        ret = map.Get();
    }

    return ret;
}

bool Parser::ParseSequenceable()
{
    lexer_.GetToken();

    String classFullName;

    Token token = lexer_.PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(token, String::Format("%s is not expected.", lexer_.DumpToken().string()));
        lexer_.SkipCurrentLine();
        return false;
    } else {
        lexer_.GetToken();
        classFullName = lexer_.GetIdentifier();
        token = lexer_.PeekToken();
    }

    if (token != Token::SEMICOLON) {
        LogError(token, String::Format("%s is not expected.", lexer_.DumpToken().string()));
        lexer_.SkipCurrentLine();
        return false;
    }

    // read ';'
    lexer_.GetToken();

    if (classFullName.IsEmpty()) {
        LogError(Token::IDENTIFIER, String("Class name is expected."));
        return false;
    } else if (!IsValidTypeName(classFullName)) {
        LogError(Token::IDENTIFIER, String::Format("Class name \"%s\" is illegal.", classFullName.string()));
        return false;
    }

    AutoPtr<ASTSequenceableType> sequenceable = new ASTSequenceableType();
    int index = classFullName.LastIndexOf('.');
    if (index != -1) {
        sequenceable->SetName(classFullName.Substring(index + 1));
        sequenceable->SetNamespace(module_->ParseNamespace(classFullName.Substring(0, index + 1)));
    } else {
        sequenceable->SetName(classFullName);
        sequenceable->SetNamespace(NameSpaceEmpty());
    }
    module_->AddSequenceable(sequenceable);

    return true;
}

bool Parser::CheckIntegrity()
{
    bool definedInterface = false;
    int interfaceNumber = module_->GetInterfaceNumber();
    for (int i = 0; i < interfaceNumber; i++) {
        if (!module_->GetInterface(i)->IsExternal()) {
            definedInterface = true;
            break;
        }
    }
    if (!definedInterface) {
        LogError(Token::UNKNOWN, String("An interface is not defined."));
        return false;
    }

    return true;
}

bool Parser::IsValidTypeName(const String& typeName)
{
    if (typeName[0] == '.') {
        return false;
    }

    if (typeName[typeName.GetLength() - 1] == '.') {
        return false;
    }

    return true;
}

void Parser::LogError(Token token, const String& message)
{
    AutoPtr<ErrorInfo> error = new ErrorInfo();

    String sourceFile = lexer_.GetSourceFile()->GetPath();
#ifdef __MINGW32__
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('\\') + 1);
#else
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('/') + 1);
#endif
    error->lineNo_ = lexer_.GetTokenLineNumber();
    error->columnNo_ = lexer_.GetTokenColumnNumber();
    error->message_ = message;

    if (errors_ == nullptr) {
        errors_ = error;
    } else {
        ErrorInfo* pos = errors_;
        while (pos->next_ != nullptr) {
            pos = pos->next_;
        }
        pos->next_ = error;
    }
}

void Parser::ShowError()
{
    ErrorInfo* error = errors_;
    while (error != nullptr) {
        Logger::E(TAG, "%s[line %d, column %d] %s", error->file_.string(),
            error->lineNo_, error->columnNo_, error->message_.string());
        error = error->next_;
    }
}

AutoPtr<ASTNamespace> Parser::NameSpaceEmpty()
{
    AutoPtr<ASTNamespace> currNspace = nullptr;
    currNspace = module_->FindNamespace("");
    if (currNspace == nullptr) {
        currNspace = new ASTNamespace("");
        module_->AddNamespace(currNspace);
    }
    return currNspace;
}
}
}
