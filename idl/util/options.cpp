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

#include "options.h"
#include "util/string_builder.h"

#include <cstring>
#include <cstdio>

namespace OHOS {
namespace Idl {
void Options::Parse(int argc, char** argv)
{
    StringBuilder errors;
    program_ = argv[0];

    int i = 1;
    while (i < argc) {
        String option(argv[i++]);
        if (option.Equals("--help")) {
            doShowUsage_ = true;
        } else if (option.Equals("--version")) {
            doShowVersion_ = true;
        } else if (option.Equals("-c")) {
            doCompile_ = true;
        } else if (option.Equals("-dump-ast")) {
            doDumpAST_ = true;
        } else if (option.Equals("-dump-metadata")) {
            doDumpMetadata_ = true;
        } else if (option.Equals("-s")) {
            doSaveMetadata_ = true;
            metadataFile_ = argv[i++];
        } else if (option.Equals("-gen-cpp")) {
            doGenerateCode_ = true;
            targetLanguage_ = "cpp";
        } else if (option.Equals("-gen-ts")) {
            doGenerateCode_ = true;
            targetLanguage_ = "ts";
        } else if (option.Equals("-d")) {
            generationDirectory_ = argv[i++];
        } else if (!option.StartsWith("-")) {
            sourceFile_ = option;
        } else {
            errors.Append(option);
            errors.Append(" ");
        }
    }

    illegalOptions_ = errors.ToString();
}

void Options::ShowErrors()
{
    if (!illegalOptions_.IsEmpty()) {
        String options = illegalOptions_;
        int index;
        while ((index = options.IndexOf(' ')) != -1) {
            printf("The Option \"%s\" is illegal.\n", options.Substring(0, index).string());
            options = options.Substring(index + 1);
        }
    }
    printf("Use \"--help\" to show usage.\n");
}

void Options::ShowVersion()
{
    printf("idl %d.%d\n"
          "Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.\n\n",
           VERSION_MAJOR, VERSION_MINOR);
}

void Options::ShowUsage()
{
    printf("Compile a .idl file and generate metadata, or generate C++ and Ts codes from metadata.\n"
           "Usage: idl [options] file\n"
           "Options:\n"
           "  --help            Display command line options\n"
           "  --version         Display toolchain version information\n"
           "  -dump-ast         Display the AST of the compiled file\n"
           "  -dump-metadata    Display the metadata generated from the compiled file\n"
           "  -c                Compile the .idl file\n"
           "  -s <file>         Place the metadata into <file>\n"
           "  -gen-cpp          Generate C++ codes\n"
           "  -gen-ts           Generate Ts codes\n"
           "  -d <directory>    Place generated codes into <directory>\n");
}
}
}
