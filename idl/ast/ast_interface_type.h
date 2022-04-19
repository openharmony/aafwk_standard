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

#ifndef OHOS_IDL_ASTINTERFACETYPE_H
#define OHOS_IDL_ASTINTERFACETYPE_H

#include <vector>
#include "ast/ast_method.h"
#include "ast/ast_type.h"
#include "util/autoptr.h"

namespace OHOS {
namespace Idl {
class ASTInterfaceType : public ASTType {
public:
    void SetNamespace(ASTNamespace* nspace) override;

    void SetLicense(const String& license)
    {
        license_ = license;
    }

    String GetLicense() const
    {
        return license_;
    }

    void SetOneway(bool oneway)
    {
        oneway_ = oneway;
    }

    bool IsOneway()
    {
        return oneway_;
    }

    void AddMethod(ASTMethod* method);

    AutoPtr<ASTMethod> GetMethod(size_t index);

    size_t GetMethodNumber()
    {
        return methods_.size();
    }

    void SetExternal(bool external)
    {
        isExternal_ = external;
    }

    bool IsExternal() const
    {
        return isExternal_;
    }

    String GetSignature() override;

    bool IsInterfaceType() override;

    String ToString() override;

    String Dump(const String& prefix) override;

private:
    String license_;
    bool oneway_ = false;
    std::vector<AutoPtr<ASTMethod>> methods_;
    bool isExternal_ = false;
};
}
}

#endif // OHOS_IDL_ASTINTERFACETYPE_H
