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

#ifndef OHOS_ZIDL_JAVACODEEMITTER_H
#define OHOS_ZIDL_JAVACODEEMITTER_H

#include <string>

#include "codegen/code_emitter.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Zidl {

class JavaCodeEmitter : public CodeEmitter {
public:
    JavaCodeEmitter(
        /* [in] */ MetaComponent* mc)
        : CodeEmitter(mc)
    {}

    void EmitInterface() override;

    void EmitInterfaceProxy() override;

    void EmitInterfaceStub() override;

private:
    void EmitInterfaceImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceCorelibImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceDBinderImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceDefinition(
        /* [in] */ StringBuilder& sb);

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

    void EmitInterfaceProxyImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyCorelibImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxySelfDefinedTypeImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyDBinderImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyParametersImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyImpl(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceProxyConstants(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyConstructor(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

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

    void EmitInterfaceStubImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubCorelibImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubSelfDefinedTypeImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubDBinderImports(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubImpl(
        /* [in] */ StringBuilder& sb);

    void EmitInterfaceStubConstants(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubConstructor(
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

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

    void EmitPackage(
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
        /* [in] */ unsigned int attributes,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReadOutVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReadArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ unsigned int attributes,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitWriteArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitWriteOutArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitReadOutArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    void EmitLocalVariable(
        /* [in] */ MetaParameter* mp,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix);

    String EmitType(
        /* [in] */ MetaType* mt);

    String FileName(
        /* [in] */ const String& name);

    String MethodName(
        /* [in] */ const String& name);

    String ConstantName(
        /* [in] */ const String& name);

    String StubName(
      /* [in] */ const String& name);

    const std::string UnderlineAdded(const String& name);
};

}
}

#endif // OHOS_ZIDL_JAVACODEEMITTER_H
