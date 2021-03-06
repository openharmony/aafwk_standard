/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

#include "hilog_wrapper.h"
#include "ability_loader.h"
#include "verify_act_data_ability.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static std::shared_ptr<NativeRdb::RdbStore> testStore;
static const std::string RDB_TEST_PATH = "/data/test/";
static const std::string DATABASE_FILE_NAME = "insert_test.db";
static const std::string DATABASE_NAME = RDB_TEST_PATH + "insert_test.db";
static const int defaultReturn = 1;
}

const std::string InsertTestOpenCallback::CREATE_TABLE_TEST =
    std::string("CREATE TABLE IF NOT EXISTS test ") + std::string("(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                                  "name TEXT NOT NULL, age INTEGER, salary "
                                                                  "REAL, blobType BLOB)");

int InsertTestOpenCallback::OnCreate(NativeRdb::RdbStore &store)
{
    return store.ExecuteSql(CREATE_TABLE_TEST);
}

int InsertTestOpenCallback::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}

void VerifyActDataAbility::OnStart(const Want &want)
{
    HILOG_INFO("VerifyActDataAbility OnStart");
    sharedList_.clear();
    Ability::OnStart(want);
}

int VerifyActDataAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("VerifyActDataAbility <<<<Insert>>>>");
    return defaultReturn;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> VerifyActDataAbility::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    int errCode = 0;
    std::string dbDir = GetDatabaseDir();
    NativeRdb::RdbStoreConfig config(dbDir + "/" + DATABASE_FILE_NAME);
    InsertTestOpenCallback helper;
    testStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, helper, errCode);
    HILOG_INFO("VerifyActDataAbility <<<<Query>>>> Patch %{public}s", (dbDir + "/" + DATABASE_FILE_NAME).c_str());

    int64_t id;
    NativeRdb::ValuesBucket values;
    int idValue = 1;
    int age = 18;
    double salary = 100.5;

    values.PutInt("id", idValue);
    values.PutString("name", std::string("zhangsan"));
    values.PutInt("age", age);
    values.PutDouble("salary", salary);
    values.PutBlob("blobType", std::vector<uint8_t> {1, 2, 3});

    if (testStore == nullptr) {
        HILOG_INFO("VerifyActDataAbility <<<<Query>>>> testStore is nullptr, data will be empty");
        return nullptr;
    }

    if (testStore->Replace(id, "test", values) != 0) {
        HILOG_ERROR("VerifyActDataAbility <<<<Query>>>> store->Replace Error");
        return nullptr;
    }

    std::unique_ptr<NativeRdb::AbsSharedResultSet> rresultSet =
        testStore->QuerySql("SELECT * FROM test WHERE name = ?", std::vector<std::string> {"zhangsan"});
    if (rresultSet == nullptr) {
        HILOG_ERROR("VerifyActDataAbility <<<<Query>>>> rresultSet is nullptr");
        return nullptr;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret(rresultSet.release());
    sharedList_.push_back(ret);
    return ret;
}

int VerifyActDataAbility::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("VerifyActDataAbility <<<<Update>>>>");
    return defaultReturn;
}
int VerifyActDataAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("VerifyActDataAbility <<<<Delete>>>>");
    return defaultReturn;
}
std::string VerifyActDataAbility::GetType(const Uri &uri)
{
    HILOG_INFO("VerifyActDataAbility <<<<GetType>>>>");
    std::string retval(uri.ToString());
    return retval;
}
int VerifyActDataAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("VerifyActDataAbility <<<<OpenFile>>>>");
    return 0;
}
int VerifyActDataAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("VerifyActDataAbility <<<<BatchInsert>>>>");
    return Ability::BatchInsert(uri, values);
}

REGISTER_AA(VerifyActDataAbility);
}  // namespace AppExecFwk
}  // namespace OHOS
