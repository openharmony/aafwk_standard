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

#ifndef OHOS_IDL_ASTTYPE_H
#define OHOS_IDL_ASTTYPE_H

#include "ast/ast_namespace.h"
#include "ast/ast_node.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {
class ASTType : public ASTNode {
public:
    virtual void SetName(const String& name);

    virtual String GetName();

    virtual void SetNamespace(ASTNamespace* nspace);

    virtual AutoPtr<ASTNamespace> GetNamespace();

    virtual String GetSignature() = 0;

    virtual bool IsBooleanType();

    virtual bool IsByteType();

    virtual bool IsShortType();

    virtual bool IsIntegerType();

    virtual bool IsLongType();

    virtual bool IsFloatType();

    virtual bool IsDoubleType();

    virtual bool IsCharType();

    virtual bool IsStringType();

    virtual bool IsListType();

    virtual bool IsMapType();

    virtual bool IsInterfaceType();

    virtual bool IsSequenceableType();

    virtual bool IsVoidType();

    virtual bool IsArrayType();

    virtual String ToShortString();

    String ToString() override;

protected:
    String name_;
    AutoPtr<ASTNamespace> namespace_;
};
}
}

#endif // OHOS_IDL_ASTTYPE_H
