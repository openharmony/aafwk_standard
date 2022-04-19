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

#ifndef OHOS_IDL_CPPCODEEMITTER_H
#define OHOS_IDL_CPPCODEEMITTER_H

#include <string>

#include "codegen/code_emitter.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
class CppCodeEmitter : public CodeEmitter {
public:
    CppCodeEmitter(MetaComponent* mc)
        : CodeEmitter(mc)
    {}

    void EmitInterface() override;

    void EmitInterfaceProxy() override;

    void EmitInterfaceStub() override;

private:
    void EmitInterfaceHeaderFile();

    void EmitInterfaceInclusions(StringBuilder& sb);

    void EmitInterfaceStdlibInclusions(StringBuilder& sb);

    void EmitInterfaceDBinderInclusions(StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb);

    bool EmitInterfaceUsings(StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeUsings(StringBuilder& sb);

    void EmitInterfaceDefinition(StringBuilder& sb);

    void EmitInterfaceBody(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethods(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethod(MetaMethod* mm, StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodParameter(MetaParameter* mp, StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodReturn(MetaType* mt, StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyHeaderFile();

    void EmitInterfaceProxyInHeaderFile(StringBuilder& sb);

    void EmitInterfaceProxyConstructor(StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyMethodDecls(StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyMethodDecl(MetaMethod* mm, StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyConstants(StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyCppFile();

    void EmitInterfaceProxyMethodImpls(StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyMethodImpl(MetaMethod* mm, StringBuilder& sb, const String& prefix);

    void EmitInterfaceProxyMethodBody(MetaMethod* mm, StringBuilder& sb, const String& prefix);

    void EmitWriteMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& sb,
        const String& prefix);

    void EmitReadMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& sb, const String& prefix);

    void EmitInterfaceStubHeaderFile();

    void EmitInterfaceStubInHeaderFile(StringBuilder& sb);

    void EmitInterfaceStubMethodDecls(StringBuilder& sb, const String& prefix);

    void EmitInterfaceStubConstants(StringBuilder& sb, const String& prefix);

    void EmitInterfaceStubCppFile();

    void EmitInterfaceStubMethodImpls(StringBuilder& sb, const String& prefix);

    void EmitInterfaceStubMethodImpl(MetaMethod* mm, StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix);

    void EmitLicense(StringBuilder& sb);

    void EmitHeadMacro(StringBuilder& sb, const String& fullName);

    void EmitTailMacro(StringBuilder& sb, const String& fullName);

    void EmitBeginNamespace(StringBuilder& sb);

    void EmitEndNamespace(StringBuilder& sb);

    void EmitWriteVariable(const String& parcelName, const std::string& name, MetaType* mt, StringBuilder& sb,
        const String& prefix);

    void EmitReadVariable(const String& parcelName, const std::string& name, MetaType* mt, StringBuilder& sb,
        const String& prefix, bool emitType = true);

    void EmitLocalVariable(MetaParameter* mp, StringBuilder& sb, const String& prefix);

    void EmitReturnParameter(const String& name, MetaType* mt, StringBuilder& sb);

    String EmitType(MetaType* mt, unsigned int attributes, bool isInnerType);

    String FileName(const String& name);

    String GetFilePath(const String& fpnp);

    String GetNamespace(const String& fpnp);

    String MacroName(const String& name);

    String CppFullName(const String& name);

    String ConstantName(const String& name);

    const std::string UnderlineAdded(const String& name);
};
}
}
#endif // OHOS_IDL_CPPCODEEMITTER_H
