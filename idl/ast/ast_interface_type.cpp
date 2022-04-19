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

#include "ast/ast_interface_type.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
void ASTInterfaceType::SetNamespace(ASTNamespace* nspace)
{
    ASTType::SetNamespace(nspace);
    if (namespace_ != nullptr) {
        namespace_->AddInterface(this);
    }
}

void ASTInterfaceType::AddMethod(ASTMethod* method)
{
    if (method == nullptr) {
        return;
    }
    methods_.push_back(method);
}

AutoPtr<ASTMethod> ASTInterfaceType::GetMethod(size_t index)
{
    if (index >= methods_.size()) {
        return nullptr;
    }

    return methods_[index];
}

String ASTInterfaceType::GetSignature()
{
    String fullName = namespace_ != nullptr ?
            namespace_->ToString() + name_ : name_;
    return "L" + fullName.Replace('.', '/') + ";";
}

bool ASTInterfaceType::IsInterfaceType()
{
    return true;
}

String ASTInterfaceType::ToString()
{
    return name_;
}

String ASTInterfaceType::Dump(const String& prefix)
{
    StringBuilder sb;

    sb.Append(prefix).Append("interface ");
    if (namespace_ != nullptr) {
        sb.Append(namespace_->ToString());
    }
    sb.Append(name_);
    if (isExternal_) {
        sb.Append(";\n");
    } else {
        sb.Append(" {\n");
        for (auto method : methods_) {
            String info = method->Dump(prefix + "  ");
            sb.Append(info);
            if (method != methods_[methods_.size() - 1]) {
                sb.Append('\n');
            }
        }
        sb.Append(prefix).Append("}\n");
    }

    return sb.ToString();
}
}
}
