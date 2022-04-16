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

#include "codegen/code_generator.h"
#include "metadata/metadata_builder.h"
#include "metadata/metadata_dumper.h"
#include "metadata/metadata_reader.h"
#include "metadata/metadata_serializer.h"
#include "parser/parser.h"
#include "util/logger.h"
#include "util/options.h"

using namespace OHOS::Idl;

static const char* TAG = "idl";

int main(int argc, char** argv)
{
    Options options(argc, argv);

    if (options.DoShowUsage()) {
        options.ShowUsage();
        return 0;
    }

    if (options.DoShowVersion()) {
        options.ShowVersion();
        return 0;
    }

    if (options.HasErrors()) {
        options.ShowErrors();
        return 0;
    }

    std::shared_ptr<MetaComponent> metadata;

    if (options.DoCompile()) {
        Parser parser(options);
        if (!parser.Parse(options.GetSourceFile())) {
            Logger::E(TAG, "Parsing .idl failed.");
            return -1;
        }

        MetadataBuilder builder(parser.GetModule());
        metadata = builder.Build();
        if (metadata == nullptr) {
            Logger::E(TAG, "Generate metadata failed.");
            return -1;
        }
    }

    if (options.DoDumpMetadata()) {
        MetadataDumper dumper(metadata.get());
        dumper.Dump("");
    }

    if (options.DoSaveMetadata()) {
        File metadataFile(options.GetMetadataFile(), File::WRITE);
        if (!metadataFile.IsValid()) {
            Logger::E(TAG, "Create metadata file failed.");
            return -1;
        }

        MetadataSerializer serializer(metadata.get());
        serializer.Serialize();
        uintptr_t data = serializer.GetData();
        int size = serializer.GetDataSize();

        metadataFile.WriteData(reinterpret_cast<void*>(data), size);
        metadataFile.Flush();
        metadataFile.Close();
    }

    if (options.DoGenerateCode()) {
        if (metadata == nullptr) {
            String metadataFile = options.GetMetadataFile();
            metadata = MetadataReader::ReadMetadataFromFile(metadataFile);
            if (metadata == nullptr) {
                Logger::E(TAG, "Get metadata from \"%s\" failed.", metadataFile.string());
                return -1;
            }
        }

        CodeGenerator codeGen(metadata.get(), options.GetTargetLanguage(),
                options.GetGenerationDirectory());
        if (!codeGen.Generate()) {
            Logger::E(TAG, "Generate \"%s\" codes failed.", options.GetTargetLanguage().string());
            return -1;
        }
    }

    return 0;
}
