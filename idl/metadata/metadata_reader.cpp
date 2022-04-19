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

#include "metadata/metadata_reader.h"
#include "metadata/metadata_serializer.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace Idl {
const char* MetadataReader::TAG = "MetadataReader";

std::shared_ptr<MetaComponent> MetadataReader::ReadMetadataFromFile(const String& filePath)
{
    File file(filePath, File::READ);
    if (!file.IsValid()) {
        Logger::E(TAG, "Open \"%s\" file failed.", filePath.string());
        return nullptr;
    }

    if (!file.Reset()) {
        Logger::E(TAG, "Reset \"%s\" file failed.", filePath.string());
        return nullptr;
    }

    MetaComponent header;

    if (!file.ReadData((void*)&header, sizeof(MetaComponent))) {
        Logger::E(TAG, "Read \"%s\" file failed.", filePath.string());
        return nullptr;
    }

    if (header.magic_ != METADATA_MAGIC_NUMBER || header.size_ < 0) {
        Logger::E(TAG, "The metadata in \"%s\" file is bad.", filePath.string());
        return nullptr;
    }

    if (!file.Reset()) {
        Logger::E(TAG, "Reset \"%s\" file failed.", filePath.string());
        return nullptr;
    }

    void* data = malloc(header.size_);
    if (data == nullptr) {
        Logger::E(TAG, "Malloc metadata failed.");
        return nullptr;
    }

    if (!file.ReadData(data, header.size_)) {
        Logger::E(TAG, "Read \"%s\" file failed.", filePath.string());
        free(data);
        return nullptr;
    }

    std::shared_ptr<MetaComponent> metadata(
        (MetaComponent*)data, [](MetaComponent* p) { free(p); });

    MetadataSerializer serializer((uintptr_t)data);
    serializer.Deserialize();

    return metadata;
}
}
}
