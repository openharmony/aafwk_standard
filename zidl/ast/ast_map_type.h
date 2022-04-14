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

#ifndef OHOS_ZIDL_ASTMAPTYPE_H
#define OHOS_ZIDL_ASTMAPTYPE_H

#include "ast/ast_type.h"
#include "util/autoptr.h"

namespace OHOS {
namespace Zidl {

class ASTMapType : public ASTType {
public:
    inline void SetKeyType(
        /* [in] */ ASTType* keyType)
    {
        keyType_ = keyType;
    }

    inline AutoPtr<ASTType> GetKeyType()
    {
        return keyType_;
    }

    inline void SetValueType(
        /* [in] */ ASTType* valueType)
    {
        valueType_ = valueType;
    }

    inline AutoPtr<ASTType> GetValueType()
    {
        return valueType_;
    }

    String GetSignature() override;

    bool IsMapType() override;

    String ToString() override;

private:
    AutoPtr<ASTType> keyType_;
    AutoPtr<ASTType> valueType_;
};

}
}

#endif // OHOS_ZIDL_ASTMAPTYPE_H