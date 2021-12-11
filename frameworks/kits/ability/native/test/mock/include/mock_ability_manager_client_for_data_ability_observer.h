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
#ifndef OHOS_APPEXECFWK_MOCK_ABILITY_MANAGER_CLIENT_FOR_DATA_ABILITY_OBSERVER_STUB_H
#define OHOS_APPEXECFWK_MOCK_ABILITY_MANAGER_CLIENT_FOR_DATA_ABILITY_OBSERVER_STUB_H

#include "gmock/gmock.h"

#include "ability_manager_client.h"
#include "ability_scheduler_proxy.h"
#include "ability_thread.h"

#include <iremote_object.h>
#include <iremote_stub.h>

#include "mock_ability_scheduler_for_observer.h"

namespace OHOS {
namespace AppExecFwk {
class MockAbilitySchedulerTools {
public:
    MockAbilitySchedulerTools():mockStatus(false) {};
    ~MockAbilitySchedulerTools() = default;

    // return Copy AbilityThread class Smart pointer
    std::shared_ptr<MockAbilitySchedulerStub> GetMockAbilityScheduler()
    {
        if (mockAbilitySchedulerStubInstance == nullptr) {
            mockAbilitySchedulerStubInstance = std::make_shared<MockAbilitySchedulerStub>();
        }
        return mockAbilitySchedulerStubInstance;
    };

    // return access proxy class sptr pointer
    sptr<AAFwk::AbilitySchedulerProxy> GetAbilitySchedulerProxy()
    {
        if (mockStatus) {
            if (abilitySchedulerProxyInstance1 == nullptr) {
                if (mockAbilitySchedulerStubInstance == nullptr) {
                    mockAbilitySchedulerStubInstance = std::make_shared<MockAbilitySchedulerStub>();
                }
                abilitySchedulerProxyInstance1 = sptr<AAFwk::AbilitySchedulerProxy>(
                    new (std::nothrow) AAFwk::AbilitySchedulerProxy(mockAbilitySchedulerStubInstance->AsObject()));
            }
            return abilitySchedulerProxyInstance1;
        } else {
            if (abilitySchedulerProxyInstance2 == nullptr) {
                if (abilityThreadInstance == nullptr) {
                    abilityThreadInstance = std::make_shared<AbilityThread>();
                }
                abilitySchedulerProxyInstance2 = sptr<AAFwk::AbilitySchedulerProxy>(
                    new (std::nothrow) AAFwk::AbilitySchedulerProxy(abilityThreadInstance->AsObject()));
            }
            return abilitySchedulerProxyInstance2;
        }
    };

    void SetMockStatus(bool nflag)
    {
        mockStatus = nflag;
    }

    static std::shared_ptr<MockAbilitySchedulerTools> GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = std::make_shared<MockAbilitySchedulerTools>();
        }
        return instance_;
    };

    static void DestoryInstance()
    {
        instance_ = nullptr;
    }

private:
    static std::shared_ptr<MockAbilitySchedulerTools> instance_;
    sptr<AAFwk::AbilitySchedulerProxy> abilitySchedulerProxyInstance1;
    std::shared_ptr<AppExecFwk::MockAbilitySchedulerStub> mockAbilitySchedulerStubInstance;
    sptr<AAFwk::AbilitySchedulerProxy> abilitySchedulerProxyInstance2;
    std::shared_ptr<AppExecFwk::AbilityThread> abilityThreadInstance;
    bool mockStatus;
};

std::shared_ptr<MockAbilitySchedulerTools> MockAbilitySchedulerTools::instance_ = nullptr;
}  // namespace AppExecFwk
}  // namespace OHOS

namespace OHOS {
namespace AAFwk {
sptr<AAFwk::IAbilityScheduler> AAFwk::AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    return AppExecFwk::MockAbilitySchedulerTools::GetInstance()->GetAbilitySchedulerProxy();
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif /* OHOS_APPEXECFWK_MOCK_ABILITY_MANAGER_CLIENT_FOR_DATA_ABILITY_OBSERVER_STUB_H */