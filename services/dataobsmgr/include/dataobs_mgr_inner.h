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

#ifndef OHOS_AAFWK_DATAOBS_MANAGER_INNER_H
#define OHOS_AAFWK_DATAOBS_MANAGER_INNER_H

#include <atomic>
#include <list>
#include <string>
#include <memory>
#include <map>
#include <mutex>

#include "iremote_object.h"
#include "refbase.h"

#include "data_ability_observer_interface.h"

#include "event_handler.h"

namespace OHOS {
namespace AAFwk {
using EventHandler = OHOS::AppExecFwk::EventHandler;
class DataObsMgrInner : public std::enable_shared_from_this<DataObsMgrInner> {
public:
    using ObsMapType = std::map<std::string, std::list<sptr<IDataAbilityObserver>>>;
    using ObsListType = std::list<sptr<IDataAbilityObserver>>;
    using ObsRecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;

    DataObsMgrInner();
    virtual ~DataObsMgrInner();

    void SetHandler(const std::shared_ptr<EventHandler> &handler);
    int HandleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);
    int HandleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver);
    int HandleNotifyChange(const Uri &uri);
    bool CheckNeedLimmit();
    bool CheckRegisteFull(const Uri &uri);
    void AtomicAddTaskCount();
    void AtomicSubTaskCount();
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

private:
    bool GetObsListFromMap(const Uri &uri, ObsListType &obslist);
    void AddObsDeathRecipient(const sptr<IDataAbilityObserver> &dataObserver);
    void RemoveObsDeathRecipient(const sptr<IDataAbilityObserver> &dataObserver);
    void HandleCallBackDiedTask(const sptr<IRemoteObject> &dataObserver);
    void RemoveObsFromMap(const sptr<IDataAbilityObserver> &dataObserver);
    bool ObsExistInMap(const sptr<IDataAbilityObserver> &dataObserver);

    std::atomic_int taskCount_;
    const int taskCount_max_ = 50;
    const unsigned int obs_max_ = 50;
    static std::mutex innerMutex_;
    std::shared_ptr<EventHandler> handler_ = nullptr;
    ObsMapType obsmap_;
    ObsRecipientMapType recipientMap_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATAOBS_MANAGER_INNER_H
