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

#include "ability_scheduler.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

AbilityScheduler::AbilityScheduler()
{}

AbilityScheduler::~AbilityScheduler()
{}

void AbilityScheduler::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState)
{
    HILOG_INFO("AbilityScheduler ScheduleAbilityTransaction %d", targetState.state);
    (void)want;
}

void AbilityScheduler::SendResult(int requestCode, int resultCode, const Want &resultWant)
{
    HILOG_INFO("AbilityScheduler SendResult %d resultCode %d", requestCode, resultCode);
    result_ = AbilityResult(requestCode, resultCode, resultWant);
}

const AbilityResult &AbilityScheduler::GetResult() const
{
    return result_;
}

void AbilityScheduler::ScheduleConnectAbility(const Want &want)
{
    (void)want;
}

void AbilityScheduler::ScheduleDisconnectAbility(const Want &want)
{}

void AbilityScheduler::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{}

void AbilityScheduler::ScheduleSaveAbilityState()
{}

void AbilityScheduler::ScheduleRestoreAbilityState(const PacMap &inState)
{}

void AbilityScheduler::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{}

std::vector<std::string> AbilityScheduler::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> values;
    return values;
}

int AbilityScheduler::OpenFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

int AbilityScheduler::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    return -1;
}

int AbilityScheduler::Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    return -1;
}

int AbilityScheduler::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    return -1;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityScheduler::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    return nullptr;
}

std::string AbilityScheduler::GetType(const Uri &uri)
{
    return result_.resultWant_.GetType();
}

int AbilityScheduler::OpenRawFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

bool AbilityScheduler::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

int AbilityScheduler::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    return -1;
}

void AbilityScheduler::NotifyMultiWinModeChanged(int32_t winModeKey, bool flag)
{}
Uri AbilityScheduler::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    return urivalue;
}

Uri AbilityScheduler::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    return urivalue;
}

// void AbilityScheduler::NotifyTopActiveAbilityChanged(bool flag)
// {}

}  // namespace AAFwk
}  // namespace OHOS
