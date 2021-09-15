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

#ifndef MOCK_ABILITY_TEST_H
#define MOCK_ABILITY_TEST_H

#include "gmock/gmock.h"

#include "ability.h"
#include "ability_loader.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {

class MockAbilityTest : public Ability {
public:

    int Insert(const Uri &uri, const ValuesBucket &value)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Insert called";
        return 20;
    }
    int Update(const Uri &uri, const ValuesBucket &value, const DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Update called";
        return 33;
    }

    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
    {
        std::vector<std::string> result;
        result.push_back("Type1");
        result.push_back("Type2");
        result.push_back("Type3");
        return result;
    }

    int OpenFile(const Uri &uri, const std::string &mode)
    {
        int fd;
        GTEST_LOG_(INFO) << "MockAbilityTest::OpenFile called";
        FILE *fd1 = fopen("/test/te.txt", "r");
        fd = fileno(fd1);

        return fd;
    }

    int OpenRawFile(const Uri &uri, const std::string &mode)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::OpenRawFile called";

        return 122;
    }

    int BatchInsert(const Uri &uri, const std::vector<ValuesBucket> &values)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::BatchInsert called";
        return 115;
    }

    bool Reload(const Uri &uri, const PacMap &extras)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Reload called";
        return true;
    }

    int Delete(const Uri &uri, const DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Delete called";
        return 234;
    }

    std::string GetType(const Uri &uri)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::GetType called";
        std::string type("Type1");
        return type;
    }

    Uri NormalizeUri(const Uri &uri)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::NormalizeUri called";
        return uri;
    }

    Uri DenormalizeUri(const Uri &uri)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::DenormalizeUri called";
        return uri;
    }

};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_ABILITY_TEST_H