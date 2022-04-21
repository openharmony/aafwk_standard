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

#ifndef OHOS_IDL_METADATASERIALIZER_H
#define OHOS_IDL_METADATASERIALIZER_H

#include <cstddef>
#include <cstdint>
#include "metadata/meta_component.h"

namespace OHOS {
namespace Idl {
class MetadataSerializer {
public:
    explicit MetadataSerializer(MetaComponent* mc)
        : metaComponent_(mc),
          baseAddr_(reinterpret_cast<uintptr_t>(mc))
    {}

    explicit MetadataSerializer(uintptr_t addr)
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
    void SerializeMetaComponent(MetaComponent* mc);

    void SerializeMetaNamespace(MetaNamespace* mn);

    void SerializeMetaSequenceable(MetaSequenceable* mp);

    void SerializeMetaInterface(MetaInterface* mi);

    void SerializeMetaMethod(MetaMethod* mm);

    void SerializeMetaParameter(MetaParameter* mp);

    void SerializeMetaType(MetaType* mt);

    ptrdiff_t SerializeAdjust(const void* addr);

    void DeserializeMetaComponent(MetaComponent* mc);

    void DeserializeMetaNamespace(MetaNamespace* mn);

    void DeserializeMetaSequenceable(MetaSequenceable* mp);

    void DeserializeMetaInterface(MetaInterface* mi);

    void DeserializeMetaMethod(MetaMethod* mm);

    void DeserializeMetaParameter(MetaParameter* mp);

    void DeserializeMetaType(MetaType* mt);

    uintptr_t DeserializeAdjust(const void* addr);

    MetaComponent* metaComponent_;
    uintptr_t baseAddr_;
};
}
}

#endif // OHOS_IDL_METADATASERIALIZER_H
