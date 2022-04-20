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

#ifndef OHOS_IDL_METADATABUILDER_H
#define OHOS_IDL_METADATABUILDER_H

#include <memory>
#include "ast/ast_module.h"
#include "metadata/meta_component.h"
#include "util/autoptr.h"
#include "util/string.h"
#include "util/string_pool.h"

namespace OHOS {
namespace Idl {
class MetadataBuilder {
public:
    explicit MetadataBuilder(ASTModule* module)
        : module_(module)
    {}

    ~MetadataBuilder() = default;

    std::shared_ptr<MetaComponent> Build();

private:
    size_t CalculateMetadataSize();

    void CalculateMetaComponent(ASTModule* module);

    void CalculateMetaNamespace(ASTNamespace* nspace);

    void CalculateMetaSequenceable(ASTSequenceableType* sequenceable);

    void CalculateMetaInterface(ASTInterfaceType* interface);

    void CalculateMetaMethod(ASTMethod* method);

    void CalculateMetaParameter(ASTParameter* parameter);

    void CalculateMetaType(ASTType* type);

    void CalculateStringPool();

    void WriteMetadata(uintptr_t base);

    void WriteMetaComponent(ASTModule* module);

    MetaNamespace* WriteMetaNamespace(ASTNamespace* nspace);

    MetaSequenceable* WriteMetaSequenceable(ASTSequenceableType* parcelabe);

    MetaInterface* WriteMetaInterface(ASTInterfaceType* interface);

    MetaMethod* WriteMetaMethod(ASTMethod* method);

    MetaParameter* WriteMetaParameter(ASTParameter* parameter);

    MetaType* WriteMetaType(ASTType* type);

    char* WriteString(const String& string);

    TypeKind Type2Kind(ASTType* type);

    static const char* TAG;
    AutoPtr<ASTModule> module_;
    std::shared_ptr<MetaComponent> metaComponent_;
    uintptr_t baseAddr_ = 0;
    size_t size_ = 0;
    StringPool stringPool_;
};
}
}

#endif // OHOS_IDL_METADATABUILDER_H
