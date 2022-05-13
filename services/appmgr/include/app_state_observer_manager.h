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

#ifndef OHOS_APP_STATE_OBSERVER_MANAGER_H
#define OHOS_APP_STATE_OBSERVER_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
class AppStateObserverManager : public std::enable_shared_from_this<AppStateObserverManager> {
    DECLARE_DELAYED_SINGLETON(AppStateObserverManager)
public:
    void Init();
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
private:
    void HandleAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state);
    void HandleOnComponentStateChanged(const AbilityStateData abilityStateData, bool isAbility);
    void HandleOnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord);
    void HandleOnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool ObserverExist(const sptr<IApplicationStateObserver> &observer);
    void AddObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer);
    void RemoveObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer);
    int VerifyObserverPermission();

private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::vector<sptr<IApplicationStateObserver>> appStateObservers_;
    std::recursive_mutex observerLock_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> recipientMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_STATE_OBSERVER_MANAGER_H