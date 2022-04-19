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

#ifndef OHOS_IDL_METATYPE_H
#define OHOS_IDL_METATYPE_H

namespace OHOS {
namespace Idl {
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

struct MetaType {
    TypeKind            kind_;
    int                 index_;
    int                 nestedTypeNumber_;
    int*                nestedTypeIndexes_;
};
}
}

#endif // OHOS_IDL_METATYPE_H