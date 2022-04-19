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

#include "ast/ast_type.h"

namespace OHOS {
namespace Idl {
void ASTType::SetName(const String& name)
{
    name_ = name;
}

String ASTType::GetName()
{
    return name_;
}

void ASTType::SetNamespace(ASTNamespace* nspace)
{
    namespace_ = nspace;
}

AutoPtr<ASTNamespace> ASTType::GetNamespace()
{
    return namespace_;
}

bool ASTType::IsBooleanType()
{
    return false;
}

bool ASTType::IsByteType()
{
    return false;
}

bool ASTType::IsShortType()
{
    return false;
}

bool ASTType::IsIntegerType()
{
    return false;
}

bool ASTType::IsLongType()
{
    return false;
}

bool ASTType::IsFloatType()
{
    return false;
}

bool ASTType::IsDoubleType()
{
    return false;
}

bool ASTType::IsCharType()
{
    return false;
}

bool ASTType::IsStringType()
{
    return false;
}

bool ASTType::IsListType()
{
    return false;
}

bool ASTType::IsMapType()
{
    return false;
}

bool ASTType::IsInterfaceType()
{
    return false;
}

bool ASTType::IsSequenceableType()
{
    return false;
}

bool ASTType::IsVoidType()
{
    return false;
}

bool ASTType::IsArrayType()
{
    return false;
}

String ASTType::ToShortString()
{
    return name_;
}

String ASTType::ToString()
{
    return namespace_ == nullptr ? name_ : namespace_->ToString() + name_;
}
}
}