/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "test_client.h"

#include "if_system_ability_manager.h"
#include "ipc_debug.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
int TestClient::ConnectService()
{
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        ZLOGE(LABEL, "get registry fail");
        return -1;
    }

    sptr<IRemoteObject> object = saMgr->GetSystemAbility(IPC_TEST_SERVICE);
    if (object != nullptr) {
        ZLOGE(LABEL, "Got test Service object");
        testService_ = (new (std::nothrow) IdlTestServiceProxy(object));
    }

    if (testService_ == nullptr) {
        ZLOGE(LABEL, "Could not find Test Service!");
        return -1;
    }

    return 0;
}

void TestClient::StartIntTransaction()
{
    if (testService_ != nullptr) {
        ZLOGE(LABEL, "StartIntTransaction");
        [[maybe_unused]] int result = 0;
        testService_->TestIntTransaction(1234, result); // 1234 : test number
        ZLOGE(LABEL, "Rec result from server %{public}d.", result);
    }
}

void TestClient::StartStringTransaction()
{
    if (testService_ != nullptr) {
        ZLOGI(LABEL, "StartIntTransaction");
        testService_->TestStringTransaction("IDL Test");
    }
}
} // namespace OHOS
