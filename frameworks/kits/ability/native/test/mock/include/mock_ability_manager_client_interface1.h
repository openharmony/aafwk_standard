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

#ifndef OHOS_AAFWK_MOCK_ABILITY_MANAGER_CLIENT2_H
#define OHOS_AAFWK_MOCK_ABILITY_MANAGER_CLIENT2_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_interface.h"
#include "ability_manager_interface.h"
#include "want.h"

#include "iremote_object.h"
#include "iremote_stub.h"

#include "ability_manager_client.h"

namespace OHOS {
namespace AppExecFwk {

class IAbilityMock : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityMock");
};

class MockAbilityStub : public IRemoteStub<IAbilityMock> {
public:
    MockAbilityStub() = default;
    virtual ~MockAbilityStub() = default;
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    };
};

class MockAbilityContextDeal : public ContextDeal {
public:
    MockAbilityContextDeal();
    virtual ~MockAbilityContextDeal();

    std::shared_ptr<HapModuleInfo> GetHapModuleInfo();

    std::shared_ptr<HapModuleInfo> hapModInfo_;
};

class MockAbilityContextTest : public AbilityContext {
public:
    MockAbilityContextTest();
    ~MockAbilityContextTest();

    void StartAbility(const AAFwk::Want &want, int requestCode);

    void SetToken(const sptr<IRemoteObject> token);

    size_t GetStartAbilityRunCount();
    void SetStartAbilityRunCount(size_t nCount);

    size_t startAbilityRunCount_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

namespace OHOS {
namespace AAFwk {

class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MockAbilityManagerClient();
    virtual ~MockAbilityManagerClient();

    ErrCode GetStartAbility();
    ErrCode GetTerminateAbility();
    ErrCode GetTerminateAbilityResult();

    void SetStartAbility(ErrCode tValue);
    void SetTerminateAbility(ErrCode tValue);
    void SetTerminateAbilityResult(ErrCode tValue);

    int GetTerminateAbilityValue();
    void SetTerminateAbilityValue(int nValue);

    int GetMockMissionId();
    void SetMockMissionId(int missionId);

    static std::shared_ptr<MockAbilityManagerClient> mock_instance_;
    static bool mock_intanceIsNull_;

    static std::shared_ptr<MockAbilityManagerClient> GetInstance();
    static void SetInstanceNull(bool flag);

private:
    ErrCode startAbility_;
    ErrCode terminateAbility_;
    ErrCode terminateAbilityResult_;

    int terminateAbilityValue_;
    int missionId_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_H
