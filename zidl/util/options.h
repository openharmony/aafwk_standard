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

#ifndef OHOS_ZIDL_OPTIONS_H
#define OHOS_ZIDL_OPTIONS_H

#include "util/string.h"

namespace OHOS {
namespace Zidl {

class Options {
public:
    inline Options(
        /* [in] */ int argc,
        /* [in] */ char** argv)
    {
        Parse(argc, argv);
    }

    ~Options() = default;

    inline bool DoShowUsage() const
    {
        return doShowUsage_;
    }

    inline bool DoShowVersion() const
    {
        return doShowVersion_;
    }

    inline bool DoCompile() const
    {
        return doCompile_;
    }

    inline bool DoDumpAST() const
    {
        return doDumpAST_;
    }

    inline bool DoDumpMetadata() const
    {
        return doDumpMetadata_;
    }

    inline bool DoSaveMetadata() const
    {
        return doSaveMetadata_;
    }

    inline bool DoGenerateCode() const
    {
        return doGenerateCode_;
    }

    inline bool HasErrors() const
    {
        return !illegalOptions_.IsEmpty() || sourceFile_.IsEmpty();
    }

    inline String GetSourceFile() const
    {
        return sourceFile_;
    }

    inline String GetMetadataFile() const
    {
        return metadataFile_;
    }

    inline String GetTargetLanguage() const
    {
        return targetLanguage_;
    }

    inline String GetGenerationDirectory() const
    {
        return generationDirectory_;
    }

    void ShowErrors();

    void ShowVersion();

    void ShowUsage();

private:
    void Parse(
        /* [in] */ int argc,
        /* [in] */ char** argv);

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

#endif // OHOS_ZIDL_OPTIONS_H
