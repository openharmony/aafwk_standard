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

#ifndef OHOS_IDL_ASTMODULE_H
#define OHOS_IDL_ASTMODULE_H

#include <unordered_map>
#include <vector>
#include "ast/ast_boolean_type.h"
#include "ast/ast_byte_type.h"
#include "ast/ast_char_type.h"
#include "ast/ast_double_type.h"
#include "ast/ast_float_type.h"
#include "ast/ast_integer_type.h"
#include "ast/ast_interface_type.h"
#include "ast/ast_long_type.h"
#include "ast/ast_namespace.h"
#include "ast/ast_node.h"
#include "ast/ast_sequenceable_type.h"
#include "ast/ast_short_type.h"
#include "ast/ast_string_type.h"
#include "ast/ast_void_type.h"
#include "util/autoptr.h"

namespace OHOS {
namespace Idl {
class ASTModule : public ASTNode {
public:
    ASTModule();

    void SetIdlFile(const String& idlFile);

    String GetName()
    {
        return name_;
    }

    void SetLicense(const String& license)
    {
        license_ = license;
    }

    String GetLicense()
    {
        return license_;
    }

    AutoPtr<ASTNamespace> ParseNamespace(const String& nspaceStr);

    void AddNamespace(ASTNamespace* nspace);

    AutoPtr<ASTNamespace> FindNamespace(const String& nspaceStr);

    AutoPtr<ASTNamespace> GetNamespace(size_t index);

    size_t GetNamespaceNumber()
    {
        return namespaces_.size();
    }

    void AddInterface(ASTInterfaceType* interface);

    AutoPtr<ASTInterfaceType> GetInterface(size_t index);

    size_t GetInterfaceNumber()
    {
        return interfaces_.size();
    }

    int IndexOf(ASTInterfaceType* interface);

    void AddSequenceable(ASTSequenceableType* sequenceable);

    AutoPtr<ASTSequenceableType> GetSequenceable(size_t index);

    size_t GetSequenceableNumber()
    {
        return sequenceables_.size();
    }

    int IndexOf(ASTSequenceableType* sequenceable);

    void AddType(ASTType* type);

    AutoPtr<ASTType> FindType(const String& typeName);

    using TypeStringMap = std::unordered_map<String, AutoPtr<ASTType>, StringHashFunc, StringEqualFunc>;

    const TypeStringMap& GetTypes()
    {
        return types_;
    }

    size_t GetTypeNumber()
    {
        return types_.size();
    }

    int IndexOf(ASTType* type);

    bool IsValid();

    String Dump(const String& prefix) override;

private:
    String name_;
    String license_;
    std::vector<AutoPtr<ASTNamespace>> namespaces_;
    std::vector<AutoPtr<ASTInterfaceType>> interfaces_;
    std::vector<AutoPtr<ASTSequenceableType>> sequenceables_;
    TypeStringMap types_;

    AutoPtr<ASTBooleanType> booleanType_;
    AutoPtr<ASTByteType> byteType_;
    AutoPtr<ASTShortType> shortType_;
    AutoPtr<ASTIntegerType> integerType_;
    AutoPtr<ASTLongType> longType_;
    AutoPtr<ASTFloatType> floatType_;
    AutoPtr<ASTDoubleType> doubleType_;
    AutoPtr<ASTCharType> charType_;
    AutoPtr<ASTStringType> stringType_;
    AutoPtr<ASTVoidType> voidType_;

    String idlFilePath_;
};
}
}

#endif // OHOS_IDL_ASTMODULE_H
