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
#ifndef OHOS_APPEXECFWK_MOCK_ABILITY_SCHEDULER_FOR_OBESERVER_H
#define OHOS_APPEXECFWK_MOCK_ABILITY_SCHEDULER_FOR_OBESERVER_H

#include "gmock/gmock.h"

#include "ability_scheduler_stub.h"

#include <iremote_object.h>
#include <iremote_stub.h>

namespace OHOS {
namespace AppExecFwk {

// copy AbilityThread class
class MockAbilitySchedulerStub : public AAFwk::AbilitySchedulerStub {
public:
    MockAbilitySchedulerStub() = default;
    virtual ~MockAbilitySchedulerStub() = default;

    MOCK_METHOD2(ScheduleAbilityTransaction, void(const AAFwk::Want &, const AAFwk::LifeCycleStateInfo &));
    MOCK_METHOD3(SendResult, void(int, int, const AAFwk::Want &));
    MOCK_METHOD1(ScheduleConnectAbility, void(const AAFwk::Want &));
    MOCK_METHOD1(ScheduleDisconnectAbility, void(const AAFwk::Want &));
    MOCK_METHOD3(ScheduleCommandAbility, void(const AAFwk::Want &, bool, int));
    MOCK_METHOD1(ScheduleSaveAbilityState, void(PacMap &));
    MOCK_METHOD1(ScheduleRestoreAbilityState, void(const PacMap &));
    MOCK_METHOD1(ScheduleUpdateConfiguration, void(const AAFwk::DummyConfiguration &));
    MOCK_METHOD2(GetFileTypes, std::vector<std::string>(const Uri &, const std::string &));
    MOCK_METHOD2(OpenFile, int(const Uri &, const std::string &));
    MOCK_METHOD2(OpenRawFile, int(const Uri &, const std::string &));
    MOCK_METHOD2(Insert, int(const Uri &, const ValuesBucket &));
    MOCK_METHOD3(Update, int(const Uri &, const ValuesBucket &, const DataAbilityPredicates &));
    MOCK_METHOD2(Delete, int(const Uri &, const DataAbilityPredicates &));
    MOCK_METHOD3(
        Query, std::shared_ptr<ResultSet>(const Uri &, std::vector<std::string> &, const DataAbilityPredicates &));
    MOCK_METHOD1(GetType, std::string(const Uri &));
    MOCK_METHOD2(Reload, bool(const Uri &, const PacMap &));
    MOCK_METHOD2(BatchInsert, int(const Uri &, const std::vector<ValuesBucket> &));
    MOCK_METHOD1(DenormalizeUri, Uri(const Uri &));
    MOCK_METHOD1(NormalizeUri, Uri(const Uri &));
    MOCK_METHOD1(NotifyTopActiveAbilityChanged, void(bool flag));
    MOCK_METHOD2(NotifyMultiWinModeChanged, void(int32_t winModeKey, bool flag));
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif /* OHOS_APPEXECFWK_MOCK_ABILITY_SCHEDULER_FOR_OBESERVER_H */