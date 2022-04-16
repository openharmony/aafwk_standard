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

#ifndef OHOS_IDL_METADATADUMPER_H
#define OHOS_IDL_METADATADUMPER_H

#include "metadata/metadata.h"
#include "util/string.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Idl {

class MetadataDumper {
public:
    explicit MetadataDumper(
        /* [in] */ MetaComponent* mc)
        : metaComponent_(mc)
    {}

    ~MetadataDumper() = default;

    void Dump(
        /* [in] */ const String& prefix);

private:
    String DumpMetaComponent(
        /* [in] */ MetaComponent* mc,
        /* [in] */ const String& prefix);

    void DumpMetaNamespace(
        /* [in] */ StringBuilder& sb,
        /* [in] */ MetaNamespace* mn,
        /* [in] */ const String& prefix);

    void DumpMetaSequenceable(
        /* [in] */ StringBuilder& sb,
        /* [in] */ MetaSequenceable* mp,
        /* [in] */ const String& prefix);

    void DumpMetaInterface(
        /* [in] */ StringBuilder& sb,
        /* [in] */ MetaInterface* mi,
        /* [in] */ const String& prefix);

    void DumpMetaMethod(
        /* [in] */ StringBuilder& sb,
        /* [in] */ MetaMethod* mm,
        /* [in] */ const String& prefix);

    void DumpMetaParameter(
        /* [in] */ StringBuilder& sb,
        /* [in] */ MetaParameter* mp,
        /* [in] */ const String& prefix);

    String DumpMetaType(
        /* [in] */ MetaType* mt);

    static const char* TAB;
    MetaComponent* metaComponent_;
};

}
}

#endif // OHOS_IDL_METADATADUMPER_H
