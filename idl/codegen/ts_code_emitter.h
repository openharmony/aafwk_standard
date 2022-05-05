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

#ifndef OHOS_IDL_TSCODEEMITTER_H
#define OHOS_IDL_TSCODEEMITTER_H

#include <string>
#include <vector>

#include "codegen/code_emitter.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
class TsCodeEmitter : public CodeEmitter {
public:
    TsCodeEmitter(MetaComponent* mc)
        : CodeEmitter(mc)
    {}

    void EmitInterface() override;

    void EmitInterfaceProxy() override;

    void EmitInterfaceStub() override;

    struct Parameter {
        bool operator< (const Parameter &para) const
        {
            if (this->attr_ == ATTR_IN) {
                return this->attr_ < para.attr_;
            } else {
                return false;
            }
        }
        std::string name_;
        std::string type_;
        unsigned int attr_ = 0;
    };

    struct Method {
        Parameter retParameter_;
        std::vector<Parameter> parameters_;
        std::string callbackName_;
        std::string name_;
        std::string exportFunction_;
        unsigned int properties_;
    };

private:
    void EmitInterfaceImports(StringBuilder& stringBuilder);

    void EmitInterfaceProxyImports(StringBuilder& stringBuilder);

    void EmitInterfaceSelfDefinedTypeImports(StringBuilder& stringBuilder);

    void EmitInterfaceDefinition(StringBuilder& stringBuilder);

    void EmitInterfaceMethods(StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceMethod(MetaMethod* metaMethod, StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceMethodParameter(MetaParameter* mp, StringBuilder& stringBuilder, const String& prefix);

    void EmitMethodInParameter(StringBuilder& stringBuilder, const std::string& name, const std::string& type,
        const String& prefix);

    void EmitInterfaceMethodExportCallback(Method& m, const Parameter& para, bool isLast);

    void EmitInterfaceProxyImpl(StringBuilder& stringBuilder);

    void EmitInterfaceProxyConstructor(StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceProxyMethodImpls(StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceProxyMethodImpl(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
        const String& prefix);

    void EmitInterfaceProxyMethodBody(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
        const String& prefix);

    void EmitWriteMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& stringBuilder,
        const String& prefix);

    void EmitReadMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& stringBuilder,
        const String& prefix);

    void EmitInterfaceStubImpl(StringBuilder& stringBuilder);

    void EmitInterfaceStubConstructor(StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceStubMethodImpls(StringBuilder& stringBuilder, const String& prefix);

    void EmitInterfaceStubMethodImpl(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
        const String& prefix);

    void EmitInterfaceMethodCommands(StringBuilder& stringBuilder);

    void EmitLicense(StringBuilder& stringBuilder);

    void EmitWriteVariable(const String& parcelName, const std::string& name, MetaType* mt,
        StringBuilder& stringBuilder,
        const String& prefix);

    void EmitReadVariable(const String& parcelName, const std::string& name, MetaType* mt, unsigned int attributes,
        StringBuilder& stringBuilder,
        const String& prefix);

    void EmitReadOutVariable(const String& parcelName, const std::string& name, MetaType* mt,
        StringBuilder& stringBuilder,
        const String& prefix);

    void EmitReadArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
        unsigned int attributes,
        StringBuilder& stringBuilder,
        const String& prefix);

    void EmitWriteArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
        StringBuilder& stringBuilder,
        const String& prefix);

    void EmitReadOutArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
        StringBuilder& stringBuilder,
        const String& prefix);

    String EmitType(MetaType* mt);

    String FileName(const String& name);

    String MethodName(const String& name);

    String ConstantName(const String& name);

    String StubName(const String& name);

    bool CheckInterfaceType();

    const std::string UnderlineAdded(const String& name);

    std::vector<Method> methods_;
};
}
}

#endif // OHOS_IDL_TSCODEEMITTER_H
