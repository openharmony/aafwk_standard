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

#include "metadata/metadata_serializer.h"

namespace OHOS {
namespace Idl {
void MetadataSerializer::Serialize()
{
    SerializeMetaComponent(metaComponent_);
}

void MetadataSerializer::SerializeMetaComponent(MetaComponent* mc)
{
    mc->name_ = reinterpret_cast<char*>(SerializeAdjust(mc->name_));

    for (int i = 0; i < mc->namespaceNumber_; i++) {
        MetaNamespace* mn = mc->namespaces_[i];
        SerializeMetaNamespace(mn);
        mc->namespaces_[i] = reinterpret_cast<MetaNamespace*>(SerializeAdjust(mn));
    }
    mc->namespaces_ = reinterpret_cast<MetaNamespace**>(SerializeAdjust(mc->namespaces_));

    for (int i = 0; i < mc->sequenceableNumber_; i++) {
        MetaSequenceable* mp = mc->sequenceables_[i];
        SerializeMetaSequenceable(mp);
        mc->sequenceables_[i] = reinterpret_cast<MetaSequenceable*>(SerializeAdjust(mp));
    }
    mc->sequenceables_ = reinterpret_cast<MetaSequenceable**>(SerializeAdjust(mc->sequenceables_));

    for (int i = 0; i < mc->interfaceNumber_; i++) {
        MetaInterface* mi = mc->interfaces_[i];
        SerializeMetaInterface(mi);
        mc->interfaces_[i] = reinterpret_cast<MetaInterface*>(SerializeAdjust(mi));
    }
    mc->interfaces_ = reinterpret_cast<MetaInterface**>(SerializeAdjust(mc->interfaces_));

    for (int i = 0; i < mc->typeNumber_; i++) {
        MetaType* mt = mc->types_[i];
        SerializeMetaType(mt);
        mc->types_[i] = reinterpret_cast<MetaType*>(SerializeAdjust(mt));
    }
    mc->types_ = reinterpret_cast<MetaType**>(SerializeAdjust(mc->types_));

    mc->stringPool_ = reinterpret_cast<char*>(SerializeAdjust(mc->stringPool_));
}

void MetadataSerializer::SerializeMetaNamespace(MetaNamespace* mn)
{
    mn->name_ = reinterpret_cast<char*>(SerializeAdjust(mn->name_));
    mn->sequenceableIndexes_ = reinterpret_cast<int*>(SerializeAdjust(mn->sequenceableIndexes_));
    mn->interfaceIndexes_ = reinterpret_cast<int*>(SerializeAdjust(mn->interfaceIndexes_));

    for (int i = 0; i < mn->namespaceNumber_; i++) {
        MetaNamespace* innermn = mn->namespaces_[i];
        SerializeMetaNamespace(innermn);
        mn->namespaces_[i] = reinterpret_cast<MetaNamespace*>(SerializeAdjust(innermn));
    }
    mn->namespaces_ = reinterpret_cast<MetaNamespace**>(SerializeAdjust(mn->namespaces_));
}

void MetadataSerializer::SerializeMetaSequenceable(MetaSequenceable* mp)
{
    mp->name_ = reinterpret_cast<char*>(SerializeAdjust(mp->name_));
    mp->namespace_ = reinterpret_cast<char*>(SerializeAdjust(mp->namespace_));
}

void MetadataSerializer::SerializeMetaInterface(MetaInterface* mi)
{
    mi->license_ = reinterpret_cast<char*>(SerializeAdjust(mi->license_));
    mi->name_ = reinterpret_cast<char*>(SerializeAdjust(mi->name_));
    mi->namespace_ = reinterpret_cast<char*>(SerializeAdjust(mi->namespace_));

    for (int i = 0; i < mi->methodNumber_; i++) {
        MetaMethod* mm = mi->methods_[i];
        SerializeMetaMethod(mm);
        mi->methods_[i] = reinterpret_cast<MetaMethod*>(SerializeAdjust(mm));
    }
    mi->methods_ = reinterpret_cast<MetaMethod**>(SerializeAdjust(mi->methods_));
}

void MetadataSerializer::SerializeMetaMethod(MetaMethod* mm)
{
    mm->name_ = reinterpret_cast<char*>(SerializeAdjust(mm->name_));
    mm->signature_ = reinterpret_cast<char*>(SerializeAdjust(mm->signature_));

    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        SerializeMetaParameter(mp);
        mm->parameters_[i] = reinterpret_cast<MetaParameter*>(SerializeAdjust(mp));
    }
    mm->parameters_ = reinterpret_cast<MetaParameter**>(SerializeAdjust(mm->parameters_));
}

void MetadataSerializer::SerializeMetaParameter(MetaParameter* mp)
{
    mp->name_ = reinterpret_cast<char*>(SerializeAdjust(mp->name_));
}

void MetadataSerializer::SerializeMetaType(MetaType* mt)
{
    mt->nestedTypeIndexes_ = reinterpret_cast<int*>(SerializeAdjust(mt->nestedTypeIndexes_));
}

ptrdiff_t MetadataSerializer::SerializeAdjust(const void* addr)
{
    return reinterpret_cast<uintptr_t>(addr) - baseAddr_;
}

void MetadataSerializer::Deserialize()
{
    DeserializeMetaComponent(metaComponent_);
}

void MetadataSerializer::DeserializeMetaComponent(MetaComponent* mc)
{
    mc->name_ = reinterpret_cast<char*>(DeserializeAdjust(mc->name_));

    mc->namespaces_ = reinterpret_cast<MetaNamespace**>(DeserializeAdjust(mc->namespaces_));
    for (int i = 0; i < mc->namespaceNumber_; i++) {
        mc->namespaces_[i] = reinterpret_cast<MetaNamespace*>(DeserializeAdjust(mc->namespaces_[i]));
        MetaNamespace* mn = mc->namespaces_[i];
        DeserializeMetaNamespace(mn);
    }

    mc->sequenceables_ = reinterpret_cast<MetaSequenceable**>(DeserializeAdjust(mc->sequenceables_));
    for (int i = 0; i < mc->sequenceableNumber_; i++) {
        mc->sequenceables_[i] = reinterpret_cast<MetaSequenceable*>(DeserializeAdjust(mc->sequenceables_[i]));
        MetaSequenceable* mp = mc->sequenceables_[i];
        DeserializeMetaSequenceable(mp);
    }

    mc->interfaces_ = reinterpret_cast<MetaInterface**>(DeserializeAdjust(mc->interfaces_));
    for (int i = 0; i < mc->interfaceNumber_; i++) {
        mc->interfaces_[i] = reinterpret_cast<MetaInterface*>(DeserializeAdjust(mc->interfaces_[i]));
        MetaInterface* mi = mc->interfaces_[i];
        DeserializeMetaInterface(mi);
    }

    mc->types_ = reinterpret_cast<MetaType**>(DeserializeAdjust(mc->types_));
    for (int i = 0; i < mc->typeNumber_; i++) {
        mc->types_[i] = reinterpret_cast<MetaType*>(DeserializeAdjust(mc->types_[i]));
        MetaType* mt = mc->types_[i];
        DeserializeMetaType(mt);
    }

    mc->stringPool_ = reinterpret_cast<char*>(DeserializeAdjust(mc->stringPool_));
}

void MetadataSerializer::DeserializeMetaNamespace(MetaNamespace* mn)
{
    mn->name_ = reinterpret_cast<char*>(DeserializeAdjust(mn->name_));
    mn->sequenceableIndexes_ = reinterpret_cast<int*>(DeserializeAdjust(mn->sequenceableIndexes_));
    mn->interfaceIndexes_ = reinterpret_cast<int*>(DeserializeAdjust(mn->interfaceIndexes_));

    mn->namespaces_ = reinterpret_cast<MetaNamespace**>(DeserializeAdjust(mn->namespaces_));
    for (int i = 0; i < mn->namespaceNumber_; i++) {
        mn->namespaces_[i] = reinterpret_cast<MetaNamespace*>(DeserializeAdjust(mn->namespaces_[i]));
        MetaNamespace* innermn = mn->namespaces_[i];
        DeserializeMetaNamespace(innermn);
    }
}

void MetadataSerializer::DeserializeMetaSequenceable(MetaSequenceable* mp)
{
    mp->name_ = reinterpret_cast<char*>(DeserializeAdjust(mp->name_));
    mp->namespace_ = reinterpret_cast<char*>(DeserializeAdjust(mp->namespace_));
}

void MetadataSerializer::DeserializeMetaInterface(MetaInterface* mi)
{
    mi->license_ = reinterpret_cast<char*>(DeserializeAdjust(mi->license_));
    mi->name_ = reinterpret_cast<char*>(DeserializeAdjust(mi->name_));
    mi->namespace_ = reinterpret_cast<char*>(DeserializeAdjust(mi->namespace_));

    mi->methods_ = reinterpret_cast<MetaMethod**>(DeserializeAdjust(mi->methods_));
    for (int i = 0; i < mi->methodNumber_; i++) {
        mi->methods_[i] = reinterpret_cast<MetaMethod*>(DeserializeAdjust(mi->methods_[i]));
        MetaMethod* mm = mi->methods_[i];
        DeserializeMetaMethod(mm);
    }
}

void MetadataSerializer::DeserializeMetaMethod(MetaMethod* mm)
{
    mm->name_ = reinterpret_cast<char*>(DeserializeAdjust(mm->name_));
    mm->signature_ = reinterpret_cast<char*>(DeserializeAdjust(mm->signature_));

    mm->parameters_ = reinterpret_cast<MetaParameter**>(DeserializeAdjust(mm->parameters_));
    for (int i = 0; i < mm->parameterNumber_; i++) {
        mm->parameters_[i] = reinterpret_cast<MetaParameter*>(DeserializeAdjust(mm->parameters_[i]));
        MetaParameter* mp = mm->parameters_[i];
        DeserializeMetaParameter(mp);
    }
}

void MetadataSerializer::DeserializeMetaParameter(MetaParameter* mp)
{
    mp->name_ = reinterpret_cast<char*>(DeserializeAdjust(mp->name_));
}

void MetadataSerializer::DeserializeMetaType(MetaType* mt)
{
    mt->nestedTypeIndexes_ = reinterpret_cast<int*>(DeserializeAdjust(mt->nestedTypeIndexes_));
}

uintptr_t MetadataSerializer::DeserializeAdjust(const void* addr)
{
    return reinterpret_cast<ptrdiff_t>(addr) + baseAddr_;
}
}
}
