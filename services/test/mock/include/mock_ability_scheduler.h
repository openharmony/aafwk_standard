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

#ifndef MOCK_OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H
#define MOCK_OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H

#include "want.h"
#include "lifecycle_state_info.h"
#include "gmock/gmock.h"
#include "ability_scheduler_interface.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityScheduler : public IAbilityScheduler {
public:
    MOCK_METHOD2(ScheduleAbilityTransaction, void(const Want &want, const LifeCycleStateInfo &targetState));
    MOCK_METHOD3(SendResult, void(int requestCode, int resultCode, const Want &resultWant));
    MOCK_METHOD1(ScheduleConnectAbility, void(const Want &want));
    MOCK_METHOD1(ScheduleDisconnectAbility, void(const Want &want));
    MOCK_METHOD3(ScheduleCommandAbility, void(const Want &want, bool restart, int startid));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
    MOCK_METHOD0(ScheduleSaveAbilityState, void());
    MOCK_METHOD1(ScheduleRestoreAbilityState, void(const PacMap &inState));
    MOCK_METHOD1(ScheduleUpdateConfiguration, void(const AppExecFwk::Configuration &config));
    MOCK_METHOD1(ScheduleNewWant, void(const Want &want));
    MOCK_METHOD2(ScheduleRegisterObserver, bool(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver));
    MOCK_METHOD2(
        ScheduleUnregisterObserver, bool(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver));
    MOCK_METHOD1(ScheduleNotifyChange, bool(const Uri &uri));
    MOCK_METHOD1(ExecuteBatch, std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>(const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations));
    MOCK_METHOD1(NotifyContinuationResult, void(int32_t result));
    MOCK_METHOD1(ContinueAbility, void(const std::string& deviceId));
    MOCK_METHOD2(DumpAbilityInfo, void(const std::vector<std::string> &params, std::vector<std::string> &info));
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
    {
        std::vector<std::string> types;
        return types;
    }

    int OpenFile(const Uri &uri, const std::string &mode)
    {
        return -1;
    }

    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
    {
        return -1;
    }

    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return -1;
    }

    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return -1;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
    {
        return nullptr;
    }

    virtual std::string GetType(const Uri &uri) override
    {
        return " ";
    }

    virtual int OpenRawFile(const Uri &uri, const std::string &mode) override
    {
        return -1;
    }

    virtual bool Reload(const Uri &uri, const PacMap &extras) override
    {
        return false;
    }

    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override
    {
        return -1;
    }

    virtual Uri NormalizeUri(const Uri &uri) override
    {
        Uri urivalue("");
        return urivalue;
    }

    virtual Uri DenormalizeUri(const Uri &uri) override
    {
        Uri urivalue("");
        return urivalue;
    }
    virtual sptr<IRemoteObject> CallRequest() override
    {
        return sptr<IRemoteObject>(nullptr);
    }
    virtual int BlockAbility() override
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_AAFWK_ABILITY_SCHEDULER_INTERFACE_H
