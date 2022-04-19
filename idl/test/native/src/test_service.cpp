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

#include "test_service.h"

#include <string_ex.h>

#include "if_system_ability_manager.h"
#include "ipc_debug.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
using namespace OHOS::HiviewDFX;

int TestService::Instantiate()
{
    ZLOGI(LABEL, "%{public}s call in", __func__);
    auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        ZLOGE(LABEL, "%{public}s:fail to get Registry", __func__);
        return -ENODEV;
    }

    sptr<IRemoteObject> newInstance = new TestService();
    int result = saMgr->AddSystemAbility(IPC_TEST_SERVICE, newInstance);
    ZLOGI(LABEL, "%{public}s: IPC_TEST_SERVICE result = %{public}d", __func__, result);
    return result;
}

TestService::TestService()
{
}

TestService::~TestService()
{
}

ErrCode TestService::TestIntTransaction(int data, int &rep)
{
    ZLOGE(LABEL, " TestService:read from client data = %{public}d", data);
    rep = data + data;
    return ERR_NONE;
}

ErrCode TestService::TestStringTransaction(const std::string &data)
{
    ZLOGE(LABEL, "TestService:read string from client data = %{public}s", data.c_str());
    return data.size();
}
} // namespace OHOS
