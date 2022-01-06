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

#ifndef MOCK_ABILITY_MANAGER_CLIENT_H
#define MOCK_ABILITY_MANAGER_CLIENT_H

#include <map>
#include <string>

#include "gtest/gtest.h"

#include "ability_manager_client.h"
#include "ability_scheduler_proxy.h"

#include <iremote_object.h>
#include <iremote_stub.h>

#include "mock_ability_thread_for_data_observer.h"

namespace OHOS {
namespace AAFwk {

class DemoAbilityManagerClientData {
public:
    DemoAbilityManagerClientData() {};
    virtual ~DemoAbilityManagerClientData() {};

    std::map<std::string, sptr<AAFwk::IAbilityScheduler>> data;   
};

static DemoAbilityManagerClientData demoAbilityMgrClientData;

sptr<AAFwk::IAbilityScheduler> AAFwk::AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    sptr<AAFwk::IAbilityScheduler> retval = nullptr;
    auto iter = demoAbilityMgrClientData.data.find(uri.ToString());
    if (iter == demoAbilityMgrClientData.data.end()) {
        GTEST_LOG_(INFO) << "mock_ability_manager_client:find data is null called";
        sptr<AppExecFwk::AbilityThread> abilityThread = AppExecFwk::MockCreateAbilityThread();
        sptr<AAFwk::AbilitySchedulerProxy> remoteProxy(new (std::nothrow) AAFwk::AbilitySchedulerProxy(abilityThread));
        demoAbilityMgrClientData.data.emplace(uri.ToString(), remoteProxy);
        
        retval = remoteProxy;
        
        if (retval == nullptr) {
        GTEST_LOG_(INFO) << "mock_ability_thread_for_data_observer:mock result retval is  nullptr called";
        }
    } else {
        retval = iter->second;
        GTEST_LOG_(INFO) << "mock_ability_manager_client:find data is  not null called";
    }

    return retval;
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    return ERR_OK;
}

}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_ABILITY_MANAGER_CLIENT_H
