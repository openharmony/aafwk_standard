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

#ifndef OHOS_AAFWK_FREE_INSTALL_MANAGER_H
#define OHOS_AAFWK_FREE_INSTALL_MANAGER_H

#include <future>

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const std::string FREE_INSTALL_TYPE = "freeInstallType";
const std::string FREE_INSTALL_UPGRADED_KEY = "freeInstallUpgraded";
class AbilityManagerService;
/**
 * @class FreeInstallManager
 * FreeInstallManager.
 */
class FreeInstallManager : public std::enable_shared_from_this<FreeInstallManager> {
public:
    explicit FreeInstallManager(const std::weak_ptr<AbilityManagerService> &server);
    virtual ~FreeInstallManager() = default;

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * OnRemoteInstallFinished, DMS has finished.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * Start to free install.
     *
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @param callerToken, caller ability token.
     * @param ifOperateRemote, is from other devices.
     * @return Returns ERR_OK on success, others on failure.
     */
    int FreeInstall(const Want &want, int32_t userId, int requestCode,
        const sptr<IRemoteObject> &callerToken, bool ifOperateRemote);

    /**
     * Start to free install from another devices.
     * The request is send from DMS.
     *
     * @param want, the want of the ability to free install.
     * @param callback, used to notify caller the result of free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode);

    /**
     * Check if the connect request is free install.
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param callerToken, caller ability token.
     * @param localDeviceId, the device id of local.
     * @return Returns ERR_OK on success, others on failure.
     */
    int IsConnectFreeInstall(const Want &want, int32_t userId, const sptr<IRemoteObject> &callerToken,
        std::string& localDeviceId);

private:
    std::weak_ptr<AbilityManagerService> server_;
    struct FreeInstallInfo {
        Want want;
        int32_t userId = -1;
        int32_t requestCode = -1;
        std::shared_ptr<std::promise<int32_t>> promise;
        bool isInstalled = false;
        sptr<IRemoteObject> callerToken = nullptr;
        sptr<IRemoteObject> dmsCallback = nullptr;
    };
    std::vector<FreeInstallInfo> freeInstallList_;
    std::vector<FreeInstallInfo> dmsFreeInstallCbs_;

    bool CheckIsFreeInstall(const Want &want);
    bool CheckTargetBundleList(const Want &want, int32_t userId, const sptr<IRemoteObject> &callerToken);
    int HandleFreeInstallErrorCode(int resultCode);
    int NotifyDmsCallback(const Want &want, int resultCode);
    bool IsTopAbility(const sptr<IRemoteObject> &callerToken);
    void NotifyFreeInstallResult(const Want &want, int resultCode);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_FREE_INSTALL_MANAGER_H
