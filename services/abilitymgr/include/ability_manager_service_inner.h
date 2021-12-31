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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_SERVICE_INNER_H
#define OHOS_AAFWK_ABILITY_MANAGER_SERVICE_INNER_H

#include <memory>

#include "iremote_object.h"
#include "mission_list_manager.h"
#include "want.h"

using IRemoteObject = OHOS::IRemoteObject;
using Want = OHOS::AAFwk::Want;

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class AbilityManagerServiceInner
 * new ams class to handle request
 */
class AbilityManagerServiceInner {
public:
    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant);
private:
    std::unique_ptr<AbilityRuntime::MissionListManager> abilityManager_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_SERVICE_INNER_H