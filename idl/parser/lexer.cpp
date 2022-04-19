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

#include "parser/lexer.h"

#include <utility>
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
static struct Keywords {
    String key_;
    Token token_;
} g_keywords[] = {
    {String("boolean"), Token::BOOLEAN},
    {String("byte"), Token::BYTE},
    {String("char"), Token::CHAR},
    {String("double"), Token::DOUBLE},
    {String("float"), Token::FLOAT},
    {String("in"), Token::IN},
    {String("inout"), Token::INOUT},
    {String("int"), Token::INTEGER},
    {String("interface"), Token::INTERFACE},
    {String("List"), Token::LIST},
    {String("long"), Token::LONG},
    {String("Map"), Token::MAP},
    {String("oneway"), Token::ONEWAY},
    {String("out"), Token::OUT},
    {String("sequenceable"), Token::SEQUENCEABLE},
    {String("short"), Token::SHORT},
    {String("String"), Token::STRING},
};

Lexer::Lexer()
{
    InitializeKeywords();
}

Lexer::~Lexer()
{
    if (currentFile_ != nullptr) {
        currentFile_->Close();
    }
}

void Lexer::InitializeKeywords()
{
    for (size_t i = 0; i < sizeof(g_keywords) / sizeof(struct Keywords); i++) {
        keywords_[g_keywords[i].key_] = g_keywords[i].token_;
    }
}

bool Lexer::OpenSourceFile(const String& filePath)
{
    currentFile_ = std::make_shared<File>(filePath, File::READ);
    if (!currentFile_->IsValid()) {
        return false;
    }

    return true;
}

Token Lexer::GetToken(bool skipComment)
{
    if (!havePeek_) {
        currentToken_ = ReadToken(skipComment);
    }
    havePeek_ = false;
    return currentToken_;
}

Token Lexer::PeekToken(bool skipComment)
{
    if (!havePeek_) {
        currentToken_ = ReadToken(skipComment);
        havePeek_ = true;
    }
    return currentToken_;
}

Token Lexer::ReadToken(bool skipComment)
{
    while (!currentFile_->IsEof()) {
        char c = currentFile_->GetChar();
        tokenLineNo_ = currentFile_->GetCharLineNumber();
        tokenColumnNo_ = currentFile_->GetCharColumnNumber();
        if (IsSpace(c)) {
            continue;
        } else if (IsAlphabet(c) || c == '_') {
            return ReadIdentifier(c);
        }
        switch (c) {
            case '<':
                currentToken_ = Token::ANGLE_BRACKETS_LEFT;
                return currentToken_;
            case '>':
                currentToken_ = Token::ANGLE_BRACKETS_RIGHT;
                return currentToken_;
            case '{':
                currentToken_ = Token::BRACES_LEFT;
                return currentToken_;
            case '}':
                currentToken_ = Token::BRACES_RIGHT;
                return currentToken_;
            case '[':
                currentToken_ = Token::BRACKETS_LEFT;
                return currentToken_;
            case ']':
                currentToken_ = Token::BRACKETS_RIGHT;
                return currentToken_;
            case ',':
                currentToken_ = Token::COMMA;
                return currentToken_;
            case '/':
                if (currentFile_->PeekChar() == '/') {
                    ReadLineComment(c);
                    if (!skipComment) {
                        return currentToken_;
                    }
                    continue;
                } else if (currentFile_->PeekChar() == '*') {
                    ReadBlockComment(c);
                    if (!skipComment) {
                        return currentToken_;
                    }
                    continue;
                }
                currentToken_ = Token::UNKNOWN;
                return currentToken_;
            case '(':
                currentToken_ = Token::PARENTHESES_LEFT;
                return currentToken_;
            case ')':
                currentToken_ = Token::PARENTHESES_RIGHT;
                return currentToken_;
            case '.':
                currentToken_ = Token::DOT;
                return currentToken_;
            case ';':
                currentToken_ = Token::SEMICOLON;
                return currentToken_;
            default:
                currentToken_ = Token::UNKNOWN;
                return currentToken_;
        }
    }
    currentToken_ = Token::END_OF_FILE;
    return currentToken_;
}

Token Lexer::ReadIdentifier(char c)
{
    StringBuilder sb;

    sb.Append(c);
    while (!currentFile_->IsEof()) {
        c = currentFile_->PeekChar();
        if (IsAlphabet(c) || c == '_' || IsDecimalDigital(c) || c == '.') {
            c = currentFile_->GetChar();
            sb.Append(c);
            continue;
        }
        if (IsSpace(c)) {
            currentFile_->GetChar();
        }
        break;
    }
    String key = sb.ToString();
    auto it = keywords_.find(key);
    if (it == keywords_.end()) {
        identifier_ = key;
        currentToken_ = Token::IDENTIFIER;
    } else {
        currentToken_ = it->second;
    }
    return currentToken_;
}

Token Lexer::ReadLineComment(char c)
{
    StringBuilder sb;

    sb.Append(c);
    while (!currentFile_->IsEof()) {
        c = currentFile_->GetChar();
        if (c == '\n') {
            break;
        }
        sb.Append(c);
    }
    comment_ = sb.ToString();
    currentToken_ = Token::COMMENT_LINE;
    return currentToken_;
}

Token Lexer::ReadBlockComment(char c)
{
    StringBuilder sb;

    sb.Append(c);
    while (!currentFile_->IsEof()) {
        c = currentFile_->GetChar();
        sb.Append(c);
        if (c == '*' && currentFile_->PeekChar() == '/') {
            c = currentFile_->GetChar();
            sb.Append(c);
            break;
        }
    }
    comment_ = sb.ToString();
    currentToken_ = Token::COMMENT_BLOCK;
    return currentToken_;
}

void Lexer::SkipCurrentLine()
{
    while (!currentFile_->IsEof()) {
        char c = currentFile_->GetChar();
        if (c == '\n') {
            currentFile_->GetChar();
            return;
        }
    }
}

bool Lexer::SkipCurrentLine(char untilChar)
{
    while (!currentFile_->IsEof()) {
        int c = currentFile_->GetChar();
        if (c == untilChar) {
            return true;
        }
        if (c == '\n') {
            currentFile_->GetChar();
            return false;
        }
    }
    return true;
}

int Lexer::TokenToChar(Token token)
{
    switch (token) {
        case Token::ANGLE_BRACKETS_LEFT:
            return '<';
        case Token::ANGLE_BRACKETS_RIGHT:
            return '>';
        case Token::BRACES_LEFT:
            return '{';
        case Token::BRACES_RIGHT:
            return '}';
        case Token::BRACKETS_LEFT:
            return '[';
        case Token::BRACKETS_RIGHT:
            return ']';
        case Token::COMMA:
            return ',';
        case Token::DOT:
            return '.';
        case Token::PARENTHESES_LEFT:
            return '(';
        case Token::PARENTHESES_RIGHT:
            return ')';
        case Token::SEMICOLON:
            return ';';
        case Token::BOOLEAN:
        case Token::BYTE:
        case Token::CHAR:
        case Token::COMMENT_BLOCK:
        case Token::COMMENT_LINE:
        case Token::DOUBLE:
        case Token::END_OF_FILE:
        case Token::FLOAT:
        case Token::IDENTIFIER:
        case Token::IN:
        case Token::INOUT:
        case Token::INTEGER:
        case Token::LIST:
        case Token::LONG:
        case Token::MAP:
        case Token::ONEWAY:
        case Token::OUT:
        case Token::SEQUENCEABLE:
        case Token::SHORT:
        case Token::STRING:
        default:
            return -1;
    }
}

String Lexer::DumpToken() const
{
    switch (currentToken_) {
        case Token::ANGLE_BRACKETS_LEFT:
            return "<";
        case Token::ANGLE_BRACKETS_RIGHT:
            return ">";
        case Token::BOOLEAN:
            return "boolean";
        case Token::BRACES_LEFT:
            return "{";
        case Token::BRACES_RIGHT:
            return "}";
        case Token::BRACKETS_LEFT:
            return "[";
        case Token::BRACKETS_RIGHT:
            return "]";
        case Token::BYTE:
            return "byte";
        case Token::CHAR:
            return "char";
        case Token::COMMA:
            return ",";
        case Token::COMMENT_BLOCK:
        case Token::COMMENT_LINE:
            return comment_;
        case Token::DOT:
            return ".";
        case Token::DOUBLE:
            return "double";
        case Token::END_OF_FILE:
            return "eof";
        case Token::FLOAT:
            return "float";
        case Token::IDENTIFIER:
            return identifier_;
        case Token::IN:
            return "in";
        case Token::INOUT:
            return "inout";
        case Token::INTEGER:
            return "int";
        case Token::LIST:
            return "List";
        case Token::LONG:
            return "long";
        case Token::MAP:
            return "Map";
        case Token::ONEWAY:
            return "oneway";
        case Token::OUT:
            return "out";
        case Token::SEQUENCEABLE:
            return "sequenceable";
        case Token::PARENTHESES_LEFT:
            return "(";
        case Token::PARENTHESES_RIGHT:
            return ")";
        case Token::SEMICOLON:
            return ";";
        case Token::SHORT:
            return "short";
        case Token::STRING:
            return "String";
        default:
            return "unknown token";
    }
}
}
}
