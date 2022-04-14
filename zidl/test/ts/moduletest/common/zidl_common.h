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

#ifndef OHOS_ZIDL_MODULE_TEST_ZIDL_COMMON_H
#define OHOS_ZIDL_MODULE_TEST_ZIDL_COMMON_H

#include <gtest/gtest.h>
#include <string>
#define private public
#define protected public
#include "codegen/code_generator.h"
#include "codegen/js_code_emitter.h"
#undef private
#undef protected
#include "metadata/metadata_builder.h"
#include "metadata/metadata_dumper.h"
#include "metadata/metadata_reader.h"
#include "metadata/metadata_serializer.h"
#include "parser/parser.h"
#include "util/file.h"
#include "util/logger.h"
#include "util/options.h"
#include "util/string.h"
#include "util/string_builder.h"
#include "zidl_file.h"

namespace OHOS {
namespace Zidl {
namespace ModuleTest {
const int ERR_FAIL = -1;
const int ERR_OK = 0;

class ZidlCommon {
public:
    ZidlCommon() = default;
    ~ZidlCommon() = default;

    int Ready(int argc, char** argv)
    {
        Options options(argc, argv);
        std::shared_ptr<MetaComponent> metadata;

        if (options.DoCompile()) {
            Parser parser(options);
            if (!parser.Parse(options.GetSourceFile())) {
                return ERR_FAIL;
            }

            MetadataBuilder builder(parser.GetModule());
            metadata = builder.Build();
            if (metadata == nullptr) {
                return ERR_FAIL;
            }
        }

        if (options.DoDumpMetadata()) {
            MetadataDumper dumper(metadata.get());
            dumper.Dump("");
        }

        if (options.DoSaveMetadata()) {
            File metadataFile(options.GetMetadataFile(), File::WRITE);
            if (!metadataFile.IsValid()) {
                return ERR_FAIL;
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
                    return ERR_FAIL;
                }
            }
            if (options.GetTargetLanguage().Equals("js")) {
                this->jsCodeGen_ = std::make_shared<JsCodeEmitter>(metadata.get());
                this->jsCodeGen_->SetDirectory(options.GetGenerationDirectory());
            }
        }
        return 0;
    }

    int PrepareZidlFile(const std::string &fileName, const std::string &fileContent)
    {
        String filePath = String::Format("%s/%s", "./", fileName.c_str());
        File file(filePath, File::WRITE);
        if (!file.IsValid()) {
            GTEST_LOG_(INFO) << "OPEN FILE FAIL";
            return ERR_FAIL;
        }

        StringBuilder stringBuilder;
        stringBuilder.Append(LICENSE_CONTENT.c_str());
        stringBuilder.Append(fileContent.c_str());

        String data = stringBuilder.ToString();
        file.WriteData(data.string(), data.GetLength());
        file.Flush();
        file.Close();
        return ERR_OK;
    }

    std::shared_ptr<JsCodeEmitter> jsCodeGen_ = nullptr;
};
}
}
}
#endif  // OHOS_ZIDL_MODULE_TEST_ZIDL_COMMON_H