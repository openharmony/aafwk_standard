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

#ifndef ABILITY_MANAGER_H
#define ABILITY_MANAGER_H

#include "ability_manager_client.h"
#include "app_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::AbilityManagerClient;
using OHOS::AAFwk::RECENT_IGNORE_UNAVAILABLE;
using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::RunningProcessInfo;

class AbilityManager {
public:
    AbilityManager() = default;
    virtual ~AbilityManager() = default;

    static AbilityManager &GetInstance();

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want Ability want.
     * @param requestCode Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    void StartAbility(const Want &want, int requestCode);

    /**
     * Clears user data of the application, which is equivalent to initializing the application.
     *
     * @param bundleName.
     */
    int32_t ClearUpApplicationData(const std::string &bundleName);

    /**
     * Obtains information about application processes that are running on the device.
     *
     * @returns Returns a list of running processes.
     */
    std::vector<RunningProcessInfo> GetAllRunningProcesses();

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    int KillProcessesByBundleName(const std::string &bundleName);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // ABILITY_MANAGER_H
