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

#ifndef OHOS_IDL_CODEEMITTER_H
#define OHOS_IDL_CODEEMITTER_H

#include "metadata/meta_component.h"
#include "util/light_refcount_base.h"
#include "util/string.h"

namespace OHOS {
namespace Idl {
class CodeEmitter : public LightRefCountBase {
public:
    CodeEmitter(MetaComponent* mc);

    void SetDirectory(const String& directory)
    {
        directory_ = directory;
    }

    virtual void EmitInterface() = 0;

    virtual void EmitInterfaceProxy() = 0;

    virtual void EmitInterfaceStub() = 0;

protected:
    static const char* TAB;

    MetaComponent* metaComponent_;
    MetaInterface* metaInterface_;
    String directory_;
    String interfaceName_;
    String interfaceFullName_;
    String proxyName_;
    String proxyFullName_;
    String stubName_;
    String stubFullName_;
};
}
}

#endif // OHOS_IDL_CODEEMITTER_H
