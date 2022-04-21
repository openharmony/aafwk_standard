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

#include "metadata/metadata_dumper.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
const char* MetadataDumper::TAB = "    ";
void MetadataDumper::Dump(const String& prefix)
{
    if (metaComponent_ == nullptr) {
        return;
    }

    String dumpStr = DumpMetaComponent(metaComponent_, prefix);
}

String MetadataDumper::DumpMetaComponent(MetaComponent* mc, const String& prefix)
{
    StringBuilder sb;

    sb.Append(prefix).Append("MetaComponent\n");
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"magic_\" : \"0x%x\",\n", mc->magic_);
    sb.Append(prefix + TAB).AppendFormat("\"size_\" : \"%d\",\n", mc->size_);
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mc->name_);
    sb.Append(prefix + TAB).AppendFormat("\"namespaceNumber_\" : \"%d\",\n", mc->namespaceNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"sequenceableNumber_\" : \"%d\",\n", mc->sequenceableNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"interfaceNumber_\" : \"%d\",\n", mc->interfaceNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"typeNumber_\" : \"%d\",\n", mc->typeNumber_);

    if (mc->namespaceNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"namespaces_\" : [],\n");
    } else {
        sb.Append(prefix + TAB).Append("\"namespaces_\" : [\n");
        for (int i = 0; i < mc->namespaceNumber_; i++) {
            DumpMetaNamespace(sb, mc->namespaces_[i], prefix + TAB + TAB);
            if (i != mc->namespaceNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("],\n");
    }

    if (mc->sequenceableNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"sequenceables_\" : [],\n");
    } else {
        sb.Append(prefix + TAB).Append("\"sequenceables_\" : [\n");
        for (int i = 0; i < mc->sequenceableNumber_; i++) {
            DumpMetaSequenceable(sb, mc->sequenceables_[i], prefix + TAB + TAB);
            if (i != mc->sequenceableNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("],\n");
    }

    if (mc->interfaceNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"interfaces_\" : [],\n");
    } else {
        sb.Append(prefix + TAB).Append("\"interfaces_\" : [\n");
        for (int i = 0; i < mc->interfaceNumber_; i++) {
            DumpMetaInterface(sb, mc->interfaces_[i], prefix + TAB + TAB);
            if (i != mc->interfaceNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("],\n");
    }

    sb.Append(prefix + TAB).AppendFormat("\"stringPoolSize_\" : \"%d\"\n", mc->stringPoolSize_);

    sb.Append(prefix).Append("}\n");

    return sb.ToString();
}

void MetadataDumper::DumpMetaNamespace(StringBuilder& sb, MetaNamespace* mn, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mn->name_);
    sb.Append(prefix + TAB).AppendFormat("\"sequenceableNumber_\" : \"%d\",\n", mn->sequenceableNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"interfaceNumber_\" : \"%d\",\n", mn->interfaceNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"namespaceNumber_\" : \"%d\",\n", mn->namespaceNumber_);

    if (mn->sequenceableNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"sequenceableIndexes_\" : [],\n");
    } else {
        sb.Append(prefix + TAB).Append("\"sequenceableIndexes_\" : [\n");
        for (int i = 0; i < mn->sequenceableNumber_; i++) {
            MetaSequenceable* mp = metaComponent_->sequenceables_[mn->sequenceableIndexes_[i]];
            sb.Append(prefix + TAB + TAB).AppendFormat("{ \"name\" : \"%s\" }", mp->name_);
            if (i != mn->sequenceableNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("],\n");
    }

    if (mn->interfaceNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"interfaceIndexes_\" : [],\n");
    } else {
        sb.Append(prefix + TAB).Append("\"interfaceIndexes_\" : [\n");
        for (int i = 0; i < mn->interfaceNumber_; i++) {
            MetaInterface* mi = metaComponent_->interfaces_[mn->interfaceIndexes_[i]];
            sb.Append(prefix + TAB + TAB).AppendFormat("{ \"name\" : \"%s\" }", mi->name_);
            if (i != mn->interfaceNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("],\n");
    }

    if (mn->namespaceNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"namespaces_\" : []\n");
    } else {
        sb.Append(prefix + TAB).Append("\"namespaces_\" : [\n");
        for (int i = 0; i < mn->namespaceNumber_; i++) {
            MetaNamespace* innermn = mn->namespaces_[i];
            DumpMetaNamespace(sb, innermn, prefix + TAB + TAB);
            if (i != mn->namespaceNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("]\n");
    }

    sb.Append(prefix).Append("}");
}

void MetadataDumper::DumpMetaSequenceable(StringBuilder& sb, MetaSequenceable* mp, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mp->name_);
    sb.Append(prefix + TAB).AppendFormat("\"namespace_\" : \"%s\"\n", mp->namespace_);
    sb.Append(prefix).Append("}");
}

void MetadataDumper::DumpMetaInterface(StringBuilder& sb, MetaInterface* mi, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mi->name_);
    sb.Append(prefix + TAB).AppendFormat("\"namespace_\" : \"%s\",\n", mi->namespace_);
    sb.Append(prefix + TAB).AppendFormat("\"properties_\" : \"%s\",\n",
        (mi->properties_ & INTERFACE_PROPERTY_ONEWAY) != 0 ? "oneway" : "");
    sb.Append(prefix + TAB).AppendFormat("\"methodNumber_\" : \"%d\",\n", mi->methodNumber_);
    sb.Append(prefix + TAB).AppendFormat("\"external_\" : \"%d\",\n", mi->external_);

    if (mi->methodNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"methods_\" : []\n");
    } else {
        sb.Append(prefix + TAB).Append("\"methods_\" : [\n");
        for (int i = 0; i < mi->methodNumber_; i++) {
            DumpMetaMethod(sb, mi->methods_[i], prefix + TAB + TAB);
            if (i != mi->methodNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("]\n");
    }

    sb.Append(prefix).Append("}");
}

void MetadataDumper::DumpMetaMethod(StringBuilder& sb, MetaMethod* mm, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mm->name_);
    sb.Append(prefix + TAB).AppendFormat("\"signature_\" : \"%s\",\n", mm->signature_);
    sb.Append(prefix + TAB).AppendFormat("\"properties_\" : \"%s\",\n",
        (mm->properties_ & METHOD_PROPERTY_ONEWAY) != 0 ? "oneway" : "");
    MetaType* type = metaComponent_->types_[mm->returnTypeIndex_];
    sb.Append(prefix + TAB).AppendFormat("\"returnType_\" : \"%s\",\n", DumpMetaType(type).string());
    sb.Append(prefix + TAB).AppendFormat("\"parameterNumber_\" : \"%d\",\n", mm->parameterNumber_);

    if (mm->parameterNumber_ == 0) {
        sb.Append(prefix + TAB).Append("\"parameters_\" : []\n");
    } else {
        sb.Append(prefix + TAB).Append("\"parameters_\" : [\n");
        for (int i = 0; i < mm->parameterNumber_; i++) {
            DumpMetaParameter(sb, mm->parameters_[i], prefix + TAB + TAB);
            if (i != mm->parameterNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append("\n" + prefix + TAB).Append("]\n");
    }

    sb.Append(prefix).Append("}");
}

void MetadataDumper::DumpMetaParameter(StringBuilder& sb, MetaParameter* mp, const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("\"name_\" : \"%s\",\n", mp->name_);
    sb.Append(prefix + TAB).Append("\"attributes_\" : \"");
    bool addComma = false;
    if ((mp->attributes_ & ATTR_IN) == ATTR_IN) {
        sb.Append("in");
        addComma = true;
    }
    if ((mp->attributes_ & ATTR_OUT) == ATTR_OUT) {
        sb.Append(addComma ? ", out" : "out");
    }
    sb.Append("\",\n");
    MetaType* type = metaComponent_->types_[mp->typeIndex_];
    sb.Append(prefix + TAB).AppendFormat("\"type_\" : \"%s\"\n", DumpMetaType(type).string());

    sb.Append(prefix).Append("}");
}

String MetadataDumper::DumpMetaType(MetaType* mt)
{
    switch (mt->kind_) {
        case TypeKind::Char:
            return "char";
        case TypeKind::Boolean:
            return "boolean";
        case TypeKind::Byte:
            return "byte";
        case TypeKind::Short:
            return "short";
        case TypeKind::Integer:
            return "int";
        case TypeKind::Long:
            return "long";
        case TypeKind::Float:
            return "float";
        case TypeKind::Double:
            return "double";
        case TypeKind::String:
            return "String";
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
        case TypeKind::List: {
            MetaType* elementMt = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            return "List<" + DumpMetaType(elementMt) + ">";
        }
        case TypeKind::Map: {
            MetaType* keyMt = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueMt = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            return "Map<" + DumpMetaType(keyMt) + ", " + DumpMetaType(valueMt) + ">";
        }
        case TypeKind::Array: {
            MetaType* elementMt = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            return DumpMetaType(elementMt) + "[]";
        }
        case TypeKind::Unknown:
        default:
            return "unknown";
    }
}
}
}
