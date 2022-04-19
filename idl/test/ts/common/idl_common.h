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

#ifndef OHOS_IDL_MODULE_TEST_IDL_COMMON_H
#define OHOS_IDL_MODULE_TEST_IDL_COMMON_H

#include <gtest/gtest.h>
#include <string>
#define private public
#define protected public
#include "codegen/code_emitter.h"
#include "codegen/code_generator.h"
#include "codegen/ts_code_emitter.h"
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
#include "idl_file.h"
#include "securec.h"

namespace OHOS {
namespace Idl {
namespace TestCommon {
const int ERR_FAIL = -1;
const int ERR_OK = 0;

class IdlCommon {
public:
    IdlCommon() = default;
    ~IdlCommon() = default;

    int Ready(int argc, char** argv)
    {
        Options options(argc, argv);
        std::shared_ptr<MetaComponent> metadata = nullptr;

        if (options.DoCompile()) {
            Parser parser(options);
            if (!parser.Parse(options.GetSourceFile())) {
                return ERR_FAIL;
            }

            MetadataBuilder builder(parser.GetModule());
            metadata = builder.Build();
            metadata_ = metadata;
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
            if (options.GetTargetLanguage().Equals("ts")) {
                this->tsCodeGen_ = std::make_shared<TsCodeEmitter>(metadata.get());
                this->tsCodeGen_->SetDirectory(options.GetGenerationDirectory());
            }
        }
        return 0;
    }

    int PrepareIdlFile(
        const std::string &fileName, const std::string &fileContent, const std::string &fileLicense = LICENSE_CONTENT)
    {
        String filePath = String::Format("%s/%s", "./", fileName.c_str());
        File file(filePath, File::WRITE);
        if (!file.IsValid()) {
            GTEST_LOG_(INFO) << "OPEN FILE FAIL";
            return ERR_FAIL;
        }

        StringBuilder stringBuilder;
        stringBuilder.Append(fileLicense.c_str());
        stringBuilder.Append(fileContent.c_str());

        String data = stringBuilder.ToString();
        file.WriteData(data.string(), data.GetLength());
        file.Flush();
        file.Close();
        return ERR_OK;
    }

    std::shared_ptr<MetaComponent> metadata_ = nullptr;
    std::shared_ptr<TsCodeEmitter> tsCodeGen_ = nullptr;
};

class ParameterArgv {
public:
    ParameterArgv(const char** args, const int argc) : argc_(argc)
    {
        argv_ = new char* [argc_] {nullptr};
        for (int i = 0; i < argc_; ++i) {
            const int itemSize = strlen(args[i]);
            argv_[i] = new char[itemSize + 1] {0};
            if (strcpy_s(argv_[i], itemSize + 1, args[i])) {
                GTEST_LOG_(ERROR) << "strcpy_s error [!EOK]";
            }
        }
    };
    ~ParameterArgv()
    {
        if (argv_ == nullptr) {
            return;
        }

        for (int i = 0; i < argc_; ++i) {
            if (argv_[i] == nullptr) {
                continue;
            }
            delete[] argv_[i];
            argv_[i] = nullptr;
        }
        delete[] argv_;
        argv_ = nullptr;
    };

    char** GetArgv()
    {
        return argv_;
    };

    char** argv_;
    const int argc_;
};
}
}
}
#endif  // OHOS_IDL_MODULE_TEST_IDL_COMMON_H