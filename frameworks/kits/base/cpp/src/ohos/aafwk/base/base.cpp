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

#include "ohos/aafwk/base/base_def.h"
#include "ohos/aafwk/base/base_types.h"
#include "ohos/aafwk/base/base_interfaces.h"

namespace OHOS {
namespace AAFwk {
const Uuid Uuid::Empty = {
    0x00000000, 0x0000, 0x0000, 0x0000, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
};

const InterfaceID g_IID_IInterface = {
    0x00000000, 0x0000, 0x0000, 0x0000, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1}
};

const InterfaceID g_IID_IBoolean = {
    0x492ef6c0, 0xe122, 0x401d, 0x80c4, {0xb, 0xb, 0x6, 0x5, 0xe, 0x2, 0x3, 0x2, 0x5, 0x7, 0x6, 0x6}
};

const InterfaceID g_IID_IChar = {
    0x6da72e7c, 0xd353, 0x4d7b, 0x85ef, {0x2, 0x1, 0x4, 0x6, 0xa, 0xd, 0xd, 0x3, 0x4, 0x2, 0x1, 0x9}
};

const InterfaceID g_IID_IByte = {
    0x2c9fd6ff, 0x73f7, 0x4a22, 0x9ea8, {0x1, 0xe, 0x3, 0x7, 0xb, 0x2, 0x2, 0x1, 0x3, 0xc, 0xa, 0x1}
};

const InterfaceID g_IID_IShort = {
    0x54fe1291, 0x0813, 0x43c9, 0xb6f7, {0xb, 0x0, 0x0, 0x8, 0xe, 0xd, 0xe, 0xf, 0x7, 0xf, 0x3, 0x4}
};

const InterfaceID g_IID_IInteger = {
    0x0672d282, 0x7cdb, 0x49b8, 0x92c7, {0x1, 0xa, 0xf, 0x3, 0x5, 0xe, 0x2, 0xd, 0x4, 0x9, 0x4, 0x9}
};

const InterfaceID g_IID_ILong = {
    0xd7550828, 0x2eaf, 0x4281, 0x8604, {0x4, 0x2, 0xa, 0x8, 0x2, 0xa, 0xb, 0x9, 0xb, 0xc, 0xd, 0xc}
};

const InterfaceID g_IID_IFloat = {
    0xb5428638, 0xca53, 0x4a27, 0x95a0, {0x3, 0xf, 0x2, 0x4, 0xe, 0x5, 0x4, 0xa, 0x5, 0x8, 0xd, 0x5}
};

const InterfaceID g_IID_IDouble = {
    0x596dc69a, 0xae7d, 0x42ad, 0xb00f, {0x0, 0xd, 0x5, 0xe, 0xb, 0x0, 0x1, 0xa, 0x2, 0x5, 0x5, 0x7}
};

const InterfaceID g_IID_IString = {
    0x69644bf4, 0xdd20, 0x417b, 0x9055, {0x5, 0xa, 0x2, 0x6, 0xe, 0x3, 0xe, 0x1, 0x7, 0x9, 0x3, 0xb}
};

const InterfaceID g_IID_IArray = {
    0x875b9da6, 0x9913, 0x4370, 0x8847, {0xe, 0x1, 0x9, 0x6, 0x1, 0xb, 0xe, 0x6, 0xe, 0x5, 0x6, 0x0}
};

const InterfaceID g_IID_IPacMap = {
    0xf92066fd, 0xfd0c, 0x401b, 0xa3f6, {0x6, 0x2, 0x6, 0xd, 0xa, 0x3, 0xb, 0xa, 0xc, 0x9, 0xd, 0x5}
};

const InterfaceID g_IID_IUserObject = {
    0x4edb325d, 0x8532, 0x4af7, 0xb42e, {0x8, 0x2, 0xf, 0x4, 0xf, 0x2, 0x9, 0xd, 0xf, 0xd, 0xe, 0xa}
};
}  // namespace AAFwk
}  // namespace OHOS