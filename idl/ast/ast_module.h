/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

    void SetIdlFile(
        /* [in] */ const String& idlFile);

    String GetName()
    {
        return name_;
    }

    void SetLicense(
        /* [in] */ const String& license)
    {
        license_ = license;
    }

    String GetLicense()
    {
        return license_;
    }

    AutoPtr<ASTNamespace> ParseNamespace(
        /* [in] */ const String& nspaceStr);

    void AddNamespace(
        /* [in] */ ASTNamespace* nspace);

    AutoPtr<ASTNamespace> FindNamespace(
        /* [in] */ const String& nspaceStr);

    AutoPtr<ASTNamespace> GetNamespace(
        /* [in] */ size_t index);

    size_t GetNamespaceNumber()
    {
        return namespaces_.size();
    }

    void AddInterface(
        /* [in] */ ASTInterfaceType* interface);

    AutoPtr<ASTInterfaceType> GetInterface(
        /* [in] */ size_t index);

    size_t GetInterfaceNumber()
    {
        return interfaces_.size();
    }

    int IndexOf(
        /* [in] */ ASTInterfaceType* interface);

    void AddSequenceable(
        /* [in] */ ASTSequenceableType* sequenceable);

    AutoPtr<ASTSequenceableType> GetSequenceable(
        /* [in] */ size_t index);

    size_t GetSequenceableNumber()
    {
        return sequenceables_.size();
    }

    int IndexOf(
        /* [in] */ ASTSequenceableType* sequenceable);

    void AddType(
        /* [in] */ ASTType* type);

    AutoPtr<ASTType> FindType(
        /* [in] */ const String& typeName);

    using TypeStringMap = std::unordered_map<String, AutoPtr<ASTType>, StringHashFunc, StringEqualFunc>;

    const TypeStringMap& GetTypes()
    {
        return types_;
    }

    size_t GetTypeNumber()
    {
        return types_.size();
    }

    int IndexOf(
        /* [in] */ ASTType* type);

    bool IsValid();

    String Dump(
        /* [in] */ const String& prefix) override;

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
