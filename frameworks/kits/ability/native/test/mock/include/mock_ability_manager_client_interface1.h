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

#include "ability_manager_client.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MockAbilityManagerClient();
    virtual ~MockAbilityManagerClient();

    ErrCode GetStartAbility();
    ErrCode GetTerminateAbility();
    ErrCode GetTerminateAbilityResult();
    ErrCode GetIsFirstInMission();
    ErrCode GetRemoveMissions();

    void SetStartAbility(ErrCode tValue);
    void SetTerminateAbility(ErrCode tValue);
    void SetTerminateAbilityResult(ErrCode tValue);
    void SetIsFirstInMission(ErrCode tValue);
    void SetRemoveMissions(ErrCode tValue);

    int GetTerminateAbilityValue();
    void SetTerminateAbilityValue(int nValue);

    int GetRemoveMissionsValue();
    void ChangeRemoveMissionsValue();

    static std::shared_ptr<MockAbilityManagerClient> mock_instance_;
    static bool mock_intanceIsNull_;

    static std::shared_ptr<MockAbilityManagerClient> GetInstance();
    static void SetInstanceNull(bool flag);

private:
    ErrCode startAbility_;
    ErrCode terminateAbility_;
    ErrCode terminateAbilityResult_;
    ErrCode isFirstInMission_;
    ErrCode removeMissionsE_;

    int terminateAbilityValue_;
    int removeMissions_;
};

}  // namespace AAFwk
}  // namespace OHOS

namespace OHOS {
namespace AppExecFwk {

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

    size_t GetStartAbilityRunCount();
    void SetStartAbilityRunCount(size_t nCount);

    size_t startAbilityRunCount_;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_MANAGER_H
