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

#ifndef OHOS_IDL_ASTNAMESPACE_H
#define OHOS_IDL_ASTNAMESPACE_H

#include <vector>
#include "ast/ast_node.h"
#include "util/autoptr.h"

namespace OHOS {
namespace Idl {

class ASTInterfaceType;
class ASTSequenceableType;

class ASTNamespace : public ASTNode {
public:
    ASTNamespace(const String& nspaceStr);

    String GetName()
    {
        return name_;
    }

    void AddNamespace(ASTNamespace* innerNspace);

    AutoPtr<ASTNamespace> FindNamespace(const String& nspaceStr);

    AutoPtr<ASTNamespace> GetNamespace(size_t index);

    size_t GetNamespaceNumber()
    {
        return innerNamespaces_.size();
    }

    void AddInterface(ASTInterfaceType* interface);

    AutoPtr<ASTInterfaceType> GetInterface(size_t index);

    size_t GetInterfaceNumber()
    {
        return interfaces_.size();
    }

    void AddSequenceable(ASTSequenceableType* sequenceable);

    AutoPtr<ASTSequenceableType> GetSequenceable(size_t index);

    size_t GetSequenceableNumber()
    {
        return sequenceables_.size();
    }

    String ToShortString()
    {
        return name_;
    }

    String ToString() override;

private:
    String name_;
    ASTNamespace* outerNamespace_;
    std::vector<AutoPtr<ASTNamespace>> innerNamespaces_;
    std::vector<AutoPtr<ASTInterfaceType>> interfaces_;
    std::vector<AutoPtr<ASTSequenceableType>> sequenceables_;
};

}
}

#endif // OHOS_IDL_ASTNAMESPACE_H
