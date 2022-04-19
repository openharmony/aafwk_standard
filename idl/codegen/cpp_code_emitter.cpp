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

#include "codegen/cpp_code_emitter.h"

#include "securec.h"
#include "util/file.h"

namespace OHOS {
namespace Idl {
void CppCodeEmitter::EmitInterface()
{
    EmitInterfaceHeaderFile();
}

void CppCodeEmitter::EmitInterfaceHeaderFile()
{
    String filePath = String::Format("%s/%s.h", directory_.string(), FileName(interfaceName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    sb.Append("\n");
    EmitHeadMacro(sb, interfaceFullName_);
    sb.Append("\n");
    EmitInterfaceInclusions(sb);
    sb.Append("\n");
    if (EmitInterfaceUsings(sb)) {
        sb.Append("\n");
    }
    EmitInterfaceDefinition(sb);
    EmitTailMacro(sb, interfaceFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCodeEmitter::EmitInterfaceInclusions(StringBuilder& sb)
{
    EmitInterfaceStdlibInclusions(sb);
    EmitInterfaceDBinderInclusions(sb);
    EmitInterfaceSelfDefinedTypeInclusions(sb);
}

void CppCodeEmitter::EmitInterfaceStdlibInclusions(StringBuilder& sb)
{
    bool includeString = false;
    bool includeList = false;
    bool includeMap = false;
    bool includeNum = false;
    for (int i = 0; i < metaComponent_->typeNumber_; i++) {
        MetaType* mt = metaComponent_->types_[i];
        switch(mt->kind_) {
            case TypeKind::Byte: {
                if (!includeNum) {
                    sb.Append("#include <cstdint>\n");
                    includeNum = true;
                }
                break;
            }
            case TypeKind::String: {
                if (!includeString) {
                    sb.Append("#include <string_ex.h>\n");
                    includeString = true;
                }
                break;
            }
            case TypeKind::Array:
            case TypeKind::List: {
                if (!includeList) {
                    sb.Append("#include <vector>\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::Map: {
                if (!includeMap) {
                    sb.Append("#include <unordered_map>\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void CppCodeEmitter::EmitInterfaceDBinderInclusions(StringBuilder& sb)
{
    sb.Append("#include <iremote_broker.h>\n");
}

String CppCodeEmitter::GetFilePath(const String& fpnp)
{
    int pos = fpnp.IndexOf("..");
    if (pos == -1) {
        return String();
    }
    String res = fpnp.Substring(0, pos + 1);
    return res;
}

String CppCodeEmitter::GetNamespace(const String& fpnp)
{
    int pos = fpnp.IndexOf("..");
    if (pos == -1) {
        return fpnp;
    }
    String res = fpnp.Substring(pos + 2);
    return res;
}

void CppCodeEmitter::EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb)
{
    for (int i = 0; i < metaComponent_->sequenceableNumber_; i++) {
        MetaSequenceable* mp = metaComponent_->sequenceables_[i];
        String filePath = GetFilePath(String(mp->namespace_));
        String fileName = FileName(filePath + mp->name_);
        sb.Append("#include ").AppendFormat("\"%s.h\"\n", fileName.string());
    }

    for (int i = 0; i < metaComponent_->interfaceNumber_; i++) {
        MetaInterface* mi = metaComponent_->interfaces_[i];
        if (mi->external_) {
            String filePath = GetFilePath(String(mi->namespace_));
            String fileName = FileName(filePath + mi->name_);
            sb.Append("#include ").AppendFormat("\"%s.h\"\n", fileName.string());
        }
    }
}

bool CppCodeEmitter::EmitInterfaceUsings(StringBuilder& sb)
{
    bool ret = false;
    for (int i = 0; i < metaComponent_->sequenceableNumber_; i++) {
        MetaSequenceable* mp = metaComponent_->sequenceables_[i];
        String np = GetNamespace(String(mp->namespace_));
        if (np.IsEmpty()) {
            continue;
        }
        String fullName = CppFullName(np + mp->name_);
        sb.Append("using ").AppendFormat("%s;\n", fullName.string());
        ret = true;
    }

    for (int i = 0; i < metaComponent_->interfaceNumber_; i++) {
        MetaInterface* mi = metaComponent_->interfaces_[i];
        if (mi->external_) {
            String np = GetNamespace(String(mi->namespace_));
            if (np.IsEmpty()) {
                continue;
            }
            String fullName = CppFullName(np + mi->name_);
            sb.Append("using ").AppendFormat("%s;\n", fullName.string());
            ret = true;
        }
    }
    return ret;
}

void CppCodeEmitter::EmitInterfaceDefinition(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.AppendFormat("class %s : public IRemoteBroker {\n", metaInterface_->name_);
    sb.Append("public:\n");
    EmitInterfaceBody(sb, TAB);
    sb.Append("};\n");
    EmitEndNamespace(sb);
}

void CppCodeEmitter::EmitInterfaceBody(StringBuilder& sb, const String& prefix)
{
    String nameWithoutPath = GetNamespace(interfaceFullName_);
    sb.Append(TAB).AppendFormat("DECLARE_INTERFACE_DESCRIPTOR(u\"%s\");\n", nameWithoutPath.string());
    sb.Append("\n");
    EmitInterfaceMethods(sb, TAB);
}

void CppCodeEmitter::EmitInterfaceMethods(StringBuilder& sb, const String& prefix)
{
    if (metaInterface_->methodNumber_ > 0) {
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            EmitInterfaceMethod(mm, sb, prefix);
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void CppCodeEmitter::EmitInterfaceMethod(MetaMethod* mm, StringBuilder& sb, const String& prefix)
{
    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (mm->parameterNumber_ == 0 && returnType->kind_ == TypeKind::Void) {
        sb.Append(prefix).AppendFormat("virtual ErrCode %s() = 0;\n", mm->name_);
    } else {
        sb.Append(prefix).AppendFormat("virtual ErrCode %s(\n", mm->name_);
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            EmitInterfaceMethodParameter(mp, sb, prefix + TAB);
            if (i != mm->parameterNumber_ - 1 || returnType->kind_ != TypeKind::Void) {
                sb.Append(",\n");
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitInterfaceMethodReturn(returnType, sb, prefix + TAB);
        }
        sb.Append(") = 0;\n");
    }
}

void CppCodeEmitter::EmitInterfaceMethodParameter(MetaParameter* mp, StringBuilder& sb, const String& prefix)
{
    if ((mp->attributes_ & ATTR_MASK) == (ATTR_IN | ATTR_OUT)) {
        sb.Append(prefix).Append("/* [in, out] */ ");
    } else if (mp->attributes_ & ATTR_IN) {
        sb.Append(prefix).Append("/* [in] */ ");
    } else {
        sb.Append(prefix).Append("/* [out] */ ");
    }

    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    const std::string name = UnderlineAdded(mp->name_);
    sb.AppendFormat("%s %s", EmitType(mt, mp->attributes_, false).string(), name.c_str());
}

void CppCodeEmitter::EmitInterfaceMethodReturn(MetaType* mt, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("/* [out] */ %s result", EmitType(mt, ATTR_OUT, false).string());
}

void CppCodeEmitter::EmitInterfaceProxy()
{
    EmitInterfaceProxyHeaderFile();
    EmitInterfaceProxyCppFile();
}

void CppCodeEmitter::EmitInterfaceProxyHeaderFile()
{
    String filePath = String::Format("%s/%s.h", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    sb.Append("\n");
    EmitHeadMacro(sb, proxyFullName_);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("#include <iremote_proxy.h>\n");
    sb.Append("\n");
    EmitInterfaceProxyInHeaderFile(sb);
    EmitTailMacro(sb, proxyFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCodeEmitter::EmitInterfaceProxyInHeaderFile(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.AppendFormat("class %s : public IRemoteProxy<%s> {\n", proxyName_.string(), interfaceName_.string());
    sb.Append("public:\n");
    EmitInterfaceProxyConstructor(sb, TAB);
    sb.Append("\n");
    EmitInterfaceProxyMethodDecls(sb, TAB);
    sb.Append("\n");
    sb.Append("private:\n");
    EmitInterfaceProxyConstants(sb, TAB);
    sb.Append("};\n");
    EmitEndNamespace(sb);
}

void CppCodeEmitter::EmitInterfaceProxyConstructor(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("explicit %s(\n", proxyName_.string());
    sb.Append(prefix + TAB).Append("/* [in] */ const sptr<IRemoteObject>& remote)\n");
    sb.Append(prefix + TAB).AppendFormat(": IRemoteProxy<%s>(remote)\n", interfaceName_.string());
    sb.Append(prefix).Append("{}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("virtual ~%s()\n", proxyName_.string());
    sb.Append(prefix).Append("{}\n");
}

void CppCodeEmitter::EmitInterfaceProxyMethodDecls(StringBuilder& sb, const String& prefix)
{
    if (metaInterface_->methodNumber_ > 0) {
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            EmitInterfaceProxyMethodDecl(mm, sb, prefix);
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void CppCodeEmitter::EmitInterfaceProxyMethodDecl(MetaMethod* mm, StringBuilder& sb, const String& prefix)
{
    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (mm->parameterNumber_ == 0 && returnType->kind_ == TypeKind::Void) {
        sb.Append(prefix).AppendFormat("ErrCode %s() override;\n", mm->name_);
    } else {
        sb.Append(prefix).AppendFormat("ErrCode %s(\n", mm->name_);
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            EmitInterfaceMethodParameter(mp, sb, prefix + TAB);
            if (i != mm->parameterNumber_ - 1 || returnType->kind_ != TypeKind::Void) {
                sb.Append(",\n");
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitInterfaceMethodReturn(returnType, sb, prefix + TAB);
        }
        sb.Append(") override;\n");
    }
}

void CppCodeEmitter::EmitInterfaceProxyConstants(StringBuilder& sb, const String& prefix)
{
    EmitInterfaceMethodCommands(sb, prefix);
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("static inline BrokerDelegator<%s> delegator_;\n", proxyName_.string());
}

void CppCodeEmitter::EmitInterfaceProxyCppFile()
{
    String filePath = String::Format("%s/%s.cpp", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(proxyName_).string());
    sb.Append("\n");
    EmitBeginNamespace(sb);
    EmitInterfaceProxyMethodImpls(sb, "");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCodeEmitter::EmitInterfaceProxyMethodImpls(StringBuilder& sb, const String& prefix)
{
    if (metaInterface_->methodNumber_ > 0) {
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            EmitInterfaceProxyMethodImpl(mm, sb, prefix);
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void CppCodeEmitter::EmitInterfaceProxyMethodImpl(MetaMethod* mm, StringBuilder& sb, const String& prefix)
{
    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (mm->parameterNumber_ == 0 && returnType->kind_ == TypeKind::Void) {
        sb.Append(prefix).AppendFormat("ErrCode %s::%s()\n", proxyName_.string(), mm->name_);
    } else {
        sb.Append(prefix).AppendFormat("ErrCode %s::%s(\n", proxyName_.string(), mm->name_);
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            EmitInterfaceMethodParameter(mp, sb, prefix + TAB);
            if (i != mm->parameterNumber_ - 1 || returnType->kind_ != TypeKind::Void) {
                sb.Append(",\n");
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitInterfaceMethodReturn(returnType, sb, prefix + TAB);
        }
        sb.Append(")\n");
    }
    EmitInterfaceProxyMethodBody(mm, sb, prefix);
}

void CppCodeEmitter::EmitInterfaceProxyMethodBody(MetaMethod* mm, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("MessageParcel data;\n");
    sb.Append(prefix + TAB).Append("MessageParcel reply;\n");
    sb.Append(prefix + TAB).AppendFormat("MessageOption option(%s);\n",
        (mm->properties_ & METHOD_PROPERTY_ONEWAY) != 0 ? "MessageOption::TF_ASYNC" : "MessageOption::TF_SYNC");
    sb.Append("\n");

    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            EmitWriteMethodParameter(mp, "data.", sb, prefix + TAB);
        }
    }
    sb.Append("\n");
    sb.Append(prefix + TAB).AppendFormat("int32_t st = Remote()->SendRequest(COMMAND_%s, data, reply, option);\n",
        ConstantName(mm->name_).string());
    sb.Append(prefix + TAB).Append("if (st != ERR_NONE) {\n");
    sb.Append(prefix + TAB).Append("    return st;\n");
    sb.Append(prefix + TAB).Append("}\n");
    if ((mm->properties_ & METHOD_PROPERTY_ONEWAY) == 0) {
        sb.Append("\n");
        sb.Append(prefix + TAB).Append("ErrCode ec = reply.ReadInt32();\n");
        sb.Append(prefix + TAB).Append("if (FAILED(ec)) {\n");
        sb.Append(prefix + TAB).Append("    return ec;\n");
        sb.Append(prefix + TAB).Append("}\n");
        sb.Append("\n");
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                EmitReadMethodParameter(mp, "reply.", sb, prefix + TAB);
            }
        }
        MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
        if (returnType->kind_ != TypeKind::Void) {
            EmitReadVariable("reply.", "result", returnType, sb, prefix + TAB, false);
        }
    }
    sb.Append(prefix + TAB).Append("return ERR_OK;\n");
    sb.Append(prefix).Append("}\n");
}

void CppCodeEmitter::EmitWriteMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& sb,
    const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    const std::string name = UnderlineAdded(mp->name_);
    EmitWriteVariable(parcelName, name, mt, sb, prefix);
}

void CppCodeEmitter::EmitReadMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& sb,
    const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    const std::string name = UnderlineAdded(mp->name_);
    EmitReadVariable(parcelName, name, mt, sb, prefix, false);
}

void CppCodeEmitter::EmitInterfaceStub()
{
    EmitInterfaceStubHeaderFile();
    EmitInterfaceStubCppFile();
}

void CppCodeEmitter::EmitInterfaceStubHeaderFile()
{
    String filePath = String::Format("%s/%s.h", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    sb.Append("\n");
    EmitHeadMacro(sb, stubFullName_);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("#include <iremote_stub.h>\n");
    sb.Append("\n");
    EmitInterfaceStubInHeaderFile(sb);
    EmitTailMacro(sb, stubFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCodeEmitter::EmitInterfaceStubInHeaderFile(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.AppendFormat("class %s : public IRemoteStub<%s> {\n", stubName_.string(), interfaceName_.string());
    sb.Append("public:\n");
    EmitInterfaceStubMethodDecls(sb, TAB);
    sb.Append("\n");
    sb.Append("private:\n");
    EmitInterfaceStubConstants(sb, TAB);
    sb.Append("};\n");
    EmitEndNamespace(sb);
}

void CppCodeEmitter::EmitInterfaceStubMethodDecls(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("int OnRemoteRequest(\n");
    sb.Append(prefix + TAB).Append("/* [in] */ uint32_t code,\n");
    sb.Append(prefix + TAB).Append("/* [in] */ MessageParcel& data,\n");
    sb.Append(prefix + TAB).Append("/* [out] */ MessageParcel& reply,\n");
    sb.Append(prefix + TAB).Append("/* [in] */ MessageOption& option) override;\n");
}

void CppCodeEmitter::EmitInterfaceStubConstants(StringBuilder& sb, const String& prefix)
{
    EmitInterfaceMethodCommands(sb, prefix);
}

void CppCodeEmitter::EmitInterfaceStubCppFile()
{
    String filePath = String::Format("%s/%s.cpp", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
    sb.Append("\n");
    EmitBeginNamespace(sb);
    EmitInterfaceStubMethodImpls(sb, "");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCodeEmitter::EmitInterfaceStubMethodImpls(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int %s::OnRemoteRequest(\n", stubName_.string());
    sb.Append(prefix + TAB).Append("/* [in] */ uint32_t code,\n");
    sb.Append(prefix + TAB).Append("/* [in] */ MessageParcel& data,\n");
    sb.Append(prefix + TAB).Append("/* [out] */ MessageParcel& reply,\n");
    sb.Append(prefix + TAB).Append("/* [in] */ MessageOption& option)\n");
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("switch (code) {\n");
    for (int i = 0; i < metaInterface_->methodNumber_; i++) {
        MetaMethod* mm = metaInterface_->methods_[i];
        EmitInterfaceStubMethodImpl(mm, sb, prefix + TAB + TAB);
    }
    sb.Append(prefix + TAB).Append(TAB).Append("default:\n");
    sb.Append(prefix + TAB).Append(TAB).Append(TAB).Append(
        "return IPCObjectStub::OnRemoteRequest(code, data, reply, option);\n");
    sb.Append(prefix + TAB).Append("}\n\n");
    sb.Append(prefix + TAB).Append("return ERR_TRANSACTION_FAILED;\n");
    sb.Append(prefix).Append("}\n");
}

void CppCodeEmitter::EmitInterfaceStubMethodImpl(MetaMethod* mm, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("case COMMAND_%s: {\n", ConstantName(mm->name_).string());
    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            MetaType* mt = metaComponent_->types_[mp->typeIndex_];
            const std::string name = UnderlineAdded(mp->name_);
            EmitReadVariable("data.", name, mt, sb, prefix + TAB);
        } else if ((mp->attributes_ & ATTR_OUT) != 0) {
            EmitLocalVariable(mp, sb, prefix + TAB);
        }
    }
    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (returnType->kind_ != TypeKind::Void) {
        if ((returnType->kind_ == TypeKind::Sequenceable) || (returnType->kind_ == TypeKind::Interface)) {
            sb.Append(prefix + TAB).AppendFormat("%s result = nullptr;\n",
                EmitType(returnType, ATTR_IN, true).string());
        } else {
            sb.Append(prefix + TAB).AppendFormat("%s result;\n", EmitType(returnType, ATTR_IN, true).string());
        }
    }
    if (mm->parameterNumber_ == 0 && returnType->kind_ == TypeKind::Void) {
        sb.Append(prefix + TAB).AppendFormat("ErrCode ec = %s();\n", mm->name_);
    } else {
        sb.Append(prefix + TAB).AppendFormat("ErrCode ec = %s(", mm->name_);
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            const std::string name = UnderlineAdded(mp->name_);
            sb.Append(name.c_str());
            if (i != mm->parameterNumber_ - 1 || returnType->kind_ != TypeKind::Void) {
                sb.Append(", ");
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitReturnParameter("result", returnType, sb);
        }
        sb.AppendFormat(");\n", mm->name_);
    }
    sb.Append(prefix + TAB).Append("reply.WriteInt32(ec);\n");
    bool hasOutParameter = false;
    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            hasOutParameter = true;
        }
    }
    if (hasOutParameter || returnType->kind_ != TypeKind::Void) {
        sb.Append(prefix + TAB).Append("if (SUCCEEDED(ec)) {\n");
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                EmitWriteMethodParameter(mp, "reply.", sb, prefix + TAB + TAB);
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitWriteVariable("reply.", "result", returnType, sb, prefix + TAB + TAB);
        }
        sb.Append(prefix + TAB).Append("}\n");
    }
    sb.Append(prefix + TAB).Append("return ERR_NONE;\n");
    sb.Append(prefix).Append("}\n");
}

void CppCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix)
{
    for (int i = 0; i < metaInterface_->methodNumber_; i++) {
        MetaMethod* mm = metaInterface_->methods_[i];
        sb.Append(prefix).AppendFormat("static constexpr int COMMAND_%s = MIN_TRANSACTION_ID + %d;\n",
            ConstantName(mm->name_).string(), i);
    }
}

void CppCodeEmitter::EmitLicense(StringBuilder& sb)
{
    sb.Append(metaInterface_->license_).Append("\n");
}

void CppCodeEmitter::EmitHeadMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#ifndef ").Append(macroName).Append("\n");
    sb.Append("#define ").Append(macroName).Append("\n");
}

void CppCodeEmitter::EmitTailMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#endif // ").Append(macroName).Append("\n\n");
}

void CppCodeEmitter::EmitBeginNamespace(StringBuilder& sb)
{
    String nspace = GetNamespace(metaInterface_->namespace_);
    int index = nspace.IndexOf('.');
    while (index != -1) {
        sb.AppendFormat("namespace %s {\n", nspace.Substring(0, index).string());
        nspace = nspace.Substring(index + 1);
        index = nspace.IndexOf('.');
    }
}

void CppCodeEmitter::EmitEndNamespace(StringBuilder& sb)
{
    String nspace = GetNamespace(metaInterface_->namespace_);
    nspace = nspace.Substring(0, nspace.GetLength() - 1);
    while (!nspace.IsEmpty()) {
        int index = nspace.LastIndexOf('.');
        sb.AppendFormat("} // namespace %s\n", index != -1 ?
            nspace.Substring(index + 1, nspace.GetLength()).string() : nspace.string());
        nspace = nspace.Substring(0, index);
    }
}

void CppCodeEmitter::EmitWriteVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& sb, const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("%sWriteInt32(%s ? 1 : 0);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("%sWriteInt32(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("%sWriteInt64(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("%sWriteFloat(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("%sWriteDouble(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("%sWriteString16(Str8ToStr16(%s));\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable:
            sb.Append(prefix).AppendFormat("%sWriteParcelable(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Interface:
            sb.Append(prefix).AppendFormat("%sWriteRemoteObject(%s->AsObject());\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Array:
        case TypeKind::List: {
            sb.Append(prefix).AppendFormat("%sWriteInt32(%s.size());\n", parcelName.string(), name.c_str());
            sb.Append(prefix).AppendFormat("for (auto it = %s.begin(); it != %s.end(); ++it) {\n",
                name.c_str(), name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitWriteVariable(parcelName, "(*it)", innerType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            sb.Append(prefix).AppendFormat("%sWriteInt32(%s.size());\n", parcelName.string(), name.c_str());
            sb.Append(prefix).AppendFormat("for (auto it = %s.begin(); it != %s.end(); ++it) {\n",
                name.c_str(), name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitWriteVariable(parcelName, "(it->first)", keyType, sb, prefix + TAB);
            EmitWriteVariable(parcelName, "(it->second)", valueType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        default:
            break;
    }
}

void CppCodeEmitter::EmitReadVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& sb, const String& prefix, bool emitType)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadInt32() == 1 ? true : false;\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadInt32() == 1 ? true : false;\n",
                    name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = (%s)%sReadInt32();\n", EmitType(mt, ATTR_IN, true).string(),
                    name.c_str(), EmitType(mt, ATTR_IN, true).string(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = (%s)%sReadInt32();\n", name.c_str(),
                    EmitType(mt, ATTR_IN, true).string(), parcelName.string());
            }
            break;
        case TypeKind::Integer:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadInt32();\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadInt32();\n", name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::Long:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadInt64();\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadInt64();\n", name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::Float:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadFloat();\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadFloat();\n", name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::Double:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadDouble();\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadDouble();\n", name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::String:
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = Str16ToStr8(%sReadString16());\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = Str16ToStr8(%sReadString16());\n",
                    name.c_str(), parcelName.string());
            }
            break;
        case TypeKind::Sequenceable: {
            MetaSequenceable* mp = metaComponent_->sequenceables_[mt->index_];
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = %sReadParcelable<%s>();\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(), parcelName.string(), mp->name_);
            } else {
                sb.Append(prefix).AppendFormat("%s = %sReadParcelable<%s>();\n",
                    name.c_str(), parcelName.string(), mp->name_);
            }
            break;
        }
        case TypeKind::Interface: {
            MetaInterface* mi = metaComponent_->interfaces_[mt->index_];
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s = iface_cast<%s>(%sReadRemoteObject());\n",
                    EmitType(mt, ATTR_IN, true).string(), name.c_str(),  mi->name_, parcelName.string());
            } else {
                sb.Append(prefix).AppendFormat("%s = iface_cast<%s>(%sReadRemoteObject());\n",
                    name.c_str(),  mi->name_, parcelName.string());
            }
            break;
        }
        case TypeKind::Array:
        case TypeKind::List: {
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s;\n", EmitType(mt, ATTR_IN, true).string(), name.c_str());
            }
            sb.Append(prefix).AppendFormat("int %sSize = %sReadInt32();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitReadVariable(parcelName, "value", innerType, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.push_back(value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            if (emitType) {
                sb.Append(prefix).AppendFormat("%s %s;\n", EmitType(mt, ATTR_IN, true).string(), name.c_str());
            }
            sb.Append(prefix).AppendFormat("int %sSize = %sReadInt32();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s[key] = value;\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        default:
            break;
    }
}

void CppCodeEmitter::EmitLocalVariable(MetaParameter* mp, StringBuilder& sb, const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    const std::string name = UnderlineAdded(mp->name_);
    if ((mt->kind_ == TypeKind::Sequenceable) || (mt->kind_ == TypeKind::Interface)) {
        sb.Append(prefix).AppendFormat("%s %s = nullptr;\n", EmitType(mt, ATTR_IN, true).string(), name.c_str());
    } else {
        sb.Append(prefix).AppendFormat("%s %s;\n", EmitType(mt, ATTR_IN, true).string(), name.c_str());
    }
}

void CppCodeEmitter::EmitReturnParameter(const String& name, MetaType* mt, StringBuilder& sb)
{
    switch (mt->kind_) {
        case TypeKind::Char:
        case TypeKind::Boolean:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
        case TypeKind::Long:
        case TypeKind::Float:
        case TypeKind::Double:
        case TypeKind::String:
        case TypeKind::Sequenceable:
        case TypeKind::Interface:
        case TypeKind::List:
        case TypeKind::Map:
        case TypeKind::Array:
            sb.Append(name);
            break;
        default:
            break;
    }
}

String CppCodeEmitter::EmitType(MetaType* mt, unsigned int attributes, bool isInnerType)
{
    switch(mt->kind_) {
        case TypeKind::Char:
            if (attributes & ATTR_IN) {
                return "zchar";
            } else {
                return "zchar&";
            }
        case TypeKind::Boolean:
            if (attributes & ATTR_IN) {
                return "bool";
            } else {
                return "bool&";
            }
        case TypeKind::Byte:
            if (attributes & ATTR_IN) {
                return "int8_t";
            } else {
                return "int8_t&";
            }
        case TypeKind::Short:
            if (attributes & ATTR_IN) {
                return "short";
            } else {
                return "short&";
            }
        case TypeKind::Integer:
            if (attributes & ATTR_IN) {
                return "int";
            } else {
                return "int&";
            }
        case TypeKind::Long:
            if (attributes & ATTR_IN) {
                return "long";
            } else {
                return "long&";
            }
        case TypeKind::Float:
            if (attributes & ATTR_IN) {
                return "float";
            } else {
                return "float&";
            }
        case TypeKind::Double:
            if (attributes & ATTR_IN) {
                return "double";
            } else {
                return "double&";
            }
        case TypeKind::String:
            if (attributes & ATTR_IN) {
                if (!isInnerType) {
                    return "const std::string&";
                } else {
                    return "std::string";
                }
            } else {
                return "std::string&";
            }
        case TypeKind::Void:
            return "void";
        case TypeKind::Sequenceable: {
            MetaSequenceable* mp = metaComponent_->sequenceables_[mt->index_];
            if ((attributes & ATTR_MASK) == (ATTR_IN | ATTR_OUT)) {
                return String::Format("%s*", mp->name_);
            } else if (attributes & ATTR_IN) {
                return String::Format("%s*", mp->name_);
            } else {
                return String::Format("%s*", mp->name_);
            }
        }
        case TypeKind::Interface: {
            MetaInterface* mi = metaComponent_->interfaces_[mt->index_];
            if (attributes & ATTR_IN) {
                if (!isInnerType) {
                    return String::Format("sptr<%s>", mi->name_);
                } else {
                    return String::Format("sptr<%s>", mi->name_);
                }
            } else {
                return String::Format("sptr<%s>&", mi->name_);
            }
        }
        case TypeKind::Array:
        case TypeKind::List: {
            MetaType* elementType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            if (attributes & ATTR_OUT) {
                return String::Format("std::vector<%s>&",
                    EmitType(elementType, ATTR_IN, true).string());
            } else {
                if (!isInnerType) {
                    return String::Format("const std::vector<%s>&",
                        EmitType(elementType, ATTR_IN, true).string());
                } else {
                    return String::Format("std::vector<%s>",
                        EmitType(elementType, ATTR_IN, true).string());
                }
            }
        }
        case TypeKind::Map: {
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            if (attributes & ATTR_OUT) {
                return String::Format("std::unordered_map<%s, %s>&",
                    EmitType(keyType, ATTR_IN, true).string(), EmitType(valueType, ATTR_IN, true).string());
            } else {
                if (!isInnerType) {
                    return String::Format("const std::unordered_map<%s, %s>&",
                        EmitType(keyType, ATTR_IN, true).string(), EmitType(valueType, ATTR_IN, true).string());
                } else {
                    return String::Format("std::unordered_map<%s, %s>",
                        EmitType(keyType, ATTR_IN, true).string(), EmitType(valueType, ATTR_IN, true).string());
                }
            }
            break;
        }
        default:
            return "unknown type";
    }
}

String CppCodeEmitter::CppFullName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    return name.Replace(".", "::");
}

String CppCodeEmitter::FileName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            // 2->Index of the last char array.
            if (i > 1 && name[i - 1] != '.' && name[i - 2] != '.') {
                sb.Append('_');
            }
            sb.Append(tolower(c));
        } else {
            sb.Append(c);
        }
    }

    return sb.ToString().Replace('.', '/');
}

String CppCodeEmitter::MacroName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String macro = name.Replace('.', '_').ToUpperCase() + "_H";
    return macro;
}

String CppCodeEmitter::ConstantName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            if (i > 1) {
                sb.Append('_');
            }
            sb.Append(c);
        } else {
            sb.Append(toupper(c));
        }
    }

    return sb.ToString();
}

const std::string CppCodeEmitter::UnderlineAdded(const String& originName)
{
    std::string underline("_");
    return underline + std::string(originName.string());
}
}
}
