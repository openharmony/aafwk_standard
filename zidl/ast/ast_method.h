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

#ifndef OHOS_ZIDL_ASTMETHOD_H
#define OHOS_ZIDL_ASTMETHOD_H

#include <vector>
#include "ast/ast_node.h"
#include "ast/ast_parameter.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace Zidl {

class ASTMethod : public ASTNode {
public:
    inline void SetName(
        /* [in] */ const String& name)
    {
        name_ = name;
    }

    inline String GetName()
    {
        return name_;
    }

    String GetSignature();

    inline void SetOneway(
        /* [in] */ bool oneway)
    {
        oneway_ = oneway;
    }

    inline bool IsOneway()
    {
        return oneway_;
    }

    inline void SetReturnType(
        /* [in] */ ASTType* type)
    {
        returnType_ = type;
    }

    inline AutoPtr<ASTType> GetReturnType()
    {
        return returnType_;
    }

    void AddParameter(
        /* [in] */ ASTParameter* parameter);

    AutoPtr<ASTParameter> GetParameter(
        /* [in] */ size_t index);

    inline size_t GetParameterNumber()
    {
        return parameters_.size();
    }

    String Dump(
        /* [in] */ const String& prefix) override;

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

#endif // OHOS_ZIDL_ASTMETHOD_H
