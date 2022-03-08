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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_NATIVE_MOCK_ABILITY_MANAGER_CLIENT_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_NATIVE_MOCK_ABILITY_MANAGER_CLIENT_H

#include <gtest/gtest.h>
#include <mutex>
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_context.h"
#include "ability_manager_interface.h"
#include "ability_scheduler_interface.h"
#include "app_log_wrapper.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "want.h"

#define OPENFILENUM (1246)
#define INSERTNUM (2345)
#define UPDATENUM (3456)
#define OPENRAWFILENUM (5678)
#define DELETENUM (6789)
#define BATCHINSERTNUM (789)

namespace OHOS {
namespace AppExecFwk {
using Configuration = AppExecFwk::Configuration;
class MockAbility : public AbilityContext {
public:
    MockAbility() = default;
    virtual ~MockAbility() = default;
};

class MockAbilityThread : public IRemoteStub<AAFwk::IAbilityScheduler> {
public:
    virtual void ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState) {};
    virtual void SendResult(int requestCode, int resultCode, const Want &resultWant) {};
    virtual void ScheduleConnectAbility(const Want &want) {};
    virtual void ScheduleDisconnectAbility(const Want &want) {};
    virtual void ScheduleCommandAbility(const Want &want, bool restart, int startId) {};
    virtual void ScheduleSaveAbilityState() {};
    virtual void ScheduleRestoreAbilityState(const PacMap &inState) {};
    virtual void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config) {};
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
    {
        return std::vector<std::string>();
    };
    virtual int OpenFile(const Uri &uri, const std::string &mode)
    {
        return OPENFILENUM;
    };
    virtual int OpenRawFile(const Uri &uri, const std::string &mode)
    {
        return OPENRAWFILENUM;
    };
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
    {
        return INSERTNUM;
    };
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return UPDATENUM;
    };
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return DELETENUM;
    };
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
    {
        return std::make_shared<NativeRdb::AbsSharedResultSet>("resultset");
    };
    virtual std::string GetType(const Uri &uri)
    {
        return std::string("Type1");
    };
    virtual bool Reload(const Uri &uri, const PacMap &extras)
    {
        return true;
    };
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
    {
        return BATCHINSERTNUM;
    };
    virtual bool ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
    {
        return true;
    };
    virtual bool ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
    {
        return true;
    };
#ifdef SUPPORT_GRAPHICS
    virtual void NotifyMultiWinModeChanged(int32_t winModeKey, bool flag) {};
    virtual void NotifyTopActiveAbilityChanged(bool flag) {};
#endif
    virtual bool ScheduleNotifyChange(const Uri &uri)
    {
        return true;
    };
    virtual Uri NormalizeUri(const Uri &uri)
    {
        return Uri("dataability:///test.aaa");
    };
    virtual Uri DenormalizeUri(const Uri &uri)
    {
        return Uri("dataability:///test.aaa");
    };
    virtual std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
    {
        return std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>();
    };
    virtual void NotifyContinuationResult(int32_t result) {};
    virtual void ContinueAbility(const std::string& deviceId) {};
    virtual void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) {};
    virtual sptr<IRemoteObject> CallRequest()
    {
        return sptr<IRemoteObject>(nullptr);
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_NATIVE_MOCK_ABILITY_MANAGER_CLIENT_H
