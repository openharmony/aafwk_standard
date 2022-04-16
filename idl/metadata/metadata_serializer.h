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

#ifndef OHOS_IDL_METADATASERIALIZER_H
#define OHOS_IDL_METADATASERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include "metadata/metadata.h"

namespace OHOS {
namespace Idl {

class MetadataSerializer {
public:
    explicit MetadataSerializer(
        /* [in] */ MetaComponent* mc)
        : metaComponent_(mc),
          baseAddr_(reinterpret_cast<uintptr_t>(mc))
    {}

    explicit MetadataSerializer(
        /* [in] */ uintptr_t addr)
        : metaComponent_(reinterpret_cast<MetaComponent*>(addr)),
          baseAddr_(addr)
    {}

    ~MetadataSerializer() = default;

    void Serialize();

    void Deserialize();

    uintptr_t GetData() const
    {
        return baseAddr_;
    }

    int GetDataSize() const
    {
        return metaComponent_->size_;
    }

private:
    void SerializeMetaComponent(
        /* [in] */ MetaComponent* mc);

    void SerializeMetaNamespace(
        /* [in] */ MetaNamespace* mn);

    void SerializeMetaSequenceable(
        /* [in] */ MetaSequenceable* mp);

    void SerializeMetaInterface(
        /* [in] */ MetaInterface* mi);

    void SerializeMetaMethod(
        /* [in] */ MetaMethod* mm);

    void SerializeMetaParameter(
        /* [in] */ MetaParameter* mp);

    void SerializeMetaType(
        /* [in] */ MetaType* mt);

    ptrdiff_t SerializeAdjust(
        /* [in] */ const void* addr);

    void DeserializeMetaComponent(
        /* [in] */ MetaComponent* mc);

    void DeserializeMetaNamespace(
        /* [in] */ MetaNamespace* mn);

    void DeserializeMetaSequenceable(
        /* [in] */ MetaSequenceable* mp);

    void DeserializeMetaInterface(
        /* [in] */ MetaInterface* mi);

    void DeserializeMetaMethod(
        /* [in] */ MetaMethod* mm);

    void DeserializeMetaParameter(
        /* [in] */ MetaParameter* mp);

    void DeserializeMetaType(
        /* [in] */ MetaType* mt);

    uintptr_t DeserializeAdjust(
        /* [in] */ const void* addr);

    MetaComponent* metaComponent_;
    uintptr_t baseAddr_;
};

}
}

#endif // OHOS_IDL_METADATASERIALIZER_H
