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

#include "codegen/ts_code_emitter.h"

#include <cctype>
#include <cstdio>
#include <algorithm>

#include "securec.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace Idl {
namespace {
const std::string PROXY = "proxy";
const std::string THIS_PROXY = "this.proxy";
static const char* TAG = "TsCodeEmitter";
const std::string UNKNOWN_TYPE = "unknown type";
const String NEWLINE = "\n";
const String RETURN_VALUE = "returnValue";
const std::string ERR_CODE_TYPE = "errCode: number";
const String ERR_CODE = "errCode";
}

void TsCodeEmitter::EmitInterface()
{
    if (!CheckInterfaceType()) {
        return;
    }
    String filePath = String::Format("%s/%s.ts", directory_.string(), FileName(interfaceName_).string());
    File file(filePath, File::WRITE);

    StringBuilder stringBuilder;

    EmitLicense(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceSelfDefinedTypeImports(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceDefinition(stringBuilder);
    stringBuilder.Append(NEWLINE);

    String data = stringBuilder.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void TsCodeEmitter::EmitInterfaceImports(StringBuilder& stringBuilder)
{
    for (const auto &item : methods_) {
        if (item.callbackName_.size() > 0) {
            stringBuilder.AppendFormat("import {%s} from \"./%s\";\n",
                item.callbackName_.c_str(), FileName(interfaceName_).string());
        }
    }
    stringBuilder.AppendFormat("import %s from \"./%s\";\n", metaInterface_->name_, FileName(interfaceName_).string());
    stringBuilder.Append("import rpc from \"@ohos.rpc\";\n");
    EmitInterfaceSelfDefinedTypeImports(stringBuilder);

    for (int index = 0; index < metaComponent_->interfaceNumber_; index++) {
        MetaInterface* mi = metaComponent_->interfaces_[index];
        if (!interfaceName_.Equals(mi->name_)) {
            String dependInterface = mi->name_;
            String dependStubName = dependInterface.StartsWith("I") ?
                dependInterface.Substring(1) + "Stub" : dependInterface + "Stub";
            stringBuilder.AppendFormat("import %s from \"./%s\";\n", dependStubName.string(),
                FileName(dependStubName).string());
        }
    }
}

void TsCodeEmitter::EmitInterfaceSelfDefinedTypeImports(StringBuilder& stringBuilder)
{
    for (int index = 0; index < metaComponent_->sequenceableNumber_; index++) {
        MetaSequenceable* mp = metaComponent_->sequenceables_[index];
        stringBuilder.AppendFormat("import %s from \"./%s\";\n", mp->name_, FileName(mp->name_).string());
    }

    for (int index = 0; index < metaComponent_->interfaceNumber_; index++) {
        MetaInterface* mi = metaComponent_->interfaces_[index];
        if (mi->external_) {
            stringBuilder.AppendFormat("import %s from \"./%s\";\n", mi->name_, FileName(mi->name_).string());
        }
    }
}

void TsCodeEmitter::EmitInterfaceDefinition(StringBuilder& stringBuilder)
{
    stringBuilder.AppendFormat("export default interface %s {\n", metaInterface_->name_);
    EmitInterfaceMethods(stringBuilder, TAB);
    stringBuilder.Append("}\n");

    for (const auto &item : methods_) {
        if (item.callbackName_.size() > 0) {
            stringBuilder.AppendFormat("%s\n", item.exportFunction_.c_str());
        }
    }
}

void TsCodeEmitter::EmitInterfaceMethods(StringBuilder& stringBuilder, const String& prefix)
{
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        EmitInterfaceMethod(metaMethod, stringBuilder, prefix);
    }
}

void TsCodeEmitter::EmitInterfaceMethod(MetaMethod* metaMethod, StringBuilder& stringBuilder, const String& prefix)
{
    MetaType* returnType = metaComponent_->types_[metaMethod->returnTypeIndex_];
    Method method;
    method.properties_ = metaMethod->properties_;
    StringBuilder callbackName;
    callbackName.AppendFormat("%sCallback", MethodName(metaMethod->name_).string());
    method.callbackName_ = callbackName.ToString();
    method.exportFunction_ = "export type " + callbackName.ToString() + " = (" + ERR_CODE_TYPE.c_str();

    bool haveOutPara = false;
    StringBuilder methodStr;
    if (returnType->kind_ != TypeKind::Void) {
        method.retParameter_.name_ = RETURN_VALUE.string();
        method.retParameter_.type_ = EmitType(returnType).string();
    }
    methodStr.Append(prefix).AppendFormat("%s(", MethodName(metaMethod->name_).string());
    method.name_ = MethodName(metaMethod->name_).string();

    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        MetaType* paraType = metaComponent_->types_[mp->typeIndex_];
        Parameter para;
        para.attr_ = mp->attributes_;
        para.name_ = mp->name_;
        para.type_ = EmitType(paraType).string();
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            haveOutPara = true;
        }
        method.parameters_.emplace_back(para);
    }

    bool isLastParaTypeIn = false;
    for (size_t index = 0; index < method.parameters_.size(); index++) {
        if ((method.parameters_[index].attr_ & ATTR_IN) != 0) {
            EmitMethodInParameter(methodStr, method.parameters_[index].name_, method.parameters_[index].type_,
                prefix + TAB);
            if (index != method.parameters_.size() - 1) {
                methodStr.Append(", ");
            } else {
                isLastParaTypeIn = true;
            }
        }
    }
    std::sort(method.parameters_.begin(), method.parameters_.end());
    if (method.callbackName_.size() > 0) {
        if (!isLastParaTypeIn) {
            methodStr.AppendFormat("callback: %s", method.callbackName_.c_str());
        } else {
            methodStr.AppendFormat(", callback: %s", method.callbackName_.c_str());
        }
        if (method.retParameter_.name_.size() > 0) {
            if (!haveOutPara) {
                method.exportFunction_ +=
                    (", " + method.retParameter_.name_ + ": " + method.retParameter_.type_ + ") => void;");
            } else {
                method.exportFunction_ +=
                    (", " + method.retParameter_.name_ + ": " + method.retParameter_.type_ + ", ");
            }
        } else {
            if (!haveOutPara) {
                method.exportFunction_ += ") => void;";
            } else {
                method.exportFunction_ += ", ";
            }
        }
        for (size_t index = 0; index < method.parameters_.size(); index++) {
            bool isLast =  (index == method.parameters_.size() - 1) ? true : false;
            if ((method.parameters_[index].attr_ & ATTR_OUT) != 0) {
                EmitInterfaceMethodExportCallback(method, method.parameters_[index], isLast);
            }
        }
    }
    methodStr.Append("): void;\n");
    stringBuilder.Append(methodStr.ToString());
    methods_.emplace_back(method);
}

void TsCodeEmitter::EmitInterfaceMethodParameter(MetaParameter* mp, StringBuilder& stringBuilder, const String& prefix)
{
    MetaType* paraType = metaComponent_->types_[mp->typeIndex_];
    stringBuilder.AppendFormat("%s: %s", mp->name_,  EmitType(paraType).string());
}

void TsCodeEmitter::EmitMethodInParameter(StringBuilder& stringBuilder, const std::string& name,
    const std::string& type, const String& prefix)
{
    stringBuilder.AppendFormat("%s: %s", name.c_str(), type.c_str());
}

void TsCodeEmitter::EmitInterfaceMethodExportCallback(Method& m, const Parameter& para, bool isLast)
{
    StringBuilder exportCallback;
    exportCallback.Append(m.exportFunction_.c_str());
    if (isLast) {
        exportCallback.AppendFormat("%s: %s) => void;", para.name_.c_str(), para.type_.c_str());
    } else {
        exportCallback.AppendFormat("%s: %s, ", para.name_.c_str(), para.type_.c_str());
    }
    m.exportFunction_ = exportCallback.ToString();
}

void TsCodeEmitter::EmitInterfaceProxy()
{
    if (!CheckInterfaceType()) {
        return;
    }
    String filePath = String::Format("%s/%s.ts", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);

    StringBuilder stringBuilder;

    EmitLicense(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceImports(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceProxyImpl(stringBuilder);
    stringBuilder.Append(NEWLINE);

    String data = stringBuilder.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void TsCodeEmitter::EmitInterfaceProxyImpl(StringBuilder& stringBuilder)
{
    stringBuilder.AppendFormat("export default class %s implements %s {\n", proxyName_.string(),
        interfaceName_.string());
    EmitInterfaceProxyConstructor(stringBuilder, TAB);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceProxyMethodImpls(stringBuilder, TAB);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceMethodCommands(stringBuilder);
    stringBuilder.Append(TAB).AppendFormat("private %s", PROXY.c_str());
    stringBuilder.Append(NEWLINE);
    stringBuilder.Append("}\n");
}

void TsCodeEmitter::EmitInterfaceProxyConstructor(StringBuilder& stringBuilder, const String& prefix)
{
    stringBuilder.Append(prefix).AppendFormat("constructor(%s) {\n", PROXY.c_str());
    stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s = %s;\n", THIS_PROXY.c_str(), PROXY.c_str());
    stringBuilder.Append(prefix).Append("}\n");
}

void TsCodeEmitter::EmitInterfaceProxyMethodImpls(StringBuilder& stringBuilder, const String& prefix)
{
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        EmitInterfaceProxyMethodImpl(metaMethod, index, stringBuilder, prefix);
        if (index != metaInterface_->methodNumber_ - 1) {
            stringBuilder.Append(NEWLINE);
        }
    }
}

void TsCodeEmitter::EmitInterfaceProxyMethodImpl(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
    const String& prefix)
{
    stringBuilder.Append(prefix).AppendFormat("%s(", MethodName(metaMethod->name_).string());
    bool isLastParaTypeIn = false;
    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            EmitInterfaceMethodParameter(mp, stringBuilder, prefix + TAB);
            if (index != metaMethod->parameterNumber_ - 1) {
                stringBuilder.Append(", ");
            } else {
                isLastParaTypeIn = true;
            }
        }
    }
    if (methods_[methodIndex].callbackName_.size() > 0) {
        if (!isLastParaTypeIn) {
            stringBuilder.AppendFormat("callback: %s", methods_[methodIndex].callbackName_.c_str());
        } else {
            stringBuilder.AppendFormat(", callback: %s", methods_[methodIndex].callbackName_.c_str());
        }
    }
    stringBuilder.Append("): void\n");
    EmitInterfaceProxyMethodBody(metaMethod, methodIndex, stringBuilder, prefix);
}

void TsCodeEmitter::EmitInterfaceProxyMethodBody(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
    const String& prefix)
{
    bool haveOutPara = false;
    stringBuilder.Append(prefix).Append("{\n");
    stringBuilder.Append(prefix).Append(TAB).AppendFormat("let _option = new rpc.MessageOption(%s);\n",
        ((metaMethod->properties_ & METHOD_PROPERTY_ONEWAY) || (metaInterface_->properties_ & METHOD_PROPERTY_ONEWAY))
        != 0 ? "rpc.MessageOption.TF_ASYNC" : "rpc.MessageOption.TF_SYNC");
    stringBuilder.Append(prefix).Append(TAB).Append("let _data = new rpc.MessageParcel();\n");
    stringBuilder.Append(prefix).Append(TAB).Append("let _reply = new rpc.MessageParcel();\n");

    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            EmitWriteMethodParameter(mp, "_data", stringBuilder, prefix + TAB);
        }
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            haveOutPara = true;
        }
    }
    stringBuilder.Append(prefix).Append(TAB).AppendFormat(
        "%s.sendRequest(%s.COMMAND_%s, _data, _reply, _option).then(function(result) {\n", THIS_PROXY.c_str(),
        proxyName_.string(), ConstantName(metaMethod->name_).string());
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).AppendFormat("if (result.errCode === 0) {\n");
    MetaType* returnType = metaComponent_->types_[metaMethod->returnTypeIndex_];
    // emit errCode
    MetaType errCode;
    errCode.kind_ = TypeKind::Integer;
    EmitReadOutVariable("result.reply", UnderlineAdded(ERR_CODE).c_str(), &errCode, stringBuilder,
        prefix + TAB + TAB + TAB);

    if (returnType->kind_ != TypeKind::Void || haveOutPara) {
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).AppendFormat(
            "if (%s != 0) {\n", UnderlineAdded(ERR_CODE).c_str());
        for (size_t index = 0; index < methods_[methodIndex].parameters_.size(); index++) {
            if ((methods_[methodIndex].parameters_[index].attr_ & ATTR_OUT) != 0) {
                stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(TAB).AppendFormat(
                    "let %s = undefined;\n", UnderlineAdded(
                        methods_[methodIndex].parameters_[index].name_.c_str()).c_str());
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(TAB).AppendFormat(
                "let %s = undefined;\n", UnderlineAdded(RETURN_VALUE).c_str());
        }
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(TAB).AppendFormat("callback(");
        stringBuilder.AppendFormat("%s", UnderlineAdded(ERR_CODE).c_str());
        if (methods_[methodIndex].retParameter_.name_.size() > 0) {
            if (haveOutPara) {
                stringBuilder.AppendFormat(", %s, ", UnderlineAdded(RETURN_VALUE).c_str());
            } else {
                stringBuilder.AppendFormat(", %s", UnderlineAdded(RETURN_VALUE).c_str());
            }
        } else {
            if (haveOutPara) {
                stringBuilder.Append(",");
            }
        }
        for (size_t index = 0; index < methods_[methodIndex].parameters_.size(); index++) {
            if ((methods_[methodIndex].parameters_[index].attr_ & ATTR_OUT) != 0) {
                stringBuilder.AppendFormat("%s",
                    UnderlineAdded(methods_[methodIndex].parameters_[index].name_.c_str()).c_str());
                if (index != methods_[methodIndex].parameters_.size() - 1) {
                    stringBuilder.Append(", ");
                }
            }
        }
        stringBuilder.Append(");\n");
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(TAB).Append("return;\n");
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append("}\n");
    }
    // emit return
    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            EmitReadMethodParameter(mp, "result.reply", stringBuilder, prefix + TAB + TAB + TAB);
        }
    }
    if (returnType->kind_ != TypeKind::Void) {
        String parcelName = "result.reply";
        EmitReadOutVariable(parcelName, UnderlineAdded(RETURN_VALUE), returnType, stringBuilder,
            prefix + TAB + TAB + TAB);
    }
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).AppendFormat("callback(");
    stringBuilder.AppendFormat("%s", UnderlineAdded(ERR_CODE).c_str());
    if (methods_[methodIndex].retParameter_.name_.size() > 0) {
        if (haveOutPara) {
            stringBuilder.AppendFormat(", %s, ", UnderlineAdded(RETURN_VALUE).c_str());
        } else {
            stringBuilder.AppendFormat(", %s", UnderlineAdded(RETURN_VALUE).c_str());
        }
    } else {
        if (haveOutPara) {
            stringBuilder.Append(",");
        }
    }
    for (size_t index = 0; index < methods_[methodIndex].parameters_.size(); index++) {
        if ((methods_[methodIndex].parameters_[index].attr_ & ATTR_OUT) != 0) {
            stringBuilder.AppendFormat("%s",
                UnderlineAdded(methods_[methodIndex].parameters_[index].name_.c_str()).c_str());
            if (index != methods_[methodIndex].parameters_.size() - 1) {
                stringBuilder.Append(", ");
            }
        }
    }
    stringBuilder.Append(");\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append("} else {\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(
        "console.log(\"sendRequest failed, errCode: \" + result.errCode);\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append("}\n");
    stringBuilder.Append(prefix).Append(TAB).Append("})\n");
    stringBuilder.Append(prefix).Append("}\n");
}

void TsCodeEmitter::EmitWriteMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& stringBuilder,
    const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    std::string name = mp->name_;
    EmitWriteVariable(parcelName, name, mt, stringBuilder, prefix);
}

void TsCodeEmitter::EmitReadMethodParameter(MetaParameter* mp, const String& parcelName, StringBuilder& stringBuilder,
    const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    std::string name = UnderlineAdded(mp->name_);
    EmitReadOutVariable(parcelName, name, mt, stringBuilder, prefix);
}

void TsCodeEmitter::EmitInterfaceStub()
{
    if (!CheckInterfaceType()) {
        return;
    }
    String filePath = String::Format("%s/%s.ts", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);

    StringBuilder stringBuilder;
    EmitLicense(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceImports(stringBuilder);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceStubImpl(stringBuilder);
    stringBuilder.Append(NEWLINE);

    String data = stringBuilder.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void TsCodeEmitter::EmitInterfaceStubImpl(StringBuilder& stringBuilder)
{
    stringBuilder.AppendFormat("export default class %s extends rpc.RemoteObject implements %s {\n",
        stubName_.string(), interfaceName_.string());
    EmitInterfaceStubMethodImpls(stringBuilder, TAB);
    stringBuilder.Append(NEWLINE);
    EmitInterfaceMethodCommands(stringBuilder);
    stringBuilder.Append("}\n");
}

void TsCodeEmitter::EmitInterfaceStubConstructor(StringBuilder& stringBuilder, const String& prefix)
{
    stringBuilder.Append(prefix).Append("constructor(des: string) {\n");
    stringBuilder.Append(prefix).Append(TAB).Append("super(des);\n");
    stringBuilder.Append(prefix).Append("}\n");
}

void TsCodeEmitter::EmitInterfaceStubMethodImpls(StringBuilder& stringBuilder, const String& prefix)
{
    EmitInterfaceStubConstructor(stringBuilder, prefix);
    stringBuilder.Append(prefix).Append(NEWLINE);
    stringBuilder.Append(prefix).AppendFormat("onRemoteRequest(code: number, data, reply, option): boolean {\n");
    stringBuilder.Append(prefix).Append(TAB).Append("console.log(\"onRemoteRequest called, code = \" + code);\n");
    stringBuilder.Append(prefix).Append(TAB).Append("switch(code) {\n");
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        EmitInterfaceStubMethodImpl(metaMethod, index, stringBuilder, prefix + TAB + TAB);
    }
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append("default: {\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append(
        "console.log(\"invalid request code\" + code);\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append(TAB).Append("break;\n");
    stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append("}\n");
    stringBuilder.Append(prefix).Append(TAB).Append("}\n");
    stringBuilder.Append(prefix).Append(TAB).Append("return false;\n");
    stringBuilder.Append(prefix).Append("}\n");
    stringBuilder.Append(prefix).Append(NEWLINE);

    // emit empty method
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        bool isLastParaTypeIn = false;
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        stringBuilder.Append(prefix).AppendFormat("%s(", MethodName(metaMethod->name_).string());
        for (int index = 0; index < metaMethod->parameterNumber_; index++) {
            MetaParameter* mp = metaMethod->parameters_[index];
            if ((mp->attributes_ & ATTR_IN) != 0) {
                EmitInterfaceMethodParameter(mp, stringBuilder, prefix + TAB);
                if (index != metaMethod->parameterNumber_ - 1) {
                    stringBuilder.Append(", ");
                } else {
                    isLastParaTypeIn = true;
                }
            }
        }
        if (!isLastParaTypeIn) {
            stringBuilder.AppendFormat("callback: %s", methods_[index].callbackName_.c_str());
        } else {
            stringBuilder.AppendFormat(", callback: %s", methods_[index].callbackName_.c_str());
        }
        stringBuilder.Append("): void{}\n");
    }
}

void TsCodeEmitter::EmitInterfaceStubMethodImpl(MetaMethod* metaMethod, int methodIndex, StringBuilder& stringBuilder,
    const String& prefix)
{
    bool haveOutPara = false;
    stringBuilder.Append(prefix).AppendFormat("case %s.COMMAND_%s: {\n", stubName_.string(),
        ConstantName(metaMethod->name_).string());
    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            MetaType* mt = metaComponent_->types_[mp->typeIndex_];
            EmitReadVariable("data", UnderlineAdded(mp->name_), mt, ATTR_IN, stringBuilder, prefix + TAB);
        }
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            haveOutPara = true;
        }
    }
    stringBuilder.Append(prefix).Append(TAB).AppendFormat("this.%s(", MethodName(metaMethod->name_).string());
    bool isLastParaTypeIn = false;
    for (int index = 0; index < metaMethod->parameterNumber_; index++) {
        MetaParameter* mp = metaMethod->parameters_[index];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            stringBuilder.Append(UnderlineAdded(mp->name_).c_str());
            if (index != metaMethod->parameterNumber_ - 1) {
                stringBuilder.Append(", ");
            } else {
                isLastParaTypeIn = true;
            }
        }
    }
    if (!isLastParaTypeIn) {
        stringBuilder.Append("(");
    } else {
        stringBuilder.Append(", (");
    }
    stringBuilder.Append(ERR_CODE);
    if (methods_[methodIndex].retParameter_.name_.size() > 0) {
        if (!haveOutPara) {
            stringBuilder.AppendFormat(", %s", RETURN_VALUE.string());
        } else {
            stringBuilder.AppendFormat(", %s", RETURN_VALUE.string()).Append(", ");
        }
    } else {
        if (haveOutPara) {
            stringBuilder.Append(", ");
        }
    }
    for (size_t index = 0; index < methods_[methodIndex].parameters_.size(); index++) {
        if ((methods_[methodIndex].parameters_[index].attr_ & ATTR_OUT) != 0) {
            stringBuilder.Append(methods_[methodIndex].parameters_[index].name_.c_str());
            if (index != methods_[methodIndex].parameters_.size() - 1) {
                stringBuilder.Append(", ");
            }
        }
    }

    stringBuilder.Append(") => {\n");
    MetaType errCode;
    errCode.kind_ = TypeKind::Integer;
    EmitWriteVariable("reply", ERR_CODE.string(), &errCode, stringBuilder, prefix + TAB + TAB);
    MetaType* returnType = metaComponent_->types_[metaMethod->returnTypeIndex_];

    if (returnType->kind_ != TypeKind::Void || haveOutPara) {
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).AppendFormat("if (%s == 0) {\n", ERR_CODE.string());
        for (int index = 0; index < metaMethod->parameterNumber_; index++) {
            MetaParameter* mp = metaMethod->parameters_[index];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                EmitWriteMethodParameter(mp, "reply", stringBuilder, prefix + TAB + TAB + TAB);
            }
        }
        if (returnType->kind_ != TypeKind::Void) {
            EmitWriteVariable("reply", RETURN_VALUE.string(), returnType, stringBuilder, prefix + TAB + TAB + TAB);
        }
        stringBuilder.Append(prefix).Append(TAB).Append(TAB).Append("}\n");
    }
    stringBuilder.Append(prefix).Append(TAB).Append("}");
    stringBuilder.Append(");\n");
    stringBuilder.Append(prefix).Append(TAB).Append("return true;\n");
    stringBuilder.Append(prefix).Append("}\n");
}

void TsCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& stringBuilder)
{
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        stringBuilder.Append(TAB).AppendFormat("static readonly COMMAND_%s = %d;\n",
            ConstantName(metaMethod->name_).string(), index + 1);
    }
}

void TsCodeEmitter::EmitLicense(StringBuilder& stringBuilder)
{
    stringBuilder.Append(metaInterface_->license_).Append(NEWLINE);
}

void TsCodeEmitter::EmitWriteVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& stringBuilder,
    const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("%s.writeInt(%s ? 1 : 0);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("%s.writeInt(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("%s.writeLong(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("%s.writeFloat(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("%s.writeDouble(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("%s.writeString(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable:
            if (EmitType(mt).Equals("IRemoteObject")) {
                stringBuilder.Append(prefix).AppendFormat("%s.writeRemoteObject(%s);\n", parcelName.string(),
                    name.c_str());
                break;
            }
            stringBuilder.Append(prefix).AppendFormat("%s.writeSequenceable(%s);\n", parcelName.string(),
                name.c_str());
            break;
        case TypeKind::Interface:
            stringBuilder.Append(prefix).AppendFormat("%s.writeRemoteObject(%s as %s);\n", parcelName.string(),
                name.c_str(), StubName(EmitType(mt)).string());
            break;
        case TypeKind::List: {
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            stringBuilder.Append(prefix).AppendFormat("%s.writeInt(%s.size());\n", parcelName.string(), name.c_str());
            stringBuilder.Append(prefix).AppendFormat("for (%s element : %s) {\n",
                EmitType(innerType).string(), name.c_str());
            EmitWriteVariable(parcelName, "element", innerType, stringBuilder, prefix + TAB);
            stringBuilder.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            stringBuilder.Append(prefix).AppendFormat("%s.writeInt(%s.size);\n", parcelName.string(), name.c_str());
            stringBuilder.Append(prefix).AppendFormat("for (let [key, value] of %s) {\n", name.c_str());
            EmitWriteVariable(parcelName, "key", keyType, stringBuilder, prefix + TAB);
            EmitWriteVariable(parcelName, "value", valueType, stringBuilder, prefix + TAB);
            stringBuilder.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitWriteArrayVariable(parcelName, name, innerType, stringBuilder, prefix);
            break;
        }
        default:
            break;
    }
}

void TsCodeEmitter::EmitWriteArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& stringBuilder,
    const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("%s.writeBooleanArray(%s);\n", parcelName.string(),
                name.c_str());
            break;
        case TypeKind::Char:
            stringBuilder.Append(prefix).AppendFormat("%s.writeCharArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Byte:
            stringBuilder.Append(prefix).AppendFormat("let %sArray = new Int8Array(%s);\n", name.c_str(),
                name.c_str());
            stringBuilder.Append(prefix).AppendFormat("%s.writeByteArray(%sArray);\n",
                parcelName.string(), name.c_str());
            break;
        case TypeKind::Short:
            stringBuilder.Append(prefix).AppendFormat("%s.writeShortArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("%s.writeIntArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("%s.writeLongArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("%s.writeFloatArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("%s.writeDoubleArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("%s.writeStringArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable: {
            String typeName = EmitType(mt).EndsWith("]") ?
                (EmitType(mt).Substring(0, EmitType(mt).GetLength() - 2)) : EmitType(mt);
            stringBuilder.Append(prefix).AppendFormat("let %sArray:Array<%s> = %s;\n", name.c_str(), typeName.string(),
                name.c_str());
            stringBuilder.Append(prefix).AppendFormat("%s.writeInt(%sArray.length);\n", parcelName.string(),
                name.c_str());
            stringBuilder.Append(prefix).AppendFormat("for (let index = 0; index < %sArray.length; index++) {\n",
                name.c_str());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.writeSequenceable(%s[index]);\n",
                parcelName.string(), name.c_str());
            stringBuilder.Append(prefix).AppendFormat("}\n");
        }
            break;
        default:
            break;
    }
}

void TsCodeEmitter::EmitReadVariable(const String& parcelName, const std::string& name, MetaType* mt,
    unsigned int attributes,
    StringBuilder& stringBuilder,
    const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readInt() == 1 ? true : false;\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readInt();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readLong();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readFloat();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readDouble();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readString();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Sequenceable:
            if ((attributes & ATTR_OUT) == 0 && EmitType(mt).Equals("IRemoteObject")) {
                stringBuilder.Append(prefix).AppendFormat("IRemoteObject %s = %s.readRemoteObject();\n",
                    name.c_str(), parcelName.string());
                break;
            }
            if ((attributes & ATTR_OUT) == 0) {
                stringBuilder.Append(prefix).AppendFormat("let %s = new %s();\n", name.c_str(), EmitType(mt).string());
            }
            stringBuilder.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.c_str());

            break;
        case TypeKind::Interface:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readRemoteObject();\n", name.c_str(),
                parcelName.string());
            break;

        case TypeKind::Map: {
            stringBuilder.Append(prefix).AppendFormat("let %s = new Map();\n", name.c_str());
            stringBuilder.Append(prefix).AppendFormat("let %sSize = %s.readInt();\n", name.c_str(),
                parcelName.string());
            stringBuilder.Append(prefix).AppendFormat("for (let i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, ATTR_IN, stringBuilder, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ATTR_IN, stringBuilder, prefix + TAB);
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.set(key, value);\n", name.c_str());
            stringBuilder.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            if ((attributes & ATTR_MASK) == ATTR_OUT) {
                EmitReadOutArrayVariable(parcelName, name, mt, stringBuilder, prefix);
            } else {
                EmitReadArrayVariable(parcelName, name, mt, attributes, stringBuilder, prefix);
            }
            break;
        }
        default:
            break;
    }
}

void TsCodeEmitter::EmitReadArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
    unsigned int attributes, StringBuilder& stringBuilder, const String& prefix)
{
    MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
    switch (innerType->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readBooleanArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Char:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readCharArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Byte:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readByteArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Short:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readShortArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readIntArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readLongArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readFloatArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readDoubleArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readStringArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Sequenceable: {
            String typeName = EmitType(mt).EndsWith("]") ?
                (EmitType(mt).Substring(0, EmitType(mt).GetLength() - 2)) : EmitType(mt);
            stringBuilder.Append(prefix).AppendFormat("let %sSize = %s.readInt();\n", name.c_str(),
                parcelName.string());
            stringBuilder.Append(prefix).AppendFormat("let %s:Array<%s> = [];\n", name.c_str(), typeName.string());
            stringBuilder.Append(prefix).AppendFormat("for (let index = 0; index < %sSize; index++) {\n",
                name.c_str());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("let %sValue = new %s();\n",
                name.c_str(), typeName.string());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.readSequenceable(%sValue);\n",
                parcelName.string(), name.c_str());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.push(%sValue);\n", name.c_str(),
                name.c_str());
            stringBuilder.Append(prefix).AppendFormat("}\n");
        }
            break;
        default:
            break;
    }
}

void TsCodeEmitter::EmitReadOutArrayVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& stringBuilder,
    const String& prefix)
{
    MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
    switch (innerType->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readBooleanArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Char:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readCharArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Byte:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readByteArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Short:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readShortArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readIntArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readLongArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readFloatArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readDoubleArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readStringArray();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Sequenceable: {
            stringBuilder.Append(prefix).AppendFormat("let %sSize = %s.readInt();\n", name.c_str(),
                parcelName.string());
            String typeName = EmitType(mt).EndsWith("]") ?
                (EmitType(mt).Substring(0, EmitType(mt).GetLength() - 2)) : EmitType(mt);
            stringBuilder.Append(prefix).AppendFormat("let %s:Array<%s> = [];\n", name.c_str(), typeName.string());
            stringBuilder.Append(prefix).AppendFormat("for (let index = 0; index < %sSize; index++) {\n",
                name.c_str());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("let %sValue = new %s();\n",
                name.c_str(), typeName.string());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.readSequenceable(%sValue);\n",
                parcelName.string(), name.c_str());
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.push(%sValue);\n", name.c_str(),
                name.c_str());
            stringBuilder.Append(prefix).AppendFormat("}\n");
        }
            break;
        default:
            break;
    }
}

void TsCodeEmitter::EmitReadOutVariable(const String& parcelName, const std::string& name, MetaType* mt,
    StringBuilder& stringBuilder,
    const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readInt() == 1 ? true : false;\n",
                name.c_str(), parcelName.string());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readInt();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Long:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readLong();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Float:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readFloat();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Double:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readDouble();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::String:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readString();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Sequenceable:
            if (EmitType(mt).Equals("IRemoteObject")) {
                stringBuilder.Append(prefix).AppendFormat("%s = %s.readRemoteObject();\n", name.c_str(),
                    parcelName.string());
                break;
            }
            stringBuilder.Append(prefix).AppendFormat("let %s = new %s();\n", name.c_str(), EmitType(mt).string());
            stringBuilder.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Interface:
            stringBuilder.Append(prefix).AppendFormat("let %s = %s.readRemoteObject();\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::List: {
            stringBuilder.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(),
                parcelName.string());
            stringBuilder.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitReadVariable(parcelName, "value", innerType, ATTR_IN, stringBuilder, prefix + TAB);
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.add(value);\n", name.c_str());
            stringBuilder.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            stringBuilder.Append(prefix).AppendFormat("let %s = new Map();\n", name.c_str());
            stringBuilder.Append(prefix).AppendFormat("let %sSize = %s.readInt();\n", name.c_str(),
                parcelName.string());
            stringBuilder.Append(prefix).AppendFormat("for (let i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, ATTR_IN, stringBuilder, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ATTR_IN, stringBuilder, prefix + TAB);
            stringBuilder.Append(prefix).Append(TAB).AppendFormat("%s.set(key, value);\n", name.c_str());
            stringBuilder.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            EmitReadOutArrayVariable(parcelName, name, mt, stringBuilder, prefix);
            break;
        }
        default:
            break;
    }
}

String TsCodeEmitter::EmitType(MetaType* mt)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            return "boolean";
        case TypeKind::Byte:
            return "number";
        case TypeKind::Short:
            return "number";
        case TypeKind::Integer:
            return "number";
        case TypeKind::Long:
            return "number";
        case TypeKind::Float:
            return "number";
        case TypeKind::Double:
            return "number";
        case TypeKind::String:
            return "string";
        case TypeKind::Void:
            return "void";
        case TypeKind::Sequenceable: {
            MetaSequenceable* mp = metaComponent_->sequenceables_[mt->index_];
            return mp->name_;
        }
        case TypeKind::Interface: {
            MetaInterface* mi = metaComponent_->interfaces_[mt->index_];
            return mi->name_;
        }
        case TypeKind::Map: {
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            if (EmitType(keyType).string() == UNKNOWN_TYPE || EmitType(valueType).string() == UNKNOWN_TYPE) {
                return String(UNKNOWN_TYPE.c_str());
            }
            return String::Format("Map<%s, %s>", EmitType(keyType).string(), EmitType(valueType).string());
        }
        case TypeKind::Array: {
            MetaType* elementType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            if (EmitType(elementType).string() == UNKNOWN_TYPE) {
                return String(UNKNOWN_TYPE.c_str());
            }
            return String::Format("%s[]", EmitType(elementType).string());
        }
        default:
            return String(UNKNOWN_TYPE.c_str());
    }
}

String TsCodeEmitter::FileName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder stringBuilder;
    for (int index = 0; index < name.GetLength(); index++) {
        char c = name[index];
        if (isupper(c) != 0) {
            if (index > 0) {
                stringBuilder.Append('_');
            }
            stringBuilder.Append(tolower(c));
        } else {
            stringBuilder.Append(c);
        }
    }
    return stringBuilder.ToString().Replace('.', '/');
}

String TsCodeEmitter::MethodName(const String& name)
{
    if (name.IsEmpty() || islower(name[0])) {
        return name;
    }
    return String::Format("%c%s", tolower(name[0]), name.Substring(1).string());
}

String TsCodeEmitter::ConstantName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder stringBuilder;

    for (int index = 0; index < name.GetLength(); index++) {
        char c = name[index];
        if (isupper(c) != 0) {
            if (index > 1) {
                stringBuilder.Append('_');
            }
            stringBuilder.Append(c);
        } else {
            stringBuilder.Append(toupper(c));
        }
    }

    return stringBuilder.ToString();
}

String TsCodeEmitter::StubName(const String& name)
{
    return name.StartsWith("I") ? (name.Substring(1) + "Stub") : (name + "Stub");
}

const std::string TsCodeEmitter::UnderlineAdded(const String& originName)
{
    std::string underline("_");
    return underline + std::string(originName.string());
}

bool TsCodeEmitter::CheckInterfaceType()
{
    for (int index = 0; index < metaInterface_->methodNumber_; index++) {
        MetaMethod* metaMethod = metaInterface_->methods_[index];
        MetaType* returnType = metaComponent_->types_[metaMethod->returnTypeIndex_];
        std::string returnTypeStr = EmitType(returnType).string();
        if (returnTypeStr == UNKNOWN_TYPE) {
            Logger::E(TAG, "unsupported type in .idl file");
            return false;
        }
        for (int index = 0; index < metaMethod->parameterNumber_; index++) {
            MetaParameter* mp = metaMethod->parameters_[index];
            MetaType* paraType = metaComponent_->types_[mp->typeIndex_];
            std::string paraTypeStr = EmitType(paraType).string();
            if (paraTypeStr == UNKNOWN_TYPE) {
                Logger::E(TAG, "unsupported type in .idl file");
                return false;
            }
        }
    }
    return true;
}
}
}
