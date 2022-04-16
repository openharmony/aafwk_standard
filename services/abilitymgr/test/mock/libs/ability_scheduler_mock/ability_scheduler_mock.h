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

#pragma once
#include "ability_scheduler_interface.h"
#include <iremote_object.h>
#include <iremote_stub.h>
#include "hilog_wrapper.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace AAFwk {
class AbilitySchedulerMock : public IRemoteStub<IAbilityScheduler> {
public:
    AbilitySchedulerMock() : code_(0)
    {}
    virtual ~AbilitySchedulerMock()
    {}

    MOCK_METHOD2(ScheduleAbilityTransaction, void(const Want &, const LifeCycleStateInfo &));
    MOCK_METHOD3(SendResult, void(int, int, const Want &));
    MOCK_METHOD1(ScheduleConnectAbility, void(const Want &));
    MOCK_METHOD1(ScheduleDisconnectAbility, void(const Want &));
    MOCK_METHOD0(ScheduleSaveAbilityState, void());
    MOCK_METHOD1(ScheduleRestoreAbilityState, void(const PacMap &));
    MOCK_METHOD1(ScheduleUpdateConfiguration, void(const AppExecFwk::Configuration &));
    MOCK_METHOD1(ScheduleNewWant, void(const Want &));
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD3(ScheduleCommandAbility, void(const Want &, bool, int));
    MOCK_METHOD1(NotifyContinuationResult, void(int32_t result));
    MOCK_METHOD1(ContinueAbility, void(const std::string& deviceId));
    MOCK_METHOD2(DumpAbilityInfo, void(const std::vector<std::string> &params, std::vector<std::string> &info));

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return 0;
    }

    int InvokeErrorSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return UNKNOWN_ERROR;
    }

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

    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
    {
        return nullptr;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return nullptr;
    }

    virtual std::string GetType(const Uri &uri) override
    {
        return " ";
    }

    int OpenRawFile(const Uri &uri, const std::string &mode)
    {
        return -1;
    }

    bool Reload(const Uri &uri, const PacMap &extras)
    {
        return false;
    }

    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
    {
        return -1;
    }

    Uri NormalizeUri(const Uri &uri)
    {
        Uri urivalue("");
        return urivalue;
    }

    Uri DenormalizeUri(const Uri &uri)
    {
        Uri urivalue("");
        return urivalue;
    }

    virtual bool ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
    {
        return true;
    }

    virtual bool ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
    {
        return true;
    }

    virtual bool ScheduleNotifyChange(const Uri &uri)
    {
        return true;
    }

    virtual std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
    {
        return std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>();
    }

    virtual sptr<IRemoteObject> CallRequest()
    {
        return sptr<IRemoteObject>(nullptr);
    }

    #ifdef ABILITY_COMMAND_FOR_TEST
    virtual int BlockAbility()
    {
        return 0;
    }
    #endif

    int code_ = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
