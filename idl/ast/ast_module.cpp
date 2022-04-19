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

#include "ast/ast_module.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
ASTModule::ASTModule()
{
    booleanType_ = new ASTBooleanType();
    byteType_ = new ASTByteType();
    shortType_ = new ASTShortType();
    integerType_ = new ASTIntegerType();
    longType_ = new ASTLongType();
    floatType_ = new ASTFloatType();
    doubleType_ = new ASTDoubleType();
    charType_ = new ASTCharType();
    stringType_ = new ASTStringType();
    voidType_ = new ASTVoidType();

    types_["boolean"] = booleanType_.Get();
    types_["byte"] = byteType_.Get();
    types_["short"] = shortType_.Get();
    types_["int"] = integerType_.Get();
    types_["long"] = longType_.Get();
    types_["float"] = floatType_.Get();
    types_["double"] = doubleType_.Get();
    types_["char"] = charType_.Get();
    types_["String"] = stringType_.Get();
    types_["void"] = voidType_.Get();
}

void ASTModule::SetIdlFile(const String& idlFile)
{
    idlFilePath_ = idlFile;
#ifdef __MINGW32__
    int index = idlFilePath_.LastIndexOf('\\');
#else
    int index = idlFilePath_.LastIndexOf('/');
#endif
    int end = idlFilePath_.LastIndexOf(".idl") == -1 ?
            idlFilePath_.LastIndexOf(".idl") : idlFilePath_.LastIndexOf(".idl");
    name_ = idlFilePath_.Substring((index == -1) ? 0 : (index + 1), end);
}

AutoPtr<ASTNamespace> ASTModule::ParseNamespace(const String& nspaceStr)
{
    AutoPtr<ASTNamespace> currNspace;
    int begin = 0;
    int index = 0;
    while ((index = nspaceStr.IndexOf('.', begin)) != -1) {
        String ns = nspaceStr.Substring(begin, index);
        AutoPtr<ASTNamespace> nspace;
        if (currNspace == nullptr) {
            nspace = FindNamespace(ns);
        } else {
            nspace = currNspace->FindNamespace(ns);
        }
        if (nspace == nullptr) {
            nspace = new ASTNamespace(ns);
            if (currNspace == nullptr) {
                AddNamespace(nspace);
            } else {
                currNspace->AddNamespace(nspace);
            }
        }
        currNspace = nspace;
        begin = index + 1;
    }
    return currNspace;
}

void ASTModule::AddNamespace(ASTNamespace* nspace)
{
    if (nspace == nullptr) {
        return;
    }
    namespaces_.push_back(nspace);
}

AutoPtr<ASTNamespace> ASTModule::FindNamespace(const String& nspaceStr)
{
    for (auto nspace : namespaces_) {
        if (nspace->ToShortString().Equals(nspaceStr)) {
            return nspace;
        }
    }
    return nullptr;
}

AutoPtr<ASTNamespace> ASTModule::GetNamespace(size_t index)
{
    if (index >= namespaces_.size()) {
        return nullptr;
    }

    return namespaces_[index];
}

void ASTModule::AddInterface(ASTInterfaceType* interface)
{
    if (interface == nullptr) {
        return;
    }

    interfaces_.push_back(interface);
    types_[interface->ToString()] = (ASTType*)interface;
}

AutoPtr<ASTInterfaceType> ASTModule::GetInterface(size_t index)
{
    if (index >= interfaces_.size()) {
        return nullptr;
    }

    return interfaces_[index];
}

int ASTModule::IndexOf(ASTInterfaceType* interface)
{
    for (size_t i = 0; i < interfaces_.size(); i++) {
        if (interfaces_[i] == interface) {
            return i;
        }
    }
    return -1;
}

void ASTModule::AddSequenceable(ASTSequenceableType* sequenceable)
{
    if (sequenceable == nullptr) {
        return;
    }

    sequenceables_.push_back(sequenceable);
    types_[sequenceable->ToString()] = (ASTType*)sequenceable;
}

AutoPtr<ASTSequenceableType> ASTModule::GetSequenceable(size_t index)
{
    if (index >= sequenceables_.size()) {
        return nullptr;
    }

    return sequenceables_[index];
}

int ASTModule::IndexOf(ASTSequenceableType* sequenceable)
{
    for (size_t i = 0; i < sequenceables_.size(); i++) {
        if (sequenceables_[i] == sequenceable) {
            return i;
        }
    }
    return -1;
}

void ASTModule::AddType(ASTType* type)
{
    if (type == nullptr) {
        return;
    }

    types_[type->ToString()] = type;
}

AutoPtr<ASTType> ASTModule::FindType(const String& typeName)
{
    if (typeName.IsEmpty()) {
        return nullptr;
    }

    auto it = types_.find(typeName);
    return it != types_.end() ? it->second : nullptr;
}

int ASTModule::IndexOf(ASTType* type)
{
    int i = 0;
    for (auto it = types_.begin(); it != types_.end(); ++it, ++i) {
        if (it->second == type) {
            return i;
        }
    }
    return -1;
}

bool ASTModule::IsValid()
{
    if (name_.IsEmpty()) {
        return false;
    }

    return interfaces_.size() > 0;
}

String ASTModule::Dump(const String& prefix)
{
    StringBuilder sb;

    sb.Append(prefix);
    sb.Append("Module[");
    sb.Append("name: ").Append(name_).Append(" ");
    sb.Append("file: ").Append(idlFilePath_);
    sb.Append("]\n");

    for (auto sequenceable : sequenceables_) {
        String info = sequenceable->Dump("  ");
        sb.Append(info);
    }
    sb.Append('\n');

    for (auto interface : interfaces_) {
        if (interface->IsExternal()) {
            String info = interface->Dump("  ");
            sb.Append(info);
        }
    }
    sb.Append('\n');

    for (auto interface : interfaces_) {
        if (!interface->IsExternal()) {
            String info = interface->Dump("  ");
            sb.Append(info);
        }
    }

    return sb.ToString();
}
}
}
