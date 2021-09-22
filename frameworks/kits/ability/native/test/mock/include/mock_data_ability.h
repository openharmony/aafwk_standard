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

#ifndef FOUNDATION_APPEXECFWK_OHOS_MOCK_DATA_ABILITY_H
#define FOUNDATION_APPEXECFWK_OHOS_MOCK_DATA_ABILITY_H

#include "ability.h"
#include <gtest/gtest.h>

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class MockDataAbility : public Ability {
public:
    MockDataAbility() = default;
    virtual ~MockDataAbility() = default;

    enum Event { ON_ACTIVE = 0, ON_BACKGROUND, ON_FOREGROUND, ON_INACTIVE, ON_START, ON_STOP, UNDEFINED };

    int OpenFile(const Uri &uri, const std::string &mode)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::OpenFile called";

        return 1;
    }

    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Insert called";

        return 1;
    }

    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Update called";

        return 1;
    }

    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Delete called";

        return 1;
    }

    int OpenRawFile(const Uri &uri, const std::string &mode)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::OpenRawFile called";

        return 1;
    }

    bool Reload(const Uri &uri, const PacMap &extras)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Reload called";

        return 1;
    }

    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::BatchInsert called";

        return 1;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::Query called";
        std::shared_ptr<NativeRdb::AbsSharedResultSet> set = std::make_shared<NativeRdb::AbsSharedResultSet>("QueryTest");
        return set;
    }

    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::GetFileTypes called";
        value.push_back(mimeTypeFilter);
        return value;
    }

    std::string GetType(const Uri &uri)
    {
        GTEST_LOG_(INFO) << "MockDataAbility::GetType called";
        std::string value("Type1");
        return value;
    }

    int datatest = 0;
    MockDataAbility::Event state_ = UNDEFINED;
    std::vector<std::string> value;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_MOCK_PAGE_ABILITY_H