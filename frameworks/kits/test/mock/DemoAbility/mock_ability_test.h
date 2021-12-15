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
#define INSERT 20
#define UPDATE 33
#define OPENRAWFILE 122
#define BATCHINSERT 115
#define DELETE 234

class MockAbilityTest : public Ability {
public:

    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Insert called";
        return INSERT;
    }
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Update called";
        return UPDATE;
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
        FILE *fd1 = fopen("/dataability_openfile_test.txt", "w+");
        if (fd1 == nullptr) {
            GTEST_LOG_(INFO) << "MockAbilityTest::OpenFile fd1 == nullptr";
            return -1;
        }
        fputs("123456", fd1);
        fclose(fd1);

        FILE *fd2 = fopen("/dataability_openfile_test.txt", "r");
        if (fd2 == nullptr) {
            GTEST_LOG_(INFO) << "MockAbilityTest::OpenFile fd2 == nullptr";
            return -1;
        }
        fd = fileno(fd2);
       
        return fd;
    }

    int OpenRawFile(const Uri &uri, const std::string &mode)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::OpenRawFile called";

        return OPENRAWFILE;
    }

    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::BatchInsert called";
        return BATCHINSERT;
    }

    bool Reload(const Uri &uri, const PacMap &extras)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Reload called";
        return true;
    }

    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::Delete called";
        return DELETE;
    }

    std::string GetType(const Uri &uri)
    {
        GTEST_LOG_(INFO) << "MockAbilityTest::GetType called";
        std::string type("Type1");
        return type;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Query called";
        std::shared_ptr<NativeRdb::AbsSharedResultSet> set =
        std::make_shared<NativeRdb::AbsSharedResultSet>("QueryTest");
        return set;
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