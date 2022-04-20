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

#ifndef OHOS_IDL_METADATADUMPER_H
#define OHOS_IDL_METADATADUMPER_H

#include "metadata/meta_component.h"
#include "util/string.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {
class MetadataDumper {
public:
    explicit MetadataDumper(MetaComponent* mc)
        : metaComponent_(mc)
    {}

    ~MetadataDumper() = default;

    void Dump(const String& prefix);

private:
    String DumpMetaComponent(MetaComponent* mc, const String& prefix);

    void DumpMetaNamespace(StringBuilder& sb, MetaNamespace* mn, const String& prefix);

    void DumpMetaSequenceable(StringBuilder& sb, MetaSequenceable* mp, const String& prefix);

    void DumpMetaInterface(StringBuilder& sb, MetaInterface* mi, const String& prefix);

    void DumpMetaMethod(StringBuilder& sb, MetaMethod* mm, const String& prefix);

    void DumpMetaParameter(StringBuilder& sb, MetaParameter* mp, const String& prefix);

    String DumpMetaType(MetaType* mt);

    static const char* TAB;
    MetaComponent* metaComponent_;
};
}
}

#endif // OHOS_IDL_METADATADUMPER_H
