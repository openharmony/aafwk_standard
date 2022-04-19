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

#ifndef OHOS_IDL_ASTMETHOD_H
#define OHOS_IDL_ASTMETHOD_H

#include <vector>
#include "ast/ast_node.h"
#include "ast/ast_parameter.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {
class ASTMethod : public ASTNode {
public:
    void SetName(const String& name)
    {
        name_ = name;
    }

    String GetName()
    {
        return name_;
    }

    String GetSignature();

    void SetOneway(bool oneway)
    {
        oneway_ = oneway;
    }

    bool IsOneway()
    {
        return oneway_;
    }

    void SetReturnType(ASTType* type)
    {
        returnType_ = type;
    }

    AutoPtr<ASTType> GetReturnType()
    {
        return returnType_;
    }

    void AddParameter(ASTParameter* parameter);

    AutoPtr<ASTParameter> GetParameter(size_t index);

    size_t GetParameterNumber()
    {
        return parameters_.size();
    }

    String Dump(const String& prefix) override;

private:
    void BuildSignature();

    String name_;
    String signature_;
    bool oneway_ = false;
    AutoPtr<ASTType> returnType_;
    std::vector<AutoPtr<ASTParameter>> parameters_;
};
}
}

#endif // OHOS_IDL_ASTMETHOD_H
