/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_AAFWK_BYTE_H
#define OHOS_AAFWK_BYTE_H

#include "base_object.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Byte final : public Object, public IByte {
public:
    inline Byte(byte value) : value_(value)
    {}

    inline ~Byte()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(byte &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IByte> Box(byte value); /* [in] */

    static byte Unbox(IByte *object); /* [in] */

    static sptr<IByte> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'B';

private:
    byte value_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_BYTE_H
