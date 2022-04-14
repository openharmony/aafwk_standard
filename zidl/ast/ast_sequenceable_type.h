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

#ifndef OHOS_ZIDL_ASTSEQUENCEABLETYPE_H
#define OHOS_ZIDL_ASTSEQUENCEABLETYPE_H

#include "ast/ast_type.h"

namespace OHOS {
namespace Zidl {

class ASTSequenceableType : public ASTType {
public:
    void SetNamespace(
        /* [in] */ ASTNamespace* nspace) override;

    String GetSignature() override;

    bool IsSequenceableType() override;

    String ToString() override;

    String Dump(
        /* [in] */ const String& prefix) override;
};

}
}

#endif // OHOS_ZIDL_ASTSEQUENCEABLETYPE_H
