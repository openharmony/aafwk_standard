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
#include "service_ability_a.h"
#include "test_ability_connection.h"

namespace OHOS {
namespace AppExecFwk {
void ServiceAbilityA::OnStart(const Want &want)
{
    HILOG_INFO("ServiceAbilityA OnStart");
    Ability::OnStart(want);
}

void ServiceAbilityA::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HILOG_INFO("ServiceAbilityA::OnCommand");
    Ability::OnCommand(want, restart, startId);
}

sptr<IRemoteObject> ServiceAbilityA::OnConnect(const Want &want)
{
    HILOG_INFO("ServiceAbilityA::OnConnect");
    sptr<IServiceRemoteTest> remoteObject = sptr<IServiceRemoteTest>(new (std::nothrow) TestAbilityConnection());

    Ability::OnConnect(want);
    return remoteObject->AsObject();
}

void ServiceAbilityA::OnDisconnect(const Want &want)
{
    HILOG_INFO("ServiceAbilityA::OnDisconnect");
    Ability::OnDisconnect(want);
}

void ServiceAbilityA::OnStop()
{
    HILOG_INFO("ServiceAbilityA::OnStop");
    Ability::OnStop();
}
REGISTER_AA(ServiceAbilityA);
}  // namespace AppExecFwk
}  // namespace OHOS
