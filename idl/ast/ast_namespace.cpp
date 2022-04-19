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

#include "ast/ast_namespace.h"
#include "ast/ast_interface_type.h"
#include "ast/ast_sequenceable_type.h"

namespace OHOS {
namespace Idl {
ASTNamespace::ASTNamespace(const String& nspaceStr)
    : name_(nspaceStr),
      outerNamespace_(nullptr)
{}

void ASTNamespace::AddNamespace(ASTNamespace* innerNspace)
{
    if (innerNspace == nullptr) {
        return;
    }

    innerNamespaces_.push_back(innerNspace);
    innerNspace->outerNamespace_ = this;
}

AutoPtr<ASTNamespace> ASTNamespace::FindNamespace(const String& nspaceStr)
{
    if (nspaceStr.IsEmpty()) {
        return nullptr;
    }

    for (auto nspace : innerNamespaces_) {
        if (nspace->name_.Equals(nspaceStr)) {
            return nspace;
        }
    }
    return nullptr;
}

AutoPtr<ASTNamespace> ASTNamespace::GetNamespace(size_t index)
{
    if (index >= innerNamespaces_.size()) {
        return nullptr;
    }

    return innerNamespaces_[index];
}

void ASTNamespace::AddInterface(ASTInterfaceType* interface)
{
    if (interface == nullptr) {
        return;
    }

    interfaces_.push_back(interface);
}

AutoPtr<ASTInterfaceType> ASTNamespace::GetInterface(size_t index)
{
    if (index >= interfaces_.size()) {
        return nullptr;
    }

    return interfaces_[index];
}

void ASTNamespace::AddSequenceable(ASTSequenceableType* sequenceable)
{
    if (sequenceable == nullptr) {
        return;
    }

    sequenceables_.push_back(sequenceable);
}

AutoPtr<ASTSequenceableType> ASTNamespace::GetSequenceable(size_t index)
{
    if (index >= sequenceables_.size()) {
        return nullptr;
    }

    return sequenceables_[index];
}

String ASTNamespace::ToString()
{
    String nspaceStr;
    ASTNamespace* nspace = this;
    while (nspace != nullptr) {
        nspaceStr = nspace->name_ + "." + nspaceStr;
        nspace = nspace->outerNamespace_;
    }
    return nspaceStr;
}
}
}
