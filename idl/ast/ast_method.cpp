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

#include "ast/ast_method.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
String ASTMethod::GetSignature()
{
    if (signature_.IsEmpty()) {
        BuildSignature();
    }
    return signature_;
}

void ASTMethod::BuildSignature()
{
    StringBuilder sb;

    sb.AppendFormat("(%s)", returnType_->GetSignature().string());
    for (size_t i = 0; i < parameters_.size(); i++) {
        sb.Append(parameters_[i]->GetType()->GetSignature());
    }
    signature_ = sb.ToString();
}

void ASTMethod::AddParameter(ASTParameter* parameter)
{
    if (parameter == nullptr) {
        return;
    }
    parameters_.push_back(parameter);
}

AutoPtr<ASTParameter> ASTMethod::GetParameter(size_t index)
{
    if (index >= parameters_.size()) {
        return nullptr;
    }

    return parameters_[index];
}

String ASTMethod::Dump(const String& prefix)
{
    StringBuilder sb;

    sb.Append(prefix);
    sb.Append(returnType_->ToString()).Append(' ');
    sb.Append(name_).Append('(');
    if (parameters_.size() != 0) {
        sb.Append('\n');
        for (auto parameter : parameters_) {
            String info = parameter->Dump(prefix + "  ");
            sb.Append(info);
            if (parameter != parameters_[parameters_.size() - 1]) {
                sb.Append(",\n");
            }
        }
    }
    sb.Append(");\n");

    return sb.ToString();
}
}
}
