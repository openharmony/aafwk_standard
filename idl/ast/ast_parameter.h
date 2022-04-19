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

#ifndef OHOS_IDL_ASTPARAMETER_H
#define OHOS_IDL_ASTPARAMETER_H

#include "ast/ast_node.h"
#include "ast/ast_type.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {
class ASTParameter : public ASTNode {
public:
    void SetName(const String& name)
    {
        name_ = name;
    }

    String GetName()
    {
        return name_;
    }

    void SetType(ASTType* type)
    {
        type_ = type;
    }

    AutoPtr<ASTType> GetType()
    {
        return type_;
    }

    void SetInParameter(bool inParameter)
    {
        isInParameter_ = inParameter;
    }

    bool IsInParameter()
    {
        return isInParameter_;
    }

    void SetOutParameter(bool outParameter)
    {
        isOutParameter_ = outParameter;
    }

    bool IsOutParameter()
    {
        return isOutParameter_;
    }

    String Dump(const String& prefix) override;

private:
    String name_;
    AutoPtr<ASTType> type_;
    bool isInParameter_ = false;
    bool isOutParameter_ = false;
};
}
}

#endif // OHOS_IDL_ASTPARAMETER_H
