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

#ifndef OHOS_IDL_METADATA_H
#define OHOS_IDL_METADATA_H

#include <cstring>

#include "meta_interface.h"
#include "meta_namespace.h"
#include "meta_sequenceable.h"
#include "meta_type.h"

namespace OHOS {
namespace Idl {
static constexpr int METADATA_MAGIC_NUMBER = 0x1DF02ED1;

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

static constexpr unsigned int INTERFACE_PROPERTY_ONEWAY = 0x1;

static constexpr unsigned int METHOD_PROPERTY_ONEWAY = 0x1;

static constexpr unsigned int ATTR_IN = 0x1;
static constexpr unsigned int ATTR_OUT = 0x2;
static constexpr unsigned int ATTR_MASK = 0x3;
}
}

#endif // OHOS_IDL_METADATA_H
