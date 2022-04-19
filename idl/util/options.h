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

#ifndef OHOS_IDL_OPTIONS_H
#define OHOS_IDL_OPTIONS_H

#include "util/string.h"

namespace OHOS {
namespace Idl {
class Options {
public:
    Options(int argc, char** argv)
    {
        Parse(argc, argv);
    }

    ~Options() = default;

    bool DoShowUsage() const
    {
        return doShowUsage_;
    }

    bool DoShowVersion() const
    {
        return doShowVersion_;
    }

    bool DoCompile() const
    {
        return doCompile_;
    }

    bool DoDumpAST() const
    {
        return doDumpAST_;
    }

    bool DoDumpMetadata() const
    {
        return doDumpMetadata_;
    }

    bool DoSaveMetadata() const
    {
        return doSaveMetadata_;
    }

    bool DoGenerateCode() const
    {
        return doGenerateCode_;
    }

    bool HasErrors() const
    {
        return !illegalOptions_.IsEmpty() || sourceFile_.IsEmpty();
    }

    String GetSourceFile() const
    {
        return sourceFile_;
    }

    String GetMetadataFile() const
    {
        return metadataFile_;
    }

    String GetTargetLanguage() const
    {
        return targetLanguage_;
    }

    String GetGenerationDirectory() const
    {
        return generationDirectory_;
    }

    void ShowErrors();

    void ShowVersion();

    void ShowUsage();

private:
    void Parse(int argc, char** argv);

    static constexpr int VERSION_MAJOR = 0;
    static constexpr int VERSION_MINOR = 1;

    String program_;
    String sourceFile_;
    String metadataFile_;
    String targetLanguage_;
    String generationDirectory_;
    String illegalOptions_;

    bool doShowUsage_ = false;
    bool doShowVersion_ = false;
    bool doCompile_ = false;
    bool doDumpAST_ = false;
    bool doDumpMetadata_ = false;
    bool doSaveMetadata_ = false;
    bool doGenerateCode_ = false;
};
}
}
#endif // OHOS_IDL_OPTIONS_H
