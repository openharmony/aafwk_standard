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

#ifndef OHOS_ZIDL_CPPCODEEMITTER_H
#define OHOS_ZIDL_CPPCODEEMITTER_H

#include <string>

#include "codegen/code_emitter.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Zidl {

class CppCodeEmitter : public CodeEmitter {
public:
    CppCodeEmitter(
        /* [in] */ MetaComponent* mc)
        : CodeEmitter(mc)
    {}

    void EmitInterface() override;

    void EmitInterfaceProxy() override;

    void EmitInterfaceStub() override;

private:
    void EmitInterfaceHeaderFile();

    void EmitInterfaceInclusions(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStdlibInclusions(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceDBinderInclusions(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceParametersInclusions(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeInclusions(
        /* [in] */ StringBuilder& sb);

    bool EmitInterfaceUsings(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeUsings(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceDefinition(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceBody(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethods(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethod(
        /* [in] */ MetaMethod* mm,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodReturn(
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyHeaderFile();

    void EmitInterfaceProxyInHeaderFile(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyConstructor(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodDecls(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodDecl(
        /* [in] */ MetaMethod* mm,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyConstants(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyCppFile();

    void EmitInterfaceProxyMethodImpls(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodImpl(
        /* [in] */ MetaMethod* mm,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodBody(
        /* [in] */ MetaMethod* mm,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitWriteMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ const String& parcelName,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReadMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ const String& parcelName,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubHeaderFile();

    void EmitInterfaceStubInHeaderFile(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubMethodDecls(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubConstants(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubCppFile();

    void EmitInterfaceStubMethodImpls(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubMethodImpl(
        /* [in] */ MetaMethod* mm,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodCommands(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitLicense(
        /* [in] */ StringBuilder& sb);

    void EmitHeadMacro(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& fullName);

    void EmitTailMacro(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& fullName);

    void EmitBeginNamespace(
        /* [in] */ StringBuilder& sb);

    void EmitEndNamespace(
        /* [in] */ StringBuilder& sb);

    void EmitWriteVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReadVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix,
        /* [in] */ bool emitType = true);

    void EmitLocalVariable(
        /* [in] */ MetaParameter* mp,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReturnParameter(
        /* [in] */ const String& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb);

    String EmitType(
        /* [in] */ MetaType* mt,
        /* [in] */ unsigned int attributes,
        /* [in] */ bool isInnerType);

    String FileName(
        /* [in] */ const String& name);

    String GetFilePath(const String& fpnp);

    String GetNamespace(const String& fpnp);

    String MacroName(
        /* [in] */ const String& name);

    String CppFullName(
        /* [in] */ const String& name);

    String ConstantName(
        /* [in] */ const String& name);

    const std::string UnderlineAdded(const String& name);
};

}
}

#endif // OHOS_ZIDL_CPPCODEEMITTER_H
