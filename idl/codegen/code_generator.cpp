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
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include "codegen/cpp_code_emitter.h"
#include "codegen/ts_code_emitter.h"
#include "util/logger.h"

namespace OHOS {
namespace Idl {
const char* CodeGenerator::TAG = "CodeGenerator";
CodeGenerator::CodeGenerator(MetaComponent* mc, const String& language, const String& dir)
    : targetLanguage_(language),
      targetDirectory_(dir),
      metaComponent_(mc)
{
    if (language.Equals("cpp")) {
        emitter_ = new CppCodeEmitter(metaComponent_);
    } else if (language.Equals("ts")) {
        emitter_ = new TsCodeEmitter(metaComponent_);
    }
}

bool CodeGenerator::ResolveDirectory()
{
#ifdef __MINGW32__
    if (targetDirectory_.IndexOf(":\\") == -1) {
        char* cmd = getcwd(nullptr, 0);
        targetDirectory_ = String::Format("%s\\%s", cmd, targetDirectory_.string());
        free(cmd);
    }
#else
    if (!targetDirectory_.StartsWith("/")) {
        char* cwd = getcwd(nullptr, 0);
        targetDirectory_ = String::Format("%s/%s", cwd, targetDirectory_.string());
        free(cwd);
    }
#endif

    if (!access(targetDirectory_.string(), R_OK | W_OK)) {
        return true;
    }

#ifdef __MINGW32__
    if (mkdir(targetDirectory_.string()) != 0) {
#else
    if (mkdir(targetDirectory_.string(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
#endif
        Logger::E(TAG, "Create \"%s\" directory failed.", targetDirectory_.string());
        return false;
    }

    return true;
}

bool CodeGenerator::Generate()
{
    if (!ResolveDirectory()) {
        return false;
    }

    emitter_->SetDirectory(targetDirectory_);

    emitter_->EmitInterface();
    emitter_->EmitInterfaceProxy();
    emitter_->EmitInterfaceStub();

    return true;
}
}
}