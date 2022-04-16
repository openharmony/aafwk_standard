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

#ifndef OHOS_IDL_METADATA_H
#define OHOS_IDL_METADATA_H

namespace OHOS {
namespace Idl {

static constexpr int METADATA_MAGIC_NUMBER = 0x1DF02ED1;

enum class TypeKind {
    Unknown     = 0,
    Char        = 1,
    Boolean     = 2,
    Byte        = 3,
    Short       = 4,
    Integer     = 5,
    Long        = 6,
    Float       = 7,
    Double      = 8,
    String      = 9,
    Void        = 10,
    Sequenceable  = 11,
    Interface   = 12,
    List        = 13,
    Map         = 14,
    Array       = 15,
};

struct MetaNamespace;
struct MetaSequenceable;
struct MetaInterface;
struct MetaMethod;
struct MetaParameter;
struct MetaType;

struct MetaComponent {
    int                 magic_;
    int                 size_;
    char*               name_;
    int                 namespaceNumber_;
    int                 sequenceableNumber_;
    int                 interfaceNumber_;
    int                 typeNumber_;
    MetaNamespace**     namespaces_;
    MetaSequenceable**    sequenceables_;
    MetaInterface**     interfaces_;
    MetaType**          types_;
    int                 stringPoolSize_;
    char*               stringPool_;
};

struct MetaNamespace {
    char*               name_;
    int                 sequenceableNumber_;
    int                 interfaceNumber_;
    int                 namespaceNumber_;
    int*                sequenceableIndexes_;
    int*                interfaceIndexes_;
    MetaNamespace**     namespaces_;
};

struct MetaSequenceable {
    char*               name_;
    char*               namespace_;
};

static constexpr unsigned int INTERFACE_PROPERTY_ONEWAY = 0x1;

struct MetaInterface {
    char*               license_;
    char*               name_;
    char*               namespace_;
    unsigned int        properties_;
    int                 methodNumber_;
    MetaMethod**        methods_;
    bool                external_;
};

static constexpr unsigned int METHOD_PROPERTY_ONEWAY = 0x1;

struct MetaMethod {
    char*               name_;
    char*               signature_;
    unsigned int        properties_;
    int                 returnTypeIndex_;
    int                 parameterNumber_;
    MetaParameter**     parameters_;
};

static constexpr unsigned int ATTR_IN = 0x1;
static constexpr unsigned int ATTR_OUT = 0x2;
static constexpr unsigned int ATTR_MASK = 0x3;

struct MetaParameter {
    char*               name_;
    unsigned int        attributes_;
    int                 typeIndex_;
};

struct MetaType {
    TypeKind            kind_;
    int                 index_;
    int                 nestedTypeNumber_;
    int*                nestedTypeIndexes_;
};

}
}

#endif // OHOS_IDL_METADATA_H
