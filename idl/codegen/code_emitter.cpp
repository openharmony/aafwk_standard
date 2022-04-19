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

#include "codegen/code_emitter.h"

namespace OHOS {
namespace Idl {
const char* CodeEmitter::TAB = "    ";
CodeEmitter::CodeEmitter(MetaComponent* mc)
    : metaComponent_(mc)
{
    metaInterface_ = nullptr;
    for (int i = 0; i < metaComponent_->interfaceNumber_; i++) {
        metaInterface_ = metaComponent_->interfaces_[i];
        if (!metaInterface_->external_) {
            break;
        }
    }

    if (metaInterface_ != nullptr) {
        interfaceName_ = metaInterface_->name_;
        interfaceFullName_ = metaInterface_->namespace_ + interfaceName_;
        proxyName_ = interfaceName_.StartsWith("I") ?
                interfaceName_.Substring(1) + "Proxy" : interfaceName_ + "Proxy";
        proxyFullName_ = metaInterface_->namespace_ + proxyName_;
        stubName_ = interfaceName_.StartsWith("I") ?
                interfaceName_.Substring(1) + "Stub" : interfaceName_ + "Stub";
        stubFullName_ = metaInterface_->namespace_ + stubName_;
    }
}
}
}